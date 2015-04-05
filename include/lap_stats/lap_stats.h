#ifndef LAP_STATS_H_
#define LAP_STATS_H_

#include "dateTime.h"
#include "geopoint.h"
#include <stddef.h>
#include "gps.h"

/**
 * A simple Time and Location sample.
 */
typedef struct _TimeLoc {
	GeoPoint point;
	millis_t time;
} TimeLoc;

void gpsConfigChanged(void);

void lapStats_init();

void lapStats_processUpdate(const GpsSnapshot *gpsSnapshot);

void resetLapCount();

int getLapCount();

tiny_millis_t getLastLapTime();

float getLastLapTimeInMinutes();

tiny_millis_t getElapsedLapTime();

float getElapsedLapTimeInMinutes();

tiny_millis_t getLastSectorTime();

float getLastSectorTimeInMinutes();

int getSector();

int getLastSector();

int getAtStartFinish();

int getAtSector();

void resetLapDistance();

float getLapDistance();

float getLapDistanceInMiles();

#endif /* LAP_STATS_H_ */
