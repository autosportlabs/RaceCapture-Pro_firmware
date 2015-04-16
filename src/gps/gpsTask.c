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

#define GPS_TASK_STACK_SIZE			200

static bool g_enableGpsDataLogging = false;

void setGpsDataLogging(bool enable) {
   g_enableGpsDataLogging = enable;
}

/**
 * Prints the output to the serial port if it is set.
 * @param buf The buffer containing the read in line.
 * @param len The length of the string in the buffer.
 */
//static void logGpsInput(const char *buf, int len) {
//   if (!g_enableGpsDataLogging) return;
//   pr_info(buf);
//}

void GPSTask(void *pvParameters) {
	Serial *serial = get_serial(SERIAL_GPS);
	uint8_t targetSampleRate = decodeSampleRate(getWorkingLoggerConfig()->GPSConfigs.speed.sampleRate);
	lapStats_init();
	gps_status_t gps_status = GPS_init(targetSampleRate, serial);
	if (!gps_status){
		pr_error("Error provisioning GPS module\r\n");
	}

	for (;;) {
		gps_msg_result_t result = GPS_processUpdate(serial);
		if (result == GPS_MSG_SUCCESS){
                   const GpsSnapshot snap = getGpsSnapshot();
			lapStats_processUpdate(&snap);
		}
		else{
			pr_warning("GPS: timeout\r\n");
		}
   }
}

void startGPSTask(int priority){
	xTaskCreate( GPSTask, ( signed portCHAR * )"GPSTask", GPS_TASK_STACK_SIZE, NULL, priority, NULL );
}
