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

#include "channel_config.h"
#include "units.h"
#include "macros.h"
#include <stdbool.h>

#define UNIT_LABEL(u, l) { .unit = (u), .label = (l) }

static const struct {
	enum unit unit;
	const char label[DEFAULT_UNITS_LENGTH];
} unit_label_map[] = {
	/* Length Units Map */
	UNIT_LABEL(UNIT_LENGTH_METERS, "m"),
	UNIT_LABEL(UNIT_LENGTH_KILOMETERS, "km"),
	UNIT_LABEL(UNIT_LENGTH_FEET, "ft"),
	UNIT_LABEL(UNIT_LENGTH_MILES, "mi"),

	/* Speed Units Map */
	UNIT_LABEL(UNIT_SPEED_METERS_SECOND, "m/s"),
	UNIT_LABEL(UNIT_SPEED_KILOMETERS_HOUR, "kph"),
	UNIT_LABEL(UNIT_SPEED_MILES_HOUR, "mph"),
};

/**
 * Searches our internal unit label mapping for the unit associated with
 * the given unit label.
 * @param label The unit label to search for.
 * @return The unit if found, UNIT_UNKNOWN otherwise.
 */
enum unit units_get_unit(const char* label)
{
	if (!label)
		return UNIT_UNKNOWN;

	for (int i = 0; i < ARRAY_LEN(unit_label_map); ++i) {
		if (STR_EQ(unit_label_map[i].label, label))
			return unit_label_map[i].unit;
	}

	return UNIT_UNKNOWN;
}

/**
 * Searches our internal unit label mapping for the unit label associated
 * with the given unit.
 * @param unit The unit to search for.
 * @return The unit label if found, NULL otherwise.
 */
const char* units_get_label(const enum unit unit)
{
	for (int i = 0; i < ARRAY_LEN(unit_label_map); ++i) {
		if (unit_label_map[i].unit == unit)
			return unit_label_map[i].label;
	}

	return NULL;
}

/**
 * Checks that the given label maps to a real unit internally.
 * @return true if it does, false otherwise.
 */
bool units_is_label(const char* label)
{
	return UNIT_UNKNOWN != units_get_unit(label);
}
