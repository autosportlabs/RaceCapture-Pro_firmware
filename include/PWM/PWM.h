/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PWM_H_
#define PWM_H_

#include "cpp_guard.h"
#include "loggerConfig.h"

#include <stdint.h>
#include <stddef.h>

CPP_GUARD_BEGIN

int PWM_init(LoggerConfig *loggerConfig);

void PWM_set_clock_frequency(uint16_t clockFrequency);
void PWM_set_duty_cycle(unsigned int channel, unsigned short duty);
unsigned short PWM_get_duty_cycle(unsigned short channel);

void PWM_channel_set_period(unsigned int channel, unsigned short period);
unsigned short PWM_channel_get_period(unsigned int channel);

void PWM_channel_start(unsigned int channel);
void PWM_channel_stop(unsigned int channel);

void PWM_channel_start_all(void);
void PWM_channel_stop_all(void);

int PWM_update_config(LoggerConfig *loggerConfig);
void PWM_channel_enable_analog(size_t channel, uint8_t enabled);

CPP_GUARD_END

#endif /* PWM_H_ */
