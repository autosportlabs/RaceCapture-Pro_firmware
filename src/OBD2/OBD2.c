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
#define _LOG_PFX            "[OBD2] "

#define STANDARD_PID_RESPONSE           0x7e8


/* tracks the state of OBD2 channels */
struct OBD2ChannelState {
        /* the current OBD2 channel value */
        float current_value;

        /* PID associated with OBD2 channel */
        uint8_t pid;

        /* holds the state for determining how to prioritize OBD2 queries */
        uint16_t sequencer_timeout;

        /* indicates if channel is disabled */
        bool squelched;
};

/* manages the running state of OBD2 queries */
struct OBD2State {
        /* points to a dynamically created array of OBD2ChannelState structs */
        struct OBD2ChannelState * current_channel_states;

        /* holds the last query timestamp, for determining OBD2 query timeouts */
        size_t last_obd2_query_timestamp;

        /* the index of the current OBD2 PID we're querying */
        size_t current_obd2_pid_index;

        /*
         * the max sample rate across all of the channels;
         * will set the time base for the fastest PID querying
         */
        size_t max_sample_rate;

        /*
         * number of OBD2 channels
         */
        size_t channel_count;
};

static struct OBD2State obd2_state;

void OBD2_init_state(void) {
    memset(&obd2_state, 0, sizeof(struct OBD2State));
}

/*
 * Init our OBD2 state with the list of current running channels
 */
bool OBD2_init_current_values(OBD2Config *obd2_config) {
        uint16_t obd2_channel_count = obd2_config->enabledPids;

        /* free any previously created channel states */
        if (obd2_state.current_channel_states != NULL)
                portFree(obd2_state.current_channel_states);

        /* start the querying from the first PID */
        obd2_state.current_obd2_pid_index = 0;
        obd2_state.last_obd2_query_timestamp = 0;

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
				obd2_state.current_channel_states[i].pid = obd2_config->pids[i].pid;
		}

		obd2_state.channel_count = obd2_config->enabledPids;
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

void check_sequence_next_obd2_query(OBD2Config * obd2_config, uint16_t enabled_obd2_pids_count)
{
		/*
		 * Should we send a PID request?
		 * We should send a PID request if we're ready for the next PID (timestamp == 0)
		 * or we've timed out.
		 */
		if (enabled_obd2_pids_count == 0)
				return;

        bool is_obd2_timeout = obd2_state.last_obd2_query_timestamp > 0 &&
        		isTimeoutMs(obd2_state.last_obd2_query_timestamp, OBD2_PID_DEFAULT_TIMEOUT_MS);

        if (is_obd2_timeout)
                pr_debug_int_msg(_LOG_PFX "Timeout requesting PID ", obd2_config->pids[obd2_state.current_obd2_pid_index].pid);

        if (! (obd2_state.last_obd2_query_timestamp == 0 || is_obd2_timeout))
        		return;

		/*
		 * find the next PID we should query -
		 * this algorithm queries OBD2 channels at a rate
		 * proportional to the channel's configured sample rate
		 */
		size_t current_pid_index = obd2_state.current_obd2_pid_index;
		bool looped = false;
		while(true) {
				current_pid_index++;
				if (current_pid_index >= enabled_obd2_pids_count) {
						if (looped)
								/* nothing was selected, bail out */
								return;
						current_pid_index = 0;
						looped = true;
				}
				struct OBD2ChannelState *state = &obd2_state.current_channel_states[current_pid_index];
				if (state->squelched)
						/* if channel is squelched then skip */
						continue;

				state->sequencer_timeout += decodeSampleRate(obd2_config->pids[current_pid_index].cfg.sampleRate);
				if (state->sequencer_timeout >= obd2_state.max_sample_rate) {
						/* we've selected a PID to query */
						state->sequencer_timeout = 0;
						break;
				}
		}

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
        PidConfig *pid_config = &cfg->pids[obd2_state.current_obd2_pid_index];
        /* Did we get an OBDII PID we were waiting for? */
        if (obd2_state.last_obd2_query_timestamp &&
            msg->addressValue == STANDARD_PID_RESPONSE &&
            msg->data[2] == pid_config->pid ) {
                    float value;
                    bool result = canmapping_map_value(&value, msg, &pid_config->mapping);
                    if (result)
                            OBD2_set_current_channel_value(obd2_state.current_obd2_pid_index, value);

                    /* PID request is complete */
                    obd2_state.last_obd2_query_timestamp = 0;
        }
}

int OBD2_request_PID(uint8_t pid, uint8_t mode, size_t timeout)
{
        pr_debug_int_msg(_LOG_PFX " Requesting PID ", pid);
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
