/*
 * accelerometer_buffer.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "accelerometer_buffer.h"
#include "loggerConfig.h"
#include "accelerometer.h"
#include "mod_string.h"

static unsigned int g_averagedAccelValues[CONFIG_ACCEL_CHANNELS];
static unsigned int g_accelBuffer[CONFIG_ACCEL_CHANNELS][ACCELEROMETER_BUFFER_SIZE];
static int g_accelBufferPointers[CONFIG_ACCEL_CHANNELS];


size_t getBufferSize(){
	return ACCELEROMETER_BUFFER_SIZE;
}

void flushAccelBuffer(){
	int resample = ACCELEROMETER_BUFFER_SIZE;
	while (resample-- > 0){
		for (int i = ACCELEROMETER_CHANNEL_MIN; i <= ACCELEROMETER_CHANNEL_MAX; i++){
			readAccelChannel(i);
		}
	}
}

void initAccelBuffer(){
	memset(g_averagedAccelValues,0,sizeof(g_averagedAccelValues));
	for (int channel = 0; channel < CONFIG_ACCEL_CHANNELS; channel++){
		for (int i = 0; i < ACCELEROMETER_BUFFER_SIZE; i++){
			g_accelBuffer[channel][i] = readAccelerometerDevice(channel);
		}
		g_accelBufferPointers[channel] = 0;
	}
}

unsigned int calculateAccelAverage(unsigned char channel){
	unsigned int total = 0;
	for (int i = 0; i < ACCELEROMETER_BUFFER_SIZE;i++){
		total+=g_accelBuffer[channel][i];
	}
	return total / ACCELEROMETER_BUFFER_SIZE;
}





