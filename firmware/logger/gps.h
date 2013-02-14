#ifndef GPS_H_
#define GPS_H_

void startGPSTask();

void GPSTask( void *pvParameters );

void setLapCount(int lapCount);

int getLapCount();

float getLastLapTime();

int getAtStartFinish();

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

char * getGPSDataLine();


void parseGGA(char *data);
void parseVTG(char *data);
void parseGSA(char *data);
void parseGLL(char *data);
void parseZDA(char *data);
void parseGSV(char *data);
void parseRMC(char *data);
			
#endif /*GPS_H_*/
