#include "accelerometer_device.h"
#include "accelerometer_mock.h"
#include "loggerConfig.h"

#define ACCEL_DEVICE_COUNTS_PER_G 				819
#define YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC	4.69

static unsigned int g_accelerometer[CONFIG_ACCEL_CHANNELS] = {0,0,0,0};

void accelerometer_mock_set_value(unsigned int channel, unsigned int value){
	g_accelerometer[channel] = value;
}

void accelerometer_device_init(){ }

unsigned int accelerometer_device_read(unsigned int channel){
	return g_accelerometer[channel];
}

unsigned int accelerometer_device_counts_per_unit(unsigned int channel){
	return (channel == ACCEL_CHANNEL_ZT ? YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC : ACCEL_DEVICE_COUNTS_PER_G);
}
