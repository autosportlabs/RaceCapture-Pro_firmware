/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CAN.h"
#include "FreeRTOS.h"
#include "OBD2.h"
#include "loggerConfig.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"
#include "mem_mang.h"
#include <string.h>
#include "stdutil.h"
#include "can_mapping.h"

#define _LOG_PFX                        "[OBD2] "
#define OBD2_11BIT_PID_RESPONSE         0x7E8
#define OBD2_29BIT_PID_RESPONSE         0x18DAF110

#define OBD2_11BIT_PID_REQUEST          0x7DF
#define OBD2_29BIT_PID_REQUEST          0x18DB33F1

#define OBD2_MODE_RESPONSE_OFFSET       0x40
#define OBD2_MODE_SHOW_CURRENT_DATA     0x01
#define OBD2_MODE_REQUEST_TROUBLE_CODES 0x03
#define OBD2_MODE_CLEAR_TROUBLE_CODES   0x04
#define OBD2_MODE_O2_SENSOR_MONITOR   0x05
#define OBD2_MODE_BODY_INFO             0x09
#define OBD2_MODE_ENHANCED_DATA         0x22
#define OBD2_TIMEOUT_DISABLE_THRESHOLD  10

enum obd2_channel_status {
        OBD2_CHANNEL_STATUS_NO_DATA = 0,
        OBD2_CHANNEL_STATUS_DATA_RECEIVED,
        OBD2_CHANNEL_STATUS_SQUELCHED
};

/* tracks the state of OBD2 channels */
struct OBD2ChannelState {
        /* the current OBD2 channel value */
        float current_value;

        /**
         * holds the state for the priority sequencer
         */
        uint16_t sequencer_count;

        /* PID associated with OBD2 channel */
        uint16_t pid;

        /* number of timeouts seen on this channel */
        uint8_t timeout_count;

        /* indicates status of channel */
        enum obd2_channel_status channel_status;
};

/* manages the running state of OBD2 queries */
struct OBD2State {
        /* points to a dynamically created array of OBD2ChannelState structs */
        struct OBD2ChannelState * current_channel_states;

        /* holds the last query timestamp, for determining OBD2 query timeouts */
        size_t last_obd2_query_timestamp;

        /* the index of the current OBD2 PID we're querying */
        uint16_t current_obd2_pid_index;

        /**
         * the max sample rate across all of the channels;
         * will set the time base for the fastest PID querying
         */
        uint16_t max_sample_rate;

        /**
         * number of OBD2 channels
         */
        uint16_t channel_count;

        /**
         * number of squelched channels
         */
        uint16_t squelched_count;

        /**
         * latency in ms for OBDII queries
         */
        uint32_t query_latency;

        /**
         * flag to indicate if config was stale
         */
        bool is_stale;

        /**
         * flag to indicate if we have a live OBDII connection
         */
        bool is_active;

        /**
         * indicates if we're using 29 bit PID requests
         */
        bool is_29bit_obd2;
};

static struct OBD2State obd2_state = {0};


void OBD2_state_stale(void)
{
	    obd2_state.is_stale = true;
}

bool OBD2_is_state_stale(void)
{
	    return obd2_state.is_stale;
}

/**
 * Sends an OBD2 PID request on the CAN bus.
 * @param pid the OBD2 PID to request
 * @param mode the OBD2 mode to request
 * @param timeout the timeout in ms for sending the OBD2 request
 */
