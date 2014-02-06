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

void doBackgroundSampling();

float GetMappedValue(float value, ScalingMap *scalingMap);

#endif /* LOGGERDATA_H_ */
