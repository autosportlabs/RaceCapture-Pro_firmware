/*
 * ADC.h
 *
 *  Created on: Feb 6, 2014
 *      Author: brentp
 */
#include "ADC.h"
#include "ADC_device.h"
#include "filter.h"
#include "loggerConfig.h"

static Filter g_adc_filter[CONFIG_ADC_CHANNELS];
static float g_adc_calibrations[CONFIG_ADC_CHANNELS];

int ADC_init(LoggerConfig *loggerConfig){
	ADCConfig *config = loggerConfig->ADCConfigs;

	for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++){
		float alpha = (config + i)->filterAlpha;
		init_filter(&g_adc_filter[i], alpha);
	}

	for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++){
		float calibration = (config + i)->calibration;
		g_adc_calibrations[i] = calibration;
	}
	return ADC_device_init();
}

void ADC_sample_all(void){

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

float ADC_read(unsigned int channel){
	return (g_adc_filter[channel].current_value * ADC_device_get_channel_scaling(channel)) * g_adc_calibrations[channel];
}

float ADC_get_voltage_range(size_t channel){
	return ADC_device_get_voltage_range(channel);
}
