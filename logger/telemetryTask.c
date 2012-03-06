/*
 * telemetryTask.c
 *
 *  Created on: Mar 6, 2012
 *      Author: brent
 */
#include "telemetryTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "modp_numtoa.h"
#include "loggerHardware.h"
#include "usb_comm.h"
#include "usart.h"
#include "string.h"

static xQueueHandle g_sampleRecordQueue = NULL;

#define TELEMETRY_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define TELEMETRY_STACK_SIZE  					200
#define SAMPLE_RECORD_QUEUE_SIZE				10

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
//#define TELEMETRY_QUEUE_WAIT_TIME					0
#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY

static int g_telemetryActive;


static int putsXbee(const char *s){
	int c = usart0_puts(s);
	return c;
}

static int putcXbee(char c){
	usart0_putchar(c);
	return 0;
}

static void putIntXbee(int num){
	char buf[10];
	modp_itoa10(num,buf);
	putsXbee(buf);
}

static void putFloatXbee(float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	putsXbee(buf);
}

static void flushXbee(void){
	usart0_flush();
}

portBASE_TYPE queueTelemetryRecord(SampleRecord * sr){
	if (NULL != g_sampleRecordQueue){
		return xQueueSend(g_sampleRecordQueue, &sr, TELEMETRY_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

static void writeSampleRecord(SampleRecord * sampleRecord){

	if (NULL == sampleRecord) return;

	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample *sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (i > 0) putsXbee(",");

		if (sample->intValue == NIL_SAMPLE) continue;

		int precision = sample->channelConfig->precision;
		if (precision > 0){
			putFloatXbee(sample->floatValue,precision);
		}
		else{
			putIntXbee(sample->intValue);
		}
	}
	putsXbee("\r\n");
}


void createTelemetryTask(){

	initUsart0(USART_MODE_8N1, 115200);

	g_telemetryActive = 0;
	g_sampleRecordQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_sampleRecordQueue){
		//TODO log error
		return;
	}
	xTaskCreate( telemetryTask,( signed portCHAR * ) "telemetry", TELEMETRY_STACK_SIZE, NULL, TELEMETRY_TASK_PRIORITY, NULL );
}

void telemetryTask(void *params){

	SampleRecord *sr = NULL;
	while(1){
		//wait for the next sample record
		xQueueReceive(g_sampleRecordQueue, &(sr), portMAX_DELAY);

		if (NULL != sr && 0 == g_telemetryActive){
			g_telemetryActive = 1;
			//writeHeaders(&g_logfile,sr);
		}

		if (g_telemetryActive){
			//a null sample record means end of sample run; like an EOF
			if (NULL != sr){
				writeSampleRecord(sr);
			}
			else{
				g_telemetryActive = 0;
			}
		}
	}
}




