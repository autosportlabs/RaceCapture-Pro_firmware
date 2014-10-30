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

#ifndef GPS_TESTING_H_
#define GPS_TESTING_H_

#include "gps.h"

/**
 * Performs a full update of the g_dtLastFix value.  Also update the g_dtFirstFix value if it hasn't
 * already been set.
 * @param fixDateTime The DateTime of the GPS fix.
 */
void updateFullDateTime(DateTime fixDateTime);

/**
 * Like atoi, but is non-destructive to the string passed in and provides an offset and length
 * functionality.  Max Len is 3.
 * @param str The start of the String to parse.
 * @param offset How far in to start reading the string.
 * @param len The number of characters to read.
 */
int atoiOffsetLenSafe(const char *str, size_t offset, size_t len);

/**
 * Updates the milliseconds since Unix Epoch
 * @param fixDateTime The Full Time and Date as provided by the GPS unit.
 */
void updateMillisSinceEpoch(DateTime fixDateTime);

#endif /* GPS_TESTING_H_ */
