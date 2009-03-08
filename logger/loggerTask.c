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


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 5 )
#define WRITER_PRIORITY						( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define WRITER_STACK_SIZE					200
#define WRITE_QUEUE_SIZE					512

int g_loggingShouldRun;

xSemaphoreHandle g_xLoggerStart;
xQueueHandle xFileWriteQueue; 
xSemaphoreHandle xSPIMutex;

void createLoggerTask(){

	g_loggingShouldRun = 0;
	vSemaphoreCreateBinary( g_xLoggerStart );
	vSemaphoreCreateBinary(xSPIMutex);
	
	xSemaphoreTake( g_xLoggerStart, 1 );
	xFileWriteQueue = xQueueCreate( WRITE_QUEUE_SIZE, ( unsigned portCHAR ) sizeof( signed portCHAR ) );
	
		
	xTaskCreate( loggerWriter,
				( signed portCHAR * ) "loggerWriter",
				WRITER_STACK_SIZE,
				NULL,
				WRITER_PRIORITY,
				NULL);
				
	xTaskCreate( loggerTask, 	
				( signed portCHAR * ) "loggerTask", 	
				LOGGER_STACK_SIZE, 	
				NULL, 	
				LOGGER_TASK_PRIORITY, 	
				NULL );
}

void fileWriteString(char *s){
	
	while ( *s ){
		xQueueSend(	xFileWriteQueue, (unsigned portCHAR *)s++, portMAX_DELAY);
	}
}

void loggerWriter(void *params){
	
	EmbeddedFile f;
	int writingEnabled = 0;
	unsigned portCHAR data;
	while(1){
		if (xQueueReceive(xFileWriteQueue, &data, portMAX_DELAY)){
			xSemaphoreTake(xSPIMutex, portMAX_DELAY);
			if (data == '!'){
				int rc = InitEFS();
				if ( rc == 0 ){
					if (OpenNextLogFile(&f) == 0){
						writingEnabled = 1;
					}
				}
			} else if (data == '~'){
				file_fclose(&f);
				UnmountEFS();
				writingEnabled = 0;
			} else if (writingEnabled){
				file_write(&f,1,(unsigned char *)&data);
			}
			xSemaphoreGive(xSPIMutex);
		}
	}
}

void loggerTask(void *params){
	
	struct LoggerConfig *loggerConfig;
	
	loggerConfig = getWorkingLoggerConfig();
	
	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) == pdTRUE){

			int accelInstalled = loggerConfig->Accel_Installed; 
			if ( accelInstalled == CONFIG_ACCEL_INSTALLED){
				xSemaphoreTake(xSPIMutex, portMAX_DELAY);
				accel_init();
				accel_setup();
				xSemaphoreGive(xSPIMutex);
			}
						
			if (InitEFS() == 0 ){
				g_loggingShouldRun = 1;
				//signal writer to open next file
				fileWriteString("!");
			}	
			
			portTickType xLastWakeTime, startTickTime;
			const portTickType xFrequency = BASE_100Hz;
			startTickTime = xLastWakeTime = xTaskGetTickCount();
			
			
			//run until we should not log anymore
			while (g_loggingShouldRun){
				
				writeGPSPosition();
				writeGPSVelocity();

				unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
				ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);				
				writeADC(a0,a1,a2,a3,a4,a5,a6,a7);

				if (accelInstalled == CONFIG_ACCEL_INSTALLED) writeAccelerometer(loggerConfig);
				
				fileWriteString("\n");			
				
				ToggleLED(LED2);
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			fileWriteString("~");
			DisableLED(LED2);
		}
	}
}

void fileWriteInt(int num){
	char buf[5];
	modp_itoa10(num,buf);
	fileWriteString(buf);
}

void writeAccelerometer(struct LoggerConfig *loggerConfig){
	
	int x = -1;
	int y = -1;
	int z = -1;
	int zt = -1;
	
	xSemaphoreTake(xSPIMutex, portMAX_DELAY);
	if (loggerConfig->AccelX_config != CONFIG_ACCEL_DISABLED) x = accel_readAxis(0);
	if (loggerConfig->AccelY_config != CONFIG_ACCEL_DISABLED) y = accel_readAxis(1);
	if (loggerConfig->AccelZ_config != CONFIG_ACCEL_DISABLED) z = accel_readAxis(2);
	if (loggerConfig->ThetaZ_config != CONFIG_ACCEL_DISABLED) zt = accel_readAxis(3);
	xSemaphoreGive(xSPIMutex);
	
	if (x > 0){
		fileWriteInt(x);
		fileWriteString(",");
	}
	if (y > 0){
		fileWriteInt(y);
		fileWriteString(",");
	}
	if (z > 0){
		fileWriteInt(z);
		fileWriteString(",");	
	}
	if (zt > 0){
		fileWriteInt(zt);
		fileWriteString(",");
	}
}

void writeADC(unsigned int a0,unsigned int a1,unsigned int a2,unsigned int a3,unsigned int a4,unsigned int a5,unsigned int a6,unsigned int a7){
	
	char buf[20];
	struct LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	float scaling;
	int precision;

	precision = 2;
	
	scaling = loggerConfig->ADC_Calibrations[0].scaling;
	modp_ftoa((scaling * (float)a0), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
	
	scaling = loggerConfig->ADC_Calibrations[1].scaling;
	modp_ftoa((scaling * (float)a1), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[2].scaling;
	modp_ftoa((scaling * (float)a2), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[3].scaling;
	modp_ftoa((scaling * (float)a3), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
 	
 	scaling = loggerConfig->ADC_Calibrations[4].scaling;
	modp_ftoa((scaling * (float)a4), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");

	scaling = loggerConfig->ADC_Calibrations[5].scaling;
	modp_ftoa((scaling * (float)a5), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
 
	scaling = loggerConfig->ADC_Calibrations[6].scaling;
	modp_ftoa((scaling * (float)a6), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");
 	
	scaling = loggerConfig->ADC_Calibrations[7].scaling;
	modp_ftoa((scaling * (float)a7), buf, precision);
	fileWriteString(buf);
	fileWriteString(",");	
}

void writeGPSPosition(){
	char buf[20];
	
	if 	(getGPSPositionUpdated()){
		modp_itoa10(getGPSQuality(),buf);
		fileWriteString(buf);
		fileWriteString(",");

		fileWriteString(getUTCTimeString());
		fileWriteString(",");
		
		modp_dtoa(getLatitude(),buf,6);
		fileWriteString(buf);
		fileWriteString(",");
		
		modp_dtoa(getLongitude(),buf,6);
		fileWriteString(buf);
		fileWriteString(",");
		
		modp_itoa10(getSatellitesUsedForPosition(),buf);
		fileWriteString(buf);
		fileWriteString(",");
		setGPSPositionStale();
	}
	else{
		fileWriteString(",");
		fileWriteString(",");
		fileWriteString(",");
		fileWriteString(",");
		fileWriteString(",");
	}
}

void writeGPSVelocity(){
	if 	(getGPSVelocityUpdated()){
		fileWriteString(getGPSVelocityString());
		fileWriteString(",");
		setGPSVelocityStale();
	}
	else{
		fileWriteString(",");	
	}
}
