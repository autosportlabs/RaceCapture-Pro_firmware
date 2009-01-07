#include "usb_comm.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "USB-CDC.h"
#include "Board.h"
#include "loggerHardware.h"
#include "sdcard.h"
#include "accelerometer.h"


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

void ListFile(char *filename){

	EmbeddedFileSystem efs;		
	char text[300];
	int res;
	
	SendString("Card Init...");
	if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
		sprintf(text,"failed with %i\r\n",res);
		SendString(text);
	}
	else{
		EmbeddedFile f;

		if ( file_fopen( &f, &efs.myFs , filename , 'r' ) != 0 ) {
			sprintf(text,"\nfile_open for %s failed", filename);
			fs_umount( &efs.myFs );
			return;
		}
		
		unsigned short e;
		unsigned char buf[101];
		
		while (( e = file_read(&f,1,buf)) != 0){
			buf[e]=0;
			SendString((char *)buf);
		}
		file_fclose(&f);
		fs_umount(&efs.myFs);
		SendString("\r\nFinished");
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
		if (theData == 'd'){
			ListRootDir();	
		}
		if (theData == 'r'){
			ListFile("one.txt");	
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
		if (theData == 'i'){
			accel_init();
		}
		if (theData == 'y'){
			accel_setup();	
		}
		if (theData == 'k'){
		 	int x = accel_readAxis(0);
		 	int y = accel_readAxis(1);
		 	int z = accel_readAxis(2);
			sprintf(text,"G X,Y,Z: %d,%d,%d\r\n",x,y,z);
			SendBytes(text,strlen(text));
		}
		if (theData == 'b'){
		 	int x = accel_readControlRegister();			
			sprintf(text,"accel control: %d\r\n",x);
			SendBytes(text,strlen(text));
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


void SendString(char *s){
	while ( *s ) vUSBSendByte(*s++ );
}

void SendBytes(char *data, unsigned int length){
	
	while (length > 0){
    	vUSBSendByte(*data);
    	data++;
    	length--;
	}	
}

