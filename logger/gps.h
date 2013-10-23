#ifndef GPS_H_
#define GPS_H_
#include <stddef.h>

void initGPS();

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

int getGPSQuality();

int getSatellitesUsedForPosition();

float getGPSSpeed();

void setGPSSpeed(float speed);
			
#endif /*GPS_H_*/
