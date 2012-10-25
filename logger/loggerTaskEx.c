/*
 * loggerTaskEx.c
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fileWriter.h"
#include "telemetryTask.h"
#include "sampleRecord.h"
#include "loggerData.h"
#include "loggerTaskEx.h"
#include "loggerHardware.h"
#include "loggerData.h"
#include "accelerometer.h"
#include "luaLoggerBinding.h"
#include "gps.h"
#include "sdcard.h"
#include "usb_comm.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

int g_loggingShouldRun;
xSemaphoreHandle g_xLoggerStart;

void createLoggerTaskEx(){

	g_loggingShouldRun = 0;

	registerLuaLoggerBindings();

	vSemaphoreCreateBinary( g_xLoggerStart );
	xSemaphoreTake( g_xLoggerStart, 1 );
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "loggerEx",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

#define SAMPLE_RECORD_BUFFER_SIZE 10

static SampleRecord g_sampleRecordBuffer[SAMPLE_RECORD_BUFFER_SIZE];

void loggerTaskEx(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	if ( loggerConfig->AccelInstalled == CONFIG_FEATURE_INSTALLED ) accel_setup();

	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, IDLE_TIMEOUT) != pdTRUE){
			ResetWatchdog();
			//perform idle tasks
		}
		else {
			const portTickType xFrequency = getHighestSampleRate(loggerConfig);

			g_loggingShouldRun = 1;

			portTickType currentTicks = 0;

			for (int i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++) initSampleRecord(loggerConfig,&g_sampleRecordBuffer[i]);

			int bufferIndex = 0;

			//run until signalled to stop
			portTickType xLastWakeTime = xTaskGetTickCount();
			while (g_loggingShouldRun){
				ResetWatchdog();

				ToggleLED(LED2);

				currentTicks += xFrequency;

				SampleRecord *sr = &g_sampleRecordBuffer[bufferIndex];//srBuffer[bufferIndex];

				clearSampleRecord(sr);

				populateSampleRecord(sr, currentTicks, loggerConfig);

				queueLogfileRecord(sr);
				queueTelemetryRecord(sr);

				bufferIndex++;
				if (bufferIndex >= SAMPLE_RECORD_BUFFER_SIZE ) bufferIndex = 0;

				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}

			queueLogfileRecord(NULL);
			queueTelemetryRecord(NULL);

			DisableLED(LED2);
		}
	}


}

//test code for detecting power loss via +12v bus.
/*				if (ReadADC(7) < 230){
	g_loggingShouldRun = 0;

	fileWriteString(&f,"x\r\n");
	break;
}

*/
