/*
 * loggerData.h
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
 */

#ifndef LOGGERDATA_H_
#define LOGGERDATA_H_
//#include "FreeRTOS.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include <stddef.h>

void init_logger_data();

void doBackgroundSampling();

float getAccelerometerValue(unsigned char accelChannel, AccelConfig *ac);
void calibrateAccelZero();

int populateSampleRecord(SampleRecord *sr, size_t currentTicks, LoggerConfig * config);
float GetMappedValue(float value, ScalingMap *scalingMap);

#endif /* LOGGERDATA_H_ */
