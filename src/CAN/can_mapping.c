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
#include "panic.h"
#include <stdlib.h>


float canmapping_extract_value(uint64_t raw_data, const CANMapping *mapping)
{
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            raw_data = swap_uint64(raw_data);
        #endif

        uint8_t offset = mapping->offset;
        uint8_t length = mapping->length;
        if (! mapping->bit_mode) {
                length *= 8;
                offset *= 8;
        }
        /* create the bitmask of the appropriate length */
        uint32_t bitmask = (1UL << length) - 1;

        /* extract the raw value from the 64 bit representation of the CAN message */
        uint32_t raw_value = (raw_data >> (64 - offset - length)) & bitmask;

        /* normalize endian */
        if (!mapping->big_endian) {
                raw_value = decode_little_endian_bitmode(raw_value, length);
        }

        /* convert type */
        switch (mapping->type) {
            case CANMappingType_unsigned:
                    return (float)raw_value;
        	case CANMappingType_signed:
                    if (length <= 8) {
                        return (float)*((int8_t*)&raw_value);
                    }
                    if (length <= 16){
                        return (float)*((int16_t*)&raw_value);
                    }
                    return (float)*((int32_t*)&raw_value);
            case CANMappingType_IEEE754:
                    return *((float*)&raw_value);
            case CANMappingType_sign_magnitude:
                    /**
                     *  sign-magnitude is used in cases where there's a sign bit
                     *  and an absolute value indicating magnitude.
                     *  e.g. BMW E46 steering angle sensor
                     **/
                    {
                            uint32_t sign = 1 << (length - 1);
                            return raw_value < sign ? (float)raw_value : -(float)(raw_value & (sign - 1));
                    }
        	default:
                    /* We reached an invalid enum */
                    panic(PANIC_CAUSE_UNREACHABLE);
                    return 0;
        }
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
        if (mapping->can_id == 0) return true;

        /* apply mask to CAN ID, if specified (non zero) */
        if (mapping->can_mask)
                can_id &= mapping->can_mask;

        /* If no match, give up. */
        if (can_id != mapping->can_id)
                return false;

        /* Match the sub ID index with the first data byte, if specified (>= 0) */
        return (mapping->sub_id < 0 || mapping->sub_id == can_msg->data[0]);
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
