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

static int g_atStartFinish;
static int g_prevAtStartFinish;
static tiny_millis_t g_lastStartFinishTimestamp;

static int g_atTarget;
static int g_prevAtTarget;
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
 * @return True if we have crossed the start line at least once, false otherwise.
 */
static bool isStartCrossedYet() {
   return g_lastStartFinishTimestamp != 0;
}

static int processStartFinish(const GpsSnapshot *gpsSnapshot, const Track *track,
                              const float targetRadius) {

   // First time crossing start finish.  Handle this in a special way.
   if (!isStartCrossedYet()) {
      lc_supplyGpsSnapshot(gpsSnapshot);

      if (lc_hasLaunched()) {
         g_lastStartFinishTimestamp = lc_getLaunchTime();
         g_lastSectorTimestamp = lc_getLaunchTime();
         g_prevAtStartFinish = 1;
         g_sector = 0;
         return true;
      }

      return false;
   }

   const tiny_millis_t timestamp = gpsSnapshot->deltaFirstFix;
   const tiny_millis_t elapsed = timestamp - g_lastStartFinishTimestamp;
   const struct GeoCircle sfCircle = gc_createGeoCircle(getFinishPoint(track),
                                                        targetRadius);

   /*
    * Guard against false triggering. We have to be out of the start/finish
    * target for some amount of time and couldn't have been in there during our
    * last time in this method.
    *
    * FIXME: Should have logic that checks that we left the start/finish circle
    * for some time.
    */
   const GeoPoint point = gpsSnapshot->sample.point;
   g_atStartFinish = gc_isPointInGeoCircle(&point, sfCircle);

   if (!g_atStartFinish || g_prevAtStartFinish != 0 ||
       elapsed <= START_FINISH_TIME_THRESHOLD) {
      g_prevAtStartFinish = 0;
      return false;
   }

   // If here, we are at Start/Finish and have de-bounced duplicate entries.
   pr_debug_int(g_lapCount);
   pr_debug(" Lap Detected\r\n");
   g_lapCount++;
   g_lastLapTime = elapsed;
   g_lastStartFinishTimestamp = timestamp;
   g_prevAtStartFinish = 1;

   return true;
}

static void processSector(const GpsSnapshot *gpsSnapshot, const Track *track,
                          float targetRadius) {
   // We don't process sectors until we cross Start
   if (!isStartCrossedYet())
      return;

   const GeoPoint point = getSectorGeoPointAtIndex(track, g_sector);
   const struct GeoCircle sbCircle = gc_createGeoCircle(point, targetRadius);
   const GeoPoint curr = gpsSnapshot->sample.point;

   g_atTarget = gc_isPointInGeoCircle(&curr, sbCircle);
   if (!g_atTarget) {
      g_prevAtTarget = 0;
      return;
   }

   /*
    * Past here we are sure we are at a sector boundary.
    */
   const tiny_millis_t millis = gpsSnapshot->deltaFirstFix;
   pr_debug_int(g_sector);
   pr_debug(" Sector Boundary Detected\r\n");

   g_prevAtTarget = 1;
   g_lastSectorTime = millis - g_lastSectorTimestamp;
   g_lastSectorTimestamp = millis;
   g_lastSector = g_sector;
   ++g_sector;

   // Check if we need to wrap the sectors.
   GeoPoint next = getSectorGeoPointAtIndex(track, g_sector);
   if (areGeoPointsEqual(point, next))
      g_sector = 0;
}

void gpsConfigChanged(void) {
   g_configured = 0;
}

void lapStats_init() {
   resetLapDistance();
   g_configured = 0;
   g_activeTrack = NULL;
   g_lastLapTime = 0;
   g_lastSectorTime = 0;
   g_atStartFinish = 0;
   g_prevAtStartFinish = 0;
   g_lastStartFinishTimestamp = 0;
   g_atTarget = 0;
   g_prevAtTarget = 0;
   g_lastSectorTimestamp = 0;
   g_lapCount = 0;
   g_sector = -1;
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

   g_distance += calcDistancesSinceLastSample(gpsSnapshot);
   // Seconds since first fix is good until we alter the code to use millis directly
   const int lapDetected = processStartFinish(gpsSnapshot, g_activeTrack, targetRadius);
   const tiny_millis_t millisSinceFirstFix = gpsSnapshot->deltaFirstFix;

   if (lapDetected) {
           resetLapDistance();

           /*
            * FIXME: Special handling of first start/finish crossing.  Needed
            * b/c launch control will delay the first launch notification
            */
           if (getLapCount() == 0) {
                   const GeoPoint sp = getStartPoint(g_activeTrack);
                   // Distance is in KM
                   g_distance = distPythag(&sp, gp) / 1000;

                   startFinishCrossed(&sp, g_lastStartFinishTimestamp);
                   addGpsSample(gp, millisSinceFirstFix);
           } else {
                   startFinishCrossed(gp, millisSinceFirstFix);
           }
   } else {
           addGpsSample(gp, millisSinceFirstFix);
   }

   if (sectorEnabled)
           processSector(gpsSnapshot, g_activeTrack, targetRadius);
}

void lapStats_processUpdate(const GpsSnapshot *gpsSnapshot) {
	if (!isGpsDataCold()){
           onLocationUpdated(gpsSnapshot);
	}
}
