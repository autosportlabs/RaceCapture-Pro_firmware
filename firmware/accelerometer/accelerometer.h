#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "board.h"

#define ACCELEROMETER_CHANNEL_MIN 0
#define ACCELEROMETER_CHANNEL_MAX 3

void accel_init();
void accel_setup();
void initAccelBuffer();
unsigned int calculateAccelAverage(unsigned char channel);

void accel_initSPI();
void accel_spiSetSpeed(unsigned char speed);
unsigned char accel_spiSend(unsigned char outgoing, int last);
unsigned char accel_readControlRegister();
float convertAccelRawToG(int rawValue, unsigned int zeroValue);
unsigned int readAccelChannel(unsigned char channel);
unsigned int getLastAccelRead(unsigned char channel);
unsigned int readAccelerometerDevice(unsigned char channel);


//#define readAccelAxisG(channel,zeroValue) accel_rawToG(accel_readAxis(channel),zeroValue);
//#define getLastAccelReadG(channel,zeroValue) accel_rawToG(accel_readAxis(channel),zeroValue);

#endif /*ACCELEROMETER_H_*/
