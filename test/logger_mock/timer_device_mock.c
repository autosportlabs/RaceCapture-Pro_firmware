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


#include "timer_device.h"

#define TIMER_CHANNELS 3
static int g_timer[TIMER_CHANNELS] = {0,0,0};

int timer_device_init(size_t channel, unsigned int divider, unsigned int slowChannelMode)
{
    return 1;
}

unsigned int timer_device_get_period(size_t channel)
{
    return g_timer[channel];
}

unsigned int timer_device_get_count(size_t channel)
{
    return 0;
}

void timer_device_reset_count(size_t channel) {}

void timer_device_get_all_periods(unsigned int *t0, unsigned int *t1, unsigned int *t2)
{
    *t0 = g_timer[0];
    *t1 = g_timer[1];
    *t2 = g_timer[2];
}

uint32_t timer_device_get_usec(size_t channel)
{
    return 0;
}
