/*
 * telemetryTask.h
 *
 *  Created on: Mar 6, 2012
 *      Author: brent
 */

#ifndef TELEMETRYTASK_H_
#define TELEMETRYTASK_H_
#include "sampleRecord.h"
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "queue.h"


portBASE_TYPE queueTelemetryRecord(SampleRecord * sr);

void createConnectivityTask();

#define CONN_INIT_SUCCESS 			0
#define CONN_INIT_FAIL				1

#define CONN_STATUS_NO_ERROR 		0
#define CONN_STATUS_DISCONNECTED	1

typedef struct _ConnParams{
	xQueueHandle sampleQueue;
	int (*init_connection)();
	int (*check_connection_status)(char *buffer, size_t size);
} ConnParams;


#endif /* TELEMETRYTASK_H_ */
