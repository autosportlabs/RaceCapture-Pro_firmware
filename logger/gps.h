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

float getSecondsSinceMidnight();

float getTimeDiff(float from, float to);

float getTimeSince(float t1);

float getLatitude();

float getLongitude();

int getGPSQuality();

int getSatellitesUsedForPosition();

float getGPSSpeed();
			
#endif /*GPS_H_*/
