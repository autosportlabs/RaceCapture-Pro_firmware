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

#ifndef CAN_MAPPING_H_
#define CAN_MAPPING_H_

#include "loggerConfig.h"
#include "CAN.h"

CPP_GUARD_BEGIN

bool canmapping_match_id(CAN_msg *can_msg, CANMapping *mapping);

bool canmapping_map_value(float *value, CAN_msg *can_msg, CANMapping *mapping);

float canmapping_apply_formula(float value, CANMapping *mapping);

float canmapping_extract_value(uint64_t raw_data, CANMapping *mapping);

CPP_GUARD_END
#endif /* CAN_MAPPING_H_ */
