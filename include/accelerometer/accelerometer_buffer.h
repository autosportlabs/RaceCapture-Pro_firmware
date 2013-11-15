/*
 * accelerometer_buffer.h
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */

#ifndef ACCELEROMETER_BUFFER_H_
#define ACCELEROMETER_BUFFER_H_
#include <stddef.h>

void initAccelFilter();
int getCurrentAccelValue(size_t channel);
int averageAccelValue(size_t channel, int rawValue);

#endif /* ACCELEROMETER_BUFFER_H_ */
