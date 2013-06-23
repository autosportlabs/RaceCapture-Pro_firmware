/*
 * loggerHardware.c
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#include "loggerHardware.h"

void InitLoggerHardware(){

}

void InitGPIO(LoggerConfig *loggerConfig){}

void InitSDCard(void){}

int isCardPresent(void){
	return 1;
}
int isCardWritable(void){
	return 1;
}

int isButtonPressed(void){
	return 0;
}

void InitADC(void){}

void InitLEDs(void){}

void InitPushbutton(void){}

void enableLED(unsigned int Led){}

void disableLED(unsigned int Led){}

void toggleLED (unsigned int Led){}

void ResetWatchdog(){}

void InitWatchdog(int timeoutMs){}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){}


void setGpio(unsigned int channel, unsigned int state){}

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
						unsigned int *a7 ){}

//Read specified ADC channel
unsigned int readADC(unsigned int channel){
	return 0;
}

//PWM Channel Functions
void InitPWM(LoggerConfig *loggerConfig){}

void StartPWM(unsigned int pwmChannel){}
void StartAllPWM(){}

void StopPWM(unsigned int pwmChannel){}
void StopAllPWM(){}

//Set PWM period for specified channel
void PWM_SetPeriod(	unsigned int channel, unsigned short period){}

unsigned short PWM_GetPeriod(unsigned int channel){}

//Set PWM duty cycle for specified channel
void PWM_SetDutyCycle(unsigned int channel,	unsigned short duty){}

unsigned short PWM_GetDutyCycle(unsigned int channel){}

//Configure PWM clock
//void PWM_ConfigureClocks
//    (unsigned int clka,
//     unsigned int clkb,
//     unsigned int mck);

//Retrieve PWM clock configuration
//unsigned short PWM_GetClockConfiguration(
//    unsigned int frequency,
//    unsigned int mck);

//Configure PWM channel
//void PWM_ConfigureChannel(
//    unsigned char channel,
//    unsigned int prescaler,
//    unsigned int alignment,
//    unsigned int polarity);


void initTimerChannels(LoggerConfig *loggerConfig){}

void initTimer0(TimerConfig *timerConfig){}

void initTimer1(TimerConfig *timerConfig){}

void initTimer2(TimerConfig *timerConfig){}

unsigned int timerClockFromDivider(unsigned short divider){
	return 0;
}

unsigned int getTimerPeriod(unsigned int channel){
	return 0;
}

unsigned int getTimerCount(unsigned int channel){
	return 0;
}

void resetTimerCount(unsigned int channel){}

void getAllTimerPeriods(unsigned int *t0, unsigned int *t1, unsigned int *t2){}


unsigned int getTimer0Period(){
	return 0;
}

unsigned int getTimer1Period(){
	return 0;
}

unsigned int getTimer2Period(){
	return 0;
}

unsigned int calculateRPM(unsigned int timerTicks, unsigned int scaling){
	return 0;
}

unsigned int calculateFrequencyHz(unsigned int timerTicks, unsigned int scaling){
	return 0;
}

unsigned int calculatePeriodMs(unsigned int timerTicks, unsigned int scaling){
	return 0;
}

unsigned int calculatePeriodUsec(unsigned int timerTicks, unsigned int scaling){
	return 0;
}

void calibrateAccelZero(){}

int flashLoggerConfig(){}

