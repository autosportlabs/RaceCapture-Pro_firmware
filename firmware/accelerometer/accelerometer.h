#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "board.h"

#define ACCELEROMETER_CHANNEL_MIN 0
#define ACCELEROMETER_CHANNEL_MAX 3

void accel_init();
void accel_initSPI();
void accel_spiSetSpeed(unsigned char speed);
unsigned char accel_spiSend(unsigned char outgoing, int last);
void accel_setup();
unsigned char accel_readControlRegister();
unsigned int accel_readAxis(unsigned char axis);
float accel_rawToG(int rawValue, unsigned int zeroValue);
float accel_readAxisG(unsigned char axis, unsigned int zeroValue);
                      

#endif /*ACCELEROMETER_H_*/
