#include "lap_stats.h"
#include "dateTime.h"
#include "gps.h"
#include "geopoint.h"
#include "geoCircle.h"
#include "geoTrigger.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "mod_string.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include "tracks.h"
#include "test.h"

#include <stdint.h>

#include "auto_track.h"
#include "launch_control.h"

#define KMS_TO_MILES_CONSTANT (.621371)

// In Millis now.
#define START_FINISH_TIME_THRESHOLD 10000

#define TIME_NULL -1

static const Track * g_activeTrack;
track_status_t g_track_status = TRACK_STATUS_WAITING_TO_CONFIG;
static int g_sector_enabled = 0;
static int g_start_finish_enabled = 0;

static int g_configured;
static int g_atStartFinish;
static int g_prevAtStartFinish;
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

TESTABLE_STATIC void set_active_track(const Track* track)
{
    g_activeTrack = track;
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
    if (g_activeTrack == NULL) {
        return 0;
    } else {
        return g_activeTrack->trackId;
    }
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

TESTABLE_STATIC void reset_current_lap()
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

TESTABLE_STATIC void reset_elapsed_time()
{
    g_elapsed_lap_time = 0;
}

TESTABLE_STATIC void update_elapsed_time(const GpsSnapshot *snap)
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
TESTABLE_STATIC void lap_finished_event(const GpsSnapshot *gpsSnapshot)
{
    pr_debug_int_msg("Finished Lap ", g_lapCount);

    ++g_lapCount;
    end_lap_timing(gpsSnapshot);
    finishLap(gpsSnapshot);

    // If in Circuit Mode, don't set cool off after finish.
    if (g_activeTrack->track_type == TRACK_TYPE_CIRCUIT)
        return;

    /*
     * Reset our startGeoTrigger so that we get away from start
     * finish before we re-arm the system.  This is mainly used
     * in stage situations.
     */
    resetGeoTrigger(&g_start_geo_trigger);
}

static void _lap_started_event(const tiny_millis_t time, const GeoPoint *sp,
                               const float distance, const GpsSnapshot *gpsSnapshot)
{
    // Timing and predictive timing
    ++g_lap;
    start_lap_timing(time);
    startLap(sp, time);
    reset_elapsed_time();

    // Reset the sector logic
    g_lastSectorTimestamp = time;
    g_sector = 0;

    // Reset distance logic
    set_distance(distance);

    /*
     * Reset our finishGeoTrigger so that we get away from
     * finish before we re-arm the system.
     */
    resetGeoTrigger(&g_finish_geo_trigger);
}

/**
 * Called whenever we start a new lap the normal way (ie no launch control).
 */
TESTABLE_STATIC void lap_started_normal_event(const GpsSnapshot *gpsSnapshot)
{
    const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
    const GeoPoint gp = gpsSnapshot->sample.point;
    pr_debug_int_msg("Start Lap ", g_lapCount);
    _lap_started_event(time, &gp, 0, gpsSnapshot);
}

/**
 * Called whenever we start a new lap using Launch Control.  Has to be specially
 * handled due to the delayed timing information.
 */
static void lap_started_launched_event(const GpsSnapshot *gpsSnapshot)
{
    const tiny_millis_t time = lc_getLaunchTime();
    const GeoPoint sp = getStartPoint(g_activeTrack);
    const GeoPoint gp = gpsSnapshot->sample.point;
    const float distance = distPythag(&sp, &gp) / 1000;
    pr_debug_int_msg("Start lap w/launch ", g_lapCount);
    _lap_started_event(time, &sp, distance, gpsSnapshot);
}

/**
 * Called whenever we have hit a sector boundary.
 */
static void sectorBoundaryEvent(const GpsSnapshot *gpsSnapshot)
{
    const tiny_millis_t millis = gpsSnapshot->deltaFirstFix;

    pr_debug_int_msg("Sector boundary ", g_sector);

    g_lastSectorTime = millis - g_lastSectorTimestamp;
    g_lastSectorTimestamp = millis;
    g_lastSector = g_sector;
    ++g_sector;
}

/**
 * All logic associated with determining if we are at the finish line.
 */
static void processFinishLogic(const GpsSnapshot *gpsSnapshot,
                               const Track *track, const float targetRadius)
{
    if (!lapstats_lap_in_progress())
        return;
    if (!isGeoTriggerTripped(&g_finish_geo_trigger))
        return;

    const GeoPoint point = gpsSnapshot->sample.point;
    const GeoPoint finishPoint = getFinishPoint(track);
    const struct GeoCircle finishCircle = gc_createGeoCircle(finishPoint,
                                          targetRadius);
    if (!gc_isPointInGeoCircle(&point, finishCircle))
        return;

    // If we get here, then we have completed a lap.
    lap_finished_event(gpsSnapshot);
}

/**
 * All logic associated with determining if we are at the start line.
 */
static void processStartLogic(const GpsSnapshot *gpsSnapshot,
                              const Track *track, const float targetRadius)
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
    if (g_lapCount > 0 && track->track_type == TRACK_TYPE_CIRCUIT) {
        lap_started_normal_event(gpsSnapshot);
        return;
    }

    /*
     * If in Stage mode or not launched, we need to do launch control
     * on this lap start.  This way we can support multiple stage style
     * laps.  This also buys us predictive timing.
     */
    if (lc_hasLaunched()) {
        lc_reset();
        lc_setup(track, targetRadius);
    }

    lc_supplyGpsSnapshot(gpsSnapshot);
    if (!lc_hasLaunched())
        return;

    // If here, then the lap has started
    lap_started_launched_event(gpsSnapshot);
}

