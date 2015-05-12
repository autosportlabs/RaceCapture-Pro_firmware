#include "imu_device.h"
#include "imu_mock.h"
#include "loggerConfig.h"

#define ACCEL_DEVICE_COUNTS_PER_G 				819
#define YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC	4.69

static unsigned int g_imuDevice[CONFIG_IMU_CHANNELS] = {0,0,0,0};

void imu_mock_set_value(unsigned int channel, unsigned int value)
{
    g_imuDevice[channel] = value;
}

void imu_device_init() { }

int imu_device_read(unsigned int channel)
{
    return g_imuDevice[channel];
}

float imu_device_counts_per_unit(unsigned int channel)
{
    return (channel == IMU_CHANNEL_YAW ? YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC : ACCEL_DEVICE_COUNTS_PER_G);
}
