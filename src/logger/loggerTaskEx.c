/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeRTOS.h"
#include "led.h"
#include "capabilities.h"
#include "connectivityTask.h"
#include "fileWriter.h"
#include "gps.h"
#include "imu.h"
#include "lap_stats.h"
#include "logger.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "loggerHardware.h"
#include "loggerSampleData.h"
#include "loggerTaskEx.h"
#include "macros.h"
#include <string.h>
#include "panic.h"
#include "printk.h"
#include "sampleRecord.h"
#include "semphr.h"
#include "serial.h"
#include "task.h"
#include "taskUtil.h"
#include "watchdog.h"

#define LOGGER_STACK_SIZE	200
#define IDLE_TIMEOUT	configTICK_RATE_HZ / 1

#define BACKGROUND_SAMPLE_RATE	SAMPLE_50Hz

int g_loggingShouldRun;
int g_configChanged;
int g_telemetryBackgroundStreaming;

xSemaphoreHandle onTick;

/* This should be 0'd out accroding to C standards */
static struct sample g_sample_buffer[LOGGER_MESSAGE_BUFFER_SIZE];

static LoggerMessage getLogStartMessage()
{
        return create_logger_message(LoggerMessageType_Start, 0, NULL);
}

static LoggerMessage getLogStopMessage()
{
        return create_logger_message(LoggerMessageType_Stop, 0, NULL);
}

/**
 * Called into by FreeRTOS during the ISR that handles the tick timer.
 */
void vApplicationTickHook(void)
{
    if (onTick)
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
    led_disable(LED_LOGGER);
    pr_info("Logging started\r\n");
}

static void logging_stopped()
{
    logging_set_logging_start(0);
    led_disable(LED_LOGGER);
    pr_info("Logging stopped\r\n");
}

void startLoggerTaskEx(int priority)
{
        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "Logger Task    ";
        const bool status = xTaskCreate(loggerTaskEx, task_name,
                                        LOGGER_STACK_SIZE, NULL,
                                        priority, NULL );
        if (!status)
                panic(PANIC_CAUSE_TASK_CREATE);
}

static int init_sample_ring_buffer(LoggerConfig *loggerConfig)
{
        const size_t channel_count = get_enabled_channel_count(loggerConfig);
        struct sample *s = g_sample_buffer;
        const struct sample * const end = s + LOGGER_MESSAGE_BUFFER_SIZE;
        int i;

        for (i = 0; s < end; ++s, ++i) {
                const size_t bytes = init_sample_buffer(s, channel_count);
                if (0 == bytes) {
                        /* If here, then can't alloc memory for buffers */
                        pr_error("Failed to allocate memory for sample buffers\r\n");
                        break;
                }
        }

        pr_debug_int_msg("Sample buffers allocated: ", i);
        return i;
}

static int calcTelemetrySampleRate(LoggerConfig *config, int desiredSampleRate)
{
    int maxRate = getConnectivitySampleRateLimit();
    return isHigherSampleRate(desiredSampleRate, maxRate) ? maxRate : desiredSampleRate;
}

void updateSampleRates(LoggerConfig *loggerConfig, int *loggingSampleRate,
                       int *telemetrySampleRate, int *timebaseSampleRate)
{
    *loggingSampleRate = getHighestSampleRate(loggerConfig);
    *timebaseSampleRate = *loggingSampleRate;
    *timebaseSampleRate = getHigherSampleRate(BACKGROUND_SAMPLE_RATE, *timebaseSampleRate);
    *telemetrySampleRate = calcTelemetrySampleRate(loggerConfig, *loggingSampleRate);

    pr_info("timebase/logging/telemetry sample rate: ");
    pr_info_int(decodeSampleRate(*timebaseSampleRate));
    pr_info("/");
    pr_info_int(decodeSampleRate(*loggingSampleRate));
    pr_info("/");
    pr_info_int(decodeSampleRate(*telemetrySampleRate));
    pr_info("\r\n");
}

