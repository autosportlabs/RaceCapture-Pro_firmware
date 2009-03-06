#include "usb_comm.h"
#include "task.h"
#include "string.h"
#include "USB-CDC.h"
#include "Board.h"
#include "loggerHardware.h"
#include "accelerometer.h"
#include "memory.h"
#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "gps.h"
#include "usart.h"


//* Global variable
extern struct LoggerConfig g_savedLoggerConfig;
extern struct LoggerConfig g_workingLoggerConfig;

unsigned short duty = MIN_DUTY_CYCLE;
unsigned short period = MAX_DUTY_CYCLE;


extern unsigned int _CONFIG_HEAP_SIZE;
extern unsigned portCHAR  _heap_address[];

void onUSBCommTask(void *pvParameters){
	
	portCHAR theData;
	while (!vUSBIsInitialized()){
		vTaskDelay(1);
	}
	
    while (1){
    	vUSBReceiveByte(&theData);

		if (theData == 'm'){
			SendString("heap size: ");
			SendUint((unsigned int)(&_CONFIG_HEAP_SIZE));
			SendString("; heap address: ");
			SendUint((unsigned int) _heap_address);
			SendCrlf();
		}
		if (theData == 'u'){
			g_workingLoggerConfig.AccelX_config++;
			g_workingLoggerConfig.AccelY_config++;
			g_workingLoggerConfig.ThetaZ_config++;	
			g_workingLoggerConfig.extra2[127]++;
		}
		if (theData == 'w'){
			SendString("flashing...");
			unsigned int result = flashLoggerConfig();
			SendString("done: ");
			SendInt(result);
			SendCrlf();
		}
		if (theData == 'p'){
			SendString("working: ");
			SendInt(g_workingLoggerConfig.AccelX_config);
			SendString(" ");
			SendInt(g_workingLoggerConfig.AccelY_config);
			SendString(" ");
			SendInt(g_workingLoggerConfig.ThetaZ_config);
			SendString(" ");
			SendInt(g_workingLoggerConfig.extra2[127]);
			SendCrlf();
			SendString("saved: ");
			SendInt(g_savedLoggerConfig.AccelX_config);
			SendInt(g_savedLoggerConfig.AccelY_config);
			SendInt(g_savedLoggerConfig.ThetaZ_config);
			SendInt(g_savedLoggerConfig.extra2[127]);
			SendCrlf();
			SendString("address: ");
			SendUint((unsigned int)&g_savedLoggerConfig);
			SendString("sizeof config: ");
			SendUint((unsigned int)sizeof(struct LoggerConfig));
			SendCrlf();
		}
		if (theData == 'z'){
			duty--;
			SendString("Duty: ");
			SendInt(duty);
			SendCrlf();
			PWM_SetDutyCycle(0,duty);	
		}
		if (theData == 'X'){
			SendString("Reset Duty");
			SendCrlf();
			duty=MIN_DUTY_CYCLE;
			PWM_SetDutyCycle(0,duty);	
		}
		if (theData == 'x'){
			duty++;
			SendString("Duty: ");
			SendInt(duty);
			SendCrlf();
			PWM_SetDutyCycle(0,duty);
		}
		if (theData == '1'){
			period--;
			SendString("Period: ");
			SendInt(period);
			SendCrlf();
			PWM_SetPeriod(0,period);
		}
		if (theData == '2'){
			period++;
			SendString("Period: ");
			SendInt(period);
			SendCrlf();
			PWM_SetPeriod(0,period);
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
			accel_init();
			accel_setup();
		 	int x = accel_readAxis(0);
		 	int y = accel_readAxis(1);
		 	int z = accel_readAxis(2);
		 	SendString("G X,Y,Z: ");
		 	SendInt(x);
		 	SendString(",");
		 	SendInt(y);
		 	SendString(",");
		 	SendInt(z);
		 	SendCrlf();
		}
		if (theData == 'b'){
		 	int x = accel_readControlRegister();
		 	SendString("accel control: ");
		 	SendInt(x);
		 	SendCrlf();			
		}
		if (theData == 'd'){
			unsigned int size = getRx1QueueWaiting();
			SendString("usart rx1 queue waiting ");
			SendInt(size);
			SendCrlf();
		}
		if (theData == 'g'){
			SendFloat(getLongitude(),6);
			SendString(",");
			SendFloat(getLatitude(),6);
			SendString(" Vel: ");
			SendFloat(getGPSVelocity(),2);
			SendString(" Qual: ");
			SendInt(getGPSQuality());
			SendString(" Sats: ");
			SendInt(getSatellitesUsedForPosition());
			SendCrlf();
			//SendString(getGPSDataLine());
			//SendCrlf();	
		}
#if ( configUSE_TRACE_FACILITY == 1 )		
		if (theData == 't'){
			SendString("status\tpri\tstack\tnum\tname\r\n");
			char text[300]; //fixme
			vTaskList(text);
			SendString(text);
			SendCrlf();
		}
#endif
		if (theData == 'q'){
			SendString("timer: ");
			SendInt(getTimer0Period());
			SendString(",");
			SendInt(getTimer1Period());
			SendString(",");
			SendInt(getTimer2Period());
			SendString("  ");
			SendInt(AT91C_BASE_TC0->TC_CV);
			SendString(",");
			SendInt(AT91C_BASE_TC1->TC_CV);
			SendString(",");
			SendInt(AT91C_BASE_TC2->TC_CV);
			SendString("  ");
			SendInt(AT91C_BASE_TC0->TC_RB);
			SendString(",");
			SendInt(AT91C_BASE_TC1->TC_RB);
			SendString(",");
			SendInt(AT91C_BASE_TC2->TC_RB);
			SendCrlf();
		}
		if (theData == 'a'){
			unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
			ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);
			SendString("ADC: ");
			SendInt(a0);
			SendString(",");
			SendInt(a1);
			SendString(",");
			SendInt(a2);
			SendString(",");
			SendInt(a3);
			SendString(",");
			SendInt(a4);
			SendString(",");
			SendInt(a5);
			SendString(",");
			SendInt(a6);
			SendString(",");
			SendInt(a7);
			SendCrlf();
		}
    }
}

void SendInt(int n){
	char buf[12];
	modp_itoa10(n,buf);
	SendString(buf);
}

void SendFloat(float f,int precision){
	char buf[12];
	modp_ftoa(f,buf,precision);
	SendString(buf);	
}

void SendUint(unsigned int n){
	char buf[20];
	modp_uitoa10(n,buf);
	SendString(buf);	
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


void SendCrlf(){
	SendString("\r\n");
}