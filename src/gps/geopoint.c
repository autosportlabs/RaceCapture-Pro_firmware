/*
 * geopoint.c
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#include "geopoint.h"
#include "gps.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Converts a given value to radians.
 * @param val The provided value in non-radian form.
 * @return The radian value.
 */
static float toRad(float val)
{
    return val * (M_PI / 180.0);
}

float distPythag(const GeoPoint *a, const GeoPoint *b)
{
    const float dLatRad = toRad(b->latitude - a->latitude);
    const float dLonRad = toRad(b->longitude - a->longitude);
    const float latARad = toRad(a->latitude);
    const float latBRad = toRad(b->latitude);

    const float tmp = dLonRad * cos((latARad + latBRad) / 2);

    return sqrt(tmp * tmp + dLatRad * dLatRad) * GP_EARTH_RADIUS_M;
}

int isValidPoint(const GeoPoint *p)
{
    return p->latitude != 0.0 || p->longitude != 0.0;
}
