/*
 * ADC.h
 *
 *  Created on: Feb 6, 2014
 *      Author: brentp
 */
#include "ADC.h"
#include "ADC_device.h"

#define ADC_CHANNELS 8

static unsigned int g_adc[8];

int ADC_init(void){
	return ADC_device_init();
}

void ADC_sample_all(void){
	ADC_device_sample_all(&g_adc[0], &g_adc[1], &g_adc[2], &g_adc[3], &g_adc[4], &g_adc[5], &g_adc[6], &g_adc[7]);
}

void ADC_read_all(unsigned int *a0,
						unsigned int *a1,
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){
	*a0 = g_adc[0];
	*a1 = g_adc[1];
	*a2 = g_adc[2];
	*a3 = g_adc[3];
	*a4 = g_adc[4];
	*a5 = g_adc[5];
	*a6 = g_adc[6];
	*a7 = g_adc[7];
}

unsigned int ADC_read(unsigned int channel){
	return g_adc[channel];
}
