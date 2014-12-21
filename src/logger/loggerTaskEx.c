/*
 * loggerTaskEx.c
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "mod_string.h"
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
#include "imu.h"
#include "gps.h"
#include "printk.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

#define BACKGROUND_SAMPLE_RATE				SAMPLE_50Hz

int g_loggingShouldRun;
int g_isLogging;
int g_configChanged;
int g_telemetryBackgroundStreaming;

xSemaphoreHandle onTick;

#define LOGGER_MESSAGE_BUFFER_SIZE 10
static LoggerMessage g_sampleRecordMsgBuffer[LOGGER_MESSAGE_BUFFER_SIZE];

static LoggerMessage getEmptyLoggerMessage(const enum LoggerMessageType t) {
   LoggerMessage msg;
   msg.type = t;
   return msg;
}

static LoggerMessage getLogStartMessage() {
   return getEmptyLoggerMessage(LoggerMessageType_Start);
}

static LoggerMessage getLogStopMessage() {
   return getEmptyLoggerMessage(LoggerMessageType_Stop);
}

/**
 * Called into by FreeRTOS during the ISR that handles the tick timer.
 */
void vApplicationTickHook(void){
   xSemaphoreGiveFromISR(onTick, pdFALSE);
}

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

void startLoggerTaskEx(int priority){
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "logger",	LOGGER_STACK_SIZE, NULL, priority, NULL );
}

static size_t initSampleRecords(LoggerConfig *loggerConfig){
	size_t channelSampleCount = get_enabled_channel_count(loggerConfig);

	for (size_t i=0; i < LOGGER_MESSAGE_BUFFER_SIZE; i++){
		LoggerMessage *msg = (g_sampleRecordMsgBuffer + i);
		msg->type = LoggerMessageType_Sample;
		if (msg->channelSamples != NULL){
			vPortFree(msg->channelSamples);
		}
		ChannelSample *channelSamples = create_channel_sample_buffer(loggerConfig, channelSampleCount);
		init_channel_sample_buffer(loggerConfig, channelSamples, channelSampleCount);
		msg->channelSamples = channelSamples;
	}
	return channelSampleCount;
}

static int calcTelemetrySampleRate(LoggerConfig *config, int desiredSampleRate){
	int maxRate = getConnectivitySampleRateLimit();
	return isHigherSampleRate(desiredSampleRate, maxRate) ? maxRate : desiredSampleRate;
}

size_t updateSampleRates(LoggerConfig *loggerConfig, int *loggingSampleRate,
                         int *telemetrySampleRate, int *sampleRateTimebase) {
	*loggingSampleRate = getHighestSampleRate(loggerConfig);
	*sampleRateTimebase = *loggingSampleRate;
   *sampleRateTimebase = getHigherSampleRate(BACKGROUND_SAMPLE_RATE,
                                             *sampleRateTimebase);

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

void loggerTaskEx(void *params) {
LoggerMessage logStartMsg = getLogStartMessage();
LoggerMessage logStopMsg = getLogStopMessage();

g_loggingShouldRun = 0;
memset(&g_sampleRecordMsgBuffer, 0, sizeof(g_sampleRecordMsgBuffer));
vSemaphoreCreateBinary(onTick);

LoggerConfig *loggerConfig = getWorkingLoggerConfig();

g_loggingShouldRun = 0;
g_isLogging = 0;
size_t bufferIndex = 0;
size_t currentTicks = 0;
g_configChanged = 1;
size_t channelCount = 0;

int loggingSampleRate = SAMPLE_DISABLED;
int sampleRateTimebase = SAMPLE_DISABLED;
int telemetrySampleRate = SAMPLE_DISABLED;

while (1) {
    xSemaphoreTake(onTick, portMAX_DELAY);
    watchdog_reset();
    ++currentTicks;

    // XXX STIEG: Should this be higher for MK2?  Seems like it should be.
    if (currentTicks % BACKGROUND_SAMPLE_RATE == 0) doBackgroundSampling();

    if (g_configChanged) {
        currentTicks = 0;
        channelCount = updateSampleRates(loggerConfig, &loggingSampleRate,
                &telemetrySampleRate, &sampleRateTimebase);
        resetLapCount();
        resetGpsDistance();
        g_configChanged = 0;
    }

    if (g_loggingShouldRun && !g_isLogging) {
        pr_info("Logging started\r\n");
        g_isLogging = 1;
        LED_disable(3);

        queue_logfile_record(&logStartMsg);
        queueTelemetryRecord(&logStartMsg);
    }

    if (!g_loggingShouldRun && g_isLogging) {
        pr_info("Logging stopped\r\n");
        g_isLogging = 0;
        LED_disable(2);

        queue_logfile_record(&logStopMsg);
        queueTelemetryRecord(&logStopMsg);
    }

    // Check if we need to actually populate the buffer.
    LoggerMessage *msg = &g_sampleRecordMsgBuffer[bufferIndex];
    if (!g_isLogging) continue;

    int sampledRate = populate_sample_buffer(msg, channelCount, currentTicks);
    msg->sampleCount = channelCount;

    // If here, not time for a sample.  Move along.
    if (sampledRate == SAMPLE_DISABLED)
        continue;

    if (sampledRate >= loggingSampleRate && queue_logfile_record(msg) != pdTRUE)
        LED_enable(3);

    if ((sampledRate >= telemetrySampleRate
            || currentTicks % telemetrySampleRate == 0)
            && loggerConfig->ConnectivityConfigs.telemetryConfig.backgroundStreaming) {
        queueTelemetryRecord(msg);
    }

    ++bufferIndex;
    bufferIndex %= LOGGER_MESSAGE_BUFFER_SIZE;
}
}
