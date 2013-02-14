#include "gps.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "usart.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "geometry.h"

#include <string.h>

#define GPS_DATA_LINE_BUFFER_LEN 	200
#define GPS_TASK_PRIORITY 			( tskIDLE_PRIORITY + 2 )
#define GPS_TASK_STACK_SIZE			100


#define GPS_QUALITY_NO_FIX 0
#define GPS_QUALITY_SPS 1
#define GPS_QUALITY_DIFFERENTIAL 2

#define GPS_LOCK_FLASH_COUNT 2
#define GPS_NOFIX_FLASH_COUNT 10

#define LATITUDE_DATA_LEN 12
#define LONGITUDE_DATA_LEN 13

#define UTC_TIME_BUFFER_LEN 11
#define UTC_SPEED_BUFFER_LEN 10

#define START_FINISH_TIME_THRESHOLD 30

static char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

static float	g_latitude;
static float 	g_longitude;

static float	g_UTCTime;
static float 	g_secondsSinceMidnight;

static float	g_speed;

static int		g_gpsQuality;

static int		g_satellitesUsedForPosition;

static int g_atStartFinish;

static int g_prevAtStartFinish;

static float g_lastStartFinishTimestamp;

static float g_lastLapTime;

static int g_lapCount;

void setLapCount(int lapCount){
	g_lapCount = lapCount;
}

int getLapCount(){
	return g_lapCount;
}

float getLastLapTime(){
	return g_lastLapTime;
}

float getTimeSince(float t1){
	return getTimeDiff(t1,getSecondsSinceMidnight());
}

float getTimeDiff(float from, float to){
	if (to < from){
		to+=86400;
	}
	return to - from;
}

int getAtStartFinish(){
	return g_atStartFinish;
}

float getUTCTime(){
	return g_UTCTime;
}

float getSecondsSinceMidnight(){
	return g_secondsSinceMidnight;
}

void getUTCTimeFormatted(char * buf){
	
}

float getLatitude(){
	return g_latitude;
}

float getLongitude(){
	return g_longitude;
}

int getGPSQuality(){
	return g_gpsQuality;
}

int getSatellitesUsedForPosition(){
	return g_satellitesUsedForPosition;
}

float getGPSSpeed(){
	return g_speed;
}

char * getGPSDataLine(){
	return g_GPSdataLine;
}

static int atStartFinish(GPSConfig *gpsConfig){

	struct circ_area area;
	struct point p;
	p.x = gpsConfig->startFinishLongitude;
	p.y = gpsConfig->startFinishLatitude;

	struct point currentP;
	currentP.x = getLongitude();
	currentP.y = getLatitude();

	create_circ(&area,&p,gpsConfig->startFinishRadius);
	int within =  within_circ(&area,&currentP);
/*	SendFloat(p.x,10);
	SendString(" ");
	SendFloat(p.y,10);
	SendString(" within: ");
	SendInt(within);
	SendString(" ");
	SendFloat(g_startFinishRadius,10);
	SendCrlf();
	*/
	return within;
}

static int isStartFinishDetectionEnabled(GPSConfig *gpsConfig){
	return gpsConfig->startFinishLatitude != 0 && gpsConfig->startFinishLongitude != 0;
}


static void updateStartFinish(void){
	GPSConfig *gpsConfig = &(getWorkingLoggerConfig()->GPSConfig);
	if (! isStartFinishDetectionEnabled(gpsConfig)) return;

	g_atStartFinish = atStartFinish(gpsConfig);
	if (g_atStartFinish){
		if (g_prevAtStartFinish == 0){
			if (g_lastStartFinishTimestamp == 0){
				g_lastStartFinishTimestamp = getSecondsSinceMidnight();
			}
			else{
				//guard against false triggering.
				//We have to be out of the start/finish target for some amount of time
				float currentTimestamp = getSecondsSinceMidnight();
				float elapsed = getTimeDiff(g_lastStartFinishTimestamp,currentTimestamp);
				if (elapsed > START_FINISH_TIME_THRESHOLD){
					//NOW we can record an new lap
					float lapTime = elapsed / 60.0;
					g_lapCount++;
					g_lastLapTime = lapTime;
					g_lastStartFinishTimestamp = currentTimestamp;
				}
			}
		}
		g_prevAtStartFinish = 1;
	}
	else{
		g_prevAtStartFinish = 0;
	}
}



void startGPSTask(){
	g_latitude = 0.0;
	g_longitude = 0.0;
	g_UTCTime = 0.0;
	g_gpsQuality = GPS_QUALITY_NO_FIX;
	g_satellitesUsedForPosition = 0;
	g_speed = 0.0;
	g_atStartFinish = 0;
	g_lastLapTime = 0;
	g_prevAtStartFinish = 0;
	g_lastStartFinishTimestamp = 0;
	g_lapCount = 0;
	
	initUsart1(USART_MODE_8N1, 38400);
	xTaskCreate( GPSTask, ( signed portCHAR * ) "GPSTask", GPS_TASK_STACK_SIZE, NULL, 	GPS_TASK_PRIORITY, 	NULL );
}

