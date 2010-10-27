#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

struct point {
	float x;
	float y;
};

enum parallel_icept {
	PARALLEL_ICEPT_HI = 0,
	PARALLEL_ICEPT_LO = 1,
	PARALLEL_ICEPT_MAX = 2,
};

struct parallel {
	float icepts[PARALLEL_ICEPT_MAX];
	float slope;
	unsigned int ymask;
};

struct circ_area {
	struct point center;
	float rsquared;
};

struct quad_area {
	struct parallel lines[2];
};

enum geometry_direction {
	GEOMETRY_DIRECTION_L = -1,
	GEOMETRY_DIRECTION_R =  1,
};

void create_circ(struct circ_area *area,
		 const struct point *center,
		 float radius);
int within_circ(const struct circ_area *area,
		const struct point *point);

void create_quad(struct quad_area *area,
		 const struct point base[2],
		 float length,
		 enum geometry_direction dir);
int within_quad(const struct quad_area *area,
		const struct point *point);

#endif

