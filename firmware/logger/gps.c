#include "gps.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "usart.h"
#include "string.h"
#include "modp_numtoa.h"
#include <stdlib.h>


#define GPS_DATA_LINE_BUFFER_LEN 	200
#define GPS_TASK_PRIORITY 			( tskIDLE_PRIORITY + 1 )
#define GPS_TASK_STACK_SIZE			100


#define GPS_QUALITY_NO_FIX 0
#define GPS_QUALITY_SPS 1
#define GPS_QUALITY_DIFFERENTIAL 2

#define GPS_LOCK_FLASH_COUNT 2
#define GPS_NOFIX_FLASH_COUNT 10

char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

char 	g_UTCTime[11];
char 	g_latitude[15];
char 	g_longitude[15];
int		g_gpsQuality;
int		g_satellitesUsedForPosition;
int		g_gpsPositionUpdated;

char 	g_velocity[11];
int		g_gpsVelocityUpdated;

char * getUTCTime(){
	return g_UTCTime;
}

char * getLatitude(){
	return g_latitude;
}

char * getLongitude(){
	return g_longitude;
}

int getGPSQuality(){
	return g_gpsQuality;
}

int getSatellitesUsedForPosition(){
	return g_satellitesUsedForPosition;
}

int getGPSPositionUpdated(){
	return g_gpsPositionUpdated;
}

void setGPSPositionStale(){
	g_gpsPositionUpdated = 0;	
}

char * getGPSVelocity(){
	return g_velocity;
}

int getGPSVelocityUpdated(){
	return g_gpsVelocityUpdated;
}


void setGPSVelocityStale(){
	g_gpsVelocityUpdated = 0;
}





void startGPSTask(){
	strcpy(g_UTCTime,"000000.000");
	strcpy(g_latitude,"00000.0000N");
	strcpy(g_longitude,"00000.0000W");
	g_gpsQuality = GPS_QUALITY_NO_FIX;
	g_satellitesUsedForPosition = 0;
	g_gpsPositionUpdated = 0;
	strcpy(g_velocity,"0000.00");
	g_gpsVelocityUpdated = 0;
	
	
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
				} else if (strstr(data,"GLL,")){ //Geographic Position – Latitude/Longitude
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
	
	char * delim = strchr(data,',');
	int param = 0;
	
	while (delim != NULL){
		*delim = '\0';
		switch (param){
			case 0:
				strcpy(g_UTCTime, data);
				break;
			case 1:
				strcpy(g_latitude, data);			
				break;
			case 2:
				strcat(g_latitude, data);
				break;
			case 3:
				strcpy(g_longitude, data );
				break;
			case 4:
				strcat(g_longitude, data);
				break;
			case 5:
				g_gpsQuality = atoi(data);
				break;
			case 6:
				g_satellitesUsedForPosition = atoi(data);
				break;
		}
		param++;
		data = delim + 1;
		delim = strchr(data,',');
	}
	usart0_puts("time: ");
	usart0_puts(g_UTCTime);
	usart0_puts(";lat: ");
	usart0_puts(g_latitude);
	usart0_puts(";long: ");
	usart0_puts(g_longitude);
	usart0_puts(";quality: ");
	switch(g_gpsQuality){
		case GPS_QUALITY_NO_FIX:
			usart0_puts("No fix");
			break;
		case GPS_QUALITY_SPS:
			usart0_puts("SPS");
			break;
		case GPS_QUALITY_DIFFERENTIAL:
			usart0_puts("Differential");
			break;
		default:
			usart0_puts("??: ");
			char qual[10];
			modp_itoa10(g_gpsQuality,qual);
			usart0_puts(qual);
	}
	char satellites[10];
	modp_itoa10(g_satellitesUsedForPosition,satellites);
	usart0_puts(";Sats Used: ");
	usart0_puts(satellites);
	usart0_puts("\r\n");
	if (g_gpsQuality != GPS_QUALITY_NO_FIX) g_gpsPositionUpdated = 1;
}

//Parse GNSS DOP and Active Satellites
void parseGSA(char *data){
	
}

//Parse Course Over Ground and Ground Speed
void parseVTG(char *data){

	char * delim = strchr(data,',');
	int param = 0;
	
	while (delim != NULL){
		*delim = '\0';
		switch (param){
			case 6: //Speed over ground
				strcpy(g_velocity, data);
				break;
			default:
				break;
		}
		param++;
		data = delim + 1;
		delim = strchr(data,',');
	}
	usart0_puts("Velocity: ");
	usart0_puts(g_velocity);
	usart0_puts("\r\n");	
	if (g_gpsQuality != GPS_QUALITY_NO_FIX) g_gpsVelocityUpdated = 1;
}

//Parse Geographic Position – Latitude / Longitude
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
