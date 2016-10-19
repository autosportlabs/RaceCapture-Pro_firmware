/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "auto_track.h"
#include "convert.h"
#include "dateTime.h"
#include "geoCircle.h"
#include "geoTrigger.h"
#include "geopoint.h"
#include "gps.h"
#include "lap_stats.h"
#include "launch_control.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "modp_numtoa.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include "tracks.h"
#include <stdint.h>
#include <string.h>
#define _LOG_PFX            "[lapstats] "

/* Make the radius 2x the size of start/finish radius.*/
#define GEO_CIRCLE_RADIUS_MIN		1
#define GEO_TRIGGER_RADIUS_MULTIPLIER	2
#define MEASUREMENT_SPEED_MIN_KPH	1
/* In Millis */
#define START_FINISH_TIME_THRESHOLD 	10000
#define TIME_NULL -1

static Track g_active_track;
static float g_geo_circle_radius;

track_status_t g_track_status = TRACK_STATUS_WAITING_TO_CONFIG;
static int g_sector_enabled;
static int g_start_finish_enabled;

static struct {
        struct GeoCircle start;
        struct GeoCircle finish;
        struct GeoCircle sector;
} g_geo_circles;

static int g_configured;
static int g_at_sf;
static tiny_millis_t g_lapStartTimestamp = -1;
static tiny_millis_t g_elapsed_lap_time;

static int g_at_sector;
static tiny_millis_t g_lastSectorTimestamp;

static int g_sector = -1;
static int g_lastSector = -1;

static tiny_millis_t g_lastLapTime;
static tiny_millis_t g_lastSectorTime;

static int g_lap;
static int g_lapCount;
static float g_distance;

// Our GeoTriggers so we don't unintentionally trigger on start/finish
static struct GeoTrigger g_start_geo_trigger;
static struct GeoTrigger g_finish_geo_trigger;

static void reset_elapsed_time()
{
    g_elapsed_lap_time = 0;
}

void resetLapCount()
{
        g_lapCount = 0;
        g_lap = 0;
}

/**
 * This less invasive reset will cause all the stats to reset to their
 * default values. This DOES_NOT alter the track settings in any way.
 */
void lapstats_reset()
{
        g_at_sector = 0;
        g_at_sf = 0;
        g_lapStartTimestamp = -1;
        g_lastLapTime = 0;
        g_lastSector = -1; // Indicates no previous sector.
        g_lastSectorTime = 0;
        g_lastSectorTimestamp = 0;
        g_sector = -1;     // Indicates we haven't crossed start/finish yet.
        lapstats_reset_distance();
        resetPredictiveTimer();
        resetLapCount();
        reset_elapsed_time();
        lc_reset();
}

/**
 * Resets all the track information. This will cause us to become
 * unconfigured which may prompt an automatic reconfiguration.
 */
static void reset_track()
{
        /*
         * Reset items related to the track so that if the track supplied
         * is invalid we can still do basic stuff like keep track of
         * distance.
         */
        g_configured = 0;
        g_track_status = TRACK_STATUS_WAITING_TO_CONFIG;
        memset(&g_active_track, 0, sizeof(Track));

        g_geo_circle_radius = 0;
        g_start_finish_enabled = false;
        g_sector_enabled = false;
}

/**
 * The GeoTriggers used here are effectively software de-bouncing
 * systems.  They ensure that we don't accidentally trigger an
 * event too soon.  Think of them as a GPS based Schmitt-trigger.
 * The tradeoff in using these is that we expect certain
 * minimum distances be traveled by the vehicle before debouncing
 * happens.
 * @param track Pointer to the track that we are at.
 * @param radius Radius of the geo circles in meters.
 */
static void setup_geo_triggers(const Track *track, const float radius)
{
        GeoPoint gp;
        struct GeoCircle gc;

        gp = getStartPoint(track);
        gc = gc_createGeoCircle(gp, radius);
        g_start_geo_trigger = createGeoTrigger(&gc);

        /*
         * Trip start geo trigger as unit may get powered up in start
         * geo circle
         */
        geo_trigger_trip(&g_start_geo_trigger);

        gp = getFinishPoint(track);
        gc = gc_createGeoCircle(gp, radius);
        g_finish_geo_trigger = createGeoTrigger(&gc);
}

static void update_sector_geo_circle(const int sector)
{
        const GeoPoint point =
                getSectorGeoPointAtIndex(&g_active_track, sector);
        g_geo_circles.sector = gc_createGeoCircle(point, g_geo_circle_radius);
}

/**
 * Sets the start and finish geo circles used for start and finish lines
 * respectively.
 * @param track Pointer to the track where we are at.
 * @param radius Radius of the geo circles at this track in meters.
 */
