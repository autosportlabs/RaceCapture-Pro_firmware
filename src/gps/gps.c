#include "gps.h"
#include "gps_device.h"
#include "mod_string.h"

#define GPS_LOCK_FLASH_COUNT 5
#define GPS_NOFIX_FLASH_COUNT 50

#define KMS_TO_MILES_CONSTANT (.621371)
#define KNOTS_TO_KPH (1.852)

GpsSamp g_gpsSample;

static int g_flashCount;

static float g_prevLatitude;
static float g_prevLongitude;

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

static bool isGpsSignalUsable(enum GpsSignalQuality q) {
   return q != GPS_QUALITY_NO_FIX;
}

/**
 * @return true if we haven't parsed any data yet, false otherwise.
 */
static bool isGpsDataCold() {
   return g_utcMillisAtSample == 0;
}

millis_t getMillisSinceEpoch() {
   // If we have no GPS data, return 0 to indicate that.
   if (isGpsDataCold()){
      return 0;
   }

   const tiny_millis_t deltaSinceSample = getUptime() - g_uptimeAtSample;
   return g_utcMillisAtSample + deltaSinceSample;
}

long long getMillisSinceEpochAsLongLong() {
   return (long long) getMillisSinceEpoch();
}

tiny_millis_t getUptimeAtSample() {
   return g_uptimeAtSample;
}

void resetGpsDistance() {
   g_gpsSample.distance = 0;
}

void setGpsDistanceKms(float dist) {
	g_gpsSample.distance = dist;
}

float getGpsDistanceKms() {
   return g_gpsSample.distance;
}

float getGpsDistanceMiles() {
	return KMS_TO_MILES_CONSTANT * g_gpsSample.distance;
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

DateTime getLastFixDateTime() {
   return g_dtLastFix;
}

GeoPoint getGeoPoint(){
	return g_gpsSample.point;
}

GpsSamp * getGpsSample(){
	return &g_gpsSample;
}

static float calcDistancesSinceLastSample(GpsSamp *gpsSample) {
   const GeoPoint prev = {g_prevLatitude, g_prevLongitude};

   if (!isValidPoint(&prev) || !isValidPoint(&gpsSample->point)){
      return 0.0;
   }
   // Return distance in KM
   return distPythag(&prev, &gpsSample->point) / 1000;
}

tiny_millis_t getMillisSinceFirstFix() {
   return getTimeDeltaInTinyMillis(g_gpsSample.lastFix, g_gpsSample.firstFix);
}

static void GPS_sample_update(GpsSamp * gpsSample){
	if (!isGpsSignalUsable(gpsSample->quality)){
		return;
	}

	float dist = calcDistancesSinceLastSample(gpsSample);
	gpsSample->distance += dist;

	g_prevLatitude = gpsSample->point.latitude;
	g_prevLongitude = gpsSample->point.longitude;
}

void initGPS(){
	memset(&g_gpsSample, 0, sizeof(GpsSamp));
	g_dtFirstFix = g_dtLastFix = (DateTime) { 0 };
	g_utcMillisAtSample = 0;
	g_flashCount = 0;
	g_distance = 0;
	g_uptimeAtSample = 0;
	g_speed = 0;
	g_prevLatitude = 0.0;
	g_prevLongitude = 0.0;

}

int GPS_processUpdate(Serial *serial){
	gps_msg_result_t result = GPS_device_get_update(&g_gpsSample, serial);
	flashGpsStatusLed(g_gpsSample.quality);
	if (result == GPS_MSG_SUCCESS){
		GPS_sample_update(&g_gpsSample);
	}
	return result;
}
