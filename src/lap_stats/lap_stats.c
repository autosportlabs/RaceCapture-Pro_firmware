#include "lap_stats.h"
#include "dateTime.h"
#include "gps.h"
#include "geopoint.h"
#include "geoCircle.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "mod_string.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include "tracks.h"

#include <stdint.h>

#include "auto_track.h"
#include "launch_control.h"

#define KMS_TO_MILES_CONSTANT (.621371)

// In Millis now.
#define START_FINISH_TIME_THRESHOLD 10000

#define TIME_NULL -1

static const Track * g_activeTrack;

static int g_configured;
static tiny_millis_t g_cooloffTime;
static int g_atStartFinish;
static int g_prevAtStartFinish;
static tiny_millis_t g_lapStartTimestamp;

static int g_atTarget;
static tiny_millis_t g_lastSectorTimestamp;

static int g_sector;
static int g_lastSector;

static tiny_millis_t g_lastLapTime;
static tiny_millis_t g_lastSectorTime;

static int g_lapCount;
static float g_distance;

static float calcDistancesSinceLastSample(const GpsSnapshot *gpsSnapshot) {
   const GeoPoint prev = gpsSnapshot->previousPoint;
   const GeoPoint curr = gpsSnapshot->sample.point;

   if (!isValidPoint(&prev) || !isValidPoint(&curr)) {
      return 0.0;
   }

   // Return distance in KM
   return distPythag(&prev, &curr) / 1000;
}

static float degreesToMeters(float degrees) {
   // There are 110574.27 meters per degree of latitude at the equator.
   return degrees * 110574.27;
}

void resetLapDistance() {
   g_distance = 0.0;
}

float getLapDistance() {
   return g_distance;
}

float getLapDistanceInMiles() {
   return KMS_TO_MILES_CONSTANT * g_distance;
}

void resetLapCount() {
   g_lapCount = 0;
}

int getLapCount() {
   return g_lapCount;
}

int getSector() {
   return g_sector;
}

int getLastSector() {
   return g_lastSector;
}

tiny_millis_t getLastLapTime() {
   return g_lastLapTime;
}

float getLastLapTimeInMinutes() {
   return tinyMillisToMinutes(getLastLapTime());
}

tiny_millis_t getLastSectorTime() {
   return g_lastSectorTime;
}

float getLastSectorTimeInMinutes() {
   return tinyMillisToMinutes(getLastSectorTime());
}

int getAtStartFinish() {
   return g_atStartFinish;
}

int getAtSector() {
   return g_atTarget;
}

/**
 * @return True if we are in the middle of a lap.  False otherwise.
 */
static bool isLapTimingInProgress() {
   return g_lapStartTimestamp != 0;
}

/**
 * Called when we start a lap.  Handles timing information.
 */
static void startLapTiming(const tiny_millis_t startTime) {
        g_lapStartTimestamp = startTime;
}

/**
 * Called when we finish a lap.  Handles timing information.
 */
static void endLapTiming(const GpsSnapshot *gpsSnapshot) {
        g_lastLapTime = gpsSnapshot->deltaFirstFix - g_lapStartTimestamp;
        g_lapStartTimestamp = 0;
}

/**
 * True if we are in the cool off period for start/finish.  False otherwise.
 */
static bool isCoolOffInEffect(const GpsSnapshot *gpsSnapshot) {
        return gpsSnapshot->deltaFirstFix < g_cooloffTime;
}

/**
 * Sets the cooloff Window to be now plus X milliseconds.  When cooloff
 * is active we will not trigger start or finish events.  This is used
 * to prevent false triggering.
 * @param time The amount of time (in ms) that we need to cooloff
 */
static void setCooloffWindow(const GpsSnapshot *gpsSnapshot,
                             const tiny_millis_t time) {
        g_cooloffTime = gpsSnapshot->deltaFirstFix + time;
}

/**
 * Called whenever we finish a lap.
 */
static void lapFinishedEvent(const GpsSnapshot *gpsSnapshot) {
        pr_debug("Finished Lap ");
        pr_debug_int(g_lapCount);
        pr_debug("\r\n");

        endLapTiming(gpsSnapshot);
        finishLap(gpsSnapshot);

        // If in Circuit Mode, don't set cool off after finish.
        if (g_activeTrack->track_type == TRACK_TYPE_CIRCUIT) return;

        /*
         * Set cool off window in stage mode to give driver time to get
         * away from start circle in case start and finish lines are
         * close to one another.
         */
        setCooloffWindow(gpsSnapshot, START_FINISH_TIME_THRESHOLD);
}


/**
 * Called whenever we start a new lap.
 */
static void lapStartedEvent(const GpsSnapshot *gpsSnapshot) {
        // Increment and log that we started a lap.
        pr_debug("Started Lap ");
        pr_debug_int(++g_lapCount);
        pr_debug("\r\n");

        const tiny_millis_t launchTime = lc_getLaunchTime();
        const GeoPoint sp = getStartPoint(g_activeTrack);
        const GeoPoint gp = gpsSnapshot->sample.point;

        startLapTiming(launchTime);
        startLap(&sp, launchTime);

        // Reset the sector logic
        g_lastSectorTimestamp = launchTime;
        g_sector = 0;

        resetLapDistance();
        g_distance = distPythag(&sp, &gp) / 1000;

        /*
         * Set this value so we don't accidentally trigger a finish
         * event in a circuit track (since circuit track has same
         * start & finish area.
         */
        setCooloffWindow(gpsSnapshot, START_FINISH_TIME_THRESHOLD);
}

/**
 * Called whenever we have hit a sector boundary.
 */
