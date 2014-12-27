/*
 * sampleRecord.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#include "sampleRecord.h"
#include "loggerConfig.h"
#include "mem_mang.h"
#include "FreeRTOS.h"
#include "taskUtil.h"

ChannelSample * create_channel_sample_buffer(LoggerConfig *loggerConfig, size_t channelCount){
	size_t size = sizeof(ChannelSample[channelCount]);
	ChannelSample * samples = (ChannelSample *)portMalloc(size);
	return samples;
}

int isValidLoggerMessageAge(LoggerMessage *lm) {
    return (getCurrentTicks() - lm->ticks) < 10;
}
