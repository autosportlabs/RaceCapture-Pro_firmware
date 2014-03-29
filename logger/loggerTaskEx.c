/*
 * loggerTaskEx.c
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "luaTask.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "sampleRecord.h"
#include "loggerData.h"
#include "loggerTaskEx.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "accelerometer.h"
#include "luaLoggerBinding.h"
#include "gps.h"
#include "sdcard.h"
#include "printk.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

#define BACKGROUND_SAMPLE_RATE				SAMPLE_50Hz

int g_loggingShouldRun;
int g_isLogging;
int g_configChanged;
int g_telemetryBackgroundStreaming;

#define SAMPLE_RECORD_BUFFER_SIZE 10
static LoggerMessage g_sampleRecordMsgBuffer[SAMPLE_RECORD_BUFFER_SIZE];
static SampleRecord g_sampleRecordBuffer[SAMPLE_RECORD_BUFFER_SIZE];
static LoggerMessage g_startLogMessage;
static LoggerMessage g_endLogMessage;


void configChanged(){
	g_configChanged = 1;
}

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
	registerLuaLoggerBindings(getLua());
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "loggerEx",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

static void initSampleRecords(LoggerConfig *loggerConfig){
	for (size_t i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++){
		LoggerMessage *msg = (g_sampleRecordMsgBuffer + i);
		SampleRecord *sr = (g_sampleRecordBuffer + i);
		initSampleRecord(loggerConfig, sr);
		msg->messageType = LOGGER_MSG_SAMPLE;
		msg->sampleRecord = sr;
	}
}

static size_t calcTelemetrySampleRate(LoggerConfig *config, size_t desiredSampleRate){
	size_t maxRate = getConnectivitySampleRateLimit();
	if HIGHER_SAMPLE(desiredSampleRate, maxRate) desiredSampleRate = maxRate;
	return desiredSampleRate;
}

void updateSampleRates(LoggerConfig *loggerConfig, int *loggingSampleRate, int *telemetrySampleRate, int *sampleRateTimebase){
	*loggingSampleRate = getHighestSampleRate(loggerConfig);
	*sampleRateTimebase = *loggingSampleRate;
	if HIGHER_SAMPLE(BACKGROUND_SAMPLE_RATE, *sampleRateTimebase) *sampleRateTimebase = BACKGROUND_SAMPLE_RATE;
	*telemetrySampleRate = calcTelemetrySampleRate(loggerConfig, *loggingSampleRate);
	initSampleRecords(getWorkingLoggerConfig());
	pr_info_int(*telemetrySampleRate);
	pr_info("=tel sr\r\n");
	pr_info_int(*loggingSampleRate);
	pr_info("=log sr\r\n");
	pr_info_int(*sampleRateTimebase);
	pr_info("=timebase sr\r\n");
}


void loggerTaskEx(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	g_startLogMessage.messageType = LOGGER_MSG_START_LOG;
	g_endLogMessage.messageType = LOGGER_MSG_END_LOG;

	g_loggingShouldRun = 0;
	g_isLogging = 0;
	size_t bufferIndex = 0;
	size_t currentTicks = 0;
	g_configChanged = 1;

	int loggingSampleRate = SAMPLE_DISABLED;
	int sampleRateTimebase = SAMPLE_DISABLED;
	int telemetrySampleRate = SAMPLE_DISABLED;

	while(1){
		portTickType xLastWakeTime = xTaskGetTickCount();
		currentTicks += sampleRateTimebase;

		if (currentTicks % BACKGROUND_SAMPLE_RATE == 0) doBackgroundSampling();

		if (g_configChanged){
			currentTicks = 0;
			updateSampleRates(loggerConfig, &loggingSampleRate, &telemetrySampleRate, &sampleRateTimebase);
			resetLapCount();
			resetDistance();
			g_configChanged = 0;
		}

		if (g_loggingShouldRun && ! g_isLogging){
			pr_info("startLog\r\n");
			g_isLogging = 1;
			queue_logfile_record(&g_startLogMessage);
			queueTelemetryRecord(&g_startLogMessage);
			disableLED(LED3);
		}
		else if (! g_loggingShouldRun && g_isLogging){
			g_isLogging = 0;
			queue_logfile_record(&g_endLogMessage);
			queueTelemetryRecord(&g_endLogMessage);
			disableLED(LED2);
		}

		LoggerMessage *msg = &g_sampleRecordMsgBuffer[bufferIndex];
		clearSampleRecord(msg->sampleRecord);
		int sampledRate = populateSampleRecord(msg->sampleRecord, currentTicks, loggerConfig);

		if (g_isLogging && (sampledRate != SAMPLE_DISABLED && sampledRate >= loggingSampleRate)){
			if (queue_logfile_record(msg) != pdTRUE) enableLED(LED3);
			toggleLED(LED2);
		}

		if ((sampledRate != SAMPLE_DISABLED && (sampledRate >= telemetrySampleRate || currentTicks % telemetrySampleRate == 0)) && (loggerConfig->ConnectivityConfigs.backgroundStreaming|| g_isLogging)){
			queueTelemetryRecord(msg);
		}

		if (sampledRate != SAMPLE_DISABLED){
			bufferIndex++;
			if (bufferIndex >= SAMPLE_RECORD_BUFFER_SIZE ) bufferIndex = 0;
		}

		ResetWatchdog();
		vTaskDelayUntil( &xLastWakeTime, sampleRateTimebase );
	}
}

