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

#define OBD2_PID_DEFAULT_TIMEOUT_MS 300

int OBD2_request_PID(unsigned char pid, int *value, size_t timeout);
void OBD2_set_current_PID_value(size_t index, int value);
int OBD2_get_current_PID_value(int index);

CPP_GUARD_END

#endif /* OBD2_H_ */
