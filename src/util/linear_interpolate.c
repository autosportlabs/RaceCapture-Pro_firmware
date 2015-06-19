/*
 * linear_interpolate.c
 *
 *  Created on: Jul 27, 2013
 *      Author: brent
 */
#include "linear_interpolate.h"


float LinearInterpolate(float x, float x1, float y1, float x2, float y2)
{
    return y1 + (((y2 - y1))  / (x2 - x1)) * (x - x1);
}
