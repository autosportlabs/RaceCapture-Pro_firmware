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
#include <string.h>

#define OBD2_TASK_STACK 	128
#define OBD2_FEATURE_DISABLED_DELAY_MS 2000
#define CAN_MAPPING_FEATURE_DISABLED_MS 2000
#define CAN_RX_DELAY 1000

#define _LOG_PFX            "[CAN] "
typedef struct _CANTaskParams {
        uint8_t can_bus;
        bool obd2_enabled;
} CANTaskParams;

static float * CAN_current_values = NULL;

bool CAN_init_current_values(size_t values) {
    if (CAN_current_values != NULL)
            portFree(CAN_current_values);

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

void CAN_set_current_channel_value(int index, float value)
{
    if (CAN_current_values == NULL)
            return;
    CAN_current_values[index] = value;
}

void startOBD2Task(int priority)
{
    for (uint8_t bus = 0; bus < CAN_CHANNELS; bus++ ) {
            CANTaskParams * params = (CANTaskParams *)portMalloc(sizeof(CANTaskParams));
            params->can_bus = bus;
            params->obd2_enabled = bus == 0 ? true : false;

            /* Make all task names 16 chars including NULL char*/
            static const signed portCHAR task_name[] = "CAN Task";
            xTaskCreate(OBD2Task, task_name, OBD2_TASK_STACK, params, priority, NULL );
    }
}

/*
static void OBD2Task2(void *pvParameters)
{
    pr_info("Start OBD2 task\r\n");
    size_t max_obd2_samplerate = msToTicks((TICK_RATE_HZ / MAX_OBD2_SAMPLE_RATE));
    LoggerConfig *config = getWorkingLoggerConfig();
    OBD2Config *oc = &config->OBD2Configs;
    while(1) {
        while(oc->enabled && oc->enabledPids > 0) {
            for (size_t i = 0; i < oc->enabledPids; i++) {
                PidConfig *pidCfg = &oc->pids[i];
                int value;
                unsigned char pid = pidCfg->pid;
                if (OBD2_request_PID(pid, &value, OBD2_PID_DEFAULT_TIMEOUT_MS)) {
                    OBD2_set_current_PID_value(i, value);
                    if (TRACE_LEVEL) {
                        pr_trace("read OBD2 PID ");
                        pr_trace_int(pid);
                        pr_trace("=")
                        pr_trace_int(value);
                        pr_trace("\r\n");
                    }
                } else {
                    pr_warning_int_msg("OBD2: PID read fail: ", pid);
                }
                delayTicks(max_obd2_samplerate);
            }
        }
        delayMs(OBD2_FEATURE_DISABLED_DELAY_MS);
    }
}
*/
void OBD2Task(void *parameters)
{
        CANTaskParams *params = (CANTaskParams *)parameters;
        uint8_t can_bus = params->can_bus;
        bool obd2_enabled = params->obd2_enabled;
        portFree(params);

        obd2_enabled = obd2_enabled;
        pr_info_int_msg(_LOG_PFX "Start CAN task ", can_bus);

        LoggerConfig *config = getWorkingLoggerConfig();
        CANChannelConfig *ccc = &config->can_channel_cfg;
        size_t enabled_mapping_count = 0;
        while(1) {
                size_t new_enabled_mapping_count = ccc->enabled_mappings;
                /* if the number of mappings changed, re-initialize our current values buffer */
                if (new_enabled_mapping_count != enabled_mapping_count) {
                        bool success = CAN_init_current_values(new_enabled_mapping_count);
                        enabled_mapping_count = success ? new_enabled_mapping_count : 0;
                        if (!success)
                                pr_error_int_msg("Failed to create buffer for CAN channels; size ", new_enabled_mapping_count);
                }
                while(ccc->enabled && enabled_mapping_count > 0 && ccc->enabled_mappings == enabled_mapping_count) {
                        CAN_msg msg;
                        int result = CAN_rx_msg(can_bus, &msg, CAN_RX_DELAY );
                        if (!result)
                                continue;

                        for (size_t i = 0; i < ccc->enabled_mappings; i++) {
                                CANMapping *mapping = &ccc->can_channels[i].mapping;
                                /* only process the mapping for the bus we're handling messages for */
                                if (can_bus != mapping->can_channel)
                                        continue;

                                float value;
                                /* map the CAN message to the value */
                                bool result = canmapping_map_value(&value, &msg, mapping);
                                if (!result)
                                        continue;

                                CAN_set_current_channel_value(i, value);
                                }
                }
                delayMs(CAN_MAPPING_FEATURE_DISABLED_MS);
        }
}
