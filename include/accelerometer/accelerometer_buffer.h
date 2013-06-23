/*
 * accelerometer_buffer.h
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */

#ifndef ACCELEROMETER_BUFFER_H_
#define ACCELEROMETER_BUFFER_H_
#include <stddef.h>

#define ACCELEROMETER_BUFFER_SIZE 10

size_t getBufferSize();
void initAccelBuffer();
void flushAccelBuffer();
unsigned int calculateAccelAverage(unsigned char channel);

#endif /* ACCELEROMETER_BUFFER_H_ */
