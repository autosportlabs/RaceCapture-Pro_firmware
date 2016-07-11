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


#include "FreeRTOS.h"
#include "auto_logger.h"
#include "gps.h"
#include "gpsTask.h"
#include "gps_device.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include "printk.h"
#include "serial.h"
#include "task.h"
#include "taskUtil.h"

#define GPS_TASK_STACK_SIZE	256
#define MSG_FAILURES_TRIGGER	3

static bool g_enableGpsDataLogging = false;

void setGpsDataLogging(bool enable)
{
    g_enableGpsDataLogging = enable;
}

void GPSTask(void *pvParameters)
{
        LoggerConfig *lc = getWorkingLoggerConfig();
        struct Serial *serial = serial_device_get(SERIAL_GPS);
        const uint8_t targetSampleRate =
                decodeSampleRate(lc->GPSConfigs.speed.sampleRate);

        /* Call this here to effectively reset lapstats */
        lapstats_config_changed();

        auto_logger_init(&lc->auto_logger_cfg);

        while(1) {
                size_t failures = 0;

                const gps_status_t gps_status = GPS_init(targetSampleRate, serial);
                if (!gps_status) {
                        pr_error("GPS: Error provisioning\r\n");
                }

                for (;;) {
                        gps_msg_result_t result = GPS_processUpdate(serial);
                        if (result == GPS_MSG_SUCCESS) {
                                const GpsSnapshot snap = getGpsSnapshot();
                                lapstats_processUpdate(&snap);
                                auto_logger_gps_sample_cb(&snap.sample);

                                if (failures > 0)
                                        --failures;
                        } else {
                                pr_debug("GPS: Msx Rx Failure\r\n");
                                if (++failures >= MSG_FAILURES_TRIGGER) {
                                        pr_warning("GPS: Too many failures.  "
                                                   "Reenum\r\n");
                                        break;
                                }
                        }
                }
        }
}

void startGPSTask(int priority)
{
        /* Make all task names 16 chars including NULL char*/
        static const signed portCHAR task_name[] = "GPS Comm Task  ";
        xTaskCreate(GPSTask, task_name, GPS_TASK_STACK_SIZE, NULL,
                    priority, NULL );
}
