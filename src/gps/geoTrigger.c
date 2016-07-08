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


#include "geoCircle.h"
#include "geoTrigger.h"
#include "gps.h"
#include <string.h>

#include <stdbool.h>

struct GeoTrigger createGeoTrigger(const struct GeoCircle *gc)
{
    struct GeoTrigger gt;

    memcpy(&gt.gc, gc, sizeof(struct GeoCircle));
    resetGeoTrigger(&gt);

    return gt;
}

bool updateGeoTrigger(struct GeoTrigger *gt, const GeoPoint *gp)
{
    if (gt->tripped) return true;
    if (gc_isPointInGeoCircle(gp, gt->gc)) return false;
    return gt->tripped = true;
}

void resetGeoTrigger(struct GeoTrigger *gt)
{
    gt->tripped = false;
}

bool isGeoTriggerTripped(const struct GeoTrigger *gt)
{
    return gt->tripped;
}

void geo_trigger_trip(struct GeoTrigger *gt)
{
        gt->tripped = true;
}
