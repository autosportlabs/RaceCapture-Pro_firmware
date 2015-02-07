#include "gps.h"
#include "gps_device.h"
#include "mod_string.h"

#define GPS_LOCK_FLASH_COUNT 5
#define GPS_NOFIX_FLASH_COUNT 50

static GpsSample g_gpsSample;

static int g_flashCount;

static millis_t g_timeFirstFix;
static tiny_millis_t g_uptimeAtSample;

static float g_prevLatitude;
static float g_prevLongitude;

bool isGpsSignalUsable(enum GpsSignalQuality q) {
   return q != GPS_QUALITY_NO_FIX;
}

static void flashGpsStatusLed(enum GpsSignalQuality gpsQuality) {
   if (g_flashCount == 0){
      LED_disable(1);
   }
   g_flashCount++;

   int targetFlashCount = isGpsSignalUsable(gpsQuality) ?
      GPS_LOCK_FLASH_COUNT : GPS_NOFIX_FLASH_COUNT;

   if (g_flashCount >= targetFlashCount) {
      LED_enable(1);
      g_flashCount = 0;
   }
}

/**
 * @return true if we haven't parsed any data yet, false otherwise.
 */
bool isGpsDataCold() {
   return g_gpsSample.time == 0;
}

static tiny_millis_t getDeltaSinceSample() {
   return getUptime() - g_uptimeAtSample;
}

millis_t getMillisSinceEpoch() {
   // If we have no GPS data, return 0 to indicate that.
   if (isGpsDataCold()) return 0;

   //interpolate milliseconds from system clock
   return g_gpsSample.time + getDeltaSinceSample();
}

tiny_millis_t getMillisSinceFirstFix() {
   // If we have no GPS data, return 0 to indicate that.
   if (isGpsDataCold()) return 0;

   return (tiny_millis_t) (getMillisSinceEpoch() - g_timeFirstFix);
}

long long getMillisSinceEpochAsLongLong() {
   return (long long) getMillisSinceEpoch();
}

tiny_millis_t getUptimeAtSample() {
   return g_uptimeAtSample;
}

float getLatitude() {
   return g_gpsSample.point.latitude;
}

float getLongitude() {
   return g_gpsSample.point.longitude;
}

enum GpsSignalQuality getGPSQuality() {
   return g_gpsSample.quality;
}

void setGPSQuality(enum GpsSignalQuality quality) {
   g_gpsSample.quality = quality;
}

int getSatellitesUsedForPosition() {
   return g_gpsSample.satellites;
}

float getGPSSpeed() {
   return g_gpsSample.speed;
}

float getGpsSpeedInMph() {
   return getGPSSpeed() * 0.621371192; //convert to MPH
}

void setGPSSpeed(float speed) {
   g_gpsSample.speed = speed;
}

millis_t getLastFix() {
   return g_gpsSample.time;
}

GeoPoint getGeoPoint() {
   return g_gpsSample.point;
}

GeoPoint getPreviousGeoPoint() {
   return (GeoPoint) {g_prevLatitude, g_prevLongitude};
}

GpsSample getGpsSample() {
   return g_gpsSample;
}

static void updateFullDateTime(GpsSample *gpsSample) {
	g_uptimeAtSample = getUptime();

	if (g_timeFirstFix == 0) g_timeFirstFix = gpsSample->time;
}

static void GPS_sample_update(GpsSample *gpsSample){
   if (!isGpsSignalUsable(gpsSample->quality)) return;

   // Deep copy intentional
   g_gpsSample = *gpsSample;
   updateFullDateTime(gpsSample);

   g_prevLatitude = gpsSample->point.latitude;
   g_prevLongitude = gpsSample->point.longitude;
}


void GPS_init(){
   g_gpsSample = (GpsSample) { 0 };
	g_timeFirstFix = 0;
	g_flashCount = 0;
	g_uptimeAtSample = 0;
	g_gpsSample.speed = 0;
	g_prevLatitude = 0.0;
	g_prevLongitude = 0.0;

}

int GPS_processUpdate(Serial *serial){
   GpsSample s = { 0 };
	const gps_msg_result_t result = GPS_device_get_update(&s, serial);

	flashGpsStatusLed(g_gpsSample.quality);

	if (result == GPS_MSG_SUCCESS){
		GPS_sample_update(&s);
	}

	return result;
}
