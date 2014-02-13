/*
 * loggerTaskEx.c
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "watchdog.h"
#include "LED.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
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

#define LOGGER_MESSAGE_BUFFER_SIZE 10
static LoggerMessage g_sampleRecordMsgBuffer[LOGGER_MESSAGE_BUFFER_SIZE];
static LoggerMessage g_startLogMessage;
static LoggerMessage g_endLogMessage;

xSemaphoreHandle onTick;

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
	registerLuaLoggerBindings();
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "loggerEx",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

static size_t initSampleRecords(LoggerConfig *loggerConfig){
	size_t channelSampleCount = get_enabled_channel_count(loggerConfig);

	for (size_t i=0; i < LOGGER_MESSAGE_BUFFER_SIZE; i++){
		LoggerMessage *msg = (g_sampleRecordMsgBuffer + i);
		ChannelSample *channelSamples = create_channel_sample_buffer(loggerConfig, channelSampleCount);
		init_channel_sample_buffer(loggerConfig, channelSamples, channelSampleCount);
		msg->messageType = LOGGER_MSG_SAMPLE;
		msg->channelSamples = channelSamples;
	}
	return channelSampleCount;
}

static size_t calcTelemetrySampleRate(LoggerConfig *config, size_t desiredSampleRate){
	size_t maxRate = getConnectivitySampleRateLimit();
	if HIGHER_SAMPLE(desiredSampleRate, maxRate) desiredSampleRate = maxRate;
	return desiredSampleRate;
}

size_t updateSampleRates(LoggerConfig *loggerConfig, int *loggingSampleRate, int *telemetrySampleRate, int *sampleRateTimebase){
	*loggingSampleRate = getHighestSampleRate(loggerConfig);
	*sampleRateTimebase = *loggingSampleRate;
	if HIGHER_SAMPLE(BACKGROUND_SAMPLE_RATE, *sampleRateTimebase) *sampleRateTimebase = BACKGROUND_SAMPLE_RATE;
	*telemetrySampleRate = calcTelemetrySampleRate(loggerConfig, *loggingSampleRate);
	size_t channelCount = initSampleRecords(loggerConfig);
	pr_info_int(*telemetrySampleRate);
	pr_info("=telemetry sample rate\r\n");
	pr_info_int(decodeSampleRate(*loggingSampleRate));
	pr_info("=loging sample rate\r\n");
	pr_info_int(decodeSampleRate(*sampleRateTimebase));
	pr_info("=timebase sr\r\n");
	return channelCount;
}

void loggerTaskEx(void *params){

	vSemaphoreCreateBinary( onTick );

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	g_startLogMessage.messageType = LOGGER_MSG_START_LOG;
	g_endLogMessage.messageType = LOGGER_MSG_END_LOG;

	g_loggingShouldRun = 0;
	g_isLogging = 0;
	size_t bufferIndex = 0;
	size_t currentTicks = 0;
	g_configChanged = 1;
	size_t channelCount = 0;

	int loggingSampleRate = SAMPLE_DISABLED;
	int sampleRateTimebase = SAMPLE_DISABLED;
	int telemetrySampleRate = SAMPLE_DISABLED;

	while(1){
		xSemaphoreTake(onTick, portMAX_DELAY);
		currentTicks += sampleRateTimebase;

		if (g_configChanged){
			currentTicks = 0;
			channelCount = updateSampleRates(loggerConfig, &loggingSampleRate, &telemetrySampleRate, &sampleRateTimebase);
			resetLapCount();
			resetDistance();
			g_configChanged = 0;
		}

		if (g_loggingShouldRun && ! g_isLogging){
			pr_info("start logging\r\n");
			g_isLogging = 1;
			queue_logfile_record(&g_startLogMessage);
			queueTelemetryRecord(&g_startLogMessage);
			LED_disable(3);
		}
		else if (! g_loggingShouldRun && g_isLogging){
			g_isLogging = 0;
			queue_logfile_record(&g_endLogMessage);
			queueTelemetryRecord(&g_endLogMessage);
			LED_disable(2);
		}

		LoggerMessage *msg = &g_sampleRecordMsgBuffer[bufferIndex];
		int sampledRate = populate_sample_buffer(msg->channelSamples, channelCount, currentTicks);
		msg->sampleCount = channelCount;

		if (g_isLogging && (sampledRate != SAMPLE_DISABLED && sampledRate >= loggingSampleRate)){
			if (queue_logfile_record(msg) != pdTRUE) LED_enable(3);
			LED_toggle(2);
		}

		if ((sampledRate != SAMPLE_DISABLED && (sampledRate >= telemetrySampleRate || currentTicks % telemetrySampleRate == 0)) && (loggerConfig->ConnectivityConfigs.backgroundStreaming|| g_isLogging)){
			queueTelemetryRecord(msg);
		}

		if (sampledRate != SAMPLE_DISABLED){
			bufferIndex++;
			if (bufferIndex >= LOGGER_MESSAGE_BUFFER_SIZE ) bufferIndex = 0;
		}
		watchdog_reset();
		if (currentTicks % BACKGROUND_SAMPLE_RATE == 0) doBackgroundSampling();
	}
}

