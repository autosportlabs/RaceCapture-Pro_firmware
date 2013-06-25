#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

//1G point for Kionix KXR94-2353
#define ACCEL_COUNTS_PER_G 				819

//ST LY330ALH is 3.752 mV/dps
#define YAW_COUNTS_PER_DEGREE_PER_SEC	4.69

#define ACCELEROMETER_CHANNEL_MIN 0
#define ACCELEROMETER_CHANNEL_MAX 3

#define YAW_RAW_TO_DEGREES_PER_SEC(RAW, ZERO_VALUE) ((float)((int)RAW - (int)ZERO_VALUE) / (float)YAW_COUNTS_PER_DEGREE_PER_SEC)
#define ACCEL_RAW_TO_GFORCE(RAW, ZERO_VALUE) ((float)((int)RAW - (int)ZERO_VALUE) / (float)ACCEL_COUNTS_PER_G)

void accel_init();

unsigned int readAccelChannel(unsigned char channel);
unsigned int getLastAccelRead(unsigned char channel);
unsigned int readAccelerometerDevice(unsigned char channel);

void calibrateAccelZero();

#endif /*ACCELEROMETER_H_*/
