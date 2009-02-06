#ifndef LOGGERHARDWARE_H_
#define LOGGERHARDWARE_H_

#include "loggerPinDefs.h"

//Init GPIO ports
void InitGPIO();


//Init ADC ports
void InitADC();

//PWM Channel Functions
void EnableAllPWM();
void EnablePWM0();
void EnablePWM1();
void EnablePWM2();
void EnablePWM3();

void StartPWM(unsigned int pwmChannel);
void StartAllPWM();

void StopPWM(unsigned int pwmChannel);
void StopAllPWM();

//Init LED ports
void InitLEDs();

//Enable LED
void EnableLED(unsigned int Led);
//Clear LEDs
void DisableLED(unsigned int Led);

//Toggle LED state
void ToggleLED (unsigned int Led);

//Set specified bits on PIO
void SetGPIOBits(unsigned int portBits);

//Clear specified bits on PIO
void ClearGPIOBits(unsigned int portBits);

//Set bit for specified Frequency/Analog port
void SetFREQ_ANALOG(unsigned int freqAnalogPort);

//Clear bit for specified Frequency/Analog port
void ClearFREQ_ANALOG(unsigned int freqAnalogPort);

//Read all ADC ports

void ReadAllADC(unsigned int *a0, 
						unsigned int *a1, 
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 );
					
//Read specified ADC channel												
unsigned int ReadADC(unsigned int channel);

//Configure PWM clock
void PWM_ConfigureClocks
    (unsigned int clka,
     unsigned int clkb,
     unsigned int mck);
     
//Retrieve PWM clock configuration
unsigned short GetClockConfiguration(
    unsigned int frequency,
    unsigned int mck);

//Set PWM period for specified channel
void PWM_SetPeriod(	
	unsigned char channel, 
	unsigned short period);

//Configure PWM channel
void PWM_ConfigureChannel(
    unsigned char channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity);

//Set PWM duty cycle for specified channel
void PWM_SetDutyCycle(
	unsigned char channel, 
	unsigned short duty);
	
//Enable PWM channel
void PWM_EnableChannel(
	unsigned char channel);
	
void initTimerChannels();	
void initTimer0();
void initTimer1();
void initTimer2();

unsigned int getTimer0Period();
unsigned int getTimer1Period();
unsigned int getTimer2Period();

#endif /*LOGGERHARDWARE_H_*/
