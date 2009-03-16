#ifndef LOGGERHARDWARE_H_
#define LOGGERHARDWARE_H_

#include "loggerConfig.h"
#include "loggerPinDefs.h"

#define TC_CLKS_MCK2            0x0
#define TC_CLKS_MCK8            0x1
#define TC_CLKS_MCK32           0x2
#define TC_CLKS_MCK128          0x3
#define TC_CLKS_MCK1024         0x4

//Init GPIO ports
void InitGPIO();


//Init ADC ports
void InitADC();

//Init LED ports
void InitLEDs();

//Enable LED
void EnableLED(unsigned int Led);
//Clear LEDs
void DisableLED(unsigned int Led);

//Toggle LED state
void ToggleLED (unsigned int Led);

unsigned int GetGPIOBits();

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


//PWM Channel Functions
void InitPWM(struct LoggerConfig *loggerConfig);
void EnablePWMChannel(unsigned int channel, struct PWMConfig *config);

void StartPWM(unsigned int pwmChannel);
void StartAllPWM();

void StopPWM(unsigned int pwmChannel);
void StopAllPWM();


//Configure PWM clock
void PWM_ConfigureClocks
    (unsigned int clka,
     unsigned int clkb,
     unsigned int mck);
     
//Retrieve PWM clock configuration
unsigned short PWM_GetClockConfiguration(
    unsigned int frequency,
    unsigned int mck);

//Set PWM period for specified channel
void PWM_SetPeriod(	unsigned int channel, unsigned short period);

unsigned short PWM_GetPeriod(unsigned int channel);

//Configure PWM channel
void PWM_ConfigureChannel(
    unsigned char channel,
    unsigned int prescaler,
    unsigned int alignment,
    unsigned int polarity);

//Set PWM duty cycle for specified channel
void PWM_SetDutyCycle(unsigned int channel,	unsigned short duty);

unsigned short PWM_GetDutyCycle(unsigned int channel);

//Enable PWM channel
void PWM_EnableChannel(unsigned int channel);

void initTimerChannels(struct LoggerConfig *loggerConfig);	
void initTimer0(struct TimerConfig *timerConfig);
void initTimer1(struct TimerConfig *timerConfig);
void initTimer2(struct TimerConfig *timerConfig);
unsigned int timerClockFromDivider(unsigned int divider);

void getAllTimerPeriods(unsigned int *t0, unsigned int *t1, unsigned int *t2);
inline unsigned int getTimer0Period();
inline unsigned int getTimer1Period();
inline unsigned int getTimer2Period();

inline unsigned int calculateRPM(unsigned int timerTicks, unsigned int scaling);
inline unsigned int calculateFrequencyHz(unsigned int timerTicks, unsigned int scaling);
inline unsigned int calculatePeriodMs(unsigned int timerTicks, unsigned int scaling);
inline unsigned int calculatePeriodUsec(unsigned int timerTicks, unsigned int scaling);


#endif /*LOGGERHARDWARE_H_*/
