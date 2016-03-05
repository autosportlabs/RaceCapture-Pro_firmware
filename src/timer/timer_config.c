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

#include "timer_config.h"
#include "loggerConfig.h"
#include "mod_string.h"

#include <stdlib.h>

#define RPM_CHAN_LABEL	"RPM"
#define RPM_CHAN_UNITS	"rpm"
#define RPM_CHAN_MIN	0
#define RPM_CHAN_MAX	8000

TimerConfig* get_timer_config(int channel)
{
        const size_t idx = (size_t) channel;
        if (idx >= CONFIG_TIMER_CHANNELS)
                return NULL;

        return getWorkingLoggerConfig()->TimerConfigs + idx;
}

void set_default_timer_config(TimerConfig tim_cfg[])
{
        for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; ++i) {
                TimerConfig *tc = tim_cfg + i;
                set_default_channel_config(&tc->cfg);
                tc->filterAlpha = 1.0;
                tc->mode = MODE_LOGGING_TIMER_RPM;
                tc->pulsePerRevolution = 1;
                tc->timerSpeed = TIMER_MEDIUM;
        }

        /* Make Channel 1 the default RPM config. */
        ChannelConfig *t0cc = &tim_cfg->cfg;
        strcpy(t0cc->label, RPM_CHAN_LABEL);
        strcpy(t0cc->units, RPM_CHAN_UNITS);
        t0cc->min = RPM_CHAN_MIN;
        t0cc->max = RPM_CHAN_MAX;
}