static void setup_geo_circles(const Track *track, const float radius)
{
        const GeoPoint start_p = getStartPoint(track);
        g_geo_circles.start = gc_createGeoCircle(start_p, radius);

        const GeoPoint finish_p = getFinishPoint(track);
        g_geo_circles.finish = gc_createGeoCircle(finish_p, radius);

        /* Set sector geo circle to first circle */
        update_sector_geo_circle(0);
}

/**
 * Tells us if we have valid start and finish points for a given track.
 * @return true if we do, false otherwise.
 */
static bool isStartFinishEnabled(const Track *track)
{
        return isFinishPointValid(track) && isStartPointValid(track);
}

/**
 * Tells us if we have sectors defined for this track.
 * @return true if we do, false otherwise.
 */
static bool isSectorTrackingEnabled(const Track *track)
{
        if (!isStartFinishEnabled(track))
                return 0;

        /*
         * Must have >= one valid sector; must start at position 0.  Be careful
         * when using getSectorGeoPointAtIndex because if a point is in within
         * a valid range but is an invalid GPS value, it will return the finish
         * line (logical because the last sector boundary is always the finish
         * line).  So we must compare sector 0 against the finish line and if
         * they are the same, then we do not have sector values defined.
         */
        const GeoPoint sp0 = getSectorGeoPointAtIndex(track, 0);
        const GeoPoint fin = getFinishPoint(track);
        return !are_geo_points_equal(&fin, &sp0);
}


/**
 * Sets the active track that lap_stats uses for determining its various
 * statistics.
 * @param track Pointer to the track where we are at.
 * @param radius Radius of the geo circles at this track in meters.
 * @param track_status Status indicating who set the track value.
 * @return true if setup was successful, false otherwise.
 */
static bool set_active_track(const Track *track, const float radius,
                             const track_status_t track_status)
{
        /* We are changing our track, so we need to reset stats */
        lapstats_reset();
        reset_track();
        g_track_status = track_status;
        g_configured = 1;

        if (!track || radius < GEO_CIRCLE_RADIUS_MIN)
                return false;

        memcpy(&g_active_track, track, sizeof(Track));
        g_geo_circle_radius = radius;

        g_start_finish_enabled = isStartFinishEnabled(track);
        g_sector_enabled = isSectorTrackingEnabled(track);

        setup_geo_triggers(track, radius * GEO_TRIGGER_RADIUS_MULTIPLIER);
        setup_geo_circles(track, radius);
        lc_setup(track, radius);

        return true;
}

bool lapstats_set_active_track(const Track *track, const float radius)
{
        return set_active_track(track, radius, TRACK_STATUS_EXTERNALLY_SET);
}

/**
 * Basic method that calculates distances represented in degrees of latitude
 * at the equator into meters.  Only present to deal with legacy issues.
 */
float lapstats_degrees_to_meters(const float degrees)
{
        /*
         * LEGACY HACK. Our original Track object had the radius of
         * geoCircle measured in degrees!!! This method exists to convert
         * from that measurement to one that is measured in meters, a more
         * civilized unit of measure. I would love to murder the need to use
         * this in the future.
         * 110574.27 meters per degree of latitude at the equator.
         */
        return degrees * 110574.27;
}

/**
 * Informs the caller as to whether or not the currenctly configured
 * track (if one is configured at all) is valid. By valid we mean that
 * the track is capable of recording laps (valid start/finish points
 * are defined).
 * @return true if valid start/finish points exist, false otherwise.
 */
bool lapstats_is_track_valid()
{
        return !!g_start_finish_enabled;
}

track_status_t lapstats_get_track_status(void)
{
    return g_track_status;
}

int32_t lapstats_get_selected_track_id(void)
{
        return g_active_track.trackId;
}

bool lapstats_lap_in_progress()
{
    return g_lapStartTimestamp >= 0;
}

/**
 * Called when we start a lap.  Handles timing information.
 */
static void start_lap_timing(const tiny_millis_t startTime)
{
    g_lapStartTimestamp = startTime;
}

/**
 * Called when we finish a lap.  Handles timing information.
 */
static void end_lap_timing(const GpsSnapshot *gpsSnapshot)
{
    g_lastLapTime = gpsSnapshot->deltaFirstFix - g_lapStartTimestamp;
    g_lapStartTimestamp = -1;
}

static void update_distance(const GpsSnapshot *gps_ss)
{
        const float speed_avg =
                (gps_ss->sample.speed + gps_ss->previous_speed) / 2;

        /*
         * Filter out low speed measurements to prevent updates when
         * a vehicle is stationary or moving at trivial speed.
         */
        if (speed_avg < MEASUREMENT_SPEED_MIN_KPH)
                return;

        /*
         * Speed: KM/H
         * Delta ms: ms
         * KM/H * delta ms / 3600 = delta distance.
         */
        g_distance += speed_avg * gps_ss->delta_last_sample / 3600000.0;
}

