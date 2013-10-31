#ifndef GPS_H_
#define GPS_H_
#include <stddef.h>

#define GPS_QUALITY_NO_FIX 0
#define GPS_QUALITY_SPS 1
#define GPS_QUALITY_DIFFERENTIAL 2

void initGPS();

int checksumValid(const char *gpsData, size_t len);

void processGPSData(char *gpsData, size_t len);

void resetDistance();

float getDistance();

void resetLapCount();

int getLapCount();

float getLastLapTime();

float getLastSplitTime();

int getAtStartFinish();

int getAtSplit();

void getUTCTimeFormatted(char * buf);

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
			
#endif /*GPS_H_*/
