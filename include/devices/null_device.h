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

#ifndef NULL_DEVICE_H_
#define NULL_DEVICE_H_

#include "cpp_guard.h"
#include "stddef.h"
#include "devices_common.h"
#include "dateTime.h"

CPP_GUARD_BEGIN

int null_device_init_connection(DeviceConfig *config, millis_t * connected_at);
int null_device_check_connection_status(DeviceConfig *config);

CPP_GUARD_END

#endif /* NULL_DEVICE_H_ */
