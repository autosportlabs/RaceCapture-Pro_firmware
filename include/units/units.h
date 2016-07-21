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

#ifndef _UNITS_H_
#define _UNITS_H_

#include "cpp_guard.h"
#include <stdbool.h>

CPP_GUARD_BEGIN

enum unit_type {
	UNIT_TYPE_UNKNOWN = 0,
	UNIT_TYPE_LENGTH,
	UNIT_TYPE_SPEED,
};

#define UNIT(type, id) ((type) << 8 | (id))

/**
 * Units enum is encoded.  Each unit enum has its unit type encoded
 * into the enum value.  This is to ensure that we are able to do
 * sane conversions down the road.  Also the 0th value of any unit
 * type is always the SI standard.
 */
enum unit {
	UNIT_UNKNOWN	= UNIT(UNIT_TYPE_UNKNOWN, 0),

	/* Length Units */
	UNIT_LENGTH_METERS 	= UNIT(UNIT_TYPE_LENGTH, 0),
	UNIT_LENGTH_KILOMETERS	= UNIT(UNIT_TYPE_LENGTH, 1),
	UNIT_LENGTH_FEET	= UNIT(UNIT_TYPE_LENGTH, 2),
	UNIT_LENGTH_MILES	= UNIT(UNIT_TYPE_LENGTH, 3),

	/* Speed Units */
	UNIT_SPEED_METERS_SECOND	= UNIT(UNIT_TYPE_SPEED, 0),
	UNIT_SPEED_KILOMETERS_HOUR	= UNIT(UNIT_TYPE_SPEED, 1),
	UNIT_SPEED_MILES_HOUR		= UNIT(UNIT_TYPE_SPEED, 2),
};

enum unit units_get_unit(const char* label);
const char* units_get_label(const enum unit unit);
bool units_is_label(const char* label);

CPP_GUARD_END

#endif /* _UNITS_H_ */
