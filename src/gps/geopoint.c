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
static float to_rad(float val)
{
    return val * (M_PI / 180.0);
}

/**
 * Converts a given value to degrees.
 * @param val The provided value in radian form.
 * @return The degree value.
 */
static float to_deg(float val)
{
    return val * (180.0 / M_PI);
}

float distPythag(const GeoPoint *a, const GeoPoint *b)
{
    const float dLatRad = to_rad(b->latitude - a->latitude);
    const float dLonRad = to_rad(b->longitude - a->longitude);
    const float latARad = to_rad(a->latitude);
    const float latBRad = to_rad(b->latitude);

    const float tmp = dLonRad * cos((latARad + latBRad) / 2);

    return sqrt(tmp * tmp + dLatRad * dLatRad) * GP_EARTH_RADIUS_M;
}

float gps_heading(const GeoPoint *last, const GeoPoint *curr)
{
        /*
         * Algorithm adapted from
         * http://www.movable-type.co.uk/scripts/latlong.html
         * Had to use this one as heading is a relative calculation.
         * Hopefully this doesn't overflow the float values.
         */

        const float d_lon = to_rad(curr->longitude) - to_rad(last->longitude);
        const float lat_curr = to_rad(curr->latitude);
        const float lat_last = to_rad(last->latitude);

        const float y = sin(d_lon) * cos(lat_curr);
        const float x = cos(lat_last) * sin(lat_curr) -
                sin(lat_last) * cos(lat_curr) * cos(d_lon);

        float res = to_deg(atan2(y, x));
        return res < 0 ? res + 360 : res;
}

int isValidPoint(const GeoPoint *p)
{
    return p->latitude != 0.0 || p->longitude != 0.0;
}
