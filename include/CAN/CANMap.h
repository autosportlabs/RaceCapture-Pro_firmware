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

#ifndef CANMAP_H_
#define CANMAP_H_

#include "cpp_guard.h"
#include "loggerConfig.h"
#include "CAN.h"

#include <stdint.h>

CPP_GUARD_BEGIN

void CANMap_set_current_map_value(size_t index, int value);
int CANMap_get_current_map_value(int index);
bool CAN_process_map(CANMapConfig *cMapConf, CAN_msg *msg, int *value);

CPP_GUARD_END

#endif /* CAN_H_ */
