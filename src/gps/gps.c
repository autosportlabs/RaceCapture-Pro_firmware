#include "auto_track.h"
#include "gps.h"
#include "geopoint.h"
#include "loggerHardware.h"
#include "LED.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "geometry.h"
#include "mod_string.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include <math.h>

//kilometers
#define DISTANCE_SCALING 6371
#define PI 3.1415

#define GPS_LOCK_FLASH_COUNT 2
#define GPS_NOFIX_FLASH_COUNT 10

#define LATITUDE_DATA_LEN 12
#define LONGITUDE_DATA_LEN 13

#define UTC_TIME_BUFFER_LEN 11
#define UTC_SPEED_BUFFER_LEN 10

#define START_FINISH_TIME_THRESHOLD 10

#define TIME_NULL -1

#define GPS_LOCKED_ON(QUALITY) QUALITY != GPS_QUALITY_NO_FIX

static const Track * g_activeTrack;

static int		g_configured;
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

static int 		g_atStartFinish;
static int 		g_prevAtStartFinish;
static float 	g_lastStartFinishTimestamp;

static int 		g_atTarget;
static int 		g_prevAtTarget;
static float 	g_lastSectorTimestamp;

static int		g_sector;
static int		g_lastSector;
static float 	g_lastLapTime;
static float 	g_lastSectorTime;

static int 		g_lapCount;
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
						setUTCTime(modp_atof(data));
						secondsSinceMidnight = calculateSecondsSinceMidnight(data);
					}
				}
				break;
			case 1:
				{
					unsigned int len = strlen(data);
					if ( len > 0 && len <= LATITUDE_DATA_LEN ){
						//Raw GPS Format is ddmm.mmmmmm
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
	updateSecondsSinceMidnight(secondsSinceMidnight);
	updatePosition(latitude, longitude);
}

double calculateSecondsSinceMidnight(const char * rawTime){
	char hh[3];
	char mm[3];
	char ss[7];

	memcpy(hh, rawTime, 2);
	hh[2] = '\0';
	memcpy(mm, rawTime + 2, 2);
	mm[2] = '\0';
	memcpy(ss, rawTime + 4, 6);
	ss[6] = '\0';
	int hour = modp_atoi(hh);
	int minutes = modp_atoi(mm);
	float seconds = modp_atof(ss);

	return (hour * 60.0 * 60.0) + (minutes * 60.0) + seconds;
}

void updateSecondsSinceMidnight(float secondsSinceMidnight){
	g_prevSecondsSinceMidnight = g_secondsSinceMidnight;
	g_secondsSinceMidnight = secondsSinceMidnight;
}

void updatePosition(float latitude, float longitude){
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
						setGPSSpeed(modp_atof(data)); //convert to MPH
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

void resetGpsDistance(){
	g_distance = 0;
}

float getGpsDistance(){
	return g_distance;
}

void resetLapCount(){
	g_lapCount = 0;
}

int getLapCount(){
	return g_lapCount;
}

int getLastSector(){
	return g_lastSector;
}

float getLastLapTime(){
	return g_lastLapTime;
}

float getLastSectorTime(){
	return g_lastSectorTime;
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

int getAtSector(){
	return g_atTarget;
}

float getUTCTime(){
	return g_UTCTime;
}

void setUTCTime(float UTCTime){
	g_UTCTime = UTCTime;
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

void setGPSQuality(int quality){
	g_gpsQuality = quality;
}

int getSatellitesUsedForPosition(){
	return g_satellitesUsedForPosition;
}

float getGPSSpeed(){
	return g_speed;
}

void setGPSSpeed(float speed){
	g_speed = speed;
}

static int withinGpsTarget(const GeoPoint *point, float radius){

	struct circ_area area;
	struct point p;
	p.x = point->longitude;
	p.y = point->latitude;

	struct point currentP;
	currentP.x = getLongitude();
	currentP.y = getLatitude();

	create_circ(&area,&p, radius);
	int within =  within_circ(&area,&currentP);
	return within;
}

static int isStartFinishEnabled(const Track *track){
	int isEnabled = 0;
	if (track != NULL){
		const GeoPoint *p = &track->startFinish;
		isEnabled = p->latitude != 0 && p->longitude != 0;
	}
	return isEnabled;
}

static float toRadians(float degrees){
	return degrees * PI / 180.0;
}

static float calcDistancesSinceLastSample(){
	float d = 0;
	if (0 != g_prevLatitude && 0 != g_prevLongitude){
		float lat1 = toRadians(g_prevLatitude);
		float lon1 = toRadians(g_prevLongitude);

		float lat2 = toRadians(g_latitude);
		float lon2 = toRadians(g_longitude);

		float x = (lon2-lon1) * cos((lat1+lat2)/2);
		float y = (lat2-lat1);
		d = sqrtf(x*x + y*y) * DISTANCE_SCALING;
	}
	return d;
}

static int processStartFinish(const Track *track){
	int lapDetected = 0;
	g_atStartFinish = withinGpsTarget(&track->startFinish, track->radius);
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
					lapDetected = 1;
					g_sector = 0;
				}
			}
		}
		g_prevAtStartFinish = 1;
	}
	else{
		g_prevAtStartFinish = 0;
	}
	return lapDetected;
}

