#ifndef CONNECTIVITY_TASK_H_
#define CONNECTIVITY_TASK_H_

#include "sampleRecord.h"
#include "stddef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "devices_common.h"

typedef struct _ConnParams{
	int (*init_connection)(DeviceConfig *config);
	int (*check_connection_status)(DeviceConfig *config);
} ConnParams;

portBASE_TYPE queueTelemetryRecord(LoggerMessage * sr);

void startConnectivityTask();

void connectivityTask(void *params);

#endif /* CONNECTIVITY_TASK_H_ */
