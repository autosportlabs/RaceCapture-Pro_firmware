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

#include <stdint.h>

#include "auto_track.h"
#include "launch_control.h"

#define KMS_TO_MILES_CONSTANT (.621371)

// In Millis now.
#define START_FINISH_TIME_THRESHOLD 10000

#define TIME_NULL -1

static const Track * g_activeTrack;

static int g_configured;
static int g_atStartFinish;
static int g_prevAtStartFinish;
static tiny_millis_t g_lapStartTimestamp;
static tiny_millis_t g_elapsedLapTime;

static int g_atTarget;
static tiny_millis_t g_lastSectorTimestamp;

static int g_sector;
static int g_lastSector;

static tiny_millis_t g_lastLapTime;
static tiny_millis_t g_lastSectorTime;

static int g_lapCount;
static float g_distance;

// Our GeoTriggers so we don't unintentionally trigger on start/finish
static struct GeoTrigger startGeoTrigger;
static struct GeoTrigger finishGeoTrigger;

static float degreesToMeters(float degrees) {
   // There are 110574.27 meters per degree of latitude at the equator.
   return degrees * 110574.27;
}

void setActiveTrack(const Track *defaultTrack) {
        g_activeTrack = defaultTrack;
}

bool isLapTimingInProgress() {
        return g_lapStartTimestamp >= 0;
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
        g_lapStartTimestamp = -1;
}

static void updateDistance(const GpsSnapshot *gpsSnapshot) {
        const GeoPoint prev = gpsSnapshot->previousPoint;
        const GeoPoint curr = gpsSnapshot->sample.point;

        if (!isLapTimingInProgress()) return; // Don't update if we aren't racing.
        if (!isValidPoint(&prev) || !isValidPoint(&curr)) return;


        g_distance += distPythag(&prev, &curr) / 1000;
}