static void sectorBoundaryEvent(const GpsSnapshot *gpsSnapshot) {
        const tiny_millis_t millis = gpsSnapshot->deltaFirstFix;

        pr_debug_int(g_sector);
        pr_debug(" Sector Boundary Detected\r\n");

        g_lastSectorTime = millis - g_lastSectorTimestamp;
        g_lastSectorTimestamp = millis;
        g_lastSector = g_sector++;
}

/**
 * All logic associated with determining if we are at the finish line.
 */
static void processFinishLogic(const GpsSnapshot *gpsSnapshot,
                               const Track *track,
                               const float targetRadius) {
        if (!isLapTimingInProgress()) return;
        if (isCoolOffInEffect(gpsSnapshot)) return;

        const GeoPoint point = gpsSnapshot->sample.point;
        const GeoPoint finishPoint = getFinishPoint(track);
        const struct GeoCircle finishCircle = gc_createGeoCircle(finishPoint,
                                                                 targetRadius);
        if(!gc_isPointInGeoCircle(&point, finishCircle)) return;

        // If we get here, then we have completed a lap.
        lapFinishedEvent(gpsSnapshot);
}

/**
 * All logic associated with determining if we are at the start line.
 */
static void processStartLogic(const GpsSnapshot *gpsSnapshot,
                              const Track *track,
                              const float targetRadius) {
        if (isLapTimingInProgress()) return;
        if (isCoolOffInEffect(gpsSnapshot)) return;

        /*
         * This check goes here (before we supply launch control with GPS
         * data) to allow us to know if launch control needs a reset.  If
         * so then reset it, then start feeding it data.
         */
        if (lc_hasLaunched()) {
                lc_reset();
                lc_setup(track, targetRadius);
        }

        lc_supplyGpsSnapshot(gpsSnapshot);
        if (!lc_hasLaunched()) return;

        // If here, then the lap has started
        lapStartedEvent(gpsSnapshot);
}

static void processSectorLogic(const GpsSnapshot *gpsSnapshot,
                               const Track *track,
                               const float radius) {
        if (!isLapTimingInProgress()) return;

        const GeoPoint point = getSectorGeoPointAtIndex(track, g_sector);
        const struct GeoCircle circle = gc_createGeoCircle(point, radius);
        const GeoPoint curr = gpsSnapshot->sample.point;

        g_atTarget = gc_isPointInGeoCircle(&curr, circle);
        if (!g_atTarget) return;

        // If we are here, then we are at a Sector boundary.
        sectorBoundaryEvent(gpsSnapshot);
}

void gpsConfigChanged(void) {
   g_configured = 0;
}

void lapStats_init() {
   resetLapDistance();
   g_configured = 0;
   g_cooloffTime = -1000000; // Because we don't want to start in cooloff
   g_activeTrack = NULL;
   g_lastLapTime = 0;
   g_lastSectorTime = 0;
   g_atStartFinish = 0;
   g_prevAtStartFinish = 0;
   g_lapStartTimestamp = 0;
   g_atTarget = 0;
   g_lastSectorTimestamp = 0;
   g_lapCount = 0;
   g_sector = -1;     // Indicates we haven't crossed start/finish yet.
   g_lastSector = -1; // Indicates no previous sector.
   resetPredictiveTimer();
}

static int isStartFinishEnabled(const Track *track) {
        return isFinishPointValid(track) && isStartPointValid(track);
}

static int isSectorTrackingEnabled(const Track *track) {
        if (!isStartFinishEnabled(track)) return 0;

        // Must have >=  one valid sector; must start at position 0.
        const LoggerConfig *config = getWorkingLoggerConfig();
        const GeoPoint p0 = getSectorGeoPointAtIndex(track, 0);
        return config->LapConfigs.sectorTimeCfg.sampleRate != SAMPLE_DISABLED &&
                isValidPoint(&p0);
}

static void onLocationUpdated(const GpsSnapshot *gpsSnapshot) {
   // FIXME.  These suck.  Kill them somehow.
   static int sectorEnabled = 0;
   static int startFinishEnabled = 0;

   const LoggerConfig *config = getWorkingLoggerConfig();
   const GeoPoint *gp = &gpsSnapshot->sample.point;
   const float targetRadius = degreesToMeters(config->TrackConfigs.radius);

   // FIXME.  This active track configuration here is le no good.
   if (!g_configured) {
      const TrackConfig *trackConfig = &(config->TrackConfigs);
      const Track *defaultTrack = &trackConfig->track;
      g_activeTrack = trackConfig->auto_detect ?
         auto_configure_track(defaultTrack, gp) : defaultTrack;

      startFinishEnabled = isStartFinishEnabled(g_activeTrack);
      sectorEnabled = isSectorTrackingEnabled(g_activeTrack);
      lc_setup(g_activeTrack, targetRadius);

      g_configured = 1;
   }

   if (!startFinishEnabled) return;

   // Skip processing sectors bits if we haven't defined any... duh!
   if (sectorEnabled) processSectorLogic(gpsSnapshot, g_activeTrack,
                                         targetRadius);
   processFinishLogic(gpsSnapshot, g_activeTrack, targetRadius);
   processStartLogic(gpsSnapshot, g_activeTrack, targetRadius);

   // At this point if no lap is in progress... nothing else to do.
   if (!isLapTimingInProgress()) return;

   g_distance += calcDistancesSinceLastSample(gpsSnapshot);
   addGpsSample(gpsSnapshot);
}

void lapStats_processUpdate(const GpsSnapshot *gpsSnapshot) {
	if (!isGpsDataCold()){
           onLocationUpdated(gpsSnapshot);
	}
}
