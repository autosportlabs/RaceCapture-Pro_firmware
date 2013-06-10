/*
 * p2pTelemetry.cpp
 *
 *  Created on: Sep 5, 2012
 *      Author: brent
 */
#include "p2pTelemetry.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "usart.h"

#define DEFAULT_DESTINATION_ADDRESS				0x000000000000FFFF
#define DEFAULT_BROADCAST_RADIUS				0x00
#define	DEFAULT_TRANSMIT_OPTIONS				0x00
#define FRAME_PREAMBLE_LENGTH					14
#define START_DELIMETER							0x7E
#define TRANSMIT_FRAME_TYPE						0x10
#define RESERVED1								0xFF
#define RESERVED2								0XFE

#define IDLE_TIMEOUT							configTICK_RATE_HZ / 1

#define SWIZZLE_BIGENDIAN(VAL, BYTE, BITLEN) ((VAL >> ((((BITLEN / 8) - 1) - BYTE) << 3)) & 0xFF)

static int g_telemetryActive;

static TxFrame g_xBeeFrame;

static void putcXbee(char c){
	usart0_putchar(c);
}

static void putEscXbee(char c){
	switch(c){
		case 0x7e:
		case 0x7d:
		case 0x11:
		case 0x13:
			putcXbee(0x7d);
			c^= 0x20;
			break;
		default:
			break;
	}
	putcXbee(c);
}

static void initTxFrame(TxFrame *frame){
	memset(frame,0,sizeof(TxFrame));

	frame->startDelimeter = START_DELIMETER;
	frame->frameType = TRANSMIT_FRAME_TYPE;

	ConnectivityConfig *config = &(getWorkingLoggerConfig()->ConnectivityConfigs);

	for (int i = 0; i < ADDRESS_LENGTH / 2;i++){
		frame->address[i] = SWIZZLE_BIGENDIAN(config->p2pConfig.p2pDestinationAddrHigh,i,32);
	}

	for (int i = 0; i < ADDRESS_LENGTH / 2;i++){
		frame->address[i+ (ADDRESS_LENGTH / 2)] = SWIZZLE_BIGENDIAN(config->p2pConfig.p2pDestinationAddrLow,i,32);
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

static void writeSampleRecordBinary(SampleRecord * sampleRecord, uint32_t sampleTick){

	TxFrame * txFrame = &g_xBeeFrame;

	txFrame->sampleTick = sampleTick;

	int fieldIndex = 0;
	if (NULL != sampleRecord){
		//go until we reach the end of enabled sample records, or until the max transmittable
		//field count is reached
		for (int i = 0; i < SAMPLE_RECORD_CHANNELS && fieldIndex < MAX_FIELD_COUNT; i++){
			ChannelSample * sample = &(sampleRecord->Samples[i]);
			ChannelConfig * channelConfig = sample->channelConfig;

			if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

			if (sample->intValue == NIL_SAMPLE) continue;

			strncpy(txFrame->field[fieldIndex].name, sample->channelConfig->label,FIELD_NAME_LENGTH - 1);

			int precision = sample->precision;
			if (precision > 0){
				txFrame->field[fieldIndex].floatValue = sample->floatValue;
			}
			else{
				txFrame->field[fieldIndex].intValue = sample->intValue;
			}
			fieldIndex++;
		}
	}
	finalizeTxFrame(txFrame,fieldIndex);
	sendTxFrameBinary(txFrame,fieldIndex);
}


void p2pTelemetryTask(void *params){

	initUsart0(USART_MODE_8N1, 115200);
	xQueueHandle sampleRecordQueue = (xQueueHandle)params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;
	g_telemetryActive = 0;
	while(1){
		//wait for the next sample record
		char res = xQueueReceive(sampleRecordQueue, &(sr), IDLE_TIMEOUT);
		sampleTick++;
		if (pdFALSE == res){
			initTxFrame(&g_xBeeFrame);
			writeSampleRecordBinary(NULL,sampleTick);
		}
		else{
			if (NULL != sr && 0 == g_telemetryActive){
				g_telemetryActive = 1;
				initTxFrame(&g_xBeeFrame);
			}

			if (g_telemetryActive){
				//a null sample record means end of sample run; like an EOF
				if (NULL != sr){
					//need to make this selectable - configurable
					writeSampleRecordBinary(sr,sampleTick);
					//writeSampleRecordJSON(sr,sampleTick);
				}
				else{
					g_telemetryActive = 0;
				}
			}
		}
	}
}
