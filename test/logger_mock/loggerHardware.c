/*
 * loggerHardware.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "loggerHardware.h"
#include "loggerHardware_mock.h"
#include "loggerConfig.h"
#include "accelerometer.h"


static int g_isFlashed = 0;

void InitLoggerHardware(){
	accelerometer_init();
}


void InitLEDs(void){}

void InitPushbutton(void){}

void ResetWatchdog(){}

void InitWatchdog(int timeoutMs){}

void mock_setIsFlashed(int isFlashed){
	g_isFlashed = isFlashed;
}

int mock_getIsFlashed(){
	return g_isFlashed;
}

//Set bit for specified Frequency/Analog port
void SetFREQ_ANALOG(unsigned int freqAnalogPort){}

//Clear bit for specified Frequency/Analog port
void ClearFREQ_ANALOG(unsigned int freqAnalogPort){}

int flashLoggerConfig(){
	g_isFlashed = 1;
	return 1;
}

