/*
 * telemetryTask.h
 *
 *  Created on: Mar 6, 2012
 *      Author: brent
 */

#ifndef TELEMETRYTASK_H_
#define TELEMETRYTASK_H_
#include "FreeRTOS.h"
#include "sampleRecord.h"
#include "stdint.h"

portBASE_TYPE queueTelemetryRecord(SampleRecord * sr);

#define MAX_FIELD_COUNT 12
#define MAX_DATA_LENGTH 96
#define FIELD_NAME_LENGTH 4
#define SAMPLE_TICK_LENGTH sizeof(uint32_t)
#define ADDRESS_LENGTH sizeof(uint64_t)
#define FIELD_VALUE_LENGTH sizeof(int32_t)

typedef struct _TxField{
	char name[FIELD_NAME_LENGTH];
	union{
		uint8_t raw[FIELD_VALUE_LENGTH];
		float floatValue;
		int32_t intValue;
	};
}TxField;

typedef struct _TxFrame{
	uint8_t startDelimeter;
	uint8_t lengthHigh;
	uint8_t	lengthLow;
	uint8_t	frameType;
	uint8_t	frameId;
	uint8_t address[ADDRESS_LENGTH];
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t broadcastRadius;
	uint8_t transmitOptions;
	uint32_t sampleTick;
	union{
		char data[MAX_DATA_LENGTH];
		TxField field[MAX_FIELD_COUNT];
	};
	uint8_t checksum;
} TxFrame;

void createTelemetryTask();

void telemetryTask(void *params);

#endif /* TELEMETRYTASK_H_ */
