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

int g_loggingShouldRun;
xSemaphoreHandle g_xLoggerStart;

#define BASE_100Hz 3
#define BASE_50Hz 6
#define BASE_30Hz 10
#define BASE_20Hz 15
#define BASE_10Hz 30


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define WRITER_PRIORITY						( tskIDLE_PRIORITY + 5 )
#define LOGGER_STACK_SIZE  					200
#define WRITER_STACK_SIZE					200
#define WRITE_QUEUE_SIZE					512

xQueueHandle xFileWriteQueue; 

void createLoggerTask(){

	g_loggingShouldRun = 0;
	vSemaphoreCreateBinary( g_xLoggerStart );
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
			if (data == '!'){
				if (InitEFS() == 0 ){
					if (OpenNextLogFile(&f) == 0){
						writingEnabled = 1;
					}
				}
			} else if (data == '~'){
				file_fclose(&f);
				UnmountEFS();
				writingEnabled = 0;
			} else if (writingEnabled){
				file_write(&f,1,(char *)&data);			
			}
		}
	}
}

void loggerTask(void *params){
	
	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) == pdTRUE){
			EmbeddedFile f;
			if (InitEFS() == 0 ){
				if (OpenNextLogFile(&f) == 0){
					g_loggingShouldRun = 1;
				}
			}
			
			//signal writer to open next file
			fileWriteString("!");
				
			portTickType xLastWakeTime, startTickTime;
			const portTickType xFrequency = BASE_10Hz;
			startTickTime = xLastWakeTime = xTaskGetTickCount();
			
			
			//run until we should not log anymore
			while (g_loggingShouldRun){
				
				writeGPSPosition();
				writeGPSVelocity();

				unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
				ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);				
				writeADC(a0,a1,a2,a3,a4,a5,a6,a7);

				fileWriteString("\n");			
				
				ToggleLED(LED2);
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			fileWriteString("~");
			DisableLED(LED2);
		}
		vTaskDelay(1);
	}
}

void writeADC(unsigned int a0,unsigned int a1,unsigned int a2,unsigned int a3,unsigned int a4,unsigned int a5,unsigned int a6,unsigned int a7){
	
	char buf[20];
	modp_itoa10(a0,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a1,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a2,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a3,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a4,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a5,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a6,buf);
	fileWriteString(buf);
	fileWriteString(",");

	modp_itoa10(a7,buf);
	fileWriteString(buf);
	fileWriteString(",");	
}

void writeGPSPosition(){
	char buf[20];
	
	if 	(getGPSPositionUpdated()){
		modp_itoa10(getGPSQuality(),buf);
		fileWriteString(buf);
		fileWriteString(",");

		fileWriteString(getUTCTime());
		fileWriteString(",");
		
		fileWriteString(getLatitude());
		fileWriteString(",");
		
		fileWriteString(getLongitude());
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
		fileWriteString(getGPSVelocity());
		fileWriteString(",");
		setGPSVelocityStale();
	}
	else{
		fileWriteString(",");	
	}
}