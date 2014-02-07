/*
 * timer_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brentp
 */

#ifndef TIMER_DEVICE_H_
#define TIMER_DEVICE_H_
#include <stddef.h>

int timer_device_init(size_t channel, unsigned int divider, unsigned int slowChannelMode);

unsigned int timer_device_get_period(unsigned int channel);
unsigned int timer_device_get_count(unsigned int channel);
void timer_device_reset_count(unsigned int channel);
void timer_device_get_all_periods(unsigned int *t0, unsigned int *t1, unsigned int *t2);

#endif /* TIMER_DEVICE_H_ */
