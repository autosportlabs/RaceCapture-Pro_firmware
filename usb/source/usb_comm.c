#include "usb_comm.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "USB-CDC.h"
#include "Board.h"
#include "loggerHardware.h"
#include "sdcard.h"
#include "accelerometer.h"
#include "memory.h"
#include "loggerConfig.h"

//* Global variable
extern struct LoggerConfig g_savedLoggerConfig;
extern struct LoggerConfig g_workingLoggerConfig;


extern char					debugMsg[100];
#define MSG_SIZE 				1000

unsigned short duty = 1;
char text[300];

void ListFile(char *filename){

	EmbeddedFileSystem efs;		
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
		while (( e = file_read(&f,1,text)) != 0){
			text[e]=0;
			SendString((char *)text);
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
	
    while (1){
    	vUSBReceiveByte(&theData);
		if (theData == 'V'){
			text[0] = 3;
			text[1] = 2;
			text[2] = 3;	
			SendBytes(text,3);
		}	
		if (theData == 'd'){
			ListRootDir();	
		}
		if (theData == 'u'){
			g_workingLoggerConfig.AccelX_config++;
			g_workingLoggerConfig.AccelY_config++;
			g_workingLoggerConfig.ThetaZ_config++;	
		}
		if (theData == 'w'){
			SendString("flashing...");
			int result = flashLoggerConfig();
			sprintf(text,"done: %d\r\n",result);
			SendString(text);
		}
		if (theData == 'p'){
			sprintf(text,"working: %d,%d,%d\r\n", g_workingLoggerConfig.AccelX_config,g_workingLoggerConfig.AccelY_config,g_workingLoggerConfig.ThetaZ_config);
			SendString(text);	
			sprintf(text,"saved: %d,%d,%d\r\n", g_savedLoggerConfig.AccelX_config,g_savedLoggerConfig.AccelY_config,g_savedLoggerConfig.ThetaZ_config);
			SendString(text);
			sprintf(text,"page number %d\r\n", getMemoryPageNumber(&g_savedLoggerConfig));
			SendString(text);
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
		
		if (theData == '!'){
			for (int x = 0; x < 1000; x++){
				sprintf(text,"%d\n\r", x);
				SendString(text);	
			}
		}
		if (theData == 't'){
			vTaskList(text);
			SendString(text);	
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

