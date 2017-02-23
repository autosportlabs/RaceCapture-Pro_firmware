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


#include "OBD2_task.h"
#include "taskUtil.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "CAN.h"
#include "OBD2.h"
#include "printk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "taskUtil.h"
#include "capabilities.h"
#include "mem_mang.h"
#include "can_mapping.h"
#include "stdutil.h"
#include <string.h>

#define OBD2_TASK_STACK 	128
#define OBD2_FEATURE_DISABLED_DELAY_MS 2000
#define CAN_MAPPING_FEATURE_DISABLED_MS 2000
#define CAN_RX_DELAY 300


#define STANDARD_PID_RESPONSE           0x7e8

#define _LOG_PFX            "[CAN] "

/* tracks the state of OBD2 channels */
struct OBD2ChannelState {
        /* the current OBD2 channel value */
        float current_value;

        /* holds the state for determining how to prioritize OBD2 queries */
        uint16_t sequencer_timeout;
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
};

static struct OBD2State obd2_state;

/* CAN bus channels current channel values */
static float * CAN_current_values = NULL;

static void OBD2_init_state(void) {
    memset(&obd2_state, 0, sizeof(struct OBD2State));
}

/*
 * Init our OBD2 state with the list of current running channels;
 */
static bool OBD2_init_current_values(OBD2Config *obd2_config) {
        uint16_t obd2_channel_count = obd2_config->enabledPids;
        /* free any previously created channel states */
        if (obd2_state.current_channel_states != NULL)
                portFree(obd2_state.current_channel_states);

        /* start the querying from the first PID */
        obd2_state.current_obd2_pid_index = 0;
        obd2_state.last_obd2_query_timestamp = 0;

        size_t max_sample_rate = 0;
        /* determine the fastest sample rate, which will set our PID querying timebase */
        for (size_t i = 0; i < obd2_channel_count; i++) {
                max_sample_rate = MAX(max_sample_rate,
                                      decodeSampleRate(obd2_config->pids[i].cfg.sampleRate));
        }
        obd2_state.max_sample_rate = max_sample_rate;
        pr_debug_int_msg(_LOG_PFX " Max OBD2 sample rate: ", obd2_state.max_sample_rate);

        if (obd2_channel_count > 0) {
                /* malloc the collection of OBD2 channels */
                size_t size = sizeof(struct OBD2ChannelState[obd2_channel_count]);
                obd2_state.current_channel_states = portMalloc(size);
                return obd2_state.current_channel_states != NULL;
        }
        else{
                /* if no OBD2 channels are enabled, don't malloc */
                obd2_state.current_channel_states = NULL;
                return true;
        }
}

float OBD2_get_current_channel_value(int index) {
    if (obd2_state.current_channel_states == NULL)
            return 0;
    return obd2_state.current_channel_states[index].current_value;
}

static void OBD2_set_current_channel_value(int index, float value)
{
        if (obd2_state.current_channel_states == NULL)
                return;
        obd2_state.current_channel_states[index].current_value = value;
}

static bool CAN_init_current_values(size_t values) {
        if (CAN_current_values != NULL)
                portFree(CAN_current_values);

        values = MAX(1, values);
        size_t size = sizeof(float[values]);
        CAN_current_values = portMalloc(size);

        if (CAN_current_values != NULL)
                memset(CAN_current_values, 0, size);

        return CAN_current_values != NULL;
}

float CAN_get_current_channel_value(int index) {
        if (CAN_current_values == NULL)
                return 0;
        return CAN_current_values[index];
}

static void CAN_set_current_channel_value(int index, float value)
{
        if (CAN_current_values == NULL)
                return;
        CAN_current_values[index] = value;
}

