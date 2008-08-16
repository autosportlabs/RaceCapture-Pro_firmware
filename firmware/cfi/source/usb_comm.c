#include "usb_comm.h"
#include "ignition_config.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "led_debug.h"
#include "USB-CDC.h"
#include "Board.h"
#include "runtime.h"

//* Global variable
extern unsigned int			g_wheelSyncAttempts;
extern unsigned int			g_toothCountAtLastSyncAttempt;
extern unsigned int			g_lastInterToothPeriodRaw;
extern unsigned int 		g_lastCrankRevolutionPeriodRaw;
extern unsigned int			g_lastCrankRevolutionPeriodUSec;
extern unsigned int 		g_wheelSynchronized;
extern unsigned int			g_engineIsRunning;
extern unsigned int 		g_currentRPM;
extern unsigned int 		g_currentLoad;
extern unsigned int 		g_currentLoadBin;
extern unsigned int 		g_currentRPMBin;
extern int 					g_currentAdvance;
extern unsigned int 		g_currentDwellDegrees;
extern unsigned int			g_currentDwellUSec;
extern unsigned int			g_currentTooth;
extern char					debugMsg[100];
extern struct logical_coil_driver 	*g_active_logical_coil_drivers;
unsigned int				g_logicalCoilDriverCount;

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
		if (theData == '$'){
			text[0] = g_currentAdvance;
			text[1] = g_currentRPM & 0xFF00;
			text[2] = g_currentRPM & 0xFF;
			text[3] = g_currentRPMBin | g_currentLoadBin;
			text[4] = g_currentLoad;
			text[5] = 0;
			SendBytes(text,6);
		}
		if (theData == '!'){
			sprintf(text,"tim=%d\r\n",g_lastInterToothPeriodRaw);
			SendBytes(text,strlen(text));
		}
		if (theData == '?'){
		  	for (int i = 0; i < 10000; i ++){
		  		sprintf(text,"i=%d\r\n",i);
		  		SendBytes(text,strlen(text));
		  	}
		}
		if (theData == 'f'){
			AT91C_BASE_TC2->TC_RC = 30000;
			AT91C_BASE_TC2->TC_CCR = AT91C_TC_SWTRG;
		}
		if (theData =='1'){
			AT91PS_TC TC_pt = AT91C_BASE_TC2;
			sprintf(text,"tc2_cv=%d tc2_rc=%d\r\n",TC_pt->TC_CV,TC_pt->TC_RC);
			SendBytes(text,strlen(text));		   
		}
		if (theData =='2'){
			AT91PS_TC TC_pt = AT91C_BASE_TC0;
			sprintf(text,"tc0_cv=%d tc0_rc=%d\r\n",TC_pt->TC_CV,TC_pt->TC_RC);
			SendBytes(text,strlen(text));		   
		}
		if (theData =='3'){
			sprintf(text,"sync=%d;sync attempts=%d;last sync attempt tooth count=%d\r\n",g_wheelSynchronized,g_wheelSyncAttempts,g_toothCountAtLastSyncAttempt);
			SendBytes(text,strlen(text));		    
		}
		if (theData =='4'){
			sprintf(text,"tooth=%d\r\n",g_currentTooth);
			SendBytes(text,strlen(text));		    
		}
		if (theData == 'd'){
			SendBytes(debugMsg,strlen(debugMsg));	
		}
		if (theData =='s'){
			sprintf(text,"run=%d;per=%d;usec=%d;rpm=%d;load=%d;loadBin=%d;rpmBin=%d;adv=%d;dwell=%d\r\n",
			g_engineIsRunning,g_lastCrankRevolutionPeriodRaw,g_lastCrankRevolutionPeriodUSec,g_currentRPM,g_currentLoad,g_currentLoadBin,g_currentRPMBin,g_currentAdvance,g_currentDwellDegrees);
			SendBytes(text,strlen(text));
		}
		if (theData == 'r'){
			for (int i = 0;i < g_logicalCoilDriverCount; i++){
				struct logical_coil_driver *coil_driver = g_active_logical_coil_drivers + i;
				sprintf(text,"cyl:%d\r\ntdc:%d\r\nonDeg:%d\r\nonTooth:%d\r\nonInterTooth:%d\r\nfireDeg:%d\r\nfireTooth:%d\r\nfireInterTooth:%d\r\nports:%d\r\n----\r\n",
						i,
						coil_driver->cylinderTDC,
						coil_driver->coilOnDegrees,
						coil_driver->coilOnTooth,
						coil_driver->coilOnInterToothDegrees,
						coil_driver->coilFireDegrees,
						coil_driver->coilFireTooth,
						coil_driver->coilFireInterToothDegrees,
						coil_driver->physicalCoilDriverPorts
						);
				SendBytes(text,strlen(text));
			}
		}
		if (theData =='d'){
			g_currentDwellUSec+=100;	
			sprintf(text,"dwell=%dUSec\r\n",g_currentDwellUSec);
			SendBytes(text,strlen(text));
		}
		if (theData =='f'){
			g_currentDwellUSec-=100;	
			sprintf(text,"dwell=%dUSec\r\n",g_currentDwellUSec);
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