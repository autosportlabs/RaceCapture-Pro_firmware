#ifndef CONNECTIVITY_TASK_H_
#define CONNECTIVITY_TASK_H_

#include <stdint.h>
#include "sampleRecord.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "devices_common.h"
#include "serial.h"

typedef struct _ConnParams{
	uint8_t isPrimary;
	char * connectionName;
	int (*init_connection)(DeviceConfig *config);
	int (*check_connection_status)(DeviceConfig *config);
	serial_id_t serial;
	size_t periodicMeta;
	xQueueHandle sampleQueue;
} ConnParams;

void queueTelemetryRecord(LoggerMessage * sr);

void startConnectivityTask(int16_t priority);

void connectivityTask(void *params);

#endif /* CONNECTIVITY_TASK_H_ */