static void set_distance(const float distance)
{
    g_distance = distance;
}

void lapstats_reset_distance()
{
    set_distance(0);
}

/* This distance is in km */
float getLapDistance()
{
	return g_distance;
}

float getLapDistanceInMiles()
{
	return convert_km_mi(g_distance);
}

int lapstats_current_lap()
{
    return g_lap;
}

int getLapCount()
{
    return g_lapCount;
}

int getSector()
{
    return g_sector;
}

int getLastSector()
{
    return g_lastSector;
}

bool lapstats_track_has_sectors()
{
        return g_sector_enabled;
}

float lapstats_get_geo_circle_radius()
{
        return g_geo_circle_radius;
}

tiny_millis_t getLastLapTime()
{
    return g_lastLapTime;
}

float getLastLapTimeInMinutes()
{
    return tinyMillisToMinutes(getLastLapTime());
}

void update_elapsed_time(const GpsSnapshot *snap)
{
    if (!lapstats_lap_in_progress())
        return;

    g_elapsed_lap_time = snap->deltaFirstFix - g_lapStartTimestamp;
}

tiny_millis_t lapstats_elapsed_time()
{
    return g_elapsed_lap_time;
}

float lapstats_elapsed_time_minutes()
{
    return tinyMillisToMinutes(lapstats_elapsed_time());
}

tiny_millis_t getLastSectorTime()
{
    return g_lastSectorTime;
}

float getLastSectorTimeInMinutes()
{
    return tinyMillisToMinutes(getLastSectorTime());
}

int getAtStartFinish()
{
    return g_at_sf;
}

int getAtSector()
{
    return g_at_sector;
}

/**
 * Called whenever we finish a lap.
 */
static void lap_finished_event(const GpsSnapshot *gpsSnapshot)
{
        pr_debug_int_msg(_LOG_PFX "Finished lap ", ++g_lapCount);

        end_lap_timing(gpsSnapshot);
        finishLap(gpsSnapshot);
        g_at_sf = true;
        lc_reset();

        /*
         * In Stage mode, enforce a de-bounce trigger for start to ensure
         * we don't accidentally trigger a false start if the finish line
         * and start line are close together.
         *
         * We only do this in STAGE mode becuse in circuit mode the start
         * line _is_ the finish line and thus resetting this trigger in
         * CIRCUIT mode would cause our start logic to function improperly.
         */
        if (g_active_track.track_type == TRACK_TYPE_STAGE)
                resetGeoTrigger(&g_start_geo_trigger);
}

static void lap_started_event(const tiny_millis_t time, const GeoPoint *sp,
                              const float distance)
{
        pr_debug_int_msg(_LOG_PFX "Starting lap ", ++g_lap);
        g_at_sf = true;

        // Timing and predictive timing
        start_lap_timing(time);
        startLap(sp, time);
        reset_elapsed_time();

        // Reset the sector logic
        g_lastSectorTimestamp = time;
        g_sector = 0;
        update_sector_geo_circle(g_sector);

        // Reset distance logic
        set_distance(distance);

        /*
         * Reset our finishGeoTrigger so that we get away from
         * finish before we re-arm the system.
         */
        resetGeoTrigger(&g_finish_geo_trigger);
}

/**
 * Called whenever we have hit a sector boundary.
 */
static void sector_boundary_event(const GpsSnapshot *gpsSnapshot)
{
    const tiny_millis_t millis = gpsSnapshot->deltaFirstFix;

    pr_debug_int_msg(_LOG_PFX "Sector boundary ", g_sector);

    g_lastSectorTime = millis - g_lastSectorTimestamp;
    g_lastSectorTimestamp = millis;
    g_lastSector = g_sector;
    g_at_sector = true;
    update_sector_geo_circle(++g_sector);
}

/**
 * All logic associated with determining if we are at the finish line.
 */
static void process_finish_logic(const GpsSnapshot *gpsSnapshot)
{
    if (!lapstats_lap_in_progress())
        return;

    if (!isGeoTriggerTripped(&g_finish_geo_trigger))
        return;

    const GeoPoint point = gpsSnapshot->sample.point;
    if (!gc_isPointInGeoCircle(&point, g_geo_circles.finish))
        return;

    // If we get here, then we have completed a lap.
    lap_finished_event(gpsSnapshot);
}

static void process_start_logic_no_lc(const GpsSnapshot *gpsSnapshot)
{
        const GeoPoint point = gpsSnapshot->sample.point;
        if (!gc_isPointInGeoCircle(&point, g_geo_circles.start))
                return;

        const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
        lap_started_event(time, &point, 0);
}

