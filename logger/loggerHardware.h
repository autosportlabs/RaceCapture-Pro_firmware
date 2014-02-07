#ifndef LOGGERHARDWARE_H_
#define LOGGERHARDWARE_H_

#include "loggerConfig.h"
#include "loggerPinDefs.h"
#include <stddef.h>

#define TC_CLKS_MCK2            0x0
#define TC_CLKS_MCK8            0x1
#define TC_CLKS_MCK32           0x2
#define TC_CLKS_MCK128          0x3
#define TC_CLKS_MCK1024         0x4

void InitLoggerHardware();

void InitGPIO(LoggerConfig *loggerConfig);
void InitSDCard(void);

int isCardPresent(void);
int isCardWritable(void);
int isButtonPressed(void);

void InitLEDs(void);
void InitPushbutton(void);

void enableLED(unsigned int Led);
void disableLED(unsigned int Led);
void toggleLED (unsigned int Led);

void ResetWatchdog();

void InitWatchdog(int timeoutMs);


void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3);

int readGpio(unsigned int channel);

void setGpio(unsigned int channel, unsigned int state);

//Set bit for specified Frequency/Analog port
void SetFREQ_ANALOG(unsigned int freqAnalogPort);

//Clear bit for specified Frequency/Analog port
void ClearFREQ_ANALOG(unsigned int freqAnalogPort);

//PWM Channel Functions
void InitPWM(LoggerConfig *loggerConfig);

void StartPWM(unsigned int pwmChannel);
void StartAllPWM();

void StopPWM(unsigned int pwmChannel);
void StopAllPWM();

//Set PWM period for specified channel
void PWM_SetPeriod(	unsigned int channel, unsigned short period);

unsigned short PWM_GetPeriod(unsigned int channel);

//Set PWM duty cycle for specified channel
void PWM_SetDutyCycle(unsigned int channel,	unsigned short duty);

unsigned short PWM_GetDutyCycle(unsigned int channel);

int flashLoggerConfig();

#endif /*LOGGERHARDWARE_H_*/
