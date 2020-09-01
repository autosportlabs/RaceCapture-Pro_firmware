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
#include "gps_device.h"
#include "imu_gsum.h"
#include "macros.h"
#include "math.h"
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
#include "taskUtil.h"

#define _LOG_PFX            "[lapstats] "

/* Make the radius 2x the size of start/finish radius.*/
#define GEO_CIRCLE_RADIUS_MIN		1
#define GEO_TRIGGER_RADIUS_MULTIPLIER	2
#define MEASUREMENT_SPEED_MIN_KPH	1
/* In Millis */
#define START_FINISH_TIME_THRESHOLD 	10000
#define FINISH_TRIGGER_MINIMUM_DISTANCE_KM 0.1f
#define TIME_NULL -1

/* Threshold where we start interpolating between GPS samples. 10Hz */
#define INTERPOLATION_THRESHOLD_MS 100

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
static tiny_millis_t g_session_time = 0;

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
static struct GeoTrigger g_start_geo_trigger = {0};
static struct GeoTrigger g_finish_geo_trigger = {0};

static float last_speed = 0;
static float current_speed = 0;
static size_t last_distance_sample_at = 0;

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
void lapstats_reset(bool reset_session)
{
        track_config_sanitize();
        lap_config_sanitize();
        if (reset_session)
                g_session_time = getUptime();
        g_distance = 0;
        last_speed = 0;
        current_speed = 0;
        last_distance_sample_at = 0;
        g_at_sector = 0;
        g_at_sf = 0;
        g_lapStartTimestamp = -1;
        g_lastLapTime = 0;
        g_lastSector = -1; // Indicates no previous sector.
        g_lastSectorTime = 0;
        g_lastSectorTimestamp = 0;
        g_sector = -1;     // Indicates we haven't crossed start/finish yet.
        lapstats_reset_distance();
	reset_gsum_maxes();
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
        lapstats_reset(false);
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
        return degrees * 110574.27f;
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

void lapstats_update_distance(void)
{
        const float speed_avg =
                (current_speed + last_speed) / 2;

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
        size_t current_time = getCurrentTicks();

        if (last_distance_sample_at > 0) {
                g_distance += speed_avg * (ticksToMs(current_time - last_distance_sample_at)) / 3600000.0f;
        }
        last_distance_sample_at = current_time;
        last_speed = current_speed;
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
                g_elapsed_lap_time = getUptime();
        else
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

float lapstats_session_time_minutes()
{
        return tinyMillisToMinutes(getUptime() - g_session_time);
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

        if (g_distance > FINISH_TRIGGER_MINIMUM_DISTANCE_KM) {
                // If we get here, then we have completed a lap.
                lap_finished_event(gpsSnapshot);
        }
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
        lapstats_reset(false);
        reset_track();
}

static void lapstats_location_updated(const GpsSnapshot *gps_snapshot)
{
        /* Reset at_* flags on every sample. */
        g_at_sf = false;
        g_at_sector = false;

        update_elapsed_time(gps_snapshot);

        if (!g_start_finish_enabled) {
                return;
        }

        /* Process data fields first. */
        const GeoPoint *gp = &gps_snapshot->sample.point;
        updateGeoTrigger(&g_start_geo_trigger, gp);
        updateGeoTrigger(&g_finish_geo_trigger, gp);
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

static void debug_print_gps_snapshot(const GpsSnapshot *gps_snapshot)
{
        pr_debug("GPS Snap:");
        pr_debug("\r\nlat/lon:       ");
        pr_debug_float(gps_snapshot->sample.point.latitude);
        pr_debug(",");
        pr_debug_float(gps_snapshot->sample.point.longitude);
        pr_debug("\r\nTime:          ");
        pr_debug_int(gps_snapshot->sample.time);
        pr_debug("\r\nSpeed:         ");
        pr_debug_float(gps_snapshot->sample.speed);
        pr_trace("\r\nAltitude:      ");
        pr_trace_float(gps_snapshot->sample.altitude);
        pr_trace("\r\nSats:          ");
        pr_trace_int(gps_snapshot->sample.satellites);
        pr_trace("\r\nFixmode:       ");
        pr_trace_int(gps_snapshot->sample.fixMode);
        pr_trace("\r\nDOP:           ");
        pr_trace_float(gps_snapshot->sample.DOP);
        pr_debug("\r\ndeltaFirstFix: ");
        pr_debug_int(gps_snapshot->deltaFirstFix);
        pr_debug("\r\ndeltaLastSam:  ");
        pr_debug_int(gps_snapshot->delta_last_sample);
        pr_debug("\r\nprevPoint:     ");
        pr_debug_float(gps_snapshot->previousPoint.latitude);
        pr_debug(",");
        pr_debug_float(gps_snapshot->previousPoint.longitude);
        pr_debug("\r\nprevSpeed:     ");
        pr_debug_float(gps_snapshot->previous_speed);
        pr_debug("\r\n\r\n");
}

void lapstats_process_incremental(const GpsSample *sample)
{
        current_speed = sample->speed;
}

void lapstats_processUpdate(GpsSnapshot *gps_snapshot)
{
        if (!g_configured)
                lapstats_setup(gps_snapshot);

        if (isGpsDataCold()) {
                update_elapsed_time(gps_snapshot);
                return; /* No valid GPS data to work with */
        }

        if (! g_configured)
                return;

        /**
         * Split samples into even intervals, with a minimum of 1 sample as a bozo filter
         * This allows us to up-sample slow GPS sources to 10Hz as neccessary
         */
        int32_t delta_since_last = gps_snapshot->delta_last_sample;

        /* dont run samples if way over the threshold either */
        if (delta_since_last > INTERPOLATION_THRESHOLD_MS * 100)
                return;

        uint32_t interval_count = MAX(1, delta_since_last / INTERPOLATION_THRESHOLD_MS);

        if (interval_count == 1) {
                /* GPS data is arriving fast enough; no interpolation needed */
                lapstats_location_updated(gps_snapshot);
                return;
        }

        /**
         * ============================================
         * Set up interpolation for GPS Point
         * ============================================
         */
        float lat1 = gps_snapshot->previousPoint.latitude;
        float lon1 = gps_snapshot->previousPoint.longitude;
        float lat2 = gps_snapshot->sample.point.latitude;
        float lon2 = gps_snapshot->sample.point.longitude;

        /**
         * Evenly split up difference in latitiude into intervals
         * longitude is linearly interpolated based on changing latitude
         */
        float lat_interval = fabsf(lat2 - lat1) / (float)interval_count;
        if (lat1 > lat2)
                /* Account for reverse direction */
                lat_interval = -lat_interval;

        /* Set the starting interpolated latitude */
        float interp_lat = lat1;

        /**
         * ============================================
         * Set up interpolation for Speed
         * ============================================
         */
        float speed1 = gps_snapshot->previous_speed;
        float speed2 = gps_snapshot->sample.speed;

        /* evenly split up changes in speed based on the interval */
        float speed_interval = fabsf(speed2 - speed1)/ (float)interval_count;
        if (speed1 > speed2)
                /* Account for reverse direction */
                speed_interval = -speed_interval;

        /* Set the starting speed */
        float interp_speed = speed1;

        /**
         * ============================================
         * Set up interpolation for Time
         * ============================================
         */
        millis_t interp_time = gps_snapshot->sample.time - delta_since_last;
        tiny_millis_t interp_delta_ff = gps_snapshot->deltaFirstFix - delta_since_last;

        /* Evenly split up time between current and last sample */
        tiny_millis_t time_interval = delta_since_last / interval_count;

        if (DEBUG_LEVEL) {
                pr_debug("---------------\r\n");
                pr_debug_float_msg("Interval count: ", interval_count);
                pr_debug_float_msg("Speed interval: ", speed_interval);
                pr_debug_float_msg("Lat. interval:  ", lat_interval);
                pr_debug_int_msg  ("Time interval:  ", time_interval);
        }

        for (size_t i = 0; i < interval_count; i++) {
                /* Linearly interpolate longitude from latitude */
                float interp_lon = lon1 + (interp_lat - lat1) * ((lon2 - lon1) / (lat2 - lat1));

                /* Update current GPS snapshot with interpolated values */
                gps_snapshot->sample.point.latitude = interp_lat;
                gps_snapshot->sample.point.longitude = interp_lon;
                gps_snapshot->sample.time = interp_time;
                gps_snapshot->sample.speed = interp_speed;
                gps_snapshot->delta_last_sample = time_interval;
                gps_snapshot->deltaFirstFix = interp_delta_ff;

                if (DEBUG_LEVEL)
                        debug_print_gps_snapshot(gps_snapshot);

                lapstats_location_updated(gps_snapshot);

                /* update interpolated intervals */
                gps_snapshot->previousPoint = gps_snapshot->sample.point;

                interp_lat += lat_interval;
                interp_time += time_interval;
                interp_delta_ff += time_interval;

                gps_snapshot->previous_speed = interp_speed;
                interp_speed += speed_interval;
        }
        pr_debug("---------------\r\n");
}
