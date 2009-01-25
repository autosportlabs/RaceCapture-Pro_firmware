#include "gps.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "loggerHardware.h"
#include "usart.h"
#include "string.h"

#define GPS_DATA_LINE_BUFFER_LEN 	200
#define GPS_TASK_PRIORITY 			( tskIDLE_PRIORITY + 1 )
#define GPS_TASK_STACK_SIZE			100

char g_GPSdataLine[GPS_DATA_LINE_BUFFER_LEN];

void startGPSTask(){
	xTaskCreate( GPSTask, ( signed portCHAR * ) "GPSTask", GPS_TASK_STACK_SIZE, NULL, 	GPS_TASK_PRIORITY, 	NULL );
}

void GPSTask( void *pvParameters ){
	for( ;; )
	{
		int len = usart1_readLine(g_GPSdataLine, GPS_DATA_LINE_BUFFER_LEN);
		if (len > 0){
			ToggleLED(LED1);
			usart0_puts(g_GPSdataLine);
		}
	}
}
