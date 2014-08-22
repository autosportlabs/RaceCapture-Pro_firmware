/*
 * gpsTask.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "gpsTask.h"
#include "gps.h"
#include "FreeRTOS.h"
#include "printk.h"
#include "task.h"
#include "serial.h"

#define GPS_DATA_LINE_BUFFER_LEN 	200
#define GPS_TASK_STACK_SIZE			130

static char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];
static bool g_enableGpsDataLogging = false;

void setGpsDataLogging(bool enable) {
   g_enableGpsDataLogging = enable;
}

/**
 * Prints the output to the serial port if it is set.
 * @param buf The buffer containing the read in line.
 * @param len The length of the string in the buffer.
 */
static void logGpsInput(const char *buf, int len) {
   if (!g_enableGpsDataLogging) return;
   pr_info(buf);
}

void GPSTask(void *pvParameters) {
	Serial *gpsSerial = get_serial(SERIAL_GPS);

	for (;;) {
      int len = gpsSerial->get_line(g_GPSdataLine, GPS_DATA_LINE_BUFFER_LEN - 1);
      g_GPSdataLine[len] = '\0';
      logGpsInput(g_GPSdataLine, len);
      processGPSData(g_GPSdataLine, len);
   }
}

void startGPSTask(int priority){
	initGPS();
	xTaskCreate( GPSTask, ( signed portCHAR * )"GPSTask", GPS_TASK_STACK_SIZE, NULL, priority, NULL );
}

