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

#include <stdint.h>
#include <stdbool.h>

CPP_GUARD_BEGIN

typedef struct _ConnParams {
        bool always_streaming;
        uint8_t isPrimary;
        char * connectionName;
        int (*disconnect)(DeviceConfig *config);
        int (*init_connection)(DeviceConfig *config);
        int (*check_connection_status)(DeviceConfig *config);
        serial_id_t serial;
        size_t periodicMeta;
        uint32_t connection_timeout;
        xQueueHandle sampleQueue;
        int max_sample_rate;
} ConnParams;

void queueTelemetryRecord(const LoggerMessage *msg);

void startConnectivityTask(int16_t priority);

void connectivityTask(void *params);

CPP_GUARD_END

#endif /* CONNECTIVITY_TASK_H_ */
