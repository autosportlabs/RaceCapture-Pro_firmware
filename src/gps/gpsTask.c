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
#include "gps.h"
#include "gpsTask.h"
#include "gps_device.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include "printk.h"
#include "serial.h"
#include "task.h"
#include "taskUtil.h"

#define GPS_TASK_STACK_SIZE	300

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

        lapStats_init();

    while(1) {
        const gps_status_t gps_status = GPS_init(targetSampleRate, serial);
        if (!gps_status) {
            pr_error("GPS: Error provisioning\r\n");
        }

        for (;;) {
            gps_msg_result_t result = GPS_processUpdate(serial);
            if (result == GPS_MSG_SUCCESS) {
                const GpsSnapshot snap = getGpsSnapshot();
                lapstats_processUpdate(&snap);
            } else {
                pr_warning("GPS: timeout\r\n");
                break;
            }
        }
    }
}

void startGPSTask(int priority)
{
    xTaskCreate( GPSTask, ( signed portCHAR * )"GPSTask", GPS_TASK_STACK_SIZE, NULL, priority, NULL );
}
