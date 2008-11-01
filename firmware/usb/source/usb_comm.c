#include "usb_comm.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "led_debug.h"
#include "USB-CDC.h"
#include "Board.h"

//* Global variable
extern char					debugMsg[100];
#define MSG_SIZE 				1000


void onUSBEchoTask(void *pvParameters){
	
	portCHAR theData;
	while (!vUSBIsInitialized()){
		vTaskDelay(1);
	}
	
    while(1)
    {
    	vUSBReceiveByte(&theData);
    	vUSBSendByte(theData);
    }	
    	
}

void onUSBCommTask(void *pvParameters){
	
	portCHAR theData;
	while (!vUSBIsInitialized()){
		vTaskDelay(1);
	}
	
	char text[300];
	
    while (1){
    	vUSBReceiveByte(&theData);
		if (theData == 'V'){
			text[0] = 3;
			text[1] = 2;
			text[2] = 3;	
			SendBytes(text,3);
		}	
		if (theData == '!'){
			sprintf(text,"Hello!");
			SendBytes(text,strlen(text));
		}
    }
}



void SendBytes(portCHAR *data, unsigned int length){
	
	while (length > 0){
    	vUSBSendByte(*data);
    	data++;
    	length--;
	}	
}