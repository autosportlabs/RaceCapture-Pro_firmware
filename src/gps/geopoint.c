/*
 * geopoint.c
 *
 *  Created on: Jan 20, 2014
 *      Author: stieg
 */

#include "geopoint.h"
#include "gps.h"

#define M_PI 3.14159265358979323846

/*
 * Fast Square root implementation
 * Sources:
 * http://en.wikipedia.org/wiki/Fast_inverse_square_root
 * https://github.com/bnoordhuis/bspc/blob/master/deps/qcommon/cm_trace.c
 * http://h14s.p5r.org/2012/09/0x5f3759df.html
 */
static double fast_square_root(double number) {
	long i;
	double x, y;
	const double f = 1.5F;

	x = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y; // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 ); // what the fuck?
	y  = * ( double * ) &i;
	y  = y * ( f - ( x * y * y ) );
	y  = y * ( f - ( x * y * y ) );
	return number * y;
}

/*
 * source: http://www.ganssle.com/articles/atrig.htm
 */
double cosine(double x)
{
	double p0,p1,p2,p3,p4,p5,y,t,absx,frac,quad,pi2;
	t= 0.0;
	p0= 0.999999999781;
	p1=-0.499999993585;
	p2= 0.041666636258;
	p3=-0.0013888361399;
	p4= 0.00002476016134;
	p5=-0.00000026051495;
	pi2=1.570796326794896; 		/* pi/2 */
	absx=x;
	if (x<0) absx=-absx; 	     /* absolute value of input */
	quad=(int) (absx/pi2);       	/* quadrant (0 to 3) */
	frac= (absx/pi2) - quad;     	/* fractional part of input */
	if(quad==0) t=frac * pi2;
	if(quad==1) t=(1-frac) * pi2;
	if(quad==2) t=frac * pi2;
	if(quad==3) t=(frac-1) * pi2;
	t=t * t;
	y=p0 + (p1*t) + (p2*t*t) + (p3*t*t*t) + (p4*t*t*t*t) + (p5*t*t*t*t*t);
	if((quad==2) | (quad==1)) y=-y;  /* correct sign */
	return(y);
}

/**
 * Converts a given value to radians.
 * @param val The provided value in non-radian form.
 * @return The radian value.
 */
static double toRad(double val) {
	return val * (M_PI / 180.0);
}

float distPythag(const GeoPoint *a, const GeoPoint *b) {
	double dLatRad = toRad(b->latitude - a->latitude);
	double dLonRad = toRad(b->longitude - a->longitude);
	double latARad = toRad(a->latitude);
	double latBRad = toRad(b->latitude);

	double  tmp = dLonRad * cosine((latARad + latBRad) / 2);
	return (float)fast_square_root(tmp * tmp + dLatRad * dLatRad) * GP_EARTH_RADIUS_M;
}

int isValidPoint(const GeoPoint *p) {
   return p->latitude != 0.0 || p->longitude != 0.0;
}
