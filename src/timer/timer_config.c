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

#include "api.h"
#include "loggerConfig.h"
#include "macros.h"
#include "timer_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RPM_CHAN_UNITS	"rpm"
#define RPM_CHAN_MIN	0
#define RPM_CHAN_MAX	8000

#define TIMER_DEFAULT_FILTER_ALPHA 1.0
#define TIMER_DEFAULT_PULSE_PER_REV 1
#define TIMER_DEFAULT_DIVIDER TIMER_MEDIUM
#define TIMER_DEFAULT_FILTER	TIMER_FILTER_VALUE_AUTO
#define TIMER_DEFAULT_EDGE	TIMER_EDGE_FALLING

#define TIMER_EDGE_RISING_STR	"rising"
#define TIMER_EDGE_FALLING_STR	"falling"

TimerConfig* get_timer_config(int channel)
{
        const size_t idx = (size_t) channel;
        if (idx >= CONFIG_TIMER_CHANNELS)
                return NULL;

        return getWorkingLoggerConfig()->TimerConfigs + idx;
}

void set_default_timer_config(TimerConfig tim_cfg[], const size_t cnt)
{
	for (unsigned int i = 0; i < cnt; ++i) {
		TimerConfig *tc = tim_cfg + i;

		ChannelConfig *tcc = &tc->cfg;
		set_default_channel_config(tcc);
		snprintf(tcc->label, ARRAY_LEN(tcc->label),
			 i ? "RPM%d" : "RPM", i + 1);
		strcpy(tcc->units, RPM_CHAN_UNITS);
		tcc->min = RPM_CHAN_MIN;
		tcc->max = RPM_CHAN_MAX;

		tc->filterAlpha = TIMER_DEFAULT_FILTER_ALPHA;
		tc->mode = MODE_LOGGING_TIMER_RPM;
		tc->pulsePerRevolution = TIMER_DEFAULT_PULSE_PER_REV;
		tc->timerSpeed = TIMER_MEDIUM;
		tc->filter_period_us = TIMER_DEFAULT_FILTER;
		tc->edge = TIMER_DEFAULT_EDGE;
	}
}

enum timer_edge get_timer_edge_enum(const char *val)
{
        if (!strcmp(val, TIMER_EDGE_RISING_STR)) {
                return TIMER_EDGE_RISING;
        } else if (!strcmp(val, TIMER_EDGE_FALLING_STR)) {
                return TIMER_EDGE_FALLING;
        } else {
                return TIMER_DEFAULT_EDGE;
        }
}

const char* get_timer_edge_api_key(const enum timer_edge e)
{
        switch(e) {
        case TIMER_EDGE_RISING:
                return TIMER_EDGE_RISING_STR;
        case TIMER_EDGE_FALLING:
                return TIMER_EDGE_FALLING_STR;
        default:
                return unknown_api_key();
        }
}
