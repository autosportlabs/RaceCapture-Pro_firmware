/*
 * gpsTask.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "gpsTask.h"
#include "gps.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

#define GPS_DATA_LINE_BUFFER_LEN 	200
#define GPS_TASK_STACK_SIZE			100

static char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

void GPSTask( void *pvParameters ){
	for( ;; )
	{
		int len = usart1_readLine(g_GPSdataLine, GPS_DATA_LINE_BUFFER_LEN);
		processGPSData(g_GPSdataLine, len);
	}
}

void startGPSTask(int priority){
	initGPS();
	initUsart1(8, 0, 1, 38400);
	xTaskCreate( GPSTask, ( signed portCHAR * )"GPSTask", GPS_TASK_STACK_SIZE, NULL, priority, NULL );
}

