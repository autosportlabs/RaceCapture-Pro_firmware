/*
 * ADC_device.h
 *
 *  Created on: Feb 6, 2014
 *      Author: brentp
 */

#ifndef ADC_DEVICE_H_
#define ADC_DEVICE_H_
#include <stddef.h>

int ADC_device_init(void);

void ADC_device_sample_all(
		unsigned int *a0,
		unsigned int *a1,
		unsigned int *a2,
		unsigned int *a3,
		unsigned int *a4,
		unsigned int *a5,
		unsigned int *a6,
		unsigned int *a7 );

unsigned int ADC_device_sample(unsigned int channel);

float ADC_device_get_voltage_range(size_t channel);

float ADC_device_get_channel_scaling(size_t channel);

#endif /* ADC_DEVICE_H_ */
