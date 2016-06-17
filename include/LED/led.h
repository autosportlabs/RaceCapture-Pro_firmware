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

#ifndef _LED_H_
#define _LED_H_

#include "cpp_guard.h"

#include <stdbool.h>
#include <stdlib.h>

CPP_GUARD_BEGIN

/*
 * README
 *
 * ORDER MATTERS! NAMING MATTERS! DO NOT REMOVE ENTRIES!
 * You may only append here safely, and even then you may only
 * add enum values with POSITIVE numbers.  If you don't want an LED
 * enum to be ignored, you need to account for that in the driver.
 */
enum led {
        LED_UNKNOWN   = -1,
        LED_ERROR     =  0,
        LED_LOGGER    =  1,
        LED_GPS       =  2,
        LED_TELEMETRY =  3,
        LED_WIFI      =  4,
};

bool led_init(void);
bool led_set_index(const size_t index, const bool on);
bool led_set(const enum led l, const bool on);
bool led_enable(const enum led l);
bool led_disable(const enum led l);
bool led_toggle(const enum led l);
enum led get_led_enum(const char *name);

CPP_GUARD_END

#endif /* _LED_H_ */
