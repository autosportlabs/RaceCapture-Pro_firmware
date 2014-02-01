/*
 * accelerometer.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_
#include "loggerConfig.h"

void accelerometer_sample_all();

float accelerometer_read_value(unsigned char accelChannel, AccelConfig *ac);

void accelerometer_init();

unsigned int accelerometer_read(unsigned int channel);


#endif /* ACCELEROMETER_H_ */
