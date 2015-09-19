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

#ifndef _LAUNCH_CONTROL_H_
#define _LAUNCH_CONTROL_H_

#include "gps.h"
#include "tracks.h"

#include <stdbool.h>

/**
 * Resets internal state.
 */
void lc_reset();

/**
 * Sets up the needed information to allow launch control to work.
 * @param track Pointer to the track
 * @param targetRadius The radius of the target circle in Meters
 */
void lc_setup(const Track *track, const float targetRadius);

/**
 * Called when a new GpsSnapshot is available.
 * @param snap The GpsSnapshot.
 */
void lc_supplyGpsSnapshot(const GpsSnapshot *snap);

/**
 * @return true if the driver has launched (started racing).  False otherwise.
 */
bool lc_hasLaunched();

/**
 * @return true if launch control is armed but the racer hasn't launched yet,
 *         false otherwise.
 */
bool lc_is_armed();

/**
 * @return The time when the driver started racing.
 */
tiny_millis_t lc_getLaunchTime();

#endif /* _LAUNCH_CONTROL_H_ */
