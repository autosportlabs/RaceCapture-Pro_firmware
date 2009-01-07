#ifndef LOGGERHARDWARE_H_
#define LOGGERHARDWARE_H_



//define PIOs for analog/frequency outputs
#define FREQ_ANALOG_1 (1<<7)
#define FREQ_ANALOG_2 (1<<23)
#define FREQ_ANALOG_3 (1<<24)
#define FREQ_ANALOG_4 (1<<25)
#define FREQ_ANALOG_MASK (FREQ_ANALOG_1|FREQ_ANALOG_2|FREQ_ANALOG_3|FREQ_ANALOG_4)

/// PWM frequency in Hz.
#define PWM_FREQUENCY               5000

/// Maximum duty cycle value.
#define MAX_DUTY_CYCLE              1000
#define MIN_DUTY_CYCLE          	2


//define PIOs for GPIO ports
#define GPIO_1 (1<<8)
#define GPIO_2 (1<<16)
#define GPIO_3 (1<<27)
#define PIO_PUSHBUTTON_SWITCH (1 << 4)
#define GPIO_MASK (GPIO_1|GPIO_2|GPIO_3 | PIO_PUSHBUTTON_SWITCH)
#define ENABLED_GPIO_PINS GPIO_MASK
#define PUSHBUTTON_INTERRUPT_LEVEL	6

//define PIOs for status LEDs
#define LED1 (1<<1)
#define LED2 (1<<2)
#define LED_MASK        (LED1|LED2)

//Init GPIO ports
void InitGPIO();

void pushbutton_irq_handler ( void );

void onPushbuttonTask(void *pvParameters);


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

//Set LEDs
void Set_LED(unsigned int Led);
//Clear LEDs
void Clear_LED(unsigned int Led);

//Toggle LED state
void Toggle_LED (unsigned int Led);

//Set specified bits on PIO
void SetGPIOBits(unsigned int portBits);

//Clear specified bits on PIO
void ClearGPIOBits(unsigned int portBits);

//Set bit for specified Frequency/Analog port
void SetFREQ_ANALOG(unsigned int freqAnalogPort);

//Clear bit for specified Frequency/Analog port
void ClearFREQ_ANALOG(unsigned int freqAnalogPort);

//Read all ADC ports
unsigned int ReadAllADC(unsigned int *a0, 
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
#endif /*LOGGERHARDWARE_H_*/