static void processSector(const Track *track){
	const GeoPoint *nextSectorPoint = (track->sectors + g_sector);
	if (nextSectorPoint->latitude != 0 && nextSectorPoint->longitude != 0 ){ //valid sector target?
		g_atTarget = withinGpsTarget(nextSectorPoint, track->radius);
		if (g_atTarget){
			if (g_prevAtTarget == 0){ //latching effect, to avoid double triggering
				//first sector refreences from start finish; subsequent sectors reference from last sector timestamp
				float fromTimestamp = g_sector = 0 ? g_lastStartFinishTimestamp : g_lastSectorTimestamp;

				if (fromTimestamp != 0){
					float currentTimestamp = getSecondsSinceMidnight();
					float elapsed = getTimeDiff(g_lastStartFinishTimestamp, currentTimestamp);
					g_lastSectorTime = elapsed / 60.0;

					//set some channel values now
					g_lastSectorTimestamp = currentTimestamp;
					if (g_sector < SECTOR_COUNT - 1) g_sector++;
					g_lastSector = g_sector;
				}
			}
			g_prevAtTarget = 1;
		}
		else{
			g_prevAtTarget = 0;
		}
	}
}

void gpsConfigChanged(void){
	g_configured = 0;
}

void initGPS(){
	g_configured = 0;
	g_activeTrack = NULL;
	g_secondsSinceMidnight = TIME_NULL;
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
	g_lastSectorTime = 0;
	g_atStartFinish = 0;
	g_prevAtStartFinish = 0;
	g_lastStartFinishTimestamp = 0;
	g_atTarget = 0;
	g_prevAtTarget = 0;
	g_lastSectorTimestamp = 0;
	g_lapCount = 0;
	g_distance = 0;
	g_sector = 0;
	g_lastSector = 0;
	resetPredictiveTimer();
}

static void flashGpsStatusLed(){
	if (g_flashCount == 0) LED_disable(1);
	g_flashCount++;
	int targetFlashCount = (GPS_LOCKED_ON(g_gpsQuality) ? GPS_LOCK_FLASH_COUNT : GPS_NOFIX_FLASH_COUNT );
	if (g_flashCount >= targetFlashCount){
		LED_enable(1);
		g_flashCount = 0;
	}
}

void onLocationUpdated(){
	static int sectorEnabled = 0;
	static int startFinishEnabled = 0;

	if (GPS_LOCKED_ON(g_gpsQuality)) {
		LoggerConfig *config = getWorkingLoggerConfig();

		GeoPoint gp;
		populateGeoPoint(&gp);

       	if (! g_configured){
           Track *defaultTrack = &(config->TrackConfigs.track);
       	   g_activeTrack = auto_configure_track(defaultTrack, gp);
       	   startFinishEnabled = isStartFinishEnabled(g_activeTrack);
       	   sectorEnabled = config->LapConfigs.sectorTimeCfg.sampleRate != SAMPLE_DISABLED && startFinishEnabled;
     	   g_configured = 1;
       	}


       	float dist = calcDistancesSinceLastSample();
		g_distance += dist;

		if (sectorEnabled){
			processSector(g_activeTrack);
		}

		if (startFinishEnabled){
         // HACK!  Need seconds since epoch.  This solution sucks.
			float utcTime = getSecondsSinceMidnight();
			int lapDetected = processStartFinish(g_activeTrack);

			if (lapDetected){
				resetGpsDistance();
				startFinishCrossed(gp, utcTime);
			} else {
				addGpsSample(gp, utcTime);
			}
		}
	}
}

int checksumValid(const char *gpsData, size_t len){
	int valid = 0;
	unsigned char checksum = 0;
	size_t i = 0;
	for (; i < len - 1; i++){
		char c = *(gpsData + i);
		if (c == '*' || c == '\0') break;
		else if (c == '$') continue;
		else checksum ^= c;
	}
	if (len > i + 2){
		unsigned char dataChecksum = modp_xtoc(gpsData + i + 1);
		if (checksum == dataChecksum) valid = 1;
	}
	return valid;
}

void processGPSData(char *gpsData, size_t len){
	if (len > 4 && checksumValid(gpsData, len)){
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
	else{
		pr_warning("GPS: corrupt frame\r\n");
	}
}
