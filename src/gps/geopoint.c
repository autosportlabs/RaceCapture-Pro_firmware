/*
 * geopoint.c
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#include "geopoint.h"
#include "math.h"
#include "gps/gps.h"

#include <stdbool.h>

/**
 * Converts a given value to radians.
 * @param val The provided value in non-radian form.
 * @return The radian value.
 */
static float toRad(float val) {
	return val * M_PI / 180.0;
}

float distPythag(const GeoPoint *a, const GeoPoint *b) {
	float dLatRad = toRad(b->latitude - a->latitude);
	float dLonRad = toRad(b->longitude - a->longitude);
	float latARad = toRad(a->latitude);
	float latBRad = toRad(b->latitude);

	float tmp = dLonRad * cosf((latARad + latBRad) / 2);
	return sqrtf(tmp * tmp + dLatRad * dLatRad) * GP_EARTH_RADIUS_M;
}

int isValidPoint(const GeoPoint *p) {
   return p->latitude != 0.0 || p->longitude != 0.0;
}
