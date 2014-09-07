#include "auto_track.h"
#include "dateTime.h"
#include "gps.h"
#include "geopoint.h"
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

#define START_FINISH_TIME_THRESHOLD 10

#define TIME_NULL -1

#define GPS_LOCKED_ON(QUALITY) QUALITY != GPS_QUALITY_NO_FIX

static const Track * g_activeTrack;

static int g_configured;
static int g_flashCount;
static float g_prevLatitude;
static float g_prevLongitude;

static float g_latitude;
static float g_longitude;

static float g_prevSecondsSinceMidnight;
static float g_secondsSinceMidnight;

static float g_speed;

static int g_gpsQuality;

static int g_satellitesUsedForPosition;

static int g_atStartFinish;
static int g_prevAtStartFinish;
static float g_lastStartFinishTimestamp;

static int g_atTarget;
static int g_prevAtTarget;
static float g_lastSectorTimestamp;

static int g_sector;
static int g_lastSector;

static float g_lastLapTime;
static float g_lastSectorTime;

static int g_lapCount;
static float g_distance;

/**
 * Date and time this GPS fix was taken.
 */
static DateTime g_dtFirstFix;
static DateTime g_dtLastFix;
static unsigned long g_millisSinceUnixEpoch;

/**
 * @return true if we haven't parsed any data yet, false otherwise.
 */
static bool isGpsDataCold() {
   return g_millisSinceUnixEpoch == 0;
}

void updateMillisSinceEpoch(DateTime fixDateTime) {
   g_millisSinceUnixEpoch = getMillisecondsSinceUnixEpoch(fixDateTime);
}

long getMillisSinceEpoch() {
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
   double secondsSinceMidnight = 0.0;

   int keepParsing = 1;

   while (delim != NULL && keepParsing) {
      *delim = '\0';
      switch (param) {
      case 0: {
         unsigned int len = strlen(data);

         if (len > 0 && len < UTC_TIME_BUFFER_LEN)
            secondsSinceMidnight = calculateSecondsSinceMidnight(data);

      }
         break;
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
   updateSecondsSinceMidnight(secondsSinceMidnight);
   updatePosition(latitude, longitude);
}

double calculateSecondsSinceMidnight(const char * rawTime) {
   char hh[3];
   char mm[3];
   char ss[7];

   memcpy(hh, rawTime, 2);
   hh[2] = '\0';
   memcpy(mm, rawTime + 2, 2);
   mm[2] = '\0';
   memcpy(ss, rawTime + 4, 6);
   ss[6] = '\0';
   int hour = modp_atoi(hh);
   int minutes = modp_atoi(mm);
   float seconds = modp_atof(ss);

   return (hour * 60.0 * 60.0) + (minutes * 60.0) + seconds;
}

void updateSecondsSinceMidnight(float secondsSinceMidnight) {
   g_prevSecondsSinceMidnight = g_secondsSinceMidnight;
   g_secondsSinceMidnight = secondsSinceMidnight;
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

float getLastLapTime() {
   return g_lastLapTime;
}

float getLastSectorTime() {
   return g_lastSectorTime;
}

float getTimeSince(float t1) {
   return getTimeDiff(t1, getSecondsSinceMidnight());
}

float getTimeDiff(float from, float to) {
   if (to < from) {
      to += 86400;
   }
   return to - from;
}

int getAtStartFinish() {
   return g_atStartFinish;
}

int getAtSector() {
   return g_atTarget;
}

float getSecondsSinceMidnight() {
   return g_secondsSinceMidnight;
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

static float calcDistancesSinceLastSample() {
   float d = 0;
   if (0 != g_prevLatitude && 0 != g_prevLongitude) {
      float lat1 = toRadians(g_prevLatitude);
      float lon1 = toRadians(g_prevLongitude);

      float lat2 = toRadians(g_latitude);
      float lon2 = toRadians(g_longitude);

      float x = (lon2 - lon1) * cos((lat1 + lat2) / 2);
      float y = (lat2 - lat1);
      d = sqrtf(x * x + y * y) * DISTANCE_SCALING;
   }
   return d;
}

static int processStartFinish(const Track *track, float targetRadius) {
   int lapDetected = 0;
   const GeoPoint finishLine = getFinishPoint(track);
   g_atStartFinish = withinGpsTarget(&finishLine, targetRadius);
   if (g_atStartFinish) {
      if (g_prevAtStartFinish == 0) {
         if (g_lastStartFinishTimestamp == 0) {
            //loading of lap zero timestamp; e.g. first time crossing line
            g_lastStartFinishTimestamp = getSecondsSinceMidnight();
         } else {
            //guard against false triggering.
            //We have to be out of the start/finish target for some amount of time
            float currentTimestamp = getSecondsSinceMidnight();
            float elapsed = getTimeDiff(g_lastStartFinishTimestamp,
                  currentTimestamp);
            if (elapsed > START_FINISH_TIME_THRESHOLD) {
               //NOW we can record an new lap
               float lapTime = elapsed / 60.0;
               g_lapCount++;
               g_lastLapTime = lapTime;
               g_lastStartFinishTimestamp = currentTimestamp;
               lapDetected = 1;
               g_sector = 0; // Should not be needed
            }
         }
      }
      g_prevAtStartFinish = 1;
   } else {
      g_prevAtStartFinish = 0;
   }
   return lapDetected;
}

static void processSector(const Track *track, float targetRadius) {
   GeoPoint point = getSectorGeoPointAtIndex(track, g_sector);

   g_atTarget = withinGpsTarget(&point, targetRadius);
   if (!g_atTarget) {
      g_prevAtTarget = 0;
      return;
   }

   // Latch here to prevent duplicate entries.
   if (g_prevAtTarget != 0)
      return;

   g_prevAtTarget = 1;


   /*
    * Past here we are sure we are at a sector boundary and haven't counted twice.
    */

   float currentTimestamp = getSecondsSinceMidnight();
   float elapsed = getTimeDiff(g_lastSectorTimestamp, currentTimestamp);

   g_lastSectorTimestamp = currentTimestamp;
   g_lastSectorTime = elapsed / 60.0;
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
   g_secondsSinceMidnight = TIME_NULL;
   g_prevSecondsSinceMidnight = TIME_NULL;
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
   g_lastSectorTime = 0.0;
   g_atStartFinish = 0;
   g_prevAtStartFinish = 0;
   g_lastStartFinishTimestamp = 0;
   g_atTarget = 0;
   g_prevAtTarget = 0;
   g_lastSectorTimestamp = 0.0;
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

float getSecondsSinceFirstFix() {
   long diffInMillis = getTimeDeltaInMillis(g_dtLastFix, g_dtFirstFix);
   return ((float) diffInMillis) / 1000;
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
      g_configured = 1;
   }

   float dist = calcDistancesSinceLastSample();
   g_distance += dist;

   const float targetRadius = config->TrackConfigs.radius;

   if (sectorEnabled)
      processSector(g_activeTrack, targetRadius);

   if (startFinishEnabled) {
      // Seconds since first fix is good until we alter the code to use millis directly
      const float secondsSinceFirstFix = getSecondsSinceFirstFix();
      const int lapDetected = processStartFinish(g_activeTrack, targetRadius);
      if (lapDetected) {
         resetGpsDistance();
         startFinishCrossed(gp, secondsSinceFirstFix);
      } else {
         addGpsSample(gp, secondsSinceFirstFix);
      }
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
