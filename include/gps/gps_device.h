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

#ifndef GPS_DEVICE_H_
#define GPS_DEVICE_H_

#include "cpp_guard.h"
#include "serial.h"
#include "gps.h"

CPP_GUARD_BEGIN

typedef enum {
        GPS_MSG_READERR = -2,
        GPS_MSG_TIMEOUT = -1,
        GPS_MSG_NONE    = 0,
        GPS_MSG_SUCCESS = 1,
} gps_msg_result_t;

gps_status_t GPS_device_init(uint8_t targetSampleRate, struct Serial *serial);
gps_msg_result_t GPS_device_get_update(GpsSample *gpsSample, struct Serial *serial);
CPP_GUARD_END

#endif /* GPS_DEVICE_H_ */
