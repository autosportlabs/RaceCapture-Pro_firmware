#ifndef LOGGERHARDWARE_H_
#define LOGGERHARDWARE_H_

#include "board.h"

#define FREQ_ANALOG_1 (1<<7)
#define FREQ_ANALOG_2 (1<<23)
#define FREQ_ANALOG_3 (1<<24)
#define FREQ_ANALOG_4 (1<<25)

#define GPIO_1 (1<<8)
#define GPIO_2 (1<<16)
#define GPIO_3 (1<<27)

#define LED_MASK        (LED1|LED2)

#define FREQ_ANALOG_MASK (FREQ_ANALOG_1|FREQ_ANALOG_2|FREQ_ANALOG_3|FREQ_ANALOG_4)
#define GPIO_MASK (GPIO_1|GPIO_2|GPIO_3)

void InitGPIO();
void InitADC();
void InitPWM();

void SetGPIO(unsigned int gpioPort);
void SetFREQ_ANALOG(unsigned int freqAnalogPort);
void ClearFREQ_ANALOG(unsigned int freqAnalogPort);

unsigned int ReadAllADC(unsigned int *a0, 
						unsigned int *a1, 
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 );
												
unsigned int ReadADC(unsigned int channel);

void PWM_ConfigureClocks
    (unsigned int clka,
     unsigned int clkb,
     unsigned int mck);
     
unsigned short GetClockConfiguration(
    unsigned int frequency,
    unsigned int mck);

void PWM_ConfigureChannel(
    unsigned char channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity);
    
void PWM_SetPeriod(	
	unsigned char channel, 
	unsigned short period);

void PWM_ConfigureChannel(
    unsigned char channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity);

void PWM_SetDutyCycle(
	unsigned char channel, 
	unsigned short duty);
	
void PWM_EnableChannel(
	unsigned char channel);
#endif /*LOGGERHARDWARE_H_*/
