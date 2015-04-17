#ifndef LAP_STATS_H_
#define LAP_STATS_H_

#include "dateTime.h"
#include "geopoint.h"
#include <stddef.h>
#include <stdint.h>
#include "gps.h"

typedef enum {
	TRACK_STATUS_WAITING_TO_CONFIG = 0,
	TRACK_STATUS_FIXED_CONFIG,
	TRACK_STATUS_AUTO_DETECTED
} track_status_t;

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

track_status_t lapstats_get_track_status( void );

int32_t lapstats_get_selected_track_id( void );

void resetLapCount();

/**
 * @return The lap you are currently on.
 */
int lapstats_get_lap();

int getLapCount();

tiny_millis_t getLastLapTime();

float getLastLapTimeInMinutes();

/**
 * @return The elapsed lap time in milliseconds.
 */
tiny_millis_t lapstats_elapsed_time();

/**
 * @return The elapsed lap time in minutes.
 */
float lapstats_elapsed_time_minutes();

tiny_millis_t getLastSectorTime();

float getLastSectorTimeInMinutes();

int getSector();

int getLastSector();

int getAtStartFinish();

int getAtSector();

void lapstats_reset_distance();

float getLapDistance();

float getLapDistanceInMiles();

/**
 * @return True if we are in the middle of a lap.  False otherwise.
 */
bool lapstats_lap_in_progress();

#endif /* LAP_STATS_H_ */
