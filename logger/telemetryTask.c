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

#define DEFAULT_DESTINATION_ADDRESS				0x000000000000FFFF
#define DEFAULT_BROADCAST_RADIUS				0x00
#define	DEFAULT_TRANSMIT_OPTIONS				0x00
#define FRAME_PREAMBLE_LENGTH					14
#define START_DELIMETER							0x7E
#define TRANSMIT_FRAME_TYPE						0x10
#define RESERVED1								0xFF
#define RESERVED2								0XFE

#define SWIZZLE_BIGENDIAN(VAL, BYTE, BITLEN) ((VAL >> ((((BITLEN / 8) - 1) - BYTE) << 3)) & 0xFF)

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define TELEMETRY_QUEUE_WAIT_TIME					0
//#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY

static int g_telemetryActive;

static TxFrame g_xBeeFrame;

static void putQuotedStringXbee(char *s){
	usart0_putchar('"');
	usart0_puts(s);
	usart0_putchar('"');
}

static void putsXbee(const char *s){
	usart0_puts(s);
}

static void putcXbee(char c){
	usart0_putchar(c);
}

static void putEscXbee(char c){
	switch(c){
		case 0x7e:
		case 0x7d:
		case 0x11:
		case 0x13:
			usart0_putchar(0x7d);
			c^= 0x20;
			break;
		default:
			break;
	}
	putcXbee(c);
}

static void putUintXbee(uint32_t num){
	char buf[10];
	modp_uitoa10(num,buf);
	putsXbee(buf);
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

static void initTxFrame(TxFrame *frame){
	memset(frame,0,sizeof(TxFrame));

	frame->startDelimeter = START_DELIMETER;
	frame->frameType = TRANSMIT_FRAME_TYPE;

	uint64_t address = DEFAULT_DESTINATION_ADDRESS;
	for (int i = 0; i < ADDRESS_LENGTH;i++){
		frame->address[i] = SWIZZLE_BIGENDIAN(address,i,64);
	}
	frame->reserved1 = RESERVED1;
	frame->reserved2 = RESERVED2;

	frame->broadcastRadius = DEFAULT_BROADCAST_RADIUS;
	frame->transmitOptions = DEFAULT_TRANSMIT_OPTIONS;
}



static void finalizeTxFrame(TxFrame *frame,int fieldCount){

	uint16_t length = FRAME_PREAMBLE_LENGTH + SAMPLE_TICK_LENGTH + (fieldCount * (FIELD_NAME_LENGTH + FIELD_VALUE_LENGTH));
	frame->lengthHigh = length >> 8;
	frame->lengthLow = length & 0xFF;

	//calculate checksum per Digi's checksum requirements
	uint32_t cs = 0;
	cs+=frame->frameType;
	cs+=frame->frameId;
	for (int i = 0; i < ADDRESS_LENGTH; i++) cs+=frame->address[i];
	cs+=frame->reserved1;
	cs+=frame->reserved2;
	cs+=frame->broadcastRadius;
	cs+=frame->transmitOptions;
	for (int i = 0; i < SAMPLE_TICK_LENGTH;i++) cs+=SWIZZLE_BIGENDIAN(frame->sampleTick,i,32);
	for (int i = 0; i < fieldCount; i++){
		for (int ii = 0; ii < FIELD_NAME_LENGTH; ii++) cs+=frame->field[i].name[ii];
		for (int ii = 0; ii < FIELD_VALUE_LENGTH; ii++) cs+=frame->field[i].raw[ii];
	}

	frame->checksum = 0xff - (cs & 0xFF);
}

static void sendTxFrameBinary(TxFrame *txFrame,int fieldCount){

	putcXbee(txFrame->startDelimeter);
	putEscXbee(txFrame->lengthHigh);
	putEscXbee(txFrame->lengthLow);
	putEscXbee(txFrame->frameType);
	putEscXbee(txFrame->frameId);
	for (int i = 0; i < ADDRESS_LENGTH; i++){
		putEscXbee(txFrame->address[i]);
	}
	putEscXbee(txFrame->reserved1);
	putEscXbee(txFrame->reserved2);
	putEscXbee(txFrame->broadcastRadius);
	putEscXbee(txFrame->transmitOptions);
	for (int i = 0; i < SAMPLE_TICK_LENGTH;i++){
		putEscXbee(SWIZZLE_BIGENDIAN(txFrame->sampleTick,i,32));
	}
	for (int i = 0; i < fieldCount; i++){
		for (int ii = 0; ii < FIELD_NAME_LENGTH; ii++) putEscXbee(txFrame->field[i].name[ii]);
		for (int ii = 0; ii < FIELD_VALUE_LENGTH; ii++) putEscXbee(txFrame->field[i].raw[ii]);
	}
	putcXbee(txFrame->checksum);
}

static void writeSampleRecordJSON(SampleRecord * sampleRecord, uint32_t sampleTick){

	putcXbee('{');
	int fieldCount = 0;
	putsXbee("\"tick\":");
	putUintXbee(sampleTick);
	putcXbee(',');
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (sample->intValue == NIL_SAMPLE) continue;

		if (fieldCount++ > 0) putcXbee(',');

		putQuotedStringXbee(channelConfig->label);
		putcXbee(':');
		int precision = sample->channelConfig->precision;
		if (precision > 0){
			putFloatXbee(sample->floatValue,precision);
		}
		else{
			putIntXbee(sample->intValue);
		}
	}
	putsXbee("}\r\n");
}

static void writeSampleRecordBinary(SampleRecord * sampleRecord, uint32_t sampleTick){

	TxFrame * txFrame = &g_xBeeFrame;

	//the init *could* be written once at start of telemetry session,
	//so doing it every time is a bit inefficient
	initTxFrame(txFrame);

	txFrame->sampleTick = sampleTick;

	//go until we reach the end of enabled sample records, or until the max transmittable
	//field count is reached
	int fieldIndex = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS && fieldIndex < MAX_FIELD_COUNT; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (sample->intValue == NIL_SAMPLE) continue;

		strncpy(txFrame->field[fieldIndex].name, sample->channelConfig->label,FIELD_NAME_LENGTH - 1);

		int precision = sample->channelConfig->precision;
		if (precision > 0){
			txFrame->field[fieldIndex].floatValue = sample->floatValue;
		}
		else{
			txFrame->field[fieldIndex].intValue = sample->intValue;
		}
		fieldIndex++;
	}
	finalizeTxFrame(txFrame,fieldIndex);
	sendTxFrameBinary(txFrame,fieldIndex);
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
	uint32_t sampleTick = 0;
	while(1){
		//wait for the next sample record
		xQueueReceive(g_sampleRecordQueue, &(sr), portMAX_DELAY);

		//ToggleLED(LED3);
		if (NULL != sr && 0 == g_telemetryActive){
			g_telemetryActive = 1;
			sampleTick = 0;
		}

		if (g_telemetryActive){
			//a null sample record means end of sample run; like an EOF
			if (NULL != sr){
				sampleTick++;
				//need to make this selectable - configurable
				writeSampleRecordBinary(sr,sampleTick);
				//writeSampleRecordJSON(sr,sampleTick);
			}
			else{
				sampleTick = 0;
				g_telemetryActive = 0;
			}
		}
	}
}




