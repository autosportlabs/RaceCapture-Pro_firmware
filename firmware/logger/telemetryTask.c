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
#include "string.h"
#include "p2pTelemetry.h"
#include "cellTelemetry.h"
#include "btTelemetry.h"

static xQueueHandle g_sampleRecordQueue = NULL;

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define TELEMETRY_QUEUE_WAIT_TIME					0
//#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY

#define TELEMETRY_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define TELEMETRY_STACK_SIZE  					200
#define SAMPLE_RECORD_QUEUE_SIZE				10


portBASE_TYPE queueTelemetryRecord(SampleRecord * sr){
	if (NULL != g_sampleRecordQueue){
		return xQueueSend(g_sampleRecordQueue, &sr, TELEMETRY_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

void createTelemetryTask(){

	initUsart0(USART_MODE_8N1, 115200);

	g_sampleRecordQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_sampleRecordQueue){
		//TODO log error
		return;
	}

	switch(getWorkingLoggerConfig()->LoggerOutputConfig.telemetryMode){
		case TELEMETRY_MODE_DISABLED:
			break;
		case TELEMETRY_MODE_CELL:
			xTaskCreate( cellTelemetryTask, ( signed portCHAR * ) "telemetry", TELEMETRY_STACK_SIZE, g_sampleRecordQueue, TELEMETRY_TASK_PRIORITY, NULL );
			break;
		case TELEMETRY_MODE_P2P:
			xTaskCreate( p2pTelemetryTask, ( signed portCHAR * ) "telemetry", TELEMETRY_STACK_SIZE, g_sampleRecordQueue, TELEMETRY_TASK_PRIORITY, NULL );
			break;
		case TELEMETRY_MODE_BLUETOOTH:
			xTaskCreate( btTelemetryTask, ( signed portCHAR * ) "telemetry", TELEMETRY_STACK_SIZE, g_sampleRecordQueue, TELEMETRY_TASK_PRIORITY, NULL );
			break;
	}
}




