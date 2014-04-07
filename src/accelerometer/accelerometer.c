#include "accelerometer.h"
#include "accelerometer_device.h"
#include "loggerConfig.h"
#include "filter.h"
#include "stddef.h"

//Channel Filters
#define ACCEL_ALPHA 0.1
static Filter g_accel_filter[CONFIG_ACCEL_CHANNELS];

static void init_filters(){
	for (size_t i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		init_filter(&g_accel_filter[i], ACCEL_ALPHA);
	}
}

void accelerometer_sample_all(){
	for (size_t i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		update_filter(&g_accel_filter[i], accelerometer_read(i));
	}
}

float accelerometer_read_value(unsigned char accelChannel, AccelConfig *ac){
	size_t physicalChannel = ac->physicalChannel;
	unsigned int raw = g_accel_filter[physicalChannel].current_value;
	float countsPerUnit = accelerometer_device_counts_per_unit(accelChannel);
	float accelG = ((float)((int)raw - (int)ac->zeroValue) / countsPerUnit);

	//invert physical channel to match industry-standard accelerometer mappings
	switch(physicalChannel){
		case ACCEL_CHANNEL_X:
		case ACCEL_CHANNEL_Y:
		case ACCEL_CHANNEL_ZT:
			accelG = -accelG;
			break;
		default:
			break;
	}

	//now invert based on configuration
	switch (ac->mode){
	case MODE_ACCEL_NORMAL:
		break;
	case MODE_ACCEL_INVERTED:
		accelG = -accelG;
		break;
	case MODE_ACCEL_DISABLED:
	default:
		accelG = 0;
		break;
	}
	return accelG;
}

static void flushAccelBuffer(size_t physicalChannel){
	for (size_t i = 0; i < 1000; i++){
		update_filter(&g_accel_filter[physicalChannel], accelerometer_read(physicalChannel));
	}
}

void calibrateAccelZero(){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig * c = getAccelConfigChannel(i);
		size_t physicalChannel = c->physicalChannel;
		flushAccelBuffer(physicalChannel);
		unsigned int zeroValue = g_accel_filter[physicalChannel].current_value;
		//adjust for gravity
		float accelCountsPerUnit = accelerometer_device_counts_per_unit(physicalChannel);
		if (c->physicalChannel == ACCEL_CHANNEL_Z) zeroValue-= (accelCountsPerUnit * (c->mode != MODE_ACCEL_INVERTED ? 1 : -1));
		c->zeroValue = zeroValue;
	}
}

void accelerometer_init(){
	accelerometer_device_init();
	init_filters();
}

unsigned int accelerometer_read(unsigned int channel){
	return accelerometer_device_read(channel);
}

