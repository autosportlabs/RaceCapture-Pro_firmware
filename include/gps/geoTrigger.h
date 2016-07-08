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

#ifndef _GEOTRIGGER_H_
#define _GEOTRIGGER_H_

#include "cpp_guard.h"
#include "geoCircle.h"
#include "geopoint.h"
#include <string.h>

#include <stdbool.h>

CPP_GUARD_BEGIN

struct GeoTrigger {
    struct GeoCircle gc;
    bool tripped;
};

/**
 * Creates a new #GeoTrigger object.
 */
struct GeoTrigger createGeoTrigger(const struct GeoCircle *gc);

/**
 * Used to update the state of the object by providing a current
 * #GpsPoint.
 * @return true if tripped, false otherwise.
 */
bool updateGeoTrigger(struct GeoTrigger *gt, const GeoPoint *point);

/**
 * Resets the tripped variable on the #GeoTrigger without destroying the
 * other data.
 */
void resetGeoTrigger(struct GeoTrigger *gt);

/**
 * @return true if the trigger has been tripped, false otherwise.
 */
bool isGeoTriggerTripped(const struct GeoTrigger *gt);

/**
 * Trips a GeoTrigger object.
 */
void geo_trigger_trip(struct GeoTrigger *gt);

CPP_GUARD_END

#endif /* _GEOTRIGGER_H_ */
