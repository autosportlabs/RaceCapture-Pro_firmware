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


#include "PWM_device.h"

#define PWM_CHANNELS 4

static int g_pwmPeriod[PWM_CHANNELS] = {0,0,0,0};
static int g_pwmDuty[PWM_CHANNELS] = {0,0,0,0};

int PWM_device_init(void)
{
        return 1;
}

void PWM_device_channel_init(unsigned int channel, unsigned short period, unsigned short dutyCycle)
{

}

void PWM_device_set_clock_frequency(uint16_t clockFrequency)
{

}

void PWM_device_set_duty_cycle(unsigned int channel, unsigned short duty)
{
        g_pwmDuty[channel] = duty;
}

unsigned short PWM_device_get_duty_cycle(unsigned int channel)
{
        return g_pwmDuty[channel];
}

void PWM_device_channel_set_period(unsigned int channel, unsigned short period)
{
        g_pwmPeriod[channel] = period;
}

unsigned short PWM_device_channel_get_period(unsigned int channel)
{
        return g_pwmPeriod[channel];
}

void PWM_device_channel_start(unsigned int channel)
{

}

void PWM_device_channel_stop(unsigned int channel)
{

}

void PWM_device_channel_start_all()
{

}

void PWM_device_channel_stop_all()
{

}

void PWM_device_channel_enable_analog(size_t channel, uint8_t enabled)
{

}