static void process_start_logic_with_lc(const GpsSnapshot *gpsSnapshot)
{
        lc_supplyGpsSnapshot(gpsSnapshot);
        if (!lc_hasLaunched())
                return;

        /*
         * If here, then launch control has determined that the lap
         * has begun.  Launch control unfortunately causes a delay in
         * reporting since it has to detect a launch before it can
         * report it.  Hence we must retroactively calculate the
         * distance we have traveled and the time elapsed since we
         * launched from start as it is before this point in time.
         */
        const tiny_millis_t time = lc_getLaunchTime();
        const GeoPoint sp = getStartPoint(&g_active_track);
        const GeoPoint gp = gpsSnapshot->sample.point;
        const float distance = distPythag(&sp, &gp) / 1000;
        lap_started_event(time, &sp, distance);
}

/**
 * All logic associated with determining if we are at the start line.
 */
static void process_start_logic(const GpsSnapshot *gps_ss)
{
        if (lapstats_lap_in_progress())
                return;

        if (!isGeoTriggerTripped(&g_start_geo_trigger))
                return;

        /*
         * If we have crossed start finish at least once and we are a circuit
         * track, then we need to disable launch control to prevent hiccups
         * in reporting.
         */
        if (g_lapCount > 0 &&
            g_active_track.track_type == TRACK_TYPE_CIRCUIT) {
                process_start_logic_no_lc(gps_ss);
        } else {
                process_start_logic_with_lc(gps_ss);
        }
}

static void process_sector_logic(const GpsSnapshot *gpsSnapshot)
{
        if (!g_sector_enabled)
                return;

        if (!lapstats_lap_in_progress())
                return;

        const GeoPoint point = gpsSnapshot->sample.point;
        g_at_sector = gc_isPointInGeoCircle(&point, g_geo_circles.sector);
        if (!g_at_sector)
                return;

        // If we are here, then we are at a Sector boundary.
        sector_boundary_event(gpsSnapshot);
}

void lapstats_config_changed(void)
{
        lapstats_reset();
        reset_track();
}

static void lapstats_location_updated(const GpsSnapshot *gps_snapshot)
{
        update_distance(gps_snapshot);

        /* Reset at_* flags on every sample. */
        g_at_sf = false;
        g_at_sector = false;

        if (!g_start_finish_enabled)
                return;

        /* Process data fields first. */
        const GeoPoint *gp = &gps_snapshot->sample.point;
        updateGeoTrigger(&g_start_geo_trigger, gp);
        updateGeoTrigger(&g_finish_geo_trigger, gp);
        update_elapsed_time(gps_snapshot);
        addGpsSample(gps_snapshot);

        /*
         * Now process the sector, finish and start logic in that order.
         * Each processing can invoke their respective event if the logic
         * agrees its time.
         */
        process_sector_logic(gps_snapshot);
        process_finish_logic(gps_snapshot);
        process_start_logic(gps_snapshot);
}

static void lapstats_setup(const GpsSnapshot *gps_snapshot)
{
        /*
         * I am not a fan of this method.  Really this track config stuff should
         * get moved over into its own file with its own separate testing.  It
         * should generate an event when a track is detected so that we can listen
         * and act accordingly.  For now it stays here.
         */
        const LoggerConfig *config = getWorkingLoggerConfig();
        const TrackConfig *trackConfig = &(config->TrackConfigs);
        const bool auto_detect_track = trackConfig->auto_detect;
        const GeoPoint *gp = &gps_snapshot->sample.point;

        /* If we are using auto-detect and have no valid point, we are done */
        if (auto_detect_track && !isValidPoint(gp))
                return;

        const float radius_in_meters =
                lapstats_degrees_to_meters(config->TrackConfigs.radius);
        const Track *track = NULL;
        track_status_t track_status;
        if (auto_detect_track) {
                track_status = TRACK_STATUS_AUTO_DETECTED;
                track = auto_configure_track(&trackConfig->track, gp);
                if (track != &trackConfig->track) {
                        pr_info_int_msg(_LOG_PFX "Auto-detected track from db ",
                                        track->trackId);
                } else {
                        bool track_db_exists = (get_tracks()->count > 0);
                        if (track_db_exists) {
                                track_status = TRACK_STATUS_FIXED_CONFIG;
                                pr_info_int_msg(_LOG_PFX "Could not find track in db, falling back to fixed config ", track->trackId);
                        } else {
                                pr_info_int_msg(_LOG_PFX "Using track config ", track->trackId);
                        }
                }
        } else {
                track_status = TRACK_STATUS_FIXED_CONFIG;
                track = &trackConfig->track;
                pr_info_int_msg(_LOG_PFX "Using fixed track config ", track->trackId);
        }

        set_active_track(track, radius_in_meters, track_status);
}

void lapstats_processUpdate(const GpsSnapshot *gps_snapshot)
{
	if (!g_configured)
		lapstats_setup(gps_snapshot);

	if (isGpsDataCold())
		return; /* No valid GPS data to work with */

	if (g_configured)
		lapstats_location_updated(gps_snapshot);
}