static void resetLapDistance(const float distance) {
        g_distance = distance;
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

void resetElapsedLapTime() {
        g_elapsedLapTime = 0;
}

void updateElapsedLapTime(const GpsSnapshot *snap) {
        if(!isLapTimingInProgress()) return;
        g_elapsedLapTime = snap->deltaFirstFix - g_lapStartTimestamp;
}

tiny_millis_t getElapsedLapTime() {
        return g_elapsedLapTime;
}

float getElapsedLapTimeInMinutes() {
        return tinyMillisToMinutes(getElapsedLapTime());
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
 * Called whenever we finish a lap.
 */
void lapFinishedEvent(const GpsSnapshot *gpsSnapshot) {
        pr_debug("Finished Lap ");
        pr_debug_int(g_lapCount);
        pr_debug("\r\n");

        ++g_lapCount;
        endLapTiming(gpsSnapshot);
        finishLap(gpsSnapshot);

        // If in Circuit Mode, don't set cool off after finish.
        if (g_activeTrack->track_type == TRACK_TYPE_CIRCUIT) return;

        /*
         * Reset our startGeoTrigger so that we get away from start
         * finish before we re-arm the system.  This is mainly used
         * in stage situations.
         */
        resetGeoTrigger(&startGeoTrigger);
}


static void _lapStartedEvent(const tiny_millis_t time,
                             const GeoPoint *sp,
                             const float distance,
                             const GpsSnapshot *gpsSnapshot) {
        // Timing and predictive timing
        startLapTiming(time);
        startLap(sp, time);
        resetElapsedLapTime();

        // Reset the sector logic
        g_lastSectorTimestamp = time;
        g_sector = 0;

        // Reset distance logic
        resetLapDistance(distance);

        /*
         * Reset our finishGeoTrigger so that we get away from
         * finish before we re-arm the system.
         */
        resetGeoTrigger(&finishGeoTrigger);
}

/**
 * Called whenever we start a new lap the normal way (ie no launch control).
 */
void lapStartedNormalEvent(const GpsSnapshot *gpsSnapshot) {
        const tiny_millis_t time = gpsSnapshot->deltaFirstFix;
        const GeoPoint gp = gpsSnapshot->sample.point;

        pr_debug("Starting lap ");
        pr_debug_int(g_lapCount);
        pr_debug("\r\n");

        _lapStartedEvent(time, &gp, 0, gpsSnapshot);
}

/**
 * Called whenever we start a new lap using Launch Control.  Has to be specially
 * handled due to the delayed timing information.
 */
static void lapStartedLaunchControlEvent(const GpsSnapshot *gpsSnapshot) {
        const tiny_millis_t time = lc_getLaunchTime();
        const GeoPoint sp = getStartPoint(g_activeTrack);
        const GeoPoint gp = gpsSnapshot->sample.point;
        const float distance = distPythag(&sp, &gp) / 1000;

        pr_debug("Starting lap ");
        pr_debug_int(g_lapCount);
        pr_debug(" with launch control.\r\n");

        _lapStartedEvent(time, &sp, distance, gpsSnapshot);
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
        g_lastSector = g_sector;
        ++g_sector;
}

/**
 * All logic associated with determining if we are at the finish line.
 */
static void processFinishLogic(const GpsSnapshot *gpsSnapshot,
                               const Track *track,
                               const float targetRadius) {
        if (!isLapTimingInProgress()) return;
        if (!isGeoTriggerTripped(&finishGeoTrigger)) return;

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
        if (!isGeoTriggerTripped(&startGeoTrigger)) return;

        /*
         * If we have crossed start finish at least once and we are a circuit
         * track, then we need to disable launch control to prevent hiccups
         * in reporting.
         */
        if (g_lapCount > 0 && track->track_type == TRACK_TYPE_CIRCUIT) {
                lapStartedNormalEvent(gpsSnapshot);
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
        if (!lc_hasLaunched()) return;

        // If here, then the lap has started
        lapStartedLaunchControlEvent(gpsSnapshot);
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
        resetLapDistance(0);
        resetPredictiveTimer();
        resetElapsedLapTime();
        setActiveTrack(NULL);
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

static void setupGeoTriggers(const TrackConfig *tc, const Track *track) {
        // Make the radius 3x the size of start/finish radius.  Seems safe.
        const float gtRadius = degreesToMeters(tc->radius) * 3;
        GeoPoint gp;
        struct GeoCircle gc;

        gp = getStartPoint(track);
        gc = gc_createGeoCircle(gp, gtRadius);
        startGeoTrigger = createGeoTrigger(&gc);

        gp = getFinishPoint(track);
        gc = gc_createGeoCircle(gp, gtRadius);
        finishGeoTrigger = createGeoTrigger(&gc);
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
      const Track *track = &trackConfig->track;
      if (trackConfig->auto_detect) track = auto_configure_track(track, gp);
      setActiveTrack(track);

      startFinishEnabled = isStartFinishEnabled(g_activeTrack);
      sectorEnabled = isSectorTrackingEnabled(g_activeTrack);

      lc_reset();
      lc_setup(g_activeTrack, targetRadius);

      setupGeoTriggers(trackConfig, g_activeTrack);

      g_configured = 1;
   }

   if (!startFinishEnabled) return;

   // Process data fields first.
   updateGeoTrigger(&startGeoTrigger, gp);
   updateGeoTrigger(&finishGeoTrigger, gp);
   updateElapsedLapTime(gpsSnapshot);
   updateDistance(gpsSnapshot);
   addGpsSample(gpsSnapshot);

   /*
    * Now process the sector, finish and start logic in that order.
    * Each processing can invoke their respective event if the logic
    * agrees its time.
    */
   //Skip processing sectors bits if we haven't defined any... duh!
   if (sectorEnabled) {
           processSectorLogic(gpsSnapshot, g_activeTrack, targetRadius);
   }
   processFinishLogic(gpsSnapshot, g_activeTrack, targetRadius);
   processStartLogic(gpsSnapshot, g_activeTrack, targetRadius);
}

void lapStats_processUpdate(const GpsSnapshot *gpsSnapshot) {
	if (!isGpsDataCold()){
           onLocationUpdated(gpsSnapshot);
	}
}
