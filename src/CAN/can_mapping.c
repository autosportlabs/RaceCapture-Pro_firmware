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
#include <byteswap.h>

float foo(void){
    return 33;
}

float map_value(CAN_msg *can_msg, CANMapping *mapping)
{
		uint8_t offset = mapping->offset;
		uint8_t length = mapping->length;
		if (! mapping->bit_mode) {
				length *= 8;
				offset *= 8;
		}
		uint32_t bitmask = (1 << length) - 1;
		uint32_t raw_value = (can_msg->data64  >> (7 - offset)) & bitmask;

		if (mapping->big_endian)
				raw_value = swap_uint32(raw_value);

		float value;
		value = (float)raw_value;

		value *= mapping->multiplier;
		if (mapping->divider)
				value /= mapping->divider;
		value += mapping->adder;

		return value;
}



