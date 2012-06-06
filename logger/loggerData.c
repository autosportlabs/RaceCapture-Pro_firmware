/*
 * loggerData.c
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
 */

#include "loggerData.h"

//linear interpolation routine
//            (y2 - y1)
//  y = y1 +  --------- * (x - x1)
//            (x2 - x1)
static float LinearInterpolate(float x, float x1, float y1, float x2, float y2){
	return y1 + (((y2 - y1))  / (x2 - x1)) * (x - x1);
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
		nextBin = 0;
	}
	else{
		nextBin = bin;
		if (bin < ANALOG_SCALING_BINS - 1) nextBin++;
	}
	float x1 = (float)scalingMap->rawValues[bin];
	float y1 = scalingMap->scaledValues[bin];
	float x2 = (float)scalingMap->rawValues[nextBin];
	float y2 = scalingMap->scaledValues[nextBin];
	float scaled = LinearInterpolate(value,x1,y1,x2,y2);
	return scaled;

}


