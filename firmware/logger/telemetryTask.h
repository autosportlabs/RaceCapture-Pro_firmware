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

portBASE_TYPE queueTelemetryRecord(SampleRecord * sr);

void createTelemetryTask();

#endif /* TELEMETRYTASK_H_ */
