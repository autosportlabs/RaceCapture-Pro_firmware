#include "auto_track.h"
#include "dateTime.h"
#include "gps.h"
#include "geopoint.h"
#include "launch_control.h"
#include "loggerHardware.h"
#include "LED.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "geometry.h"
#include "mod_string.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include <math.h>
#include <stdint.h>
#include "tracks.h"

//kilometers
#define DISTANCE_SCALING 6371
#define PI 3.1415

#define GPS_LOCK_FLASH_COUNT 2
#define GPS_NOFIX_FLASH_COUNT 10

#define LATITUDE_DATA_LEN 12
#define LONGITUDE_DATA_LEN 13

#define UTC_TIME_BUFFER_LEN 11
#define UTC_SPEED_BUFFER_LEN 10

// In Millis now.
#define START_FINISH_TIME_THRESHOLD 10000

#define TIME_NULL -1

#define GPS_LOCKED_ON(QUALITY) (QUALITY != GPS_QUALITY_NO_FIX)

static const Track * g_activeTrack;

static int g_configured;
static int g_flashCount;
static float g_prevLatitude;
static float g_prevLongitude;

static float g_latitude;
static float g_longitude;

static float g_speed;

static int g_gpsQuality;

static int g_satellitesUsedForPosition;

static int g_atStartFinish;
static int g_prevAtStartFinish;
static unsigned long long g_lastStartFinishTimestamp;

static int g_atTarget;
static int g_prevAtTarget;
static unsigned long long g_lastSectorTimestamp;

static int g_sector;
static int g_lastSector;

static unsigned long long g_lastLapTime;
static unsigned long long g_lastSectorTime;

static int g_lapCount;
static float g_distance;

/**
 * Date and time this GPS fix was taken.
 */
static DateTime g_dtFirstFix;
static DateTime g_dtLastFix;
static unsigned long long g_millisSinceUnixEpoch;

/**
 * @return true if we haven't parsed any data yet, false otherwise.
 */
static bool isGpsDataCold() {
   return g_millisSinceUnixEpoch == 0;
}

void updateMillisSinceEpoch(DateTime fixDateTime) {
   g_millisSinceUnixEpoch = getMillisecondsSinceUnixEpoch(fixDateTime);
}

unsigned long long getMillisSinceEpoch() {
   return g_millisSinceUnixEpoch;
}

/**
 * Performs a full update of the g_dtLastFix value.  Also update the g_dtFirstFix value if it hasn't
 * already been set.
 * @param fixDateTime The DateTime of the GPS fix.
 */
void updateFullDateTime(DateTime fixDateTime) {
   g_dtLastFix = fixDateTime;
   if (g_dtFirstFix.partialYear == 0)
      g_dtFirstFix = fixDateTime;
}

/**
 * Like atoi, but is non-destructive to the string passed in and provides an offset and length
 * functionality.  Max Len is 3.
 * @param str The start of the String to parse.
 * @param offset How far in to start reading the string.
 * @param len The number of characters to read.
 */
int atoiOffsetLenSafe(const char *str, size_t offset, size_t len) {
   char buff[4] = { 0 };

   // Bounds check.  Don't want any bleeding hearts in here...
   if (len > (sizeof(buff) - 1))
         len = sizeof(buff) - 1;

   memcpy(buff, str + offset, len);
   return modp_atoi(buff);
}

//Parse Global Positioning System Fix Data.
static void parseGGA(char *data) {

   //SendString(data);

   char * delim = strchr(data, ',');
   int param = 0;

   double latitude = 0.0;
   double longitude = 0.0;

   int keepParsing = 1;

   while (delim != NULL && keepParsing) {
      *delim = '\0';
      switch (param) {
      case 1: {
         unsigned int len = strlen(data);
         if (len > 0 && len <= LATITUDE_DATA_LEN) {
            //Raw GPS Format is ddmm.mmmmmm
            char degreesStr[3] = { 0 };
            memcpy(degreesStr, data, 2);
            degreesStr[2] = 0;
            float minutes = modp_atof(data + 2);
            minutes = minutes / 60.0;
            latitude = modp_atoi(degreesStr) + minutes;
         } else {
            latitude = 0;
            //TODO log error
         }
      }
         break;
      case 2: {
         if (data[0] == 'S') {
            latitude = -latitude;
         }
      }
         break;
      case 3: {
         unsigned int len = strlen(data);
         if (len > 0 && len <= LONGITUDE_DATA_LEN) {
            //Raw GPS Format is dddmm.mmmmmm
            char degreesStr[4] = { 0 };
            memcpy(degreesStr, data, 3);
            degreesStr[3] = 0;
            float minutes = modp_atof(data + 3);
            minutes = minutes / 60.0;
            longitude = modp_atoi(degreesStr) + minutes;
         } else {
            longitude = 0;
            //TODO log error
         }
      }
         break;
      case 4: {
         if (data[0] == 'W') {
            longitude = -longitude;
         }
      }
         break;
      case 5:
         g_gpsQuality = modp_atoi(data);
         break;
      case 6:
         g_satellitesUsedForPosition = modp_atoi(data);
         keepParsing = 0;
         break;
      }
      param++;
      data = delim + 1;
      delim = strchr(data, ',');
   }

   updatePosition(latitude, longitude);
}

