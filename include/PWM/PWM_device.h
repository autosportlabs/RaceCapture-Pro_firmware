/*
 * PWM_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brentp
 */

#ifndef PWM_DEVICE_H_
#define PWM_DEVICE_H_

int PWM_device_init(void);
void PWM_device_channel_init(unsigned int channel, unsigned short period, unsigned short dutyCycle);
void PWM_configure_clock(unsigned short clockFrequency);
void PWM_start_all();
void PWM_stop_all();


#endif /* PWM_DEVICE_H_ */
