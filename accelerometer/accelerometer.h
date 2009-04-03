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
float convertAccelRawToG(int rawValue, unsigned int zeroValue);
unsigned int readAccelAxis(unsigned char axis);
unsigned int getLastAccelRead(unsigned char axis);

#define readAccelAxisG(axis,zeroValue) accel_rawToG(accel_readAxis(axis),zeroValue);
#define getLastAccelReadG(axis,zeroValue) accel_rawToG(accel_readAxis(axis),zeroValue);

#endif /*ACCELEROMETER_H_*/
