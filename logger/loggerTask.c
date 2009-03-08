#include "loggerTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
#include "usb_comm.h"
#include "modp_numtoa.h"
#include "string.h"
#include "stdio.h"
#include "sdcard.h"
#include "gps.h"
#include "loggerConfig.h"
#include "accelerometer.h"


#define BASE_100Hz 3
#define BASE_50Hz 6
#define BASE_30Hz 10
#define BASE_20Hz 15
#define BASE_10Hz 30


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define LOGGER_STACK_SIZE  					200

#define LOGGER_LINE_LENGTH					512
int g_loggingShouldRun;

xSemaphoreHandle g_xLoggerStart;

char g_loggerLineBuffer[LOGGER_LINE_LENGTH];

void createLoggerTask(){

	g_loggingShouldRun = 0;
	vSemaphoreCreateBinary( g_xLoggerStart );
	
	xSemaphoreTake( g_xLoggerStart, 1 );
	
	xTaskCreate( loggerTask, 	
				( signed portCHAR * ) "loggerTask", 	
				LOGGER_STACK_SIZE, 	
				NULL, 	
				LOGGER_TASK_PRIORITY, 	
				NULL );
}

void lineAppendString(char *s){
	strcat(g_loggerLineBuffer, s);	
}

void lineAppendInt(int num){
	char buf[10];
	modp_itoa10(num,buf);
	lineAppendString(buf);
}

void lineAppendFloat(float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	lineAppendString(buf);
}

void lineAppendDouble(double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	lineAppendString(buf);
}

void fileWriteString(EmbeddedFile *f, char *s){
	int len = strlen(s);
	file_write(f,len,(unsigned char *)s);
}

void fileWriteInt(EmbeddedFile *f, int num){
	char buf[10];
	modp_itoa10(num,buf);
	fileWriteString(f,buf);
}

void fileWriteFloat(EmbeddedFile *f, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	fileWriteString(f,buf);
}

void fileWriteDouble(EmbeddedFile *f, double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	fileWriteString(f,buf);
}


void loggerTask(void *params){
	
	struct LoggerConfig *loggerConfig;
	
	loggerConfig = getWorkingLoggerConfig();
	
	EmbeddedFile f;
	
	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) == pdTRUE){

			int accelInstalled = loggerConfig->Accel_Installed; 
			if ( accelInstalled == CONFIG_ACCEL_INSTALLED){
				accel_init();
				accel_setup();
			}
						
			int rc = InitEFS();
			if ( rc == 0 ){
				if (OpenNextLogFile(&f) == 0){
					g_loggingShouldRun = 1;
				}
			}
			
			portTickType xLastWakeTime, startTickTime;
			const portTickType xFrequency = BASE_100Hz;
			startTickTime = xLastWakeTime = xTaskGetTickCount();
			
			
			//run until we should not log anymore
			while (g_loggingShouldRun){
				ToggleLED(LED2);

				//reset line
				g_loggerLineBuffer[0] = '\0';
								
				writeGPSPosition();
				writeGPSVelocity();

				unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
				ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);				
				writeADC(a0,a1,a2,a3,a4,a5,a6,a7);

				if (accelInstalled == CONFIG_ACCEL_INSTALLED) writeAccelerometer(loggerConfig);
				
				lineAppendString("\n");
				fileWriteString(&f, g_loggerLineBuffer);
				
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			file_fclose(&f);
			UnmountEFS();
			DisableLED(LED2);
		}
	}
}


void writeAccelerometer(struct LoggerConfig *loggerConfig){
	
	int x = -1;
	int y = -1;
	int z = -1;
	int zt = -1;
	
	if (loggerConfig->AccelX_config != CONFIG_ACCEL_DISABLED) x = accel_readAxis(0);
	if (loggerConfig->AccelY_config != CONFIG_ACCEL_DISABLED) y = accel_readAxis(1);
	if (loggerConfig->AccelZ_config != CONFIG_ACCEL_DISABLED) z = accel_readAxis(2);
	if (loggerConfig->ThetaZ_config != CONFIG_ACCEL_DISABLED) zt = accel_readAxis(3);
	
	if (x > 0){
		lineAppendInt(x);
		lineAppendString(",");
	}
	if (y > 0){
		lineAppendInt(y);
		lineAppendString(",");
	}
	if (z > 0){
		lineAppendInt(z);
		lineAppendString(",");	
	}
	if (zt > 0){
		lineAppendInt(zt);
		lineAppendString(",");
	}
}

void writeADC(unsigned int a0,unsigned int a1,unsigned int a2,unsigned int a3,unsigned int a4,unsigned int a5,unsigned int a6,unsigned int a7){
	
	struct LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	float scaling;
	int precision;

	precision = 2;
	
	scaling = loggerConfig->ADC_Calibrations[0].scaling;
	lineAppendFloat((scaling * (float)a0), precision);
	lineAppendString(",");
	
	scaling = loggerConfig->ADC_Calibrations[1].scaling;
	lineAppendFloat((scaling * (float)a1), precision);
	lineAppendString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[2].scaling;
 	lineAppendFloat((scaling * (float)a2), precision);
	lineAppendString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[3].scaling;
	lineAppendFloat((scaling * (float)a3), precision);
	lineAppendString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[4].scaling;
 	lineAppendFloat((scaling * (float)a4), precision);
	lineAppendString(",");

	scaling = loggerConfig->ADC_Calibrations[5].scaling;
	lineAppendFloat((scaling * (float)a5), precision);
	lineAppendString(",");
 
	scaling = loggerConfig->ADC_Calibrations[6].scaling;
	lineAppendFloat((scaling * (float)a6), precision);
	lineAppendString(",");
 	
	scaling = loggerConfig->ADC_Calibrations[7].scaling;
	lineAppendFloat((scaling * (float)a7), precision);
	lineAppendString(",");	
}

void writeGPSPosition(){
	
	if 	(getGPSPositionUpdated()){
		lineAppendInt(getGPSQuality());
		lineAppendString(",");

		lineAppendFloat(getUTCTime(),3);
		lineAppendString(",");
		
		lineAppendDouble(getLatitude(), 6);
		lineAppendString(",");
		
		lineAppendDouble(getLongitude(), 6);
		lineAppendString(",");
		
		lineAppendInt(getSatellitesUsedForPosition());
		lineAppendString(",");
		setGPSPositionStale();
	}
	else{
		lineAppendString(",");
		lineAppendString(",");
		lineAppendString(",");
		lineAppendString(",");
		lineAppendString(",");
	}
}

void writeGPSVelocity(){
	if 	(getGPSVelocityUpdated()){
		lineAppendFloat(getGPSVelocity(),2);
		lineAppendString(",");
		setGPSVelocityStale();
	}
	else{
		lineAppendString(",");	
	}
}