static void processSectorLogic(const GpsSnapshot *gpsSnapshot,
                               const Track *track, const float radius)
{
    if (!lapstats_lap_in_progress())
        return;

    const GeoPoint point = getSectorGeoPointAtIndex(track, g_sector);
    const struct GeoCircle circle = gc_createGeoCircle(point, radius);
    const GeoPoint curr = gpsSnapshot->sample.point;

    g_atTarget = gc_isPointInGeoCircle(&curr, circle);
    if (!g_atTarget)
        return;

    // If we are here, then we are at a Sector boundary.
    sectorBoundaryEvent(gpsSnapshot);
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
    g_configured = 0;
    g_lastLapTime = 0;
    g_lastSectorTime = 0;
    g_atStartFinish = 0;
    g_prevAtStartFinish = 0;
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

static void setupGeoTriggers(const TrackConfig *tc, const Track *track)
{
    // Make the radius 3x the size of start/finish radius.  Seems safe.
    const float gtRadius = degrees_to_meters(tc->radius) * 3;
    GeoPoint gp;
    struct GeoCircle gc;

    gp = getStartPoint(track);
    gc = gc_createGeoCircle(gp, gtRadius);
    g_start_geo_trigger = createGeoTrigger(&gc);

    gp = getFinishPoint(track);
    gc = gc_createGeoCircle(gp, gtRadius);
    g_finish_geo_trigger = createGeoTrigger(&gc);
}

static void lapstats_location_updated(const GpsSnapshot *gps_snapshot)
{
    const LoggerConfig *config = getWorkingLoggerConfig();
    const GeoPoint *gp = &gps_snapshot->sample.point;
    const float target_radius = degrees_to_meters(config->TrackConfigs.radius);

    update_distance(gps_snapshot);

    if (!g_start_finish_enabled)
        return;

    // Process data fields first.
    updateGeoTrigger(&g_start_geo_trigger, gp);
    updateGeoTrigger(&g_finish_geo_trigger, gp);
    update_elapsed_time(gps_snapshot);
    addGpsSample(gps_snapshot);

    /*
     * Now process the sector, finish and start logic in that order.
     * Each processing can invoke their respective event if the logic
     * agrees its time.
     */
    //Skip processing sectors bits if we haven't defined any... duh!
    if (g_sector_enabled) {
        processSectorLogic(gps_snapshot, g_activeTrack, target_radius);
    }
    processFinishLogic(gps_snapshot, g_activeTrack, target_radius);
    processStartLogic(gps_snapshot, g_activeTrack, target_radius);
}

static void lapstats_setup(const GpsSnapshot *gps_snapshot)
{
    const LoggerConfig *config = getWorkingLoggerConfig();
    const float target_radius = degrees_to_meters(config->TrackConfigs.radius);
    const GeoPoint *gp = &gps_snapshot->sample.point;

    const Track *track = NULL;
    const TrackConfig *trackConfig = &(config->TrackConfigs);
    if (trackConfig->auto_detect) {
        track = auto_configure_track(NULL, gp);
        if (track) {
            g_track_status = TRACK_STATUS_AUTO_DETECTED;
            pr_info_int_msg("track: detected track ", track->trackId);
        }
    } else {
        track = &trackConfig->track;
        g_track_status = TRACK_STATUS_FIXED_CONFIG;
        pr_info("track: using fixed config");
    }
    set_active_track(track);

    if (!track) return;

    g_start_finish_enabled = isStartFinishEnabled(track);
    g_sector_enabled = isSectorTrackingEnabled(track);
    lc_reset();
    lc_setup(track, target_radius);
    setupGeoTriggers(trackConfig, track);
    g_configured = 1;
}

void lapstats_processUpdate(const GpsSnapshot *gps_snapshot)
{
    if (!isGpsDataCold()) {
        if (!g_configured) {
            lapstats_setup(gps_snapshot);
        } else {
            lapstats_location_updated(gps_snapshot);
        }
    }
}
