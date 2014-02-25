/*
 * telemetryTask.c
 *
 *  Created on: Mar 6, 2012
 *      Author: brent
 */
#include "telemetryTask.h"
#include "task.h"
#include "semphr.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "modp_numtoa.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "usart.h"
#include "cellTelemetry.h"
#include "btTelemetry.h"
#include "consoleConnectivity.h"
#include "connectivityTask.h"

//devices
#include "null_device.h"
#include "bluetooth.h"
#include "sim900.h"

static ConnParams g_connParams;

#define BUFFER_SIZE 	201
static char g_buffer[BUFFER_SIZE];
size_t g_rxCount;


#define TELEMETRY_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define TELEMETRY_STACK_SIZE  					1000
#define SAMPLE_RECORD_QUEUE_SIZE				10


portBASE_TYPE queueTelemetryRecord(SampleRecord * sr){
	xQueueHandle sampleQueue = g_connParams.sampleQueue;
	if (NULL != sampleQueue){
		return xQueueSend(sampleQueue, &sr, TELEMETRY_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

void startConnectivityTask(){
	g_connParams.sampleQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_connParams.sampleQueue){
		//TODO log error
		return;
	}

	switch(getWorkingLoggerConfig()->ConnectivityConfigs.connectivityMode){
		case CONNECTIVITY_MODE_CONSOLE:
			g_connParams.check_connection_status = &null_device_check_connection_status;
			g_connParams.init_connection = &null_device_init_connection;
			break;
		case CONNECTIVITY_MODE_BLUETOOTH:
			g_connParams.check_connection_status = &bt_check_connection_status;
			g_connParams.init_connection = &bt_init_connection;
			break;
		case CONNECTIVITY_MODE_CELL:
			g_connParams.check_connection_status = &sim900_check_connection_status;
			g_connParams.init_connection = &sim900_init_connection;
			break;
	}

	xTaskCreate(connectivityTask, (signed portCHAR *) "connTask", TELEMETRY_STACK_SIZE, &g_connParams, TELEMETRY_TASK_PRIORITY, NULL );
}