void loggerTaskEx(void *params)
{
        LoggerConfig *loggerConfig = getWorkingLoggerConfig();
        size_t bufferIndex = 0;
        size_t currentTicks = 0;
        int buffer_size = 0;
        int loggingSampleRate = SAMPLE_DISABLED;
        int sampleRateTimebase = SAMPLE_DISABLED;
        int telemetrySampleRate = SAMPLE_DISABLED;

        g_loggingShouldRun = 0;
        vSemaphoreCreateBinary(onTick);
        logging_set_status(LOGGING_STATUS_IDLE);
        logging_set_logging_start(0);
        g_configChanged = 1;

        while (1) {
                xSemaphoreTake(onTick, portMAX_DELAY);
                ++currentTicks;

                if (g_configChanged) {
                        buffer_size = init_sample_ring_buffer(loggerConfig);
                        if (!buffer_size) {
                                pr_error("Failed to allocate any buffers!\r\n");
                                led_enable(LED_ERROR);

                                /*
                                 * Do this to ensure the log message gets out
                                 * and we give system time to recover.
                                 */
                                delayMs(10);
                                continue;
                        }

                        led_disable(LED_ERROR);

                        updateSampleRates(loggerConfig, &loggingSampleRate,
                                          &telemetrySampleRate,
                                          &sampleRateTimebase);
                        resetLapCount();
                        lapstats_reset_distance();
                        currentTicks = 0;
                        g_configChanged = 0;
                }

                /* Only reset the watchdog when we are configured and ready to rock */
                watchdog_reset();

                if (currentTicks % BACKGROUND_SAMPLE_RATE == 0)
                        doBackgroundSampling();

                const bool is_logging = logging_is_active();
                if (g_loggingShouldRun && !is_logging) {
                        logging_started();
                        const LoggerMessage logStartMsg = getLogStartMessage();
#if SDCARD_SUPPORT
                        queue_logfile_record(&logStartMsg);
#endif
                        queueTelemetryRecord(&logStartMsg);
                }

                if (!g_loggingShouldRun && is_logging) {
                        logging_stopped();
                        const LoggerMessage logStopMsg = getLogStopMessage();
#if SDCARD_SUPPORT
                        queue_logfile_record(&logStopMsg);
#endif
                        queueTelemetryRecord(&logStopMsg);
                        logging_set_status(LOGGING_STATUS_IDLE);
                }

                /* Prepare a Sample */
                struct sample *sample = &g_sample_buffer[bufferIndex];

                /* Check if we need to actually populate the buffer. */
                const int sampledRate = populate_sample_buffer(sample,
                                                               currentTicks);
                if (sampledRate == SAMPLE_DISABLED)
                        continue;

                /* If here, create the LoggerMessage to send with the sample */
                const LoggerMessage msg = create_logger_message(
                        LoggerMessageType_Sample, currentTicks, sample);

                /*
                 * We only log to file if the user has manually pushed the
                 * logging button.
                 */
#if SDCARD_SUPPORT
                if (is_logging && should_sample(currentTicks, loggingSampleRate)) {
                        /* XXX Move this to file writer? */
                        const portBASE_TYPE res = queue_logfile_record(&msg);
                        const logging_status_t ls = pdTRUE == res ?
                                LOGGING_STATUS_WRITING :
                                LOGGING_STATUS_ERROR_WRITING;
                        logging_set_status(ls);
                }
#endif

                /*
                 * The task is responsible for determining if it should use the
                 * sample or if it should drop it due to rate limitations.
                 */
                queueTelemetryRecord(&msg);


                /* Process callback handlers for the samples */
                logger_sample_process_callbacks(currentTicks, sample);

                ++bufferIndex;
                bufferIndex %= buffer_size;
        }

        panic(PANIC_CAUSE_UNREACHABLE);
}
