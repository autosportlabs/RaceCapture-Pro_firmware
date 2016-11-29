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

#define OBD2_TASK_STACK 	128
#define OBD2_FEATURE_DISABLED_DELAY_MS 2000

void startOBD2Task(int priority)
{
        /* Make all task names 16 chars including NULL char*/
        static const signed portCHAR task_name[] = "OBD-II Task    ";
        xTaskCreate(OBD2Task, task_name, OBD2_TASK_STACK, NULL,
                    priority, NULL );
}

void OBD2Task(void *pvParameters)
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