static int OBD2_request_PID(uint32_t pid, uint8_t mode, bool is_29_bit, size_t timeout)
{
        CAN_msg msg;
        msg.addressValue = is_29_bit ? OBD2_29BIT_PID_REQUEST : OBD2_11BIT_PID_REQUEST;
        if (mode != OBD2_MODE_SHOW_CURRENT_DATA) {
                /* treat everything that's not a standard current data request
                 * as an enhanced data request mode
                 */
                if (pid > 0xFFFF) { /* extract into a 32 bit PID as needed, big endian format */
                        msg.data[0] = 5;
                        msg.data[2] = (pid >> 24);
                        msg.data[3] = (pid >> 16) & 0xFF;
                        msg.data[4] = (pid >> 8) & 0xFF;
                        msg.data[5] = pid & 0xFF;
                }
                else { /*otherwise treat as 16 bit PID */
                        msg.data[0] = 3;
                        msg.data[2] = pid >> 8;
                        msg.data[3] = pid & 0xFF;
                        msg.data[4] = 0x55;
                        msg.data[5] = 0x55;
                }
        }
        else{ /* request current data, 8 bit PID */
                msg.data[0] = 2;
                msg.data[2] = pid;
                msg.data[3] = 0x55;
                msg.data[4] = 0x55;
                msg.data[5] = 0x55;
        }
        msg.data[1] = mode;
        msg.data[6] = 0x55;
        msg.data[7] = 0x55;
        msg.dataLength = 8;
        msg.isExtendedAddress = is_29_bit;
        return CAN_tx_msg(0, &msg, timeout);
}

bool OBD2_init_current_values(OBD2Config *obd2_config)
{
        pr_info(_LOG_PFX "Init current values\r\n");
        uint16_t obd2_channel_count = obd2_config->enabledPids;

        /* free any previously created channel states */
        if (obd2_state.current_channel_states != NULL)
                portFree(obd2_state.current_channel_states);

        /* start the querying from the first PID */
        obd2_state.current_obd2_pid_index = 0;
        obd2_state.last_obd2_query_timestamp = 0;
        obd2_state.squelched_count = 0;
        obd2_state.query_latency = 0;
        obd2_state.is_active = false;
        obd2_state.is_29bit_obd2 = false;
        /* determine the fastest sample rate, which will set our PID querying timebase */
        size_t max_sample_rate = 0;
        for (size_t i = 0; i < obd2_channel_count; i++) {
                max_sample_rate = MAX(max_sample_rate,
                                      decodeSampleRate(obd2_config->pids[i].mapping.channel_cfg.sampleRate));
        }
        obd2_state.max_sample_rate = max_sample_rate;
        pr_debug_int_msg(_LOG_PFX " Max OBD2 sample rate: ", obd2_state.max_sample_rate);

        if (obd2_channel_count == 0) {
        		/* if no OBD2 channels are enabled, don't malloc */
				obd2_state.current_channel_states = NULL;
    obd2_state.is_stale = false;
				return true;
        }

		/* malloc the collection of OBD2 channels */
		size_t size = sizeof(struct OBD2ChannelState[obd2_channel_count]);
		obd2_state.current_channel_states = portMalloc(size);

		if (obd2_state.current_channel_states == NULL) {
				pr_error_int_msg(_LOG_PFX " Failed to init OBD2ChannelState with count ", obd2_channel_count);
				/* whoops */
				return false;
		}

		/* set our current PIDs */
		for (size_t i = 0; i < obd2_channel_count; i++) {
		        struct OBD2ChannelState *state = &obd2_state.current_channel_states[i];
				state->pid = obd2_config->pids[i].pid;
				state->channel_status = OBD2_CHANNEL_STATUS_NO_DATA;
				state->timeout_count = 0;
				state->current_value = 0.0;
		}

		obd2_state.channel_count = obd2_config->enabledPids;
		obd2_state.is_stale = false;
		return true;
}

float OBD2_get_current_channel_value(int index)
{
        if (obd2_state.current_channel_states == NULL)
                return 0;
        return obd2_state.current_channel_states[index].current_value;
}

void OBD2_set_current_channel_value(int index, float value)
{
        if (obd2_state.current_channel_states == NULL)
                return;
        obd2_state.current_channel_states[index].current_value = value;
}

