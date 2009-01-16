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

int g_loggingShouldRun;
xSemaphoreHandle g_xLoggerStart;


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					400

void createLoggerTask(){

	g_loggingShouldRun = 0;
	vSemaphoreCreateBinary( g_xLoggerStart );
	xSemaphoreTake( g_xLoggerStart,1);
	
		
	xTaskCreate( loggerTask, 	
				( signed portCHAR * ) "loggerTask", 	
				LOGGER_STACK_SIZE, 	
				NULL, 	
				LOGGER_TASK_PRIORITY, 	
				NULL );
}

void writeLogFileValue(char * buf, int value, EmbeddedFile *f){
	modp_itoa10(value,buf);
	unsigned int len= strlen(buf);
	
	if (len != file_write(f,len,buf)){
		//write error occurred	
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
					EnableLED(LED1);
				}
			}
			char buf[20];
				
			portTickType xLastWakeTime;
			const portTickType xFrequency = 4;
			xLastWakeTime = xTaskGetTickCount();
			
			//run until we should not log anymore
			while (g_loggingShouldRun){
				unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
				ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);				
								
				writeLogFileValue(buf, a0, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a1, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a2, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a3, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a4, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a5, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a6, &f);
				file_write(&f,1,",");
				writeLogFileValue(buf, a7, &f);
				file_write(&f,1,"\n");
				
				ToggleLED(LED2);
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			file_fclose(&f);
			UnmountEFS();
			DisableLED(LED1);
			DisableLED(LED2);
		}
		vTaskDelay(1);
	}
}

