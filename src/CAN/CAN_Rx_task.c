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


#include "CAN_Rx_task.h"
#include "taskUtil.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "CAN.h"
#include "OBD2.h"
#include "CANMap.h"
#include "printk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "taskUtil.h"
#include "capabilities.h"

#define CAN_RX_TASK_STACK 	configMINIMAL_STACK_SIZE
#define CAN_RX_FEATURE_DISABLED_DELAY_MS 2000

void startCANRxTask(int priority)
{
        /* Make all task names 16 chars including NULL char*/
        static const signed portCHAR task_name[] = "CAN Rx Task    ";
        xTaskCreate(CANRxTask, task_name, CAN_RX_TASK_STACK, NULL,
                    priority, NULL );
}

void CANRxTask(void *pvParameters)
{
    pr_info("Start CAN Rx task\r\n");
    size_t max_can_samplerate = msToTicks((TICK_RATE_HZ / MAX_OBD2_SAMPLE_RATE));
    LoggerConfig *config = getWorkingLoggerConfig();
    CANMapConfig *cMapConf = &config->CANMapConfigs;
    OBD2Config *oc = &config->OBD2Configs;

    CAN_msg msg;
    int value;

    while(1) {
        while(cMapConf->enabled && cMapConf->enabledChannels > 0) {
            for (size_t channel = 0; channel < CAN_CHANNELS; channel++) {
                if (CAN_rx_msg(channel, &msg, DEFAULT_CAN_TIMEOUT)) {
                    if (OBD2_process_PID(oc, &msg, &value)) {

                    }
                    else if (CAN_process_map(cMapConf, &msg, &value)) {

                    }
                    else {

                    }
                }
            }

            delayTicks(max_can_samplerate);
        }
        delayMs(CAN_RX_FEATURE_DISABLED_DELAY_MS);
    }
}
