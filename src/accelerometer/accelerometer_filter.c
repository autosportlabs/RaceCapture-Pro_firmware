#include "accelerometer_filter.h"
#include "loggerConfig.h"

static int g_filteredAccelValues[CONFIG_ACCEL_CHANNELS];

//This macros defines an alpha value between 0 and 1
#define DSP_EMA_I32_ALPHA(x) ( (unsigned short)(x * 65535) )
#define ALPHA 0.01

static int dsp_ema_i32(int in, int average, unsigned short alpha){
  long long tmp0; //calcs must be done in 64-bit math to avoid overflow
  tmp0 = (long long)in * (alpha) + (long long)average * (65536 - alpha);
  return (int)((tmp0 + 32768) / 65536); //scale back to 32-bit (with rounding)
}

void initAccelFilter(){
	for (size_t i = 0; i < CONFIG_ACCEL_CHANNELS; i++) g_filteredAccelValues[i] = 0;
}

int getCurrentAccelValue(size_t channel){
	return g_filteredAccelValues[channel];
}

int averageAccelValue(size_t channel, int rawValue){
	int average = g_filteredAccelValues[channel];
	average = dsp_ema_i32(rawValue, average, DSP_EMA_I32_ALPHA(ALPHA));
	g_filteredAccelValues[channel] = average;
	return average;
}




