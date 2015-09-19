/*
 * gpsTask.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "gpsTask.h"
#include "gps.h"
#include "lap_stats.h"
#include "gps_device.h"
#include "FreeRTOS.h"
#include "printk.h"
#include "task.h"
#include "serial.h"
#include "taskUtil.h"
#include "loggerConfig.h"

#define GPS_TASK_STACK_SIZE	256

static bool g_enableGpsDataLogging = false;

void setGpsDataLogging(bool enable)
{
    g_enableGpsDataLogging = enable;
}

void GPSTask(void *pvParameters)
{
    Serial *serial = get_serial(SERIAL_GPS);
    uint8_t targetSampleRate = decodeSampleRate(getWorkingLoggerConfig()->GPSConfigs.speed.sampleRate);
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
