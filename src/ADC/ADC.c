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

#include "ADC.h"
#include "ADC_device.h"
#include "filter.h"
#include "loggerConfig.h"
#include "printk.h"

static Filter g_adc_filter[CONFIG_ADC_CHANNELS];
static float g_adc_calibrations[CONFIG_ADC_CHANNELS];

int ADC_init(LoggerConfig *loggerConfig)
{
        for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++) {
                ADCConfig *config = loggerConfig->ADCConfigs + i;
                init_filter(g_adc_filter + i, config->filterAlpha);
                g_adc_calibrations[i] = config->calibration;
        }

        return ADC_device_init();
}

void ADC_sample_all(void)
{
        for (int i = 0; i < CONFIG_ADC_CHANNELS; ++i) {
                const int val = ADC_device_sample(i);

                if (val < 0) {
                        /* Should never get here */
                        pr_error_int_msg("Sampled non-existant channel: ", i);
                        continue;
                }

                update_filter(g_adc_filter + i, val);
        }
}

float ADC_read(const size_t channel)
{
        return g_adc_filter[channel].current_value *
                ADC_device_get_channel_scaling(channel) *
                g_adc_calibrations[channel];
}