void GPSTask( void *pvParameters ){
	
	int flashCount = 0;
	for( ;; )
	{
		int len = usart1_readLine(g_GPSdataLine, GPS_DATA_LINE_BUFFER_LEN);
		if (len > 0){
			if (*g_GPSdataLine == '$' && *(g_GPSdataLine + 1) =='G' && *(g_GPSdataLine + 2) == 'P'){
				char * data = g_GPSdataLine + 3;
				if (strstr(data,"GGA,")){
					parseGGA(data + 4);
					if (flashCount == 0) disableLED(LED1);
					flashCount++;
					int targetFlashCount = (g_gpsQuality == GPS_QUALITY_NO_FIX ? GPS_NOFIX_FLASH_COUNT: GPS_LOCK_FLASH_COUNT);
					if (flashCount >= targetFlashCount){
						enableLED(LED1);
						flashCount = 0;		
					}
					updateStartFinish();
				} else if (strstr(data,"VTG,")){ //Course Over Ground and Ground Speed
					parseVTG(data + 4);
				} else if (strstr(data,"GSA,")){ //GPS Fix data
					parseGSA(data + 4);						
				} else if (strstr(data,"GSV,")){ //Satellites in view
					parseGSV(data + 4);					
				} else if (strstr(data,"RMC,")){ //Recommended Minimum Specific GNSS Data
					parseRMC(data + 4);					
				}  else if (strstr(data,"GLL,")){ //Geographic Position - Latitude/Longitude
					parseGLL(data + 4);					
				} else if (strstr(data,"ZDA,")){ //Time & Date
					parseZDA(data + 4);
				}
			}
		}
	}
}

//Parse Global Positioning System Fix Data.
void parseGGA(char *data){

	//SendString(data);
	
	char * delim = strchr(data,',');
	int param = 0;
	
	double latitude = 0.0;
	double longitude = 0.0;
	
	int keepParsing = 1;
	
	while (delim != NULL && keepParsing){
		*delim = '\0';
		switch (param){
			case 0:
				{
					unsigned int len = strlen(data);
					if (len > 0 && len < UTC_TIME_BUFFER_LEN){
						g_UTCTime = modp_atof(data);

						char hh[3];
						char mm[3];
						char ss[7];

						memcpy(hh,data,2);
						hh[2] = '\0';
						memcpy(mm,data+2,2);
						mm[2] = '\0';
						memcpy(ss,data+4,6);
						ss[6] = '\0';
						int hour = modp_atoi(hh);
						int minutes = modp_atoi(mm);
						float seconds = modp_atof(ss);

						g_secondsSinceMidnight = (hour * 60.0 * 60.0) + (minutes * 60.0) + seconds;
					}
				}
				break;
			case 1:
				{
					unsigned int len = strlen(data);
					if ( len > 0 && len <= LATITUDE_DATA_LEN ){
						//Raw GPS Format is ddmm.mmmmmm
//						latitude = modp_atod(data);
												
						char degreesStr[3];
						strncpy(degreesStr, data, 2);
						degreesStr[2] = 0;
						float minutes = modp_atof(data + 2);
						minutes = minutes / 60.0;
						latitude = modp_atoi(degreesStr) + minutes;
						
					}
					else{
						latitude = 0;
						//TODO log error	
					}
				}
				break;
			case 2:
				{
					if (data[0] == 'S'){
						latitude = -latitude;
					}
				}
				break;
			case 3:
				{	
					unsigned int len = strlen(data);
					if ( len > 0 && len <= LONGITUDE_DATA_LEN ){
						//Raw GPS Format is dddmm.mmmmmm
						
//						longitude = modp_atod(data);

						char degreesStr[4];
						strncpy(degreesStr, data, 3);
						degreesStr[3] = 0;
						float minutes = modp_atof(data + 3);
						minutes = minutes / 60.0;
						longitude = modp_atoi(degreesStr) + minutes;
					}
					else{
						longitude = 0;
						//TODO log error	
					}
				}
				break;
			case 4:
				{
					if (data[0] == 'W'){
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
		delim = strchr(data,',');
	}

	g_longitude = longitude;
	g_latitude = latitude;
}

//Parse GNSS DOP and Active Satellites
void parseGSA(char *data){
	
}

//Parse Course Over Ground and Ground Speed
void parseVTG(char *data){

	char * delim = strchr(data,',');
	int param = 0;
	
	int keepParsing = 1;
	while (delim != NULL && keepParsing){
		*delim = '\0';
		switch (param){
			case 6: //Speed over ground
				{
					if (strlen(data) >= 1){
						g_speed = modp_atof(data) * 0.621371; //convert to MPH
					}
					keepParsing = 0;
				}
				break;
			default:
				break;
		}
		param++;
		data = delim + 1;
		delim = strchr(data,',');
	}
}

//Parse Geographic Position � Latitude / Longitude
void parseGLL(char *data){
	
}

//Parse Time & Date
void parseZDA(char *data){
}

//Parse GNSS Satellites in View
void parseGSV(char *data){

}

//Parse Recommended Minimum Navigation Information
void parseRMC(char *data){
	
}
