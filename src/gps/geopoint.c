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

float distSLCos(GeoPoint *a, GeoPoint *b) {
	float latARad = toRad(a->latitude);
	float latBRad = toRad(b->latitude);
	float lonARad = toRad(a->longitude);
	float lonBRad = toRad(b->longitude);

	float tmp = sinf(latARad) * sinf(latBRad)
			+ cosf(latARad) * cosf(latBRad) * cosf(lonBRad - lonARad);
	return acosf(tmp) * GP_EARTH_RADIUS_M;
}

float distHaversine(GeoPoint *a, GeoPoint *b) {
	float dLatRad = toRad(b->latitude - a->latitude);
	float dLonRad = toRad(b->longitude - a->longitude);
	float latARad = toRad(a->latitude);
	float latBRad = toRad(b->latitude);

	float sinDLatRad = sinf(dLatRad);
	float sinDLonRad = sinf(dLonRad);
	float tmp = sinDLatRad * sinDLatRad
			+ sinDLonRad * sinDLonRad * cosf(latARad) * cosf(latBRad);
	return 2 * atan2f(sqrtf(tmp), sqrtf(1 - tmp)) * GP_EARTH_RADIUS_M;
}

bool isPointInGeoCircle(const GeoPoint point, const GeoPoint center,
                        const float radius) {
   return distPythag(&point, &center) <= radius;
}
