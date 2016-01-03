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
#include "dateTime.h"
#include "geoCircle.h"
#include "geoTrigger.h"
#include "geopoint.h"
#include "gps.h"
#include "lap_stats.h"
#include "launch_control.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "modp_numtoa.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include "tracks.h"

#include <stdint.h>

/* Make the radius 3x the size of start/finish radius.*/
#define GEO_TRIGGER_RADIUS_MULTIPLIER 3
#define KMS_TO_MILES_CONSTANT (.621371)

// In Millis now.
#define START_FINISH_TIME_THRESHOLD 10000

#define TIME_NULL -1

static const Track *g_active_track;
static float g_geo_circle_radius;

track_status_t g_track_status = TRACK_STATUS_WAITING_TO_CONFIG;
static int g_sector_enabled = 0;
static int g_start_finish_enabled = 0;

static struct {
        struct GeoCircle start;
        struct GeoCircle finish;
        struct GeoCircle sector;
} g_geo_circles;

static int g_configured;
static int g_atStartFinish;
static tiny_millis_t g_lapStartTimestamp;
static tiny_millis_t g_elapsed_lap_time;

static int g_atTarget;
static tiny_millis_t g_lastSectorTimestamp;

static int g_sector;
static int g_lastSector;

static tiny_millis_t g_lastLapTime;
static tiny_millis_t g_lastSectorTime;

static int g_lap;
static int g_lapCount;
static float g_distance;

// Our GeoTriggers so we don't unintentionally trigger on start/finish
static struct GeoTrigger g_start_geo_trigger;
static struct GeoTrigger g_finish_geo_trigger;

void set_active_track(const Track *t)
{
        g_active_track = t;
}

static float degrees_to_meters(float degrees)
{
    // There are 110574.27 meters per degree of latitude at the equator.
    return degrees * 110574.27;
}

track_status_t lapstats_get_track_status(void)
{
    return g_track_status;
}

int32_t lapstats_get_selected_track_id(void)
{
        if (NULL == g_active_track)
                return 0;

        return g_active_track->trackId;
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

static void update_distance(const GpsSnapshot *gpsSnapshot)
{
    const GeoPoint prev = gpsSnapshot->previousPoint;
    const GeoPoint curr = gpsSnapshot->sample.point;

    if (!isValidPoint(&prev) || !isValidPoint(&curr))
        return;

    g_distance += distPythag(&prev, &curr) / 1000;
}

static void set_distance(const float distance)
{
    g_distance = distance;
}

void lapstats_reset_distance()
{
    set_distance(0);
}

float getLapDistance()
{
    return g_distance;
}

float getLapDistanceInMiles()
{
    return KMS_TO_MILES_CONSTANT * g_distance;
}

static void reset_current_lap()
{
    g_lap = 0;
}

int lapstats_current_lap()
{
    return g_lap;
}

void resetLapCount()
{
    g_lapCount = 0;
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

tiny_millis_t getLastLapTime()
{
    return g_lastLapTime;
}

float getLastLapTimeInMinutes()
{
    return tinyMillisToMinutes(getLastLapTime());
}

static void reset_elapsed_time()
{
    g_elapsed_lap_time = 0;
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
    return g_atStartFinish;
}

int getAtSector()
{
    return g_atTarget;
}

/**
 * Called whenever we finish a lap.
 */
static void lap_finished_event(const GpsSnapshot *gpsSnapshot)
{
        pr_debug_int_msg("Finished lap ", ++g_lapCount);

        end_lap_timing(gpsSnapshot);
        finishLap(gpsSnapshot);
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
        if (g_active_track->track_type == TRACK_TYPE_STAGE)
                resetGeoTrigger(&g_start_geo_trigger);
}

static void update_sector_geo_circle(const int sector)
{
        const GeoPoint point =
                getSectorGeoPointAtIndex(g_active_track, sector);
        g_geo_circles.sector = gc_createGeoCircle(point, g_geo_circle_radius);
}

static void lap_started_event(const tiny_millis_t time, const GeoPoint *sp,
                              const float distance)
{
        pr_debug_int_msg("Starting lap ", ++g_lap);

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

    pr_debug_int_msg("Sector boundary ", g_sector);

    g_lastSectorTime = millis - g_lastSectorTimestamp;
    g_lastSectorTimestamp = millis;
    g_lastSector = g_sector;
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

static void process_start_normal(const GpsSnapshot *gpsSnapshot)
{
        const GeoPoint point = gpsSnapshot->sample.point;
        if (!gc_isPointInGeoCircle(&point, g_geo_circles.start))
                return;

        const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
        lap_started_event(time, &point, 0);
}

static void process_start_launch_control(const GpsSnapshot *gpsSnapshot)
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
        const GeoPoint sp = getStartPoint(g_active_track);
        const GeoPoint gp = gpsSnapshot->sample.point;
        const float distance = distPythag(&sp, &gp) / 1000;
        lap_started_event(time, &sp, distance);
}

/**
 * All logic associated with determining if we are at the start line.
 */
static void process_start(const GpsSnapshot *gps_ss)
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
            g_active_track->track_type == TRACK_TYPE_CIRCUIT) {
                process_start_normal(gps_ss);
        } else {
                process_start_launch_control(gps_ss);
        }
}

