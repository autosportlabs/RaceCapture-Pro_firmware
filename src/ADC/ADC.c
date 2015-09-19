/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ADC.h"
#include "ADC_device.h"
#include "filter.h"
#include "loggerConfig.h"

static Filter g_adc_filter[CONFIG_ADC_CHANNELS];
static float g_adc_calibrations[CONFIG_ADC_CHANNELS];

int ADC_init(LoggerConfig *loggerConfig)
{
    ADCConfig *config = loggerConfig->ADCConfigs;

    for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++) {
        float alpha = (config + i)->filterAlpha;
        init_filter(&g_adc_filter[i], alpha);
    }

    for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++) {
        float calibration = (config + i)->calibration;
        g_adc_calibrations[i] = calibration;
    }
    return ADC_device_init();
}

void ADC_sample_all(void)
{

    unsigned int a0, a1, a2, a3, a4, a5, a6, a7;

    ADC_device_sample_all(&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7);

    update_filter(&g_adc_filter[0], a0);
    update_filter(&g_adc_filter[1], a1);
    update_filter(&g_adc_filter[2], a2);
    update_filter(&g_adc_filter[3], a3);
    update_filter(&g_adc_filter[4], a4);
    update_filter(&g_adc_filter[5], a5);
    update_filter(&g_adc_filter[6], a6);
    update_filter(&g_adc_filter[7], a7);
}

float ADC_read(unsigned int channel)
{
    return (g_adc_filter[channel].current_value * ADC_device_get_channel_scaling(channel))
           * g_adc_calibrations[channel];
}