void sequence_next_obd2_query(OBD2Config * obd2_config, uint16_t enabled_obd2_pids_count)
{
        /* no PIDs, no query... */
		if (enabled_obd2_pids_count == 0)
				return;

        bool is_obd2_timeout = obd2_state.last_obd2_query_timestamp > 0 &&
        		isTimeoutMs(obd2_state.last_obd2_query_timestamp, OBD2_PID_DEFAULT_TIMEOUT_MS);

        size_t current_pid_index = obd2_state.current_obd2_pid_index;

        if (is_obd2_timeout) {
        		/* check for timeout and squelch current PID if needed */
                struct OBD2ChannelState *state = &obd2_state.current_channel_states[current_pid_index];
                pr_debug_int_msg(_LOG_PFX "Timeout requesting PID ", state->pid);

                /* only start counting timeouts if we've ever received data */
                if (obd2_state.is_active) {
                        state->timeout_count++;
                        if (state->timeout_count >= OBD2_TIMEOUT_DISABLE_THRESHOLD) {
                                state->channel_status = OBD2_CHANNEL_STATUS_SQUELCHED;
                                pr_info_int_msg(_LOG_PFX "Excessive timeouts, squelching PID ", state->pid);
                                obd2_state.squelched_count++;
                                /**
                                 * if all channels end up being squelched, then we should just reset OBD2 config
                                 * This accounts for cases where there's a complete disconnect and a reset is needed
                                 */
                                if (obd2_state.squelched_count == obd2_state.channel_count) {
                                        pr_info(_LOG_PFX "all channels timed out, resetting OBD2 state\r\n");
                                        obd2_state.is_stale = true;
                                }
                        }
                }
                /*if we have timed out and we're not active, then we should try auto-detecting 29 or 11 bit OBDII */
                else {
                        obd2_state.is_29bit_obd2 = !obd2_state.is_29bit_obd2;
                        pr_info_int_msg(_LOG_PFX "Trying OBDII bit mode ", obd2_state.is_29bit_obd2 ? 29 : 11);
                }
        }

        /* if a query is active and not timed out, exit now */
        if (obd2_state.last_obd2_query_timestamp != 0 && !is_obd2_timeout)
        		return;

		/**
		 * Scheduler algorithm.  This updates a sequencer counter
		 * based on the channel's sample rate.
		 *
		 * Whichever PID has the highest count *above* the max configured
		 * OBDII sample rate wins and is selected for querying. once this happens
		 * the counter is reset to 0.
		 *
		 * The result causes channels to be proportionately queried based on the
		 * configured sample rate.
		 *
		 * Example:
		 * Channel1 @ 1Hz - gets incremented by 1 on every pass through
		 * Channel2 @ 25Hz - gets incremented by 25 on every pass
		 * Channel3 @ 50Hz - gets incremented by 50 on every pass
		 * Max configured sample rate: 50Hz
		 *
		 * Results:
		 * Channel 1 is selected for PID querying approx. 1/50 the rate of channel 3
		 * Channel 2 is selected for PID querying approx. 1/2 the rate of channel 3
		 * Channel 3 is selected for PID querying approx. every time
		 */

		uint16_t highest_timeout_factor = 0;

		/* tracks which PID should be scheduled next */
		int most_due_pid_index = -1;

		for (size_t i = 0; i < enabled_obd2_pids_count; i++) {
                struct OBD2ChannelState *state = &obd2_state.current_channel_states[i];
                if (state->channel_status == OBD2_CHANNEL_STATUS_SQUELCHED)
                        /* if channel is squelched then skip */
                        continue;

                uint16_t timeout = state->sequencer_count;
                uint16_t sample_rate = decodeSampleRate(obd2_config->pids[i].mapping.channel_cfg.sampleRate);
                timeout += sample_rate;

                /**
                 * select the channel if:
                 * 1. the timeout has reached the trigger point
                 * 2. the timeout has taken the longest amount of time to reach the trigger point */
                uint16_t timeout_factor = timeout / sample_rate;
                if (timeout >= obd2_state.max_sample_rate && timeout_factor > highest_timeout_factor) {
                        highest_timeout_factor = timeout_factor;
                        most_due_pid_index = i;
                }
                state->sequencer_count = timeout;
		}

		if (most_due_pid_index < 0)
		        /* no PID was selected, give up */
		        return;

		current_pid_index = most_due_pid_index;
		obd2_state.current_channel_states[current_pid_index].sequencer_count = 0;

		PidConfig * pid_cfg = &obd2_config->pids[current_pid_index];
		int pid_request_result = pid_cfg->passive || OBD2_request_PID(pid_cfg->pid, pid_cfg->mode, obd2_state.is_29bit_obd2, OBD2_PID_REQUEST_TIMEOUT_MS);
		if (pid_request_result) {
				obd2_state.last_obd2_query_timestamp = getCurrentTicks();
		}
		else {
				pr_debug_int_msg("Timeout sending PID request ", pid_cfg->pid);
		}
		obd2_state.current_obd2_pid_index = current_pid_index;
}

