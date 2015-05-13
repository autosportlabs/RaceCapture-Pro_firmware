/*
 * timer_device_mock.c
 *
 *  Created on: Feb 7, 2014
 *      Author: brentp
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
