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
#include "connectivityTask.h"
#include "sampleRecord.h"
#include "loggerData.h"
#include "loggerTaskEx.h"
#include "loggerHardware.h"
#include "loggerData.h"
#include "loggerConfig.h"
#include "accelerometer.h"
#include "luaLoggerBinding.h"
#include "gps.h"
#include "sdcard.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

#define MAX_TELEMETRY_SAMPLE_RATE			SAMPLE_50Hz
#define ACCELEROMETER_SAMPLE_RATE			SAMPLE_100Hz

int g_loggingShouldRun;
int g_isLogging;


int isLogging(){
	return g_isLogging;
}

void startLogging(){
	g_loggingShouldRun = 1;
}

void stopLogging(){
	g_loggingShouldRun = 0;
}

void createLoggerTaskEx(){

	g_loggingShouldRun = 0;

	registerLuaLoggerBindings();
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "loggerEx",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

#define SAMPLE_RECORD_BUFFER_SIZE 10

static SampleRecord g_sampleRecordBuffer[SAMPLE_RECORD_BUFFER_SIZE];

void loggerTaskEx(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	size_t loggingSampleRate = getHighestSampleRate(loggerConfig);
	size_t telemetrySampleRate = loggingSampleRate;
	size_t sampleRateTimebase = loggingSampleRate;

	if HIGHER_SAMPLE(telemetrySampleRate, MAX_TELEMETRY_SAMPLE_RATE) telemetrySampleRate = MAX_TELEMETRY_SAMPLE_RATE;
	if HIGHER_SAMPLE(ACCELEROMETER_SAMPLE_RATE, sampleRateTimebase) sampleRateTimebase = ACCELEROMETER_SAMPLE_RATE;

	for (size_t i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++) initSampleRecord(loggerConfig,&g_sampleRecordBuffer[i]);
	size_t bufferIndex = 0;

	g_loggingShouldRun = 0;
	g_isLogging = 0;
	size_t currentTicks = 0;

	while(1){
		portTickType xLastWakeTime = xTaskGetTickCount();
		currentTicks += sampleRateTimebase;

		SampleRecord *sr = &g_sampleRecordBuffer[bufferIndex];
		clearSampleRecord(sr);
		populateSampleRecord(sr, currentTicks, loggerConfig);

		if ((currentTicks % loggingSampleRate) == 0){
			if (g_isLogging){
				if (g_loggingShouldRun){
					toggleLED(LED2);
					queueLogfileRecord(sr);
				}
				else{
					queueLogfileRecord(NULL);
					queueTelemetryRecord(NULL);
					g_isLogging = 0;
				}
			}
			else if (g_loggingShouldRun) g_isLogging = 1;
		}

		if ((currentTicks % MAX_TELEMETRY_SAMPLE_RATE) == 0) queueTelemetryRecord(sr);

		bufferIndex++;
		if (bufferIndex >= SAMPLE_RECORD_BUFFER_SIZE ) bufferIndex = 0;

		ResetWatchdog();
		vTaskDelayUntil( &xLastWakeTime, sampleRateTimebase );
	}
}

/*
void loggerTaskEx2(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, IDLE_TIMEOUT) != pdTRUE){
			ResetWatchdog();
			//perform idle tasks
		}
		else {
			const portTickType xFrequency = getHighestSampleRate(loggerConfig);

			g_loggingShouldRun = 1;

			resetLapCount();
			resetDistance();

			portTickType currentTicks = 0;

			for (int i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++) initSampleRecord(loggerConfig,&g_sampleRecordBuffer[i]);

			int bufferIndex = 0;

			//run until signalled to stop
			portTickType xLastWakeTime = xTaskGetTickCount();
			while (g_loggingShouldRun){
				ResetWatchdog();

				toggleLED(LED2);

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

			disableLED(LED2);
		}
	}


}

*/
