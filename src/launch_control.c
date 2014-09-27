/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "launch_control.h"
#include "geopoint.h"
#include "gps.h"
#include "tracks.h"

#include <stdbool.h>

/*
 * Arbitrarily choosing 3MPH because if you are going faster than that,
 * you are driving/racing.
 */
#define LC_SPEED_THRESHOLD 3.0

unsigned long long g_startTime;
GeoPoint g_startPoint;
float g_targetRadius;
bool g_hasLaunched;

static bool isValidStartTime() {
   return g_startTime != 0ull;
}

static bool isConfigured() {
   return isValidPoint(&g_startPoint) && g_targetRadius > 0;
}

static bool isGeoPointInStartArea(const GeoPoint p) {
   return isPointInGeoCircle(p, g_startPoint, g_targetRadius);
}

static bool isSpeedBelowThreshold(const float speed) {
   return speed < LC_SPEED_THRESHOLD;
}

bool lc_hasLaunched() {
   return g_hasLaunched;
}

unsigned long long lc_getLaunchTime() {
   return lc_hasLaunched() ? g_startTime : 0ull;
}

void lc_reset() {
   g_startTime = 0ull;
   g_targetRadius = 0.0;
   g_hasLaunched = false;
}

void lc_setup(const Track *track, const float targetRadius) {
   lc_reset();
   g_startPoint = getStartPoint(track);
   g_targetRadius = targetRadius;
}

void lc_supplyGpsSample(const struct GpsSample sample) {
   if (!isConfigured() || lc_hasLaunched())
      return;

   if (isGeoPointInStartArea(sample.point)) {
      if (!isValidStartTime() || isSpeedBelowThreshold(sample.speed))
         g_startTime = sample.time;
   } else {
      g_hasLaunched = isValidStartTime();
   }
}
