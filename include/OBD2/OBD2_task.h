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

#ifndef OBD2_TASK_H_
#define OBD2_TASK_H_

#include "cpp_guard.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

bool CAN_init_current_values(size_t values);
float CAN_get_current_channel_value(int index);
void CAN_set_current_channel_value(int index, float value);

void startOBD2Task(int priority);
void OBD2Task(void *pvParameters);

CPP_GUARD_END


#endif /* OBD2_TASK_H_ */
