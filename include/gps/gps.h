#ifndef GPS_H_
#define GPS_H_

#include "dateTime.h"
#include <stddef.h>

#define GPS_QUALITY_NO_FIX 0
#define GPS_QUALITY_SPS 1
#define GPS_QUALITY_DIFFERENTIAL 2

void gpsConfigChanged(void);

void initGPS();

int checksumValid(const char *gpsData, size_t len);

void processGPSData(char *gpsData, size_t len);

void resetGpsDistance();

float getGpsDistance();

void resetLapCount();

int getLapCount();

float getLastLapTime();

float getLastSectorTime();

int getLastSector();

int getAtStartFinish();

int getAtSector();

void getUTCTimeFormatted(char buf[], size_t len);

float getUTCTime();

void setUTCTime(float UTCTime);

float getSecondsSinceMidnight();

void updateSecondsSinceMidnight(float secondsSinceMidnight);

double calculateSecondsSinceMidnight(const char * rawTime);

float getTimeDiff(float from, float to);

float getTimeSince(float t1);

float getLatitude();

float getLongitude();

void updatePosition(float latitude, float longitude);

void onLocationUpdated();

int getGPSQuality();

void setGPSQuality(int quality);

int getSatellitesUsedForPosition();

float getGPSSpeed();

void setGPSSpeed(float speed);

/**
 * Returns Date time information as provided by the GPS system.
 */
DateTime getLastFixDateTime();

/**
 * @return Milliseconds since Unix Epoch.
 */
long getMillisSinceEpoch();

/**
 * This exists for backwards compatibility and will be deprecated in the future.  Use
 * #getLastFixDateTime() or #getMillisSinceEpoch() instead if possible.
 * @return the seconds since the first GPS fix.
 */
float getSecondsSinceFirstFix();

#endif /*GPS_H_*/
