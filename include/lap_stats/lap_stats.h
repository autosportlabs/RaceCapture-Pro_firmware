/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAP_STATS_H_
#define LAP_STATS_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include "geopoint.h"
#include "gps.h"
#include "tracks.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

CPP_GUARD_BEGIN

typedef enum {
        TRACK_STATUS_WAITING_TO_CONFIG = 0,
        TRACK_STATUS_FIXED_CONFIG,
        TRACK_STATUS_AUTO_DETECTED,
        TRACK_STATUS_EXTERNALLY_SET,
} track_status_t;

/**
 * A simple Time and Location sample.
 */
typedef struct _TimeLoc {
    GeoPoint point;
    millis_t time;
} TimeLoc;

void lapstats_config_changed(void);

void lapstats_reset(void);

void lapstats_processUpdate(const GpsSnapshot *gpsSnapshot);

track_status_t lapstats_get_track_status( void );

bool lapstats_is_track_valid();

int32_t lapstats_get_selected_track_id( void );

void resetLapCount();

/**
 * @return The lap you are currently on.
 */
int lapstats_current_lap();

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

bool lapstats_set_active_track(const Track *track, const float radius);

/**
 * @return True if we are in the middle of a lap.  False otherwise.
 */
bool lapstats_lap_in_progress();

float lapstats_degrees_to_meters(const float degrees);

float lapstats_get_geo_circle_radius();

bool lapstats_track_has_sectors();

CPP_GUARD_END

#endif /* LAP_STATS_H_ */
