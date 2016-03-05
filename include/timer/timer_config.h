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

#ifndef _TIMER_CONFIG_H_
#define _TIMER_CONFIG_H_

#include "channel_config.h"
#include "cpp_guard.h"

CPP_GUARD_BEGIN

#define MODE_LOGGING_TIMER_RPM		        0
#define MODE_LOGGING_TIMER_FREQUENCY		1
#define MODE_LOGGING_TIMER_PERIOD_MS		2
#define MODE_LOGGING_TIMER_PERIOD_USEC		3


#define TIMER_SLOW	0
#define TIMER_MEDIUM	1
#define TIMER_FAST	2

#define DEFAULT_TIMER_FILTER_ALPHA (1.0f)
#define DEFAULT_TIMER_PPR 1
#define DEFAULT_TIMER_DIVIDER TIMER_MEDIUM
#define DEFAULT_TIMER_SCALING 375428

typedef struct _TimerConfig {
        ChannelConfig cfg;
        float filterAlpha;
        unsigned char mode;
        unsigned char pulsePerRevolution;
        unsigned short timerSpeed;
} TimerConfig;

TimerConfig* get_timer_config(int channel);
void set_default_timer_config(TimerConfig cfg[]);

CPP_GUARD_END

#endif /* _TIMER_CONFIG_H_ */