void updatePosition(float latitude, float longitude) {
   g_prevLatitude = g_latitude;
   g_prevLongitude = g_longitude;
   g_longitude = longitude;
   g_latitude = latitude;
}

//Parse GNSS DOP and Active Satellites
static void parseGSA(char *data) {

}

//Parse Course Over Ground and Ground Speed
static void parseVTG(char *data) {

   char * delim = strchr(data, ',');
   int param = 0;

   int keepParsing = 1;
   while (delim != NULL && keepParsing) {
      *delim = '\0';
      switch (param) {
      case 6: //Speed over ground
      {
         if (strlen(data) >= 1) {
            setGPSSpeed(modp_atof(data)); //convert to MPH
         }
         keepParsing = 0;
      }
         break;
      default:
         break;
      }
      param++;
      data = delim + 1;
      delim = strchr(data, ',');
   }
}

//Parse Geographic Position � Latitude / Longitude
static void parseGLL(char *data) {

}

//Parse Time & Date
static void parseZDA(char *data) {
   /*
    $GPZDA

    Date & Time

    UTC, day, month, year, and local time zone.

    $--ZDA,hhmmss.ss,xx,xx,xxxx,xx,xx
    hhmmss.ss = UTC
    xx = Day, 01 to 31
    xx = Month, 01 to 12
    xxxx = Year
    xx = Local zone description, 00 to +/- 13 hours
    xx = Local zone minutes description (same sign as hours)
    */
   return;
}

//Parse GNSS Satellites in View
static void parseGSV(char *data) {

}

//Parse Recommended Minimum Navigation Information
static void parseRMC(char *data) {
   /*
    * $GPRMC,053740.000,A,2503.6319,N,12136.0099,E,2.69,79.65,100106,,,A*53
    * Message ID $GPRMC RMC protocol header
    * UTC Time 053740.000 hhmmss.sss
    * Status A A=data valid or V=data not valid
    * Latitude 2503.6319 ddmm.mmmm
    * N/S Indicator N N=north or S=south
    * Longitude 12136.0099 dddmm.mmmm
    * E/W Indicator E E=east or W=west
    * Speed over ground 2.69 knots True
    * Course over ground 79.65 degrees
    * Date 100106 Ddmmyy
    * Magnetic variation degrees Not shown
    * Variation sense E=east or W=west (Not shown)
    * Mode A
    * A=autonomous, D=DGPS, E=DR, N=Data not valid,
    * R=Coarse Position, S=Simulator
    * Checksum *53
    * <CR> <LF> End of message termination
    */
   char *delim = strchr(data, ',');
   int param = 0;
   DateTime dt = { 0 };

   while (delim) {
      *delim = '\0';
      switch (param) {
      case 0: //UTC Time (HHMMSS.SSS)
         dt.hour = (int8_t) atoiOffsetLenSafe(data, 0, 2);
         dt.minute = (int8_t) atoiOffsetLenSafe(data, 2, 2);
         dt.second = (int8_t) atoiOffsetLenSafe(data, 4, 2);
         dt.millisecond = (int16_t) atoiOffsetLenSafe(data, 7, 3);
         break;
      case 8: //Date (DDMMYY)
         dt.day = (int8_t) atoiOffsetLenSafe(data, 0, 2);
         dt.month = (int8_t) atoiOffsetLenSafe(data, 2, 2);
         dt.partialYear = (int8_t) atoiOffsetLenSafe(data, 4, 2);
         break;
      }

      ++param;
      data = ++delim;
      delim = strchr(delim, ',');
   }

   updateFullDateTime(dt);
   updateMillisSinceEpoch(dt);
}

void resetGpsDistance() {
   g_distance = 0;
}