static void check_sequence_next_obd2_query(OBD2Config * obd2_config, uint16_t enabled_obd2_pids_count)
{
        /*
         * Should we send a PID request?
         * We should send a PID request if we're ready for the next PID (timestamp == 0)
         * or we've timed out.
         * */
        bool is_obd2_timeout = obd2_state.last_obd2_query_timestamp > 0 && isTimeoutMs(obd2_state.last_obd2_query_timestamp, OBD2_PID_DEFAULT_TIMEOUT_MS);
        if (is_obd2_timeout)
                pr_debug_int_msg(_LOG_PFX "Timeout requesting PID ", obd2_config->pids[obd2_state.current_obd2_pid_index].pid);

        if (obd2_state.last_obd2_query_timestamp == 0 || is_obd2_timeout) {

                size_t current_pid_index = obd2_state.current_obd2_pid_index;

                /*
                 * find the next PID we should query -
                 * this algorithm queries OBD2 channels at a rate
                 * proportonal to the channel's configured sample rate
                 */
                while(true) {
                        current_pid_index++;
                        if (current_pid_index >= enabled_obd2_pids_count)
                                current_pid_index = 0;

                        struct OBD2ChannelState *state = &obd2_state.current_channel_states[current_pid_index];
                        state->sequencer_timeout += decodeSampleRate(obd2_config->pids[current_pid_index].cfg.sampleRate);
                        if (state->sequencer_timeout >= obd2_state.max_sample_rate) {
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
}

static void update_can_channels(CAN_msg *msg, uint8_t can_bus, CANChannelConfig *cfg, uint16_t enabled_mapping_count)
{
        for (size_t i = 0; i < enabled_mapping_count; i++) {
                CANMapping *mapping = &cfg->can_channels[i].mapping;
                /* only process the mapping for the bus we're handling messages for */
                if (can_bus != mapping->can_channel)
                        continue;

                float value;
                /* map the CAN message to the value */
                bool result = canmapping_map_value(&value, msg, mapping);
                if (!result)
                        continue;

                CAN_set_current_channel_value(i, value);
                }
}

static void update_obd2_channels(CAN_msg *msg, OBD2Config *cfg)
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

static void OBD2Task(void *parameters)
{
        OBD2_init_state();
        LoggerConfig *lc = getWorkingLoggerConfig();
        CANChannelConfig *ccc = &lc->can_channel_cfg;
        OBD2Config *oc = &lc->OBD2Configs;

        while(1) {
                uint16_t enabled_mapping_count = 0;
                uint16_t enabled_obd2_pids_count = 0;
                bool success;

                uint16_t new_enabled_mapping_count = ccc->enabled_mappings;
                success = CAN_init_current_values(new_enabled_mapping_count);
                enabled_mapping_count = success ? new_enabled_mapping_count : 0;
                if (!success)
                        pr_error_int_msg("Failed to create buffer for CAN channels; size ", new_enabled_mapping_count);

                uint16_t new_enabled_obd2_pids_count = oc->enabledPids;
                success = OBD2_init_current_values(oc);
                enabled_obd2_pids_count = success ? new_enabled_obd2_pids_count : 0;
                if (!success)
                        pr_error_int_msg("Failed to create buffer for OBD2 channels; size ", new_enabled_obd2_pids_count);


                bool config_changed = false;


                while(! config_changed) {
                        CAN_msg msg;
                        int result = CAN_rx_msg(0, &msg, CAN_RX_DELAY );

                        if (result) {
                                if (ccc->enabled)
                                        update_can_channels(&msg, 0, ccc, enabled_mapping_count);

                                if (oc->enabled)
                                        update_obd2_channels(&msg, oc);
                        }
                        if (oc->enabled)
                                check_sequence_next_obd2_query(oc, enabled_obd2_pids_count);

                        config_changed = (enabled_mapping_count != ccc->enabled_mappings || enabled_obd2_pids_count != oc->enabledPids);
                }
                delayMs(CAN_MAPPING_FEATURE_DISABLED_MS);
        }
}

void startOBD2Task(int priority)
{
        /* Make all task names 16 chars including NULL char*/
        static const signed portCHAR task_name[] = "CAN Task       ";
        xTaskCreate(OBD2Task, task_name, OBD2_TASK_STACK, NULL, priority, NULL );
}
