/*
 * accelerometer.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "accelerometer.h"
#include "accelerometer_mock.h"
#include "loggerConfig.h"

#define ACCEL_DEFAULT_VALUE 0

static unsigned int channelValues[CONFIG_ACCEL_CHANNELS];

void setAccelMockValue(unsigned int channel, unsigned int value){
	channelValues[channel] = value;
}

void accel_init(){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		channelValues[i] = ACCEL_DEFAULT_VALUE;
	}
}

unsigned int readAccelChannel(unsigned char channel){
	return readAccelerometerDevice(channel);
}

unsigned int getLastAccelRead(unsigned char channel){
	return readAccelChannel(channel);
}

unsigned int readAccelerometerDevice(unsigned char channel){
	return channelValues[channel];
}

void calibrateAccelZero(){

}
