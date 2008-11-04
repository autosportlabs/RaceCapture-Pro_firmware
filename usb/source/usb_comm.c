#include "usb_comm.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "led_debug.h"
#include "USB-CDC.h"
#include "Board.h"
#include "loggerHardware.h"

//* Global variable
extern char					debugMsg[100];
#define MSG_SIZE 				1000

unsigned short duty = 1;

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
			sprintf(text,"Hello!\r\n");
			SendBytes(text,strlen(text));
		}
		if (theData == 'z'){
			duty--;
			sprintf(text,"Duty %d\r\n",duty);
			SendBytes(text,strlen(text));
			PWM_SetDutyCycle(0,duty);	
		}
		if (theData == 'x'){
			duty++;
			sprintf(text,"Duty %d\r\n",duty);
			SendBytes(text,strlen(text));
			PWM_SetDutyCycle(0,duty);
		}
		if (theData == 'c'){
			for (int i=1; i< 1000; i++){
				vTaskDelay(4 / portTICK_RATE_MS);
				PWM_SetDutyCycle(0,i);	
			}	
			for (int i=999; i >=1; i--){
				vTaskDelay(4 / portTICK_RATE_MS);
				PWM_SetDutyCycle(0,i);	
			}
		}
		if (theData == 'a'){
			unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
/*			a1 = ReadADC(0);
			a2 = ReadADC(1);
			a3 = ReadADC(2);
			a4 = ReadADC(3);
			a5 = ReadADC(4);
			a6 = ReadADC(5);
			a7 = ReadADC(6);
*/
			ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);
			//sprintf(text,"ADC1:%d; ADC2:%d; ADC3:%d; ADC4:%d; ADC5:%d; ADC6:%d; ADC7:%d;\r\n",a1,a2,a3,a4,a5,a6,a7);
			sprintf(text,"All ADC0:%d; ADC1:%d; ADC2:%d; ADC3:%d; ADC4:%d; ADC5:%d; ADC6:%d; ADC7:%d\r\n",a0,a1,a2,a3,a4,a5,a6,a7);
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