/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "units.h"

#include <string.h>

#define ARRAY_LEN(x) (sizeof(x)/sizeof(*x))
#define UNIT(a,b) {(a), (b)}

static const struct unit units[] = {
        UNIT(UNIT_GROUP_LENGTH, "m"),
        UNIT(UNIT_GROUP_LENGTH, "Km"),
        UNIT(UNIT_GROUP_LENGTH, "ft"),
        UNIT(UNIT_GROUP_LENGTH, "mi"),

        UNIT(UNIT_GROUP_TEMPERATURE, "C"),
        UNIT(UNIT_GROUP_TEMPERATURE, "F"),

        UNIT(UNIT_GROUP_VOLUME, "mmm"),
        UNIT(UNIT_GROUP_VOLUME, "L"),
        UNIT(UNIT_GROUP_VOLUME, "mL"),
        UNIT(UNIT_GROUP_VOLUME, "gal"),

        UNIT(UNIT_GROUP_MASS, "Kg"),
        UNIT(UNIT_GROUP_MASS, "lb"),

        UNIT(UNIT_GROUP_PRESSURE, "Pa"),
        UNIT(UNIT_GROUP_PRESSURE, "psi"),
        UNIT(UNIT_GROUP_PRESSURE, "Atm"),
        UNIT(UNIT_GROUP_PRESSURE, "bar"),

        UNIT(UNIT_GROUP_SPEED, "m/s"),
        UNIT(UNIT_GROUP_SPEED, "Kph"),
        UNIT(UNIT_GROUP_SPEED, "Mph"),

        UNIT(UNIT_GROUP_TIME, "ms"),
        UNIT(UNIT_GROUP_TIME, "s"),
        UNIT(UNIT_GROUP_TIME, "min"),
        UNIT(UNIT_GROUP_TIME, "hr"),

        UNIT(UNIT_GROUP_POWER, "W"),
        UNIT(UNIT_GROUP_POWER, "hp"),

        UNIT(UNIT_GROUP_ENERGY, "J"),
        UNIT(UNIT_GROUP_ENERGY, "btu"),
        UNIT(UNIT_GROUP_ENERGY, "Whr"),
        UNIT(UNIT_GROUP_ENERGY, "KWh"),

        UNIT(UNIT_GROUP_FORCE, "N"),

        UNIT(UNIT_GROUP_TORQUE, "Nm"),
        UNIT(UNIT_GROUP_TORQUE, "lbft"),

        UNIT(UNIT_GROUP_ACCELERATION, "m/ss"),
        UNIT(UNIT_GROUP_ACCELERATION, "G"),

        UNIT(UNIT_GROUP_ANGLES, "rad"),
        UNIT(UNIT_GROUP_ANGLES, "deg"),

        UNIT(UNIT_GROUP_ANGULAR_SPEED, "rad/s"),
        UNIT(UNIT_GROUP_ANGULAR_SPEED, "deg/s"),
        UNIT(UNIT_GROUP_ANGULAR_SPEED, "rpm"),

        UNIT(UNIT_GROUP_PERCENTAGE, "%"),
};

const struct unit* units_get_unit(const char *name)
{
        if (NULL == name)
                return NULL;

        /* XXX: Replace with HASHMAP */
        const int arr_len = ARRAY_LEN(units);
        for (int i = 0; i < arr_len; ++i) {
                if (0 == strcmp(units[i].name, name))
                        return units + i;
        }

        return NULL;
}
