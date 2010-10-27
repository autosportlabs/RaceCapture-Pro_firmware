#include <math.h>
#include "geometry.h"

void create_circ(struct circ_area *area,
		 const struct point *center,
		 float radius)
{
	area->center = *center;
	area->rsquared = radius * radius;
}

int within_circ(const struct circ_area *area,
		const struct point *point)
{
	float xdelta;
	float ydelta;
	float dsquared;

	xdelta = area->center.x - point->x;
	ydelta = area->center.y - point->y;
	dsquared = (xdelta * xdelta) + (ydelta * ydelta);

	return area->rsquared >= dsquared;
}

static void translate_point(struct point *finish,
			    const struct point *start,
			    float slope,
			    float length)
{
	float xdelta;
	float ydelta;

	if (!isinf(slope)) {
		xdelta = length / sqrtf((slope * slope) + 1.0);
		ydelta = xdelta * slope;
	} else {
		xdelta = 0.0;
		ydelta = length;
	}

	finish->x = start->x + xdelta;
	finish->y = start->y + ydelta;
}

static void create_parallel(struct parallel *parallel,
			    const struct point points[2],
			    float slope)
{
	float icepts[2];
	unsigned int i;
	unsigned int index;

	if (!isinf(slope)) {
		for (i = 0; i < 2; i++)
			icepts[i] = points[i].y - points[i].x * slope;
	} else {
		for (i = 0; i < 2; i++)
			icepts[i] = points[i].x;
	}

	index = icepts[0] < icepts[1];
	for (i = 0; i < PARALLEL_ICEPT_MAX; i++) {
		parallel->icepts[i] = icepts[index];
		index ^= 1;
	}

	if (!isinf(slope)) {
		parallel->slope = slope;
		parallel->ymask = -1;
	} else {
		parallel->slope = -1.0;
		parallel->ymask = 0;
	}
}

void create_quad(struct quad_area *area,
		 const struct point base[2],
		 float length,
		 enum geometry_direction dir)
{
	float slopes[2];
	struct point points[3];
	unsigned int i;

	slopes[0] = (base[1].y - base[0].y) / (base[1].x - base[0].x);
	slopes[1] = -1.0 / slopes[0];
	dir *= -(base[1].y < base[0].y) | 1;

	points[0] = base[0];
	points[1] = base[1];
	translate_point(&points[2], &points[1], slopes[1], length * dir);

	for (i = 0; i < sizeof(area->lines) / sizeof(*area->lines); i++)
		create_parallel(area->lines + i, points + i, slopes[i ^ 1]);
}

static inline float mask_float(float value,
			       unsigned int mask)
{
	union {
		float f;
		unsigned int i;
	} u = { .f = value };

	u.i &= mask;

	return u.f;
}

int within_quad(const struct quad_area *area,
		const struct point *point)
{
	unsigned int i;

	for (i = 0; i < sizeof(area->lines) / sizeof(*area->lines); i++) {
		const struct parallel *line;
		float icept;

		line = area->lines + i;
		icept = mask_float(point->y, line->ymask) -
			line->slope * point->x;

		if ((line->icepts[PARALLEL_ICEPT_HI] < icept) |
		    (line->icepts[PARALLEL_ICEPT_LO] > icept))
			return 0;
	}

	return 1;
}

