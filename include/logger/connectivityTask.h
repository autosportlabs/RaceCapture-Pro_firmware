#ifndef CONNECTIVITY_TASK_H_
#define CONNECTIVITY_TASK_H_

#include <stdint.h>
#include <stdbool.h>
#include "sampleRecord.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "devices_common.h"
#include "serial.h"

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
} ConnParams;

void queueTelemetryRecord(const LoggerMessage *msg);

void startConnectivityTask(int16_t priority);

void connectivityTask(void *params);

#endif /* CONNECTIVITY_TASK_H_ */
