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

#include <stdio.h>

float extract_value(uint64_t raw_data, CANMapping *mapping)
{
	uint8_t offset = mapping->offset;
	uint8_t length = mapping->length;
	if (! mapping->bit_mode) {
			length *= 8;
			offset *= 8;
	}
	uint32_t bitmask = (1 << length) - 1;
	uint32_t raw_value = (raw_data >> offset) & bitmask;

	printf("blah %d %d %d\r\n", offset, bitmask,  raw_value);
	if (mapping->big_endian) {
			printf("big endian %i\r\n", mapping->big_endian);
			raw_value = swap_uint32(raw_value);
	}
	return (float)raw_value;
}

float apply_formula(float value, CANMapping *mapping)
{
		value *= mapping->multiplier;
		if (mapping->divider)
				value /= mapping->divider;
		value += mapping->adder;
		return value;
}

float map_value(CAN_msg *can_msg, CANMapping *mapping)
{
		uint64_t raw_data = can_msg->data64;
		float value = extract_value(raw_data, mapping);
		value = apply_formula(value, mapping);
		return value;
}
