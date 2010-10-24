#include "gps.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "usart.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "usb_comm.h"
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
#define UTC_VELOCITY_BUFFER_LEN 10

char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

double	g_latitude;
float 	g_longitude;

float	g_UTCTime;
float 	g_secondsSinceMidnight;

float	g_velocity;

int		g_gpsQuality;
int		g_satellitesUsedForPosition;

float getTimeDiff(float t1, float t2){
	if (t2 < t1){
		t2+=86400;
	}
	return t2 - t1;
}

float getUTCTime(){
	return g_UTCTime;
}

float getSecondsSinceMidnight(){
	return g_secondsSinceMidnight;
}

void getUTCTimeFormatted(char * buf){
	
}


double getLatitude(){
	return g_latitude;
}

double getLongitude(){
	return g_longitude;
}

int getGPSQuality(){
	return g_gpsQuality;
}

int getSatellitesUsedForPosition(){
	return g_satellitesUsedForPosition;
}

float getGPSVelocity(){
	return g_velocity;
}

char * getGPSDataLine(){
	return g_GPSdataLine;
}

void startGPSTask(){
	g_latitude = 0.0;
	g_longitude = 0.0;
	g_UTCTime = 0.0;
	g_gpsQuality = GPS_QUALITY_NO_FIX;
	g_satellitesUsedForPosition = 0;
	g_velocity = 0.0;
	
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
					if (flashCount == 0) DisableLED(LED1);
					flashCount++;
					int targetFlashCount = (g_gpsQuality == GPS_QUALITY_NO_FIX ? GPS_NOFIX_FLASH_COUNT: GPS_LOCK_FLASH_COUNT);
					if (flashCount >= targetFlashCount){
						EnableLED(LED1);
						flashCount = 0;		
					}
				} else if (strstr(data,"GSA,")){ //GPS Fix data
					parseGSA(data + 4);						
				} else if (strstr(data,"GSV,")){ //Satellites in view
					parseGSV(data + 4);					
				} else if (strstr(data,"RMC,")){ //Recommended Minimum Specific GNSS Data
					parseRMC(data + 4);					
				} else if (strstr(data,"VTG,")){ //Course Over Ground and Ground Speed
					parseVTG(data + 4);					
				} else if (strstr(data,"GLL,")){ //Geographic Position - Latitude/Longitude
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
						g_velocity = modp_atof(data);
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