static void process_sector_logic(const GpsSnapshot *gpsSnapshot)
{
        if (!g_sector_enabled)
                return;

        if (!lapstats_lap_in_progress())
                return;

        const GeoPoint point = gpsSnapshot->sample.point;
        g_atTarget = gc_isPointInGeoCircle(&point, g_geo_circles.sector);
        if (!g_atTarget)
                return;

        // If we are here, then we are at a Sector boundary.
        sector_boundary_event(gpsSnapshot);
}

void lapstats_config_changed(void)
{
    g_configured = 0;
    g_track_status = TRACK_STATUS_WAITING_TO_CONFIG;
}

void lapStats_init()
{
    lapstats_reset_distance();
    reset_elapsed_time();
    set_active_track(NULL);
    resetPredictiveTimer();
    reset_current_lap();
    g_geo_circle_radius = 0;
    g_configured = 0;
    g_lastLapTime = 0;
    g_lastSectorTime = 0;
    g_atStartFinish = 0;
    g_lapStartTimestamp = -1;
    g_atTarget = 0;
    g_lastSectorTimestamp = 0;
    g_lapCount = 0;
    g_sector = -1;     // Indicates we haven't crossed start/finish yet.
    g_lastSector = -1; // Indicates no previous sector.
}

static int isStartFinishEnabled(const Track *track)
{
    return isFinishPointValid(track) && isStartPointValid(track);
}

static int isSectorTrackingEnabled(const Track *track)
{
        if (!isStartFinishEnabled(track))
                return 0;

        const LoggerConfig *config = getWorkingLoggerConfig();
        if (SAMPLE_DISABLED == config->LapConfigs.sectorTimeCfg.sampleRate)
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
 * The GeoTriggers used here are effectively software de-bouncing
 * systems.  They ensure that we don't accidentally trigger an
 * event.  The tradeoff in using these is that we expect certain
 * minimum distances be traveled by the vehicle before debouncing
 * happens.  In this case we
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

static void lapstats_location_updated(const GpsSnapshot *gps_snapshot)
{
        update_distance(gps_snapshot);

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
        process_start(gps_snapshot);
}

static void setup_geo_circles()
{
        const GeoPoint start_p = getStartPoint(g_active_track);
        g_geo_circles.start = gc_createGeoCircle(start_p,
                                                 g_geo_circle_radius);

        const GeoPoint finish_p = getFinishPoint(g_active_track);
        g_geo_circles.finish = gc_createGeoCircle(finish_p,
                                                  g_geo_circle_radius);

        /* Set sector geo circle to first circle */
        update_sector_geo_circle(0);
}


/**
 * Useful for setting up internal state for unit tests.
 */
static void lapstats_setup_internals(const Track *track, const float gc_radius,
                                     const bool auto_detect)
{
        set_active_track(track);
        if (!track)
                return;

        g_geo_circle_radius = gc_radius;
        g_start_finish_enabled = isStartFinishEnabled(track);
        g_sector_enabled = isSectorTrackingEnabled(track);
        g_track_status = auto_detect ? TRACK_STATUS_AUTO_DETECTED :
                TRACK_STATUS_FIXED_CONFIG;

        setup_geo_triggers(track, g_geo_circle_radius);
        setup_geo_circles();

        lc_setup(track, g_geo_circle_radius);

        g_configured = 1;
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
        const GeoPoint *gp = &gps_snapshot->sample.point;

        const Track *track = NULL;
        const TrackConfig *trackConfig = &(config->TrackConfigs);
        if (trackConfig->auto_detect) {
                track = auto_configure_track(NULL, gp);
                if (track)
                        pr_info_int_msg("track: detected track ",
                                        track->trackId);
        } else {
                track = &trackConfig->track;
                pr_info("track: using fixed config");
        }

        const float gc_radius = degrees_to_meters(config->TrackConfigs.radius);
        lapstats_setup_internals(track, gc_radius, trackConfig->auto_detect);
}

void lapstats_processUpdate(const GpsSnapshot *gps_snapshot)
{
    if (isGpsDataCold())
            return; /* No valid GPS data to work with */

    if (!g_configured)
            lapstats_setup(gps_snapshot);

    if (g_configured)
            lapstats_location_updated(gps_snapshot);
}
