/*
 * loggerSampleData.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

#include "sampleRecord.h"
#include "loggerConfig.h"

int populate_sample_buffer(ChannelSample * samples,  size_t count, size_t currentTicks);
void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount);

float get_mapped_value(float value, ScalingMap *scalingMap);

#endif /* LOGGERSAMPLEDATA_H_ */
