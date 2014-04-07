
#ifndef IMU_H_
#define IMU_H_
#include "loggerConfig.h"

void imu_sample_all();

float imu_read_value(unsigned char accelChannel, ImuConfig *ac);

void imu_init();

void imu_calibrate_zero();

unsigned int imu_read(unsigned int channel);


#endif /* IMU_H_ */
