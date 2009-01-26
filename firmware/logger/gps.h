#ifndef GPS_H_
#define GPS_H_

void startGPSTask();

void GPSTask( void *pvParameters );

char * getUTCTime();
char * getLatitude();
char * getLongitude();
int getGPSQuality();
int getSatellitesUsedForPosition();
int getGPSPositionUpdated();
void setGPSPositionStale();
char * getGPSVelocity();
int getGPSVelocityUpdated();
void setGPSVelocityStale();


void parseGGA(char *data);
void parseVTG(char *data);
void parseGSA(char *data);
void parseGLL(char *data);
void parseZDA(char *data);
void parseGSV(char *data);
void parseRMC(char *data);
			
#endif /*GPS_H_*/
