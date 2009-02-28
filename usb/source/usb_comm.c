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
			SendNumber(result);
			SendCrlf();
		}
		if (theData == 'p'){
			SendString("working: ");
			SendNumber(g_workingLoggerConfig.AccelX_config);
			SendString(" ");
			SendNumber(g_workingLoggerConfig.AccelY_config);
			SendString(" ");
			SendNumber(g_workingLoggerConfig.ThetaZ_config);
			SendString(" ");
			SendNumber(g_workingLoggerConfig.extra2[127]);
			SendCrlf();
			SendString("saved: ");
			SendNumber(g_savedLoggerConfig.AccelX_config);
			SendNumber(g_savedLoggerConfig.AccelY_config);
			SendNumber(g_savedLoggerConfig.ThetaZ_config);
			SendNumber(g_savedLoggerConfig.extra2[127]);
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
			SendNumber(duty);
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
			SendNumber(duty);
			SendCrlf();
			PWM_SetDutyCycle(0,duty);
		}
		if (theData == '1'){
			period--;
			SendString("Period: ");
			SendNumber(period);
			SendCrlf();
			PWM_SetPeriod(0,period);
		}
		if (theData == '2'){
			period++;
			SendString("Period: ");
			SendNumber(period);
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
		 	int x = accel_readAxis(0);
		 	int y = accel_readAxis(1);
		 	int z = accel_readAxis(2);
		 	SendString("G X,Y,Z: ");
		 	SendNumber(x);
		 	SendString(",");
		 	SendNumber(y);
		 	SendString(",");
		 	SendNumber(z);
		 	SendCrlf();
		}
		if (theData == 'b'){
		 	int x = accel_readControlRegister();
		 	SendString("accel control: ");
		 	SendNumber(x);
		 	SendCrlf();			
		}
		if (theData == 'g'){
			SendString(getLatitude());
			SendString(" ");
			SendString(getLongitude());
			SendString(" Vel: ");
			SendString(getGPSVelocity());
			SendCrlf();	
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
			SendNumber(getTimer0Period());
			SendString(",");
			SendNumber(getTimer1Period());
			SendString(",");
			SendNumber(getTimer2Period());
			SendString("  ");
			SendNumber(AT91C_BASE_TC0->TC_CV);
			SendString(",");
			SendNumber(AT91C_BASE_TC1->TC_CV);
			SendString(",");
			SendNumber(AT91C_BASE_TC2->TC_CV);
			SendString("  ");
			SendNumber(AT91C_BASE_TC0->TC_RB);
			SendString(",");
			SendNumber(AT91C_BASE_TC1->TC_RB);
			SendString(",");
			SendNumber(AT91C_BASE_TC2->TC_RB);
			SendCrlf();
		}
		if (theData == 'a'){
			unsigned int a0,a1,a2,a3,a4,a5,a6,a7;
			ReadAllADC(&a0,&a1,&a2,&a3,&a4,&a5,&a6,&a7);
			SendString("ADC: ");
			SendNumber(a0);
			SendString(",");
			SendNumber(a1);
			SendString(",");
			SendNumber(a2);
			SendString(",");
			SendNumber(a3);
			SendString(",");
			SendNumber(a4);
			SendString(",");
			SendNumber(a5);
			SendString(",");
			SendNumber(a6);
			SendString(",");
			SendNumber(a7);
			SendCrlf();
		}
    }
}

void SendNumber(int n){
	char buf[12];
	modp_itoa10(n,buf);
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