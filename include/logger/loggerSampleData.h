/*
 * loggerSampleData.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

#include "sampleRecord.h"
#include "loggerConfig.h"

int populate_sample_buffer(ChannelSample * samples,  size_t count, size_t currentTicks);
void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount);

float get_mapped_value(float value, ScalingMap *scalingMap);

float get_time_sample(int index);
float get_analog_sample(int channelId);
float get_timer_sample(int channelId);
float get_pwm_sample(int channelId);
float get_obd2_sample(int channelId);
float get_gpio_sample(int channelId);
float get_imu_sample(int channelId);
float get_gps_sample(int channelId);
float get_lap_stat_sample(int channelId);

#endif /* LOGGERSAMPLEDATA_H_ */
