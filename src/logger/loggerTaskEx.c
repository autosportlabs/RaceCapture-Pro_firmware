/**
 * AutoSport Labs - Race Capture Pro Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture Pro firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "logger.h"
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
#include "lap_stats.h"
#include "printk.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define IDLE_TIMEOUT						configTICK_RATE_HZ / 1

#define BACKGROUND_SAMPLE_RATE				SAMPLE_50Hz

int g_loggingShouldRun;
int g_configChanged;
int g_telemetryBackgroundStreaming;

xSemaphoreHandle onTick;

#define LOGGER_MESSAGE_BUFFER_SIZE 10
static LoggerMessage g_sampleRecordMsgBuffer[LOGGER_MESSAGE_BUFFER_SIZE];

static LoggerMessage getTimeInsensativeLoggerMessage(const enum LoggerMessageType t)
{
    LoggerMessage msg;
    msg.type = t;
    msg.ticks = 0; // Time insensitive.
    return msg;
}

static LoggerMessage getLogStartMessage()
{
    return getTimeInsensativeLoggerMessage(LoggerMessageType_Start);
}

static LoggerMessage getLogStopMessage()
{
    return getTimeInsensativeLoggerMessage(LoggerMessageType_Stop);
}

/**
 * Called into by FreeRTOS during the ISR that handles the tick timer.
 */
void vApplicationTickHook(void)
{
    xSemaphoreGiveFromISR(onTick, pdFALSE);
}

void configChanged()
{
    g_configChanged = 1;
}

void startLogging()
{
    g_loggingShouldRun = 1;
}

void stopLogging()
{
    g_loggingShouldRun = 0;
}

static void logging_started()
{
    logging_set_logging_start(getUptimeAsInt());
    LED_disable(3);
    pr_info("Logging started\r\n");
}

static void logging_stopped()
{
    logging_set_logging_start(0);
    LED_disable(2);
    pr_info("Logging stopped\r\n");
}

void startLoggerTaskEx(int priority)
{
    xTaskCreate( loggerTaskEx,( signed portCHAR * ) "logger",	LOGGER_STACK_SIZE, NULL, priority, NULL );
}

static size_t initSampleRecords(LoggerConfig *loggerConfig)
{
    size_t channelSampleCount = get_enabled_channel_count(loggerConfig);

    for (size_t i=0; i < LOGGER_MESSAGE_BUFFER_SIZE; i++) {
        LoggerMessage *msg = (g_sampleRecordMsgBuffer + i);
        msg->type = LoggerMessageType_Sample;
        if (msg->channelSamples != NULL) {
            vPortFree(msg->channelSamples);
        }
        ChannelSample *channelSamples = create_channel_sample_buffer(loggerConfig, channelSampleCount);
        init_channel_sample_buffer(loggerConfig, channelSamples, channelSampleCount);
        msg->channelSamples = channelSamples;
    }
    return channelSampleCount;
}

static int calcTelemetrySampleRate(LoggerConfig *config, int desiredSampleRate)
{
    int maxRate = getConnectivitySampleRateLimit();
    return isHigherSampleRate(desiredSampleRate, maxRate) ? maxRate : desiredSampleRate;
}

size_t updateSampleRates(LoggerConfig *loggerConfig, int *loggingSampleRate,
                         int *telemetrySampleRate, int *timebaseSampleRate)
{
    *loggingSampleRate = getHighestSampleRate(loggerConfig);
    *timebaseSampleRate = *loggingSampleRate;
    *timebaseSampleRate = getHigherSampleRate(BACKGROUND_SAMPLE_RATE, *timebaseSampleRate);

    *telemetrySampleRate = calcTelemetrySampleRate(loggerConfig, *loggingSampleRate);
    size_t channelCount = initSampleRecords(loggerConfig);
    pr_info("timebase/logging/telemetry sample rate: ");
    pr_info_int(decodeSampleRate(*timebaseSampleRate));
    pr_info("/");
    pr_info_int(decodeSampleRate(*loggingSampleRate));
    pr_info("/");
    pr_info_int(decodeSampleRate(*telemetrySampleRate));
    pr_info("\r\n");
    return channelCount;
}

void loggerTaskEx(void *params)
{
    g_loggingShouldRun = 0;
    memset(&g_sampleRecordMsgBuffer, 0, sizeof(g_sampleRecordMsgBuffer));
    vSemaphoreCreateBinary(onTick);

    LoggerConfig *loggerConfig = getWorkingLoggerConfig();

    logging_set_status(LOGGING_STATUS_IDLE);
    logging_set_logging_start(0);
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

        if (currentTicks % BACKGROUND_SAMPLE_RATE == 0)
            doBackgroundSampling();

        if (g_configChanged) {
            currentTicks = 0;
            channelCount = updateSampleRates(loggerConfig, &loggingSampleRate, &telemetrySampleRate,
                                             &sampleRateTimebase);
            resetLapCount();
            lapstats_reset_distance();
            g_configChanged = 0;
        }

        {
            bool is_logging = logging_is_active();
            if (g_loggingShouldRun && !is_logging) {
                logging_started();
                LoggerMessage logStartMsg = getLogStartMessage();
                queue_logfile_record(&logStartMsg);
                queueTelemetryRecord(&logStartMsg);
            }

            if (!g_loggingShouldRun && is_logging) {
                logging_stopped();
                LoggerMessage logStopMsg = getLogStopMessage();
                queue_logfile_record(&logStopMsg);
                queueTelemetryRecord(&logStopMsg);
                logging_set_status(LOGGING_STATUS_IDLE);
            }
        }

        /* Check if we need to actually populate the buffer. */
        LoggerMessage *msg = &g_sampleRecordMsgBuffer[bufferIndex];
        int sampledRate = populate_sample_buffer(msg, channelCount, currentTicks);
        msg->sampleCount = channelCount;

        if (sampledRate == SAMPLE_DISABLED) {
            continue; /* If here, no sample to give. */
        }

        bool is_logging = logging_is_active();
        /* We only log to file if the user has manually pushed the logging button. */
        if (is_logging && sampledRate >= loggingSampleRate) {
            const portBASE_TYPE res = queue_logfile_record(msg);
            if (res == pdTRUE) {
                logging_set_status(LOGGING_STATUS_WRITING);
            } else {
                logging_set_status(LOGGING_STATUS_ERROR_WRITING);
                LED_enable(3); //error LED
            }
        }

        /* send the sample on to the telemetry task(s) */
        if (sampledRate >= telemetrySampleRate || currentTicks % telemetrySampleRate == 0) {
            queueTelemetryRecord(msg);
        }
        ++bufferIndex;
        bufferIndex %= LOGGER_MESSAGE_BUFFER_SIZE;
    }
}
