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

void mock_setIsFlashed(int isFlashed){
	g_isFlashed = isFlashed;
}

int mock_getIsFlashed(){
	return g_isFlashed;
}

int flashLoggerConfig(){
	g_isFlashed = 1;
	return 1;
}

