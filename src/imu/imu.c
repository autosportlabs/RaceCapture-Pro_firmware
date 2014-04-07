#include "imu.h"
#include "imu_device.h"
#include "loggerConfig.h"
#include "filter.h"
#include "stddef.h"

//Channel Filters
#define IMU_ALPHA 0.1
static Filter g_imu_filter[CONFIG_IMU_CHANNELS];

static void init_filters(){
	for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++){
		init_filter(&g_imu_filter[i], IMU_ALPHA);
	}
}

void imu_sample_all(){
	for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++){
		update_filter(&g_imu_filter[i], imu_read(i));
	}
}

float imu_read_value(unsigned char imuChannel, ImuConfig *ac){
	size_t physicalChannel = ac->physicalChannel;
	unsigned int raw = g_imu_filter[physicalChannel].current_value;
	float countsPerUnit = imu_device_counts_per_unit(imuChannel);
	float scaledValue = ((float)((int)raw - (int)ac->zeroValue) / countsPerUnit);

	//invert physical channel to match industry-standard accelerometer mappings
	switch(physicalChannel){
		case IMU_CHANNEL_X:
		case IMU_CHANNEL_Y:
		case IMU_CHANNEL_YAW:
			scaledValue = -scaledValue;
			break;

		default:
			break;
	}

	//now invert based on configuration
	switch (ac->mode){
	case MODE_IMU_NORMAL:
		break;
	case MODE_IMU_INVERTED:
		scaledValue = -scaledValue;
		break;
	case MODE_IMU_DISABLED:
	default:
		scaledValue = 0;
		break;
	}
	return scaledValue;
}

static void imu_flush_filter(size_t physicalChannel){
	for (size_t i = 0; i < 1000; i++){
		update_filter(&g_imu_filter[physicalChannel], imu_read(physicalChannel));
	}
}

void imu_calibrate_zero(){
	for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
		ImuConfig * c = getImuConfigChannel(i);
		size_t physicalChannel = c->physicalChannel;
		imu_flush_filter(physicalChannel);
		unsigned int zeroValue = g_imu_filter[physicalChannel].current_value;
		//adjust for gravity
		float countsPerUnit = imu_device_counts_per_unit(physicalChannel);
		if (c->physicalChannel == IMU_CHANNEL_Z) zeroValue-= (countsPerUnit * (c->mode != MODE_IMU_INVERTED ? 1 : -1));
		c->zeroValue = zeroValue;
	}
}

void imu_init(){
	imu_device_init();
	init_filters();
}

unsigned int imu_read(unsigned int channel){
	return imu_device_read(channel);
}

