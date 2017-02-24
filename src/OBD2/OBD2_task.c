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

#define _LOG_PFX            "[CANOBD2] "

#define OBD2_TASK_STACK 	128
#define OBD2_FEATURE_DISABLED_DELAY_MS 2000
#define CAN_MAPPING_FEATURE_DISABLED_MS 2000
#define CAN_RX_DELAY 300

/* CAN bus channels current channel values */
static float * CAN_current_values = NULL;

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