void update_obd2_channels(CAN_msg *msg, OBD2Config *cfg)
{
        uint16_t current_pid_index = obd2_state.current_obd2_pid_index;
        PidConfig *pid_config = &cfg->pids[current_pid_index];
        struct OBD2ChannelState *channel_state = &obd2_state.current_channel_states[current_pid_index];

        /* Did we get an OBDII PID we were waiting for? */

        uint8_t mode = pid_config->mode;

        /* valid OBD2 request timestamp? */
        if (obd2_state.last_obd2_query_timestamp &&

            /* is this CAN message an OBD2 PID response */
            (msg->addressValue == OBD2_11BIT_PID_RESPONSE || msg->addressValue == OBD2_29BIT_PID_RESPONSE) &&

            /* does the returned mode + response offeset match the one expected in the current query? ? */
            msg->data[1] == mode + OBD2_MODE_RESPONSE_OFFSET &&

            (

            /* does the 1 byte or 2 byte response match the current query? enhanced mode = 2 byte PID*/
            (msg->data[2] == pid_config->pid && msg->data[1] == OBD2_MODE_SHOW_CURRENT_DATA + OBD2_MODE_RESPONSE_OFFSET) ||

            /* or does it match match on miscellaneous modes */
            (mode == OBD2_MODE_REQUEST_TROUBLE_CODES) ||
            (mode == OBD2_MODE_CLEAR_TROUBLE_CODES) ||
            (mode == OBD2_MODE_O2_SENSOR_MONITOR) ||
            (mode == OBD2_MODE_BODY_INFO) ||

            /* otherwise account for special mode with multi-byte PIDs (e.g. 0x22) */
            ((msg->data[2] * 256 + msg->data[3]) == pid_config->pid && msg->data[1] == mode + OBD2_MODE_RESPONSE_OFFSET)

            )
            ) {
                    float value;
                    bool result = canmapping_map_value(&value, msg, &pid_config->mapping);
                    if (result) {
                            OBD2_set_current_channel_value(current_pid_index, value);
                            channel_state->channel_status = OBD2_CHANNEL_STATUS_DATA_RECEIVED;
                            channel_state->timeout_count = 0;
                    }
                    /* Save our latency */
                    obd2_state.query_latency = ticksToMs(getCurrentTicks() - obd2_state.last_obd2_query_timestamp);
                    obd2_state.is_active = true;
                    /* PID request is complete */
                    obd2_state.last_obd2_query_timestamp = 0;
        }
}

bool OBD2_get_value_for_pid(uint16_t pid, float *value)
{
		struct OBD2ChannelState *states = obd2_state.current_channel_states;
		if (states == NULL)
				return false;

		for (size_t i = 0; i < obd2_state.channel_count; i++) {
				struct OBD2ChannelState *test_state = states + i;
				if (pid == test_state->pid) {
						*value = test_state->current_value;
						return true;
				}
		}
		return false;
}
