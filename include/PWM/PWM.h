/*
 * PWM.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brentp
 */

#ifndef PWM_H_
#define PWM_H_

#include "loggerConfig.h"
#include <stdint.h>
#include <stddef.h>

int PWM_init(LoggerConfig *loggerConfig);

void PWM_set_duty_cycle(unsigned int channel, unsigned short duty);
unsigned short PWM_get_duty_cycle(unsigned short channel);

void PWM_channel_set_period(unsigned int channel, unsigned short period);
unsigned short PWM_channel_get_period(unsigned int channel);

void PWM_channel_start(unsigned int channel);
void PWM_channel_stop(unsigned int channel);

void PWM_channel_start_all(void);
void PWM_channel_stop_all(void);

void PWM_channel_enable_analog(size_t channel, uint8_t enabled);

#endif /* PWM_H_ */
