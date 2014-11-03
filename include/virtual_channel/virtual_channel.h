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
#include "loggerNotifications.h"

typedef struct _VirtualChannel{
	ChannelConfig config;
	float currentValue;
} VirtualChannel;

#define MAX_VIRTUAL_CHANNELS 10

int create_virtual_channel(const ChannelConfig chCfg);
VirtualChannel * get_virtual_channel(size_t id);
size_t get_virtual_channel_count();
void set_virtual_channel_value(size_t id, float value);
float get_virtual_channel_value(int id);

#endif /* VIRTUAL_CHANNEL_H_ */
