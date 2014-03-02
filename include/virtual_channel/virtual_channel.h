/*
 * virtual_channel.h
 *
 *  Created on: Mar 1, 2014
 *      Author: brent
 */

#ifndef VIRTUAL_CHANNEL_H_
#define VIRTUAL_CHANNEL_H_

#include <stddef.h>
#include "loggerConfig.h"

typedef struct _VirtualChannel{
	ChannelConfig config;
	float currentValue;
} VirtualChannel;

int create_virtual_channel(int channelNameId, unsigned char sampleRate);
void set_virtual_channel_value(size_t id, float value);
float get_virtual_channel_value(size_t id);

#endif /* VIRTUAL_CHANNEL_H_ */
