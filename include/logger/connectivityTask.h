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

#ifndef CONNECTIVITY_TASK_H_
#define CONNECTIVITY_TASK_H_

#include "FreeRTOS.h"
#include "cpp_guard.h"
#include "devices_common.h"
#include "queue.h"
#include "sampleRecord.h"
#include "serial.h"
#include "task.h"
#include "dateTime.h"
#include <stdint.h>
#include <stdbool.h>
#include "sdcard.h"

CPP_GUARD_BEGIN

#define BUFFER_SIZE        1025
#define BUFFER_BUFFER_SIZE 2000

typedef struct _ConnParams {
        bool always_streaming;
        char * connectionName;
        int (*disconnect)(DeviceConfig *config);
        int (*init_connection)(DeviceConfig *config, millis_t * connected_at, bool hard_init);
        int (*check_connection_status)(DeviceConfig *config);
        serial_id_t serial;
        size_t periodicMeta;
        uint32_t connection_timeout;
        xQueueHandle sampleQueue;
        int max_sample_rate;
        enum led activity_led;
} ConnParams;

typedef struct _TelemetryConnParams {
        bool always_streaming;
        char * connectionName;
        int (*disconnect)(DeviceConfig *config);
        int (*init_connection)(DeviceConfig *config, millis_t * connected_at, bool hard_init);
        int (*check_connection_status)(DeviceConfig *config);
        serial_id_t serial;
        uint32_t connection_timeout;
        xQueueHandle sampleQueue;
        int max_sample_rate;
        enum led activity_led;
} TelemetryConnParams;

typedef struct _BufferingTaskParams {
        bool always_streaming;
        char * connectionName;
        size_t periodicMeta;
        xQueueHandle sampleQueue;
        int max_sample_rate;
} BufferingTaskParams;

typedef struct _BufferedTelemetryMessage {
        enum LoggerMessageType type;
        size_t ticks;
        struct sample *sample;
} BufferedTelemetryMessage;

typedef struct _BufferedLoggerMessage {
        enum LoggerMessageType type;
        size_t ticks;
        struct sample *sample;
} BufferedLoggerMessage;

typedef struct _CellularState {
        xQueueHandle buffer_queue;
        FIL *buffer_file;
        char buffer_buffer[BUFFER_BUFFER_SIZE + 1];
        char cell_buffer[BUFFER_SIZE];
        int32_t read_index;
        bool file_open;
        bool should_reconnect;
        bool buffering_enabled;
} CellularState;

void queueTelemetryRecord(const LoggerMessage *msg);

void startConnectivityTask(int16_t priority);

void blueooth_connectivity_task(void *params);

void cellular_connectivity_task(void *params);

void cellular_buffering_task(void *params);

bool cellular_telemetry_buffering_enabled(void);

void cellular_telemetry_reconnect();

int process_rx_buffer(struct Serial *serial, char *buffer, size_t *rxCount);

CPP_GUARD_END

#endif /* CONNECTIVITY_TASK_H_ */