float getGpsDistance() {
   return g_distance;
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

unsigned long long getLastLapTime() {
   return g_lastLapTime;
}

unsigned long long getLastSectorTime() {
   return g_lastSectorTime;
}

int getAtStartFinish() {
   return g_atStartFinish;
}

int getAtSector() {
   return g_atTarget;
}

float getLatitude() {
   return g_latitude;
}

float getLongitude() {
   return g_longitude;
}

int getGPSQuality() {
   return g_gpsQuality;
}

void setGPSQuality(int quality) {
   g_gpsQuality = quality;
}

int getSatellitesUsedForPosition() {
   return g_satellitesUsedForPosition;
}

float getGPSSpeed() {
   return g_speed;
}

void setGPSSpeed(float speed) {
   g_speed = speed;
}

DateTime getLastFixDateTime() {
   return g_dtLastFix;
}

static struct GpsSample getGpsSample() {
   struct GpsSample s;
   s.point.latitude = getLatitude();
   s.point.longitude = getLongitude();
   s.time = getMillisSinceEpoch();
   s.speed = getGPSSpeed();

   return s;
}

static int withinGpsTarget(const GeoPoint *point, float radius) {

   struct circ_area area;
   struct point p;
   p.x = point->longitude;
   p.y = point->latitude;

   struct point currentP;
   currentP.x = getLongitude();
   currentP.y = getLatitude();

   create_circ(&area, &p, radius);
   int within = within_circ(&area, &currentP);
   return within;
}

static float toRadians(float degrees) {
   return degrees * PI / 180.0;
}

/**
 * @return True if we have crossed the start line at least once, false otherwise.
 */
static bool isStartCrossedYet() {
   return g_lastStartFinishTimestamp != 0ull;
}

static float calcDistancesSinceLastSample() {
   const GeoPoint prev = {g_prevLatitude, g_prevLongitude};
   const GeoPoint curr = {g_latitude, g_longitude};

   if (!isValidPoint(&prev) || !isValidPoint(&curr))
      return 0.0;

   // Return distance in KM
   return distPythag(&prev, &curr) / 1000;
}

static int processStartFinish(const Track *track, float targetRadius) {
   const GeoPoint sfPoint = isStartCrossedYet() ?
      getFinishPoint(track) : getStartPoint(track);

   // First time crossing start finish.  Handle this in a special way.
   if (!isStartCrossedYet()) {
      lc_supplyGpsSample(getGpsSample());

      if (lc_hasLaunched()) {
         g_lastStartFinishTimestamp = lc_getLaunchTime();
         g_lastSectorTimestamp = lc_getLaunchTime();
         g_prevAtStartFinish = 1;
         return true;
      }

      return false;
   }


   const unsigned long long timestamp = getMillisSinceEpoch();
   const unsigned long long elapsed = timestamp - g_lastStartFinishTimestamp;

   /*
    * Guard against false triggering. We have to be out of the start/finish
    * target for some amount of time and couldn't have been in there during our
    * last time in this method.
    *
    * FIXME: Should have logic that checks that we left the start/finish circle
    * for some time.
    */
   g_atStartFinish = withinGpsTarget(&sfPoint, targetRadius);
   if (!g_atStartFinish || g_prevAtStartFinish != 0 ||
       elapsed <= START_FINISH_TIME_THRESHOLD) {
      g_prevAtStartFinish = 0;
      return false;
   }

   // If here, we are at Start/Finish and have de-bounced duplicate entries.
   g_lapCount++;
   g_lastLapTime = elapsed;
   g_lastStartFinishTimestamp = timestamp;
   g_prevAtStartFinish = 1;

   return true;
}

static void processSector(const Track *track, float targetRadius) {
   // We don't process sectors until we cross Start
   if (!isStartCrossedYet())
      return;

   const GeoPoint point = getSectorGeoPointAtIndex(track, g_sector);
   g_atTarget = withinGpsTarget(&point, targetRadius);
   if (!g_atTarget) {
      g_prevAtTarget = 0;
      return;
   }

   /*
    * Past here we are sure we are at a sector boundary.
    */
   const unsigned long long timestamp = getMillisSinceEpoch();

   g_prevAtTarget = 1;
   g_lastSectorTime = timestamp - g_lastSectorTimestamp;
   g_lastSectorTimestamp = timestamp;
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

void initGPS() {
   g_configured = 0;
   g_activeTrack = NULL;
   g_millisSinceUnixEpoch = 0;
   g_flashCount = 0;
   g_prevLatitude = 0.0;
   g_prevLongitude = 0.0;
   g_latitude = 0.0;
   g_longitude = 0.0;
   g_gpsQuality = GPS_QUALITY_NO_FIX;
   g_satellitesUsedForPosition = 0;
   g_speed = 0.0;
   g_lastLapTime = 0;
   g_lastSectorTime = 0;
   g_atStartFinish = 0;
   g_prevAtStartFinish = 0;
   g_lastStartFinishTimestamp = 0;
   g_atTarget = 0;
   g_prevAtTarget = 0;
   g_lastSectorTimestamp = 0;
   g_lapCount = 0;
   g_distance = 0;
   g_sector = 0;
   g_lastSector = -1; // Indicates no previous sector.
   resetPredictiveTimer();
   g_dtFirstFix = g_dtLastFix = (DateTime) { 0 };
}

static void flashGpsStatusLed() {
   if (g_flashCount == 0)
      LED_disable(1);
   g_flashCount++;
   int targetFlashCount = (
         GPS_LOCKED_ON(g_gpsQuality) ?
               GPS_LOCK_FLASH_COUNT : GPS_NOFIX_FLASH_COUNT);
   if (g_flashCount >= targetFlashCount) {
      LED_enable(1);
      g_flashCount = 0;
   }
}

long long getMillisSinceFirstFix() {
   return getTimeDeltaInMillis(g_dtLastFix, g_dtFirstFix);
}

void onLocationUpdated() {
   static int sectorEnabled = 0;
   static int startFinishEnabled = 0;

   // If no GPS lock, no point in doing any of this.
   if (!GPS_LOCKED_ON(g_gpsQuality))
      return;

   LoggerConfig *config = getWorkingLoggerConfig();

   GeoPoint gp;
   populateGeoPoint(&gp);

   if (!g_configured) {
      TrackConfig *trackConfig = &(config->TrackConfigs);
      Track *defaultTrack = &trackConfig->track;
      g_activeTrack = trackConfig->auto_detect ? auto_configure_track(defaultTrack, gp) : defaultTrack;
      startFinishEnabled = isFinishPointValid(g_activeTrack) && isStartPointValid(g_activeTrack);
      sectorEnabled = config->LapConfigs.sectorTimeCfg.sampleRate !=
         SAMPLE_DISABLED && startFinishEnabled;
      lc_setup(g_activeTrack, config->TrackConfigs.radius * 1000);
      g_configured = 1;
   }

   float dist = calcDistancesSinceLastSample();
   g_distance += dist;


   if (startFinishEnabled) {
      const float targetRadius = config->TrackConfigs.radius;

      // Seconds since first fix is good until we alter the code to use millis directly
      const unsigned long long millisSinceEpoch = getMillisSinceEpoch();
      const int lapDetected = processStartFinish(g_activeTrack, targetRadius);

      if (lapDetected) {
         resetGpsDistance();

         /*
          * FIXME: Special handling of fisrt start/finish crossing.  Needed
          * b/c launch control will delay the first launch notification
          */
         if (getLapCount() == 0) {
            const GeoPoint sp = getStartPoint(g_activeTrack);
            // Distance is in KM
            g_distance = distPythag(&sp, &gp) / 1000;

            startFinishCrossed(sp, g_lastStartFinishTimestamp);
            addGpsSample(gp, millisSinceEpoch);
         } else {
            startFinishCrossed(gp, millisSinceEpoch);
         }
      } else {
         addGpsSample(gp, millisSinceEpoch);
      }

      if (sectorEnabled)
         processSector(g_activeTrack, targetRadius);
   }

}

int checksumValid(const char *gpsData, size_t len) {
   int valid = 0;
   unsigned char checksum = 0;
   size_t i = 0;
   for (; i < len - 1; i++) {
      char c = *(gpsData + i);
      if (c == '*' || c == '\0')
         break;
      else if (c == '$')
         continue;
      else
         checksum ^= c;
   }
   if (len > i + 2) {
      unsigned char dataChecksum = modp_xtoc(gpsData + i + 1);
      if (checksum == dataChecksum)
         valid = 1;
   }
   return valid;
}

void processGPSData(char *gpsData, size_t len) {
   if (len <= 4 || !checksumValid(gpsData, len) || strstr(gpsData, "$GP") != gpsData) {
      pr_warning("GPS: corrupt frame\r\n");
      return;
   }

   // Advance the pointer 3 spaces since we know it begins with "$GP"
   gpsData += 3;
   if (strstr(gpsData, "GGA,")) {
      /*
       * GGA is always the first sentence in a new NMEA paragraph from the GPS Mouse.  So if we see
       * it, call onLocationUpdated if we have parsed GPS data before.  This methodology ensures
       * that we parse all sentences before processing the GPS data.
       */
      if (!isGpsDataCold()) {
         onLocationUpdated();
         flashGpsStatusLed();
      }

      parseGGA(gpsData + 4);
   } else if (strstr(gpsData, "VTG,")) { //Course Over Ground and Ground Speed
      parseVTG(gpsData + 4);
   } else if (strstr(gpsData, "GSA,")) { //GPS Fix gpsData
      parseGSA(gpsData + 4);
   } else if (strstr(gpsData, "GSV,")) { //Satellites in view
      parseGSV(gpsData + 4);
   } else if (strstr(gpsData, "RMC,")) { //Recommended Minimum Specific GNSS Data
      parseRMC(gpsData + 4);
   } else if (strstr(gpsData, "GLL,")) { //Geographic Position - Latitude/Longitude
      parseGLL(gpsData + 4);
   } else if (strstr(gpsData, "ZDA,")) { //Time & Date
      parseZDA(gpsData + 4);
   }
}
