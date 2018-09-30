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

#define GPS_TASK_STACK_SIZE	256
#define MSG_FAILURES_TRIGGER	3

void GPSTask(void *pvParameters)
{
        struct Serial *serial = serial_device_get(SERIAL_GPS);
        const uint8_t targetSampleRate = decodeSampleRate(logger_config_get_gps_sample_rate());

        /* Call this here to effectively reset lapstats */
        lapstats_config_changed();

        while(1) {
                size_t failures = 0;

                const gps_status_t gps_status = GPS_init(targetSampleRate, serial);
                if (!gps_status) {
                        pr_error("GPS: Error provisioning\r\n");
                }

                for (;;) {
                        GpsSample s;

                        const gps_msg_result_t result = GPS_device_get_update(&s, serial);
                        gps_flash_status_led(s.quality);

                        if (result == GPS_MSG_DEFERRED) {
                                lapstats_process_incremental(&s);
                        } else if (result == GPS_MSG_SUCCESS) {
                                lapstats_process_incremental(&s);
                                GPS_sample_update(&s);

                                GpsSnapshot snap = getGpsSnapshot();
                                lapstats_processUpdate(&snap);

                                if (failures > 0)
                                        --failures;
                        } else {
                                pr_debug("GPS: Msx Rx Failure\r\n");
                                if (++failures >= MSG_FAILURES_TRIGGER) {
                                        pr_debug("GPS: Too many failures.  "
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
