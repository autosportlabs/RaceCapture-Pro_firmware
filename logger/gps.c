#include "gps.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "geometry.h"
#include "mod_string.h"
#include "predictive_timer.h"
#include <math.h>

//kilometers
#define DISTANCE_SCALING 6371
#define PI 3.1415

#define GPS_QUALITY_NO_FIX 0
#define GPS_QUALITY_SPS 1
#define GPS_QUALITY_DIFFERENTIAL 2

#define GPS_LOCK_FLASH_COUNT 2
#define GPS_NOFIX_FLASH_COUNT 10

#define LATITUDE_DATA_LEN 12
#define LONGITUDE_DATA_LEN 13

#define UTC_TIME_BUFFER_LEN 11
#define UTC_SPEED_BUFFER_LEN 10

#define START_FINISH_TIME_THRESHOLD 10

#define TIME_NULL -1

static int		g_flashCount;
static float	g_prevLatitude;
static float	g_prevLongitude;

static float	g_latitude;
static float 	g_longitude;

static float	g_UTCTime;
static float 	g_prevSecondsSinceMidnight;
static float 	g_secondsSinceMidnight;

static float	g_speed;

static int		g_gpsQuality;

static int		g_satellitesUsedForPosition;

static int g_atStartFinish;
static int g_prevAtStartFinish;
static float g_lastStartFinishTimestamp;

static int g_atSplit;
static int g_prevAtSplit;
static float g_lastSplitTimestamp;

static float g_lastLapTime;
static float g_lastSplitTime;

static int g_lapCount;

static float	g_distance;

//Parse Global Positioning System Fix Data.
static void parseGGA(char *data){

	//SendString(data);

	char * delim = strchr(data,',');
	int param = 0;

	double latitude = 0.0;
	double longitude = 0.0;
	double secondsSinceMidnight = 0.0;

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

						secondsSinceMidnight = (hour * 60.0 * 60.0) + (minutes * 60.0) + seconds;
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

	g_prevSecondsSinceMidnight = g_secondsSinceMidnight;
	g_secondsSinceMidnight = secondsSinceMidnight;
	g_prevLatitude = g_latitude;
	g_prevLongitude = g_longitude;
	g_longitude = longitude;
	g_latitude = latitude;
}

//Parse GNSS DOP and Active Satellites
static void parseGSA(char *data){

}

