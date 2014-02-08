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

static int g_isCardPresent = 0;
static int g_isCardWritable = 0;
static int g_isButtonPressed = 0;
static int g_leds[3] = {0,0,0};
static int g_gpio[CONFIG_GPIO_CHANNELS] = {0,0,0};

static int g_isFlashed = 0;

void InitLoggerHardware(){
	accelerometer_init();
}

void InitGPIO(LoggerConfig *loggerConfig){}

void InitSDCard(void){}

void mock_setIsCardPresent(int present){
	g_isCardPresent = present;
}

void mock_setIsFlashed(int isFlashed){
	g_isFlashed = isFlashed;
}

int mock_getIsFlashed(){
	return g_isFlashed;
}

int isCardPresent(void){
	return g_isCardPresent;
}

void mock_setIsCardWritable(int writable){
	g_isCardWritable = writable;
}

int isCardWritable(void){
	return g_isCardWritable;
}

void mock_setIsButtonPressed(int pressed){
	g_isButtonPressed = pressed;
}

int isButtonPressed(void){
	return g_isButtonPressed;
}

void InitLEDs(void){}

void InitPushbutton(void){}

void enableLED(unsigned int Led){
	g_leds[Led] = 1;
}

void disableLED(unsigned int Led){
	g_leds[Led] = 0;
}

void toggleLED (unsigned int Led){
	g_leds[Led] = g_leds[Led] == 1 ? 0 : 1;
}

void ResetWatchdog(){}

void InitWatchdog(int timeoutMs){}

int readGpio(unsigned int channel){
	return g_gpio[channel];
}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){
	*gpio1 = g_gpio[0];
	*gpio2 = g_gpio[1];
	*gpio3 = g_gpio[3];
}


void setGpio(unsigned int channel, unsigned int state){
	g_gpio[channel] = state;
}

//Set bit for specified Frequency/Analog port
void SetFREQ_ANALOG(unsigned int freqAnalogPort){}

//Clear bit for specified Frequency/Analog port
void ClearFREQ_ANALOG(unsigned int freqAnalogPort){}

int flashLoggerConfig(){
	g_isFlashed = 1;
	return 1;
}

