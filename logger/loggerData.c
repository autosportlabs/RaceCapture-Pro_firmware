/*
 * loggerData.c
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
 */

#include "loggerData.h"
#include "loggerHardware.h"
#include "accelerometer.h"
#include "ADC.h"
#include "gps.h"
#include "linear_interpolate.h"
#include "predictive_timer.h"



void init_logger_data(){
}

void doBackgroundSampling(){
	accelerometer_sample_all();
	ADC_sample_all();
}



float GetMappedValue(float value, ScalingMap *scalingMap){
	unsigned short *bins;
	unsigned int bin, nextBin;

	bins = scalingMap->rawValues + ANALOG_SCALING_BINS - 1;
	bin = nextBin = ANALOG_SCALING_BINS - 1;

	while (value < *bins && bin > 0){
		bins--;
		bin--;
	}
	if (bin == 0 && value < *bins){
		return scalingMap->scaledValues[0];
	}
	else{
		nextBin = bin;
		if (bin < ANALOG_SCALING_BINS - 1){
			nextBin++;
		}
		else{
			return scalingMap->scaledValues[ANALOG_SCALING_BINS - 1];
		}
	}
	float x1 = (float)scalingMap->rawValues[bin];
	float y1 = scalingMap->scaledValues[bin];
	float x2 = (float)scalingMap->rawValues[nextBin];
	float y2 = scalingMap->scaledValues[nextBin];
	float scaled = LinearInterpolate(value,x1,y1,x2,y2);
	return scaled;
}
