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
#ifndef OBD2_H_
#define OBD2_H_

#include "cpp_guard.h"
#include "CAN.h"
#include "stddef.h"

CPP_GUARD_BEGIN

#define OBD2_PID_DEFAULT_TIMEOUT_MS 1000
#define OBD2_PID_REQUEST_TIMEOUT_MS 1

int OBD2_get_value_for_pid(uint8_t pid);
int OBD2_request_PID(uint8_t pid, uint8_t mode, size_t timeout);

CPP_GUARD_END

#endif /* OBD2_H_ */
