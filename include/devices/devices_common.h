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

#ifndef DEVICES_COMMON_H_
#define DEVICES_COMMON_H_

#include "cpp_guard.h"
#include "serial.h"
#include "serial_buffer.h"

CPP_GUARD_BEGIN

typedef struct serial_buffer DeviceConfig;

#define DEVICE_INIT_SUCCESS 			0
#define DEVICE_INIT_FAIL				1

#define DEVICE_STATUS_NO_ERROR 			0
#define DEVICE_STATUS_DISCONNECTED		1

CPP_GUARD_END

#endif /* DEVICES_COMMON_H_ */
