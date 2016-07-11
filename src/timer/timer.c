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
#include "timer_config.h"
#include "timer_device.h"
#include "filter.h"
#include "printk.h"

/* Adds 25% to the max RPM value */
#define MAX_RPM_MULT	1.25
#define SEC_IN_A_MIN	60.0
#define SEC_IN_A_SEC	1.0
#define US_IN_A_SEC	1000000

static Filter g_timer_filter[CONFIG_TIMER_CHANNELS];

/**
 * Calculates the highest quiet period usable based on the timer
 * configurations.  We calculate this by figuring out the expected
 * maximum HZ of the engine based in the info provided.  Then we simply
 * take the inverse of that and multiply it by the number of micro seconds
 * in a second.
 */
static uint32_t calc_quiet_period(const TimerConfig *tc,
                                  const float period)
{
        const float ppr = tc->pulsePerRevolution;
	if (!ppr)
		return 0;

        const float max_rpm = tc->cfg.max;
        const float max_hz = max_rpm * ppr * MAX_RPM_MULT / period;

        const uint32_t res = max_hz ? US_IN_A_SEC / max_hz : 0;
        pr_debug_int_msg("[timer] Calculated QP: ", res);
        return res;
}

static uint32_t get_quiet_period(const TimerConfig *tc)
{
        if (tc->filter_period_us >= 0)
                return (uint32_t) tc->filter_period_us;

        switch(tc->mode) {
        case MODE_LOGGING_TIMER_RPM:
                return calc_quiet_period(tc, SEC_IN_A_MIN);
        default:
                return calc_quiet_period(tc, SEC_IN_A_SEC);
        }
}

int timer_init(LoggerConfig *loggerConfig)
{
        for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; i++) {
                TimerConfig *tc = &loggerConfig->TimerConfigs[i];
                const uint32_t qp_us = get_quiet_period(tc);

                timer_device_init(i, tc->timerSpeed, qp_us, tc->edge);
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

float timer_get_sample(const int cid)
{
        if (cid >= TIMER_CHANNELS)
                return -1;

        TimerConfig *c = getWorkingLoggerConfig()->TimerConfigs + cid;
        const float ppr = c->pulsePerRevolution;
	if (0 == ppr)
		return 0;

        switch (c->mode) {
        case MODE_LOGGING_TIMER_RPM:
                return timer_get_rpm(cid) / ppr;
        case MODE_LOGGING_TIMER_FREQUENCY:
                return timer_get_hz(cid) / ppr;
        case MODE_LOGGING_TIMER_PERIOD_MS:
                return timer_get_ms(cid) * ppr;
        case MODE_LOGGING_TIMER_PERIOD_USEC:
                return timer_get_usec(cid) * ppr;
        default:
                return -1;
        }
}