//Parse Course Over Ground and Ground Speed
static void parseVTG(char *data){

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
static void parseGLL(char *data){

}

//Parse Time & Date
static void parseZDA(char *data){
}

//Parse GNSS Satellites in View
static void parseGSV(char *data){

}

//Parse Recommended Minimum Navigation Information
static void parseRMC(char *data){

}

void resetDistance(){
	g_distance = 0;
}

float getDistance(){
	return g_distance;
}

void resetLapCount(){
	g_lapCount = 0;
}

int getLapCount(){
	return g_lapCount;
}

float getLastLapTime(){
	return g_lastLapTime;
}

float getLastSplitTime(){
	return g_lastSplitTime;
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

int getAtSplit(){
	return g_atSplit;
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

static int withinGpsTarget(GPSTargetConfig *targetConfig){

	struct circ_area area;
	struct point p;
	p.x = targetConfig->longitude;
	p.y = targetConfig->latitude;

	struct point currentP;
	currentP.x = getLongitude();
	currentP.y = getLatitude();

	create_circ(&area,&p,targetConfig->targetRadius);
	int within =  within_circ(&area,&currentP);
	return within;
}

static int isGpsTargetEnabled(GPSTargetConfig *targetConfig){
	return targetConfig->latitude != 0 && targetConfig->longitude != 0;
}

static float toRadians(float degrees){
	return degrees * PI / 180.0;
}

static float calcDistancesSinceLastSample(){

	float lat1 = toRadians(g_prevLatitude);
	float lon1 = toRadians(g_prevLongitude);

	float lat2 = toRadians(g_latitude);
	float lon2 = toRadians(g_longitude);

	float x = (lon2-lon1) * cos((lat1+lat2)/2);
	float y = (lat2-lat1);
	float d = sqrtf(x*x + y*y) * DISTANCE_SCALING;

	return d;
}

static float calcTimeSinceLastSample(){
	float time = 0;
	if (g_prevSecondsSinceMidnight >= 0) time = getTimeDiff(g_prevSecondsSinceMidnight, g_secondsSinceMidnight);
	return time;
}

static void processStartFinish(void){
	GPSTargetConfig *targetConfig = &(getWorkingLoggerConfig()->TrackConfigs.startFinishConfig);

	if (! isGpsTargetEnabled(targetConfig)) return;

	g_atStartFinish = withinGpsTarget(targetConfig);
	if (g_atStartFinish){
		if (g_prevAtStartFinish == 0){
			if (g_lastStartFinishTimestamp == 0){
				//loading of lap zero timestamp; e.g. first time crossing line
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
					resetDistance();
					end_lap();
				}
			}
		}
		g_prevAtStartFinish = 1;
	}
	else{
		g_prevAtStartFinish = 0;
	}
}

static void processSplit(void){
	GPSTargetConfig *targetConfig = &(getWorkingLoggerConfig()->TrackConfigs.splitConfig);

	if (! isGpsTargetEnabled(targetConfig)) return;

	g_atSplit = withinGpsTarget(targetConfig);
	if (g_atSplit){
		if (g_prevAtSplit == 0){
			if (g_lastSplitTimestamp == 0){
				g_lastSplitTimestamp = getSecondsSinceMidnight();
			}
			else{
				if (g_lastStartFinishTimestamp != 0){
					float currentTimestamp = getSecondsSinceMidnight();
					float elapsed = getTimeDiff(g_lastStartFinishTimestamp, currentTimestamp);
					g_lastSplitTime = elapsed / 60.0;

				}
			}
		}
		g_prevAtSplit = 1;
	}
	else{
		g_prevAtSplit = 0;
	}
}

void initGPS(){
	g_secondsSinceMidnight = 0;
	g_prevSecondsSinceMidnight = TIME_NULL;
	g_flashCount = 0;
	g_prevLatitude = 0.0;
	g_prevLongitude = 0.0;
	g_latitude = 0.0;
	g_longitude = 0.0;
	g_UTCTime = 0.0;
	g_gpsQuality = GPS_QUALITY_NO_FIX;
	g_satellitesUsedForPosition = 0;
	g_speed = 0.0;
	g_lastLapTime = 0;
	g_lastSplitTime = 0;
	g_atStartFinish = 0;
	g_prevAtStartFinish = 0;
	g_lastStartFinishTimestamp = 0;
	g_atSplit = 0;
	g_prevAtSplit = 0;
	g_lastSplitTimestamp = 0;
	g_lapCount = 0;
	g_distance = 0;
	init_predictive_timer();
}

static void flashGpsStatusLed(){
	if (g_flashCount == 0) disableLED(LED1);
	g_flashCount++;
	int targetFlashCount = (g_gpsQuality == GPS_QUALITY_NO_FIX ? GPS_NOFIX_FLASH_COUNT: GPS_LOCK_FLASH_COUNT);
	if (g_flashCount >= targetFlashCount){
		enableLED(LED1);
		g_flashCount = 0;
	}
}

static void onLocationUpdated(){
	if (g_gpsQuality != GPS_QUALITY_NO_FIX){
		processStartFinish();
		processSplit();

		float dist = calcDistancesSinceLastSample();
		float time = calcTimeSinceLastSample();
		g_distance += dist;
		add_predictive_sample(g_speed,dist,time);
	}
}

void processGPSData(char *gpsData, size_t len){
	if (len > 0){
		if (*gpsData == '$' && *(gpsData + 1) =='G' && *(gpsData + 2) == 'P'){
			char * data = gpsData + 3;
			if (strstr(data,"GGA,")){
				parseGGA(data + 4);
				flashGpsStatusLed();
				onLocationUpdated();
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
