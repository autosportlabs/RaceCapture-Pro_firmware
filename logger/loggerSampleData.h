/*
 * loggerSampleData.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

float get_analog_sample(int channelId);
float get_timer_sample(int channelId);
float get_pwm_sample(int channelId);
float get_gpio_sample(int channelId);
float get_accel_sample(int channelId);
float get_gps_sample(int channelId);
float get_lap_stat_sample(int channelId);

#endif /* LOGGERSAMPLEDATA_H_ */
