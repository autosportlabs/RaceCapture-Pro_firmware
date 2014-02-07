#include "ADC_device.h"
#include "loggerConifg.h"

static unsigned int g_adc[CONFIG_ADC_CHANNELS] = {0,0,0,0,0,0};

void ADC_device_sample_all(unsigned int *a0,
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

int ADC_device_init(void){}

//Read specified ADC channel
unsigned int ADC_device_sample(unsigned int channel){
	return g_adc[channel];
}

void ADC_mock_set_value(unsigned int channel, unsigned int value){
	g_adc[channel] = value;
}

