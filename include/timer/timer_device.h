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

#ifndef TIMER_DEVICE_H_
#define TIMER_DEVICE_H_

#include "cpp_guard.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

CPP_GUARD_BEGIN

/**
 * Initializes the timer channel.  Causes a reset in the state of
 * that channel.  Also adjusts the quiet period based on the pulses
 * per revolution.
 * @param channel The channel to init
 * @param speed The Speed enum value.  Controls
 */
bool timer_device_init(const size_t channel, const uint32_t speed,
                       const uint32_t quiet_period_us,
                       const enum timer_edge edge);
uint32_t timer_device_get_period(size_t channel);
uint32_t timer_device_get_usec(size_t channel);
uint32_t timer_device_get_count(size_t channel);
void timer_device_reset_count(size_t channel);

CPP_GUARD_END

#endif /* TIMER_DEVICE_H_ */
