/*
 * loggerSampleData.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

#include <stddef.h>

#include "loggerConfig.h"
#include "sampleRecord.h"

/**
 * Checks that the interval time in the LoggerMessage struct matches that within the ChannelSample
 * buffer.
 * @param lm The LoggerMessage to validate
 * @return true if its valid, false otherwise.
 */
int checkSampleTimestamp(LoggerMessage *lm);

int populate_sample_buffer(LoggerMessage *lm,  size_t count, size_t currentTicks);
void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount);

float get_mapped_value(float value, ScalingMap *scalingMap);

#endif /* LOGGERSAMPLEDATA_H_ */
