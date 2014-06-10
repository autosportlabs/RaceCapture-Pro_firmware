/*
 * RaceCapture: gps.testing.h
 *
 *  Created on: May 3, 2014
 *      Author: stieg
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
