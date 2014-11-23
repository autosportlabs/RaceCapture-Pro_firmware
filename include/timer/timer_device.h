/*
 * timer_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brentp
 */

#ifndef TIMER_DEVICE_H_
#define TIMER_DEVICE_H_
#include <stdint.h>
#include <stddef.h>

int32_t timer_device_init(size_t channel, uint32_t speed, uint32_t slowChannelMode);

uint32_t timer_device_get_period(size_t channel);
uint32_t timer_device_get_usec(size_t channel);
uint32_t timer_device_get_count(size_t channel);
void timer_device_reset_count(size_t channel);

#endif /* TIMER_DEVICE_H_ */
