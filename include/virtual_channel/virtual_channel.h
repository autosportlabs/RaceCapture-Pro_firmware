/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIRTUAL_CHANNEL_H_
#define VIRTUAL_CHANNEL_H_

#include "channel_config.h"
#include "cpp_guard.h"
#include "loggerConfig.h"
#include "loggerNotifications.h"

#include <stddef.h>

CPP_GUARD_BEGIN

typedef struct _VirtualChannel {
    ChannelConfig config;
    float currentValue;
} VirtualChannel;

#define INVALID_VIRTUAL_CHANNEL -1

int find_virtual_channel(const char * channel_name);
int create_virtual_channel(const ChannelConfig chCfg);
VirtualChannel * get_virtual_channel(size_t id);
size_t get_virtual_channel_count(void);
void set_virtual_channel_value(size_t id, float value);
float get_virtual_channel_value(int id);
void reset_virtual_channels(void);

/**
 * @return The highest sample rate among all the virtual channels
 */
int get_virtual_channel_high_sample_rate(void);

CPP_GUARD_END

#endif /* VIRTUAL_CHANNEL_H_ */
