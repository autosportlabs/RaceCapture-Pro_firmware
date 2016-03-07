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

#ifndef TIMER_H_
#define TIMER_H_

#include "cpp_guard.h"
#include "loggerConfig.h"

#include <stdint.h>

CPP_GUARD_BEGIN

int timer_init(LoggerConfig *loggerConfig);
uint32_t timer_get_raw(size_t channel);
uint32_t timer_get_usec(size_t channel);
uint32_t timer_get_ms(size_t channel);
uint32_t timer_get_rpm(size_t channel);
uint32_t timer_get_hz(size_t channel);
uint32_t timer_get_count(size_t channel);
void timer_reset_count(size_t channel);
float timer_get_sample(const int cid);

unsigned int timerPeriodToUs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToMs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToHz(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToRpm(unsigned int ticks, unsigned int scaling);

CPP_GUARD_END

#endif /* TIMER_H_ */
