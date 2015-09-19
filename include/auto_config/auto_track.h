/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

#ifndef __AUTO_TRACK_H__
#define __AUTO_TRACK_H__

#include "geopoint.h"
#include "loggerConfig.h"
#include "tracks.h"

/**
 * Using 5KM as max distance from start finish as if you are farther than
 * that from start/finish then just wow.
 */
#define MAX_DIST_FROM_SF 5000

/**
 * Automatically picks the best track (if available) and updates the config to use this
 * track.
 * @param cfg Pointer to the config that will be used for the config info.
 * @param gp The GeoPoint that we got when we picked up our first good fix.
 * @return A status indicator that tells the caller the result of the call.
 */
const Track* auto_configure_track(const Track *defaultCfg, const GeoPoint *gp);

#endif // __AUTO_TRACK_H__
