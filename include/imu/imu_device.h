
#ifndef IMU_DEVICE_H_
#define IMU_DEVICE_H_

void imu_device_init();

int imu_device_read(unsigned int channel);

float imu_device_counts_per_unit(unsigned int channel);

#endif /* IMU_DEVICE_H_ */
