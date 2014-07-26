#include "imu_device.h"
#include "loggerConfig.h"
#include "printk.h"
#include "modp_numtoa.h"

#define IMU_DEVICE_COUNTS_PER_G 				819
#define IMU_DEVICE_COUNTS_PER_DEGREE_PER_SEC	4.69

#define ACCEL_MAX_RANGE 				ACCEL_COUNTS_PER_G * 4

void imu_device_init(){
}

unsigned int imu_device_read(unsigned int channel){
	return 0;
}

float imu_device_counts_per_unit(unsigned int channel){
	return (channel == IMU_CHANNEL_YAW ? IMU_DEVICE_COUNTS_PER_DEGREE_PER_SEC : IMU_DEVICE_COUNTS_PER_G);
}
