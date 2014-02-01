/*
 * accelerometer_device.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef ACCELEROMETER_DEVICE_H_
#define ACCELEROMETER_DEVICE_H_

void accelerometer_device_init();

unsigned int accelerometer_device_read(unsigned int channel);

unsigned int accelerometer_device_counts_per_unit(unsigned int channel);

#endif /* ACCELEROMETER_DEVICE_H_ */
