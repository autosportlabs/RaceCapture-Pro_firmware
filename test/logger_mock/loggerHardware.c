/*
 * loggerHardware.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "loggerHardware.h"
#include "loggerConfig.h"

static int g_isCardPresent = 0;
static int g_isCardWritable = 0;
static int g_isButtonPressed = 0;
static int g_leds[3] = {0,0,0};
static int g_gpio[CONFIG_GPIO_CHANNELS] = {0,0,0};
static unsigned int g_adc[CONFIG_ADC_CHANNELS] = {0,0,0,0,0,0};
static int g_pwmPeriod[CONFIG_PWM_CHANNELS] = {0,0,0,0};
static int g_pwmDuty[CONFIG_PWM_CHANNELS] = {0,0,0,0};
static int g_timer[CONFIG_TIMER_CHANNELS] = {0,0,0};

static int g_isFlashed = 0;

void InitLoggerHardware(){

}

float readAccelerometer(unsigned char channel, AccelConfig *ac){
	return 0;
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

void InitADC(void){}

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

//Read all ADC ports

void readAllADC(unsigned int *a0,
						unsigned int *a1,
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 ){
	*a0 = g_adc[0];
	*a1 = g_adc[1];
	*a2 = g_adc[2];
	*a3 = g_adc[3];
	*a4 = g_adc[4];
	*a5 = g_adc[5];
	*a6 = g_adc[6];
	*a7 = g_adc[7];
}

//Read specified ADC channel
unsigned int readADC(unsigned int channel){
	return g_adc[channel];
}

//PWM Channel Functions
void InitPWM(LoggerConfig *loggerConfig){}

void StartPWM(unsigned int pwmChannel){}
void StartAllPWM(){}

void StopPWM(unsigned int pwmChannel){}
void StopAllPWM(){}

//Set PWM period for specified channel
void PWM_SetPeriod(	unsigned int channel, unsigned short period){
	g_pwmPeriod[channel] = period;
}

unsigned short PWM_GetPeriod(unsigned int channel){
	return g_pwmPeriod[channel];
}

//Set PWM duty cycle for specified channel
void PWM_SetDutyCycle(unsigned int channel,	unsigned short duty){
	g_pwmDuty[channel] = duty;
}

unsigned short PWM_GetDutyCycle(unsigned int channel){
	return g_pwmDuty[channel];
}

void initTimerChannels(LoggerConfig *loggerConfig){}

void initTimer0(TimerConfig *timerConfig){}

void initTimer1(TimerConfig *timerConfig){}

void initTimer2(TimerConfig *timerConfig){}

unsigned int timerClockFromDivider(unsigned short divider){
	return 0;
}

unsigned int getTimerPeriod(unsigned int channel){
	return g_timer[channel];
}

unsigned int getTimerCount(unsigned int channel){
	return 0;
}

void resetTimerCount(unsigned int channel){}

void getAllTimerPeriods(unsigned int *t0, unsigned int *t1, unsigned int *t2){
	*t0 = g_timer[0];
	*t1 = g_timer[1];
	*t2 = g_timer[2];
}

unsigned int getTimer0Period(){
	return g_timer[0];
}

unsigned int getTimer1Period(){
	return g_timer[1];
}

unsigned int getTimer2Period(){
	return g_timer[2];
}

int flashLoggerConfig(){
	g_isFlashed = 1;
	return 1;
}

