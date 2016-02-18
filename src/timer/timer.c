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


#include "timer.h"
#include "timer_device.h"
#include "filter.h"

static Filter g_timer_filter[CONFIG_TIMER_CHANNELS];

static uint16_t get_timer_quiet_period(const TimerConfig *tc)
{
        /* Only use filtering when in RPM mode */
        if (MODE_LOGGING_TIMER_RPM != tc->mode)
                return 0;

        /*
         * Software Filter Hack Represents 15K Max.  For release
         * in 2.8.8.  Have to divide value by pulses per revolution
         * since this must scale depending on the number of pulses
         * per engine rotation.
         */
        return 4500 / tc->pulsePerRevolution;
}

int timer_init(LoggerConfig *loggerConfig)
{
        for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; i++) {
                TimerConfig *tc = &loggerConfig->TimerConfigs[i];
                const uint16_t qp_us = get_timer_quiet_period(tc);
                timer_device_init(i, tc->timerSpeed, qp_us);
                init_filter(&g_timer_filter[i], tc->filterAlpha);
        }

        return 1;
}

uint32_t timer_get_raw(size_t channel)
{
    return timer_device_get_period(channel);
}

uint32_t timer_get_hz(size_t channel)
{
    uint32_t usec = timer_get_usec(channel);
    return usec > 0 ? 1000000 / usec : 0;
}

uint32_t timer_get_ms(size_t channel)
{
    return timer_get_usec(channel) / 1000;
}

uint32_t timer_get_rpm(size_t channel)
{
    return timer_get_hz(channel) * 60;
}

uint32_t timer_get_usec(size_t channel)
{
    Filter *filter = &g_timer_filter[channel];
    unsigned int period = timer_device_get_usec(channel);
    update_filter(filter, period);
    return filter->current_value;
}

uint32_t timer_get_count(size_t channel)
{
    return timer_device_get_count(channel);
}

void timer_reset_count(size_t channel)
{
    timer_device_reset_count(channel);
}
