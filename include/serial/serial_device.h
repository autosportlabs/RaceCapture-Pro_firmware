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

#ifndef _SERIAL_DEVICE_H_
#define _SERIAL_DEVICE_H_

#include "cpp_guard.h"
#include "serial.h"

#include <stdbool.h>

CPP_GUARD_BEGIN

/* STIEG: Update Enum type to be proper later */
typedef enum {
        SERIAL_USB = 0,
        SERIAL_GPS,
        SERIAL_TELEMETRY,
        SERIAL_WIRELESS,
        SERIAL_AUX,
        __SERIAL_COUNT, /* ALWAYS AT THE END */
} serial_id_t;

struct Serial* serial_device_get(const serial_id_t port);

CPP_GUARD_END

#endif /* _SERIAL_DEVICE_H_ */
