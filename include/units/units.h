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

#ifndef _UNITS_H_
#define _UNITS_H_

#include "cpp_guard.h"

CPP_GUARD_BEGIN

enum unit_group {
        UNIT_GROUP_LENGTH,
        UNIT_GROUP_TEMPERATURE,
        UNIT_GROUP_VOLUME,
        UNIT_GROUP_MASS,
        UNIT_GROUP_PRESSURE,
        UNIT_GROUP_SPEED,
        UNIT_GROUP_TIME,
        UNIT_GROUP_POWER,
        UNIT_GROUP_ENERGY,
        UNIT_GROUP_ELECTROMOTIVE,
        UNIT_GROUP_CURRENT,
        UNIT_GROUP_FORCE,
        UNIT_GROUP_TORQUE,
        UNIT_GROUP_ACCELERATION,
        UNIT_GROUP_ANGLES,
        UNIT_GROUP_ANGULAR_VELOCITY,
        UNIT_GROUP_FREQUENCY,
        UNIT_GROUP_PERCENTAGE,
        UNIT_GROUP_COUNTING,
};

struct unit {
        const enum unit_group group;
        const char *name;
};

const struct unit* units_get_unit(const char *name);

CPP_GUARD_END

#endif /* _UNITS_H_ */
