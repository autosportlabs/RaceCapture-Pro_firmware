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

int ADC_init(LoggerConfig *loggerConfig){
	ADCConfig *config = loggerConfig->ADCConfigs;
	for (size_t i = 0; i < CONFIG_ADC_CHANNELS; i++){
		float alpha = (config + i)->filterAlpha;
		init_filter(&g_adc_filter[i], alpha);
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

void ADC_read_all(unsigned int *a0,
						unsigned int *a1,
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){
	*a0 = g_adc_filter[0].current_value;
	*a1 = g_adc_filter[1].current_value;
	*a2 = g_adc_filter[2].current_value;
	*a3 = g_adc_filter[3].current_value;
	*a4 = g_adc_filter[4].current_value;
	*a5 = g_adc_filter[5].current_value;
	*a6 = g_adc_filter[6].current_value;
	*a7 = g_adc_filter[07].current_value;
}

unsigned int ADC_read(unsigned int channel){
	return g_adc_filter[channel].current_value;
}

float ADC_get_voltage_range(size_t channel){
	return ADC_device_get_voltage_range(channel);
}
