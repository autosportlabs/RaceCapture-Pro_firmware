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
#include "printk.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

#define MAX_TELEMETRY_SAMPLE_RATE			SAMPLE_50Hz
#define ACCELEROMETER_SAMPLE_RATE			SAMPLE_100Hz

int g_loggingShouldRun;
int g_isLogging;

#define SAMPLE_RECORD_BUFFER_SIZE 10
static LoggerMessage g_sampleRecordMsgBuffer[SAMPLE_RECORD_BUFFER_SIZE];
static SampleRecord g_sampleRecordBuffer[SAMPLE_RECORD_BUFFER_SIZE];
static LoggerMessage g_startLogMessage;
static LoggerMessage g_endLogMessage;

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

static void initSampleRecords(LoggerConfig *loggerConfig){
	for (size_t i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++){
		SampleRecord *sr = &g_sampleRecordBuffer[i];
		initSampleRecord(loggerConfig, sr);
		g_sampleRecordMsgBuffer[i].messageType = LOGGER_MSG_SAMPLE;
		g_sampleRecordMsgBuffer[i].sampleRecord = sr;
	}
}

void loggerTaskEx(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	size_t loggingSampleRate = getHighestSampleRate(loggerConfig);
	size_t telemetrySampleRate = loggingSampleRate;
	size_t sampleRateTimebase = loggingSampleRate;

	if HIGHER_SAMPLE(telemetrySampleRate, MAX_TELEMETRY_SAMPLE_RATE) telemetrySampleRate = MAX_TELEMETRY_SAMPLE_RATE;
	if HIGHER_SAMPLE(ACCELEROMETER_SAMPLE_RATE, sampleRateTimebase) sampleRateTimebase = ACCELEROMETER_SAMPLE_RATE;

	initSampleRecords(loggerConfig);
	size_t bufferIndex = 0;

	g_loggingShouldRun = 0;
	g_isLogging = 0;
	size_t currentTicks = 0;

	g_startLogMessage.messageType = LOGGER_MSG_START_LOG;
	g_endLogMessage.messageType = LOGGER_MSG_END_LOG;

	while(1){
		portTickType xLastWakeTime = xTaskGetTickCount();
		currentTicks += sampleRateTimebase;

		sampleAllAccel();

		if (g_loggingShouldRun && ! g_isLogging){
			pr_info("startLog\r\n");
			g_isLogging = 1;
			queueLogfileRecord(&g_startLogMessage);
			queueTelemetryRecord(&g_startLogMessage);
		}
		else if (! g_loggingShouldRun && g_isLogging){
			g_isLogging = 0;
			queueLogfileRecord(&g_endLogMessage);
			queueTelemetryRecord(&g_endLogMessage);
			disableLED(LED2);
		}

		LoggerMessage *msg = &g_sampleRecordMsgBuffer[bufferIndex];
		clearSampleRecord(msg->sampleRecord);
		populateSampleRecord(msg->sampleRecord, currentTicks, loggerConfig);

		if (g_isLogging && ((currentTicks % loggingSampleRate) == 0)){
			queueLogfileRecord(msg);
			toggleLED(LED2);
		}

		if ((currentTicks % telemetrySampleRate) == 0) queueTelemetryRecord(msg);

		bufferIndex++;
		if (bufferIndex >= SAMPLE_RECORD_BUFFER_SIZE ) bufferIndex = 0;

		ResetWatchdog();
		vTaskDelayUntil( &xLastWakeTime, sampleRateTimebase );
	}
}

