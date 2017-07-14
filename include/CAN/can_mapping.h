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

/**
 * match the can message based on the specified CAN mapping ID and ID mask
 * @param can_msg the CAN message to test
 * @param mapping the can mapping to check against
 * @return true if the CAN message matches the mapping's CAN ID and ID mask. A mapping with CAN ID of 0 is a wildcard and will match any ID in the CAN_msg
 */
bool canmapping_match_id(const CAN_msg *can_msg, const CANMapping *mapping);

/**
 * performs the full mapping against for the CAN message using the CAN mapping
 * @param value the mapped value is set in this parameter if the CAN mapping matches the specified message
 * @param can_msg the CAN message containing the raw data
 * @param mapping the mapping to be applied to the message
 * @return true if the mapping was successfully applied
 */
bool canmapping_map_value(float *value, const CAN_msg *can_msg, const CANMapping *mapping);

/**
 * apply the mapping's formula against the specified value
 * @param value the raw value being applied to the formula
 * @param the mapping containing the formula
 * @return the value after transformed by the formula
 */
float canmapping_apply_formula(float value, const CANMapping *mapping);

/**
 * extract the raw value using the specified mapping
 * @param the raw data containing
 * @param the mapping that specifies the offset and bit length within the raw data
 * @return the extracted value
 */
float canmapping_extract_value(uint64_t raw_data, const CANMapping *mapping);

CPP_GUARD_END
#endif /* CAN_MAPPING_H_ */
