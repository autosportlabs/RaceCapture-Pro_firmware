/*
 * loggerData.h
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
 */

#ifndef LOGGERDATA_H_
#define LOGGERDATA_H_
#include "FreeRTOS.h"
#include "loggerConfig.h"
#include "sampleRecord.h"


void populateSampleRecord(SampleRecord *sr, portTickType currentTicks, LoggerConfig * config);

float GetMappedValue(float value, ScalingMap *scalingMap);


#endif /* LOGGERDATA_H_ */
