#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "board.h"

//1G point for Kionix KXR94-2353
#define ACCEL_COUNTS_PER_G 				819

//ST LY330ALH is 3.752 mV/dps
#define YAW_COUNTS_PER_DEGREE_PER_SEC	4.69

#define ACCELEROMETER_CHANNEL_MIN 0
#define ACCELEROMETER_CHANNEL_MAX 3
#define ACCELEROMETER_BUFFER_SIZE 10


void accel_init();
void accel_setup();
void initAccelBuffer();
unsigned int calculateAccelAverage(unsigned char channel);

void accel_initSPI();
void accel_spiSetSpeed(unsigned char speed);
unsigned char accel_spiSend(unsigned char outgoing, int last);
unsigned char accel_readControlRegister();
float convertYawRawToDegreesPerSec(int yawRaw, unsigned int zeroVale);
float convertAccelRawToG(int rawValue, unsigned int zeroValue);
unsigned int readAccelChannel(unsigned char channel);
unsigned int getLastAccelRead(unsigned char channel);
unsigned int readAccelerometerDevice(unsigned char channel);


//#define readAccelAxisG(channel,zeroValue) accel_rawToG(accel_readAxis(channel),zeroValue);
//#define getLastAccelReadG(channel,zeroValue) accel_rawToG(accel_readAxis(channel),zeroValue);

#endif /*ACCELEROMETER_H_*/
