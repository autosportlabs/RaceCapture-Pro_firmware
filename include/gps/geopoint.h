/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#ifndef GEOPOINT_H_
#define GEOPOINT_H_

#include <stdbool.h>

typedef struct _GeoPoint {
    float latitude;
    float longitude;
} GeoPoint;

// Make into Enum?
#define GP_EARTH_RADIUS_KM	6371
#define GP_EARTH_RADIUS_M	6371000

/**
 * Finds the distance between the two geopoints using the
 * basic Pythagoras' Theorem.  This is only useful for small distances as
 * the inaccuracies will increase as the distance does.  However this is
 * also one of the fastest methods of calculating this distance.
 * @param a Point a
 * @param b Point b
 * @return The distance between the two points in Meters
 */
float distPythag(const GeoPoint *a, const GeoPoint *b);

/**
 * @return true if the given point is valid, false otherwise.
 */
int isValidPoint(const GeoPoint *p);

/**
 * Tests if both GeoPoint values are the same.
 * @return True if they are, false otherwise.
 */
bool are_geo_points_equal(const GeoPoint *p1, const GeoPoint *p2);

#endif /* GEOPOINT_H_ */
