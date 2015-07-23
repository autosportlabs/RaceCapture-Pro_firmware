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
 * @param val The provided value in degree form (0 - 360).
 * @return The radian value.
 */
static float toRad(float val)
{
    return val * (M_PI / 180.0);
}

/**
 * Converts a given value to degrees.
 * @param val The provided value in radian form.
 * @return The degree value.
 */
static float toDeg(float val)
{
    return val * (180.0 / M_PI);
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

float gps_bearing(const GeoPoint *last, const GeoPoint *curr)
{
        /*
         * Algorithm adapted from
         * http://www.movable-type.co.uk/scripts/latlong.html
         * Had to use this one as bearing is a relative calculation.
         * Hopefully this doesn't overflow the float values.
         */

        const float d_lon = toRad(curr->longitude) - toRad(last->longitude);
        const float lat_curr = toRad(curr->latitude);
        const float lat_last = toRad(last->latitude);

        const float y = sin(d_lon) * cos(lat_curr);
        const float x = cos(lat_last) * sin(lat_curr) -
                sin(lat_last) * cos(lat_curr) * cos(d_lon);

        float res = toDeg(atan2(y, x));
        return res < 0 ? res + 360 : res;
}

int isValidPoint(const GeoPoint *p)
{
    return p->latitude != 0.0 || p->longitude != 0.0;
}
