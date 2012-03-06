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

portBASE_TYPE queueTelemetryRecord(SampleRecord * sr);

void createTelemetryTask();

void telemetryTask(void *params);

#endif /* TELEMETRYTASK_H_ */
