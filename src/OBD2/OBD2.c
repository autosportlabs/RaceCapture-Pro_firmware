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
#define STANDARD_PID_RESPONSE           0x7e8
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
        uint8_t pid;

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
         * flag to indicate if config was stale
         */
        bool is_stale;
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

bool OBD2_init_current_values(OBD2Config *obd2_config)
{
        uint16_t obd2_channel_count = obd2_config->enabledPids;

        /* free any previously created channel states */
        if (obd2_state.current_channel_states != NULL)
                portFree(obd2_state.current_channel_states);

        /* start the querying from the first PID */
        obd2_state.current_obd2_pid_index = 0;
        obd2_state.last_obd2_query_timestamp = 0;
        obd2_state.squelched_count = 0;

        /* determine the fastest sample rate, which will set our PID querying timebase */
        size_t max_sample_rate = 0;
        for (size_t i = 0; i < obd2_channel_count; i++) {
                max_sample_rate = MAX(max_sample_rate,
                                      decodeSampleRate(obd2_config->pids[i].cfg.sampleRate));
        }
        obd2_state.max_sample_rate = max_sample_rate;
        pr_debug_int_msg(_LOG_PFX " Max OBD2 sample rate: ", obd2_state.max_sample_rate);

        if (obd2_channel_count == 0) {
        		/* if no OBD2 channels are enabled, don't malloc */
				obd2_state.current_channel_states = NULL;
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
		}

		obd2_state.channel_count = obd2_config->enabledPids;
		obd2_state.is_stale = false;
		return true;
}

float OBD2_get_current_channel_value(int index) {
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
                state->timeout_count++;
                if (state->timeout_count >= OBD2_TIMEOUT_DISABLE_THRESHOLD) {
                		state->channel_status = OBD2_CHANNEL_STATUS_SQUELCHED;
                		pr_debug_int_msg(_LOG_PFX "Excessive timeouts, squelching PID ", state->pid);
                		obd2_state.squelched_count++;
                		/**
                		 * if all channels end up being squelched, then we should just reset OBD2 config
                		 * This accounts for cases where there's a complete disconnect and a reset is needed
                		 */
                		if (obd2_state.squelched_count == obd2_state.channel_count) {
                				pr_debug(_LOG_PFX "all channels timed out, resetting OBD2 state\r\n");
                				obd2_state.is_stale = true;
                		}
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
                uint16_t sample_rate = decodeSampleRate(obd2_config->pids[i].cfg.sampleRate);
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
		int pid_request_result = OBD2_request_PID(pid_cfg->pid, pid_cfg->mode, OBD2_PID_REQUEST_TIMEOUT_MS);
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
        if (obd2_state.last_obd2_query_timestamp &&
            msg->addressValue == STANDARD_PID_RESPONSE &&
            msg->data[2] == pid_config->pid ) {
                    float value;
                    bool result = canmapping_map_value(&value, msg, &pid_config->mapping);
                    if (result) {
                            OBD2_set_current_channel_value(current_pid_index, value);
                            channel_state->channel_status = OBD2_CHANNEL_STATUS_DATA_RECEIVED;
                            channel_state->timeout_count = 0;
                    }
                    /* PID request is complete */
                    obd2_state.last_obd2_query_timestamp = 0;
        }
}

int OBD2_request_PID(uint8_t pid, uint8_t mode, size_t timeout)
{
		CAN_msg msg;
		msg.addressValue = 0x7df;
		msg.data[0] = 2;
		msg.data[1] = mode;
		msg.data[2] = pid;
		msg.data[3] = 0x55;
		msg.data[4] = 0x55;
		msg.data[5] = 0x55;
		msg.data[6] = 0x55;
		msg.data[7] = 0x55;
		msg.dataLength = 8;
		msg.isExtendedAddress = 0;
		return CAN_tx_msg(0, &msg, timeout);
}

bool OBD2_get_value_for_pid(uint8_t pid, float *value)
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
