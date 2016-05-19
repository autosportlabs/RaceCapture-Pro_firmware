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

#include "ADC_device.h"
#include "loggerConfig.h"

#define ADC_PORT_VOLTAGE_RANGE 		5.0f
#define ADC_SYSTEM_VOLTAGE_RANGE	20.0f
#define SCALING_5V 	       		0.00122070312f
#define SCALING_20V    			0.0048828125f

static unsigned int g_adc[CONFIG_ADC_CHANNELS];

int ADC_device_init(void)
{
        return 1;
}

static bool channel_in_bounds(const size_t channel)
{
        return channel < CONFIG_ADC_CHANNELS;
}

int ADC_device_sample(const size_t channel)
{
        return channel_in_bounds(channel) ? g_adc[channel] : -1;
}

void ADC_mock_set_value(const size_t channel, const unsigned int value)
{
        if (!channel_in_bounds(channel))
                return;

        g_adc[channel] = value;
}

float ADC_device_get_voltage_range(const size_t channel)
{
        switch (channel) {
        case 7:
                return ADC_SYSTEM_VOLTAGE_RANGE;
        default:
                return ADC_PORT_VOLTAGE_RANGE;
        }
}

float ADC_device_get_channel_scaling(const size_t channel)
{
        float scaling = 0;
        switch(channel) {
        case 7:
                scaling = SCALING_20V;
                break;
        default:
                scaling = SCALING_5V;
                break;
        }
        return scaling;
}
