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
#include "can_mapping.h"
#include "byteswap.h"
#include "units_conversion.h"

float canmapping_extract_value(uint64_t raw_data, const CANMapping *mapping)
{
        uint8_t offset = mapping->offset;
        uint8_t length = mapping->length;
        if (! mapping->bit_mode) {
                length *= 8;
                offset *= 8;
        }
        uint32_t bitmask = (1UL << length) - 1;
        uint32_t raw_value = (raw_data >> offset) & bitmask;
        if (mapping->big_endian) {
                switch (mapping->length) {
                    case 2:
                        raw_value = swap_uint16(raw_value);
                        break;
                    case 3:
                        raw_value = swap_uint24(raw_value);
                        break;
                    case 4:
                        raw_value = swap_uint32(raw_value);
                        break;
                    default:
                        break;
                }
        }
        return (float)raw_value;
}

float canmapping_apply_formula(float value, const CANMapping *mapping)
{
		value *= mapping->multiplier;
		if (mapping->divider)
				value /= mapping->divider;
		value += mapping->adder;
		return value;
}

bool canmapping_match_id(const CAN_msg *can_msg, const CANMapping *mapping)
{
        uint32_t can_id  = can_msg->addressValue;
        if (mapping->can_mask)
                can_id &= mapping->can_mask;

        return can_id == mapping->can_id;
}

bool canmapping_map_value(float *value, const CAN_msg *can_msg, const CANMapping *mapping)
{
        if (! canmapping_match_id(can_msg, mapping))
                return false;

		uint64_t raw_data = can_msg->data64;
		*value = canmapping_extract_value(raw_data, mapping);
		*value = canmapping_apply_formula(*value, mapping);
		*value = convert_units(mapping->conversion_filter_id, *value);
		return true;
}
