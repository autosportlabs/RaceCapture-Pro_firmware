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

#define SCALING_5V 					0.00122070312f
#define SCALING_20V 				0.0048828125f

static unsigned int g_adc[CONFIG_ADC_CHANNELS] = {0,0,0,0,0,0};

int ADC_device_init(void)
{
    return 1;
}

//Read specified ADC channel
unsigned int ADC_device_sample(unsigned int channel)
{
    return g_adc[channel];
}

void ADC_device_sample_all(unsigned int *a0,
                           unsigned int *a1,
                           unsigned int *a2,
                           unsigned int *a3,
                           unsigned int *a4,
                           unsigned int *a5,
                           unsigned int *a6,
                           unsigned int *a7 )
{
    *a0 = g_adc[0];
    *a1 = g_adc[1];
    *a2 = g_adc[2];
    *a3 = g_adc[3];
    *a4 = g_adc[4];
    *a5 = g_adc[5];
    *a6 = g_adc[6];
    *a7 = g_adc[7];
}


void ADC_mock_set_value(unsigned int channel, unsigned int value)
{
    g_adc[channel] = value;
}

float ADC_device_get_voltage_range(size_t channel)
{
    switch (channel) {
    case 7:
        return ADC_SYSTEM_VOLTAGE_RANGE;
    default:
        return ADC_PORT_VOLTAGE_RANGE;
    }
}

float ADC_device_get_channel_scaling(size_t channel)
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
