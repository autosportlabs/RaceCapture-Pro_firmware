/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#ifndef _LAP_STATS_TESTING_H_
#define _LAP_STATS_TESTING_H_

#include "lap_stats.h"
#include "tracks.h"

/*
 * Since these are methods that are not meant to be exposed outside the
 * normal runtime, they have been prefixed with "lapstats".
 */

void set_active_track(const Track *defaultTrack);
void lap_started_normal_event(const GpsSnapshot *gpsSnapshot);
void lap_finished_event(const GpsSnapshot *gpsSnapshot);
void update_elapsed_time(const GpsSnapshot *snap);
void reset_elapsed_time();
void reset_current_lap();

#endif /* _LAP_STATS_TESTING_H_ */
