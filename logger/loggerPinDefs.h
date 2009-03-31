#ifndef LOGGERPINDEFS_H_
#define LOGGERPINDEFS_H_

//define PIOs for analog/frequency outputs
#define FREQ_ANALOG_1 (1<<7)
#define FREQ_ANALOG_2 (1<<23)
#define FREQ_ANALOG_3 (1<<24)
#define FREQ_ANALOG_4 (1<<25)
#define FREQ_ANALOG_MASK (FREQ_ANALOG_1|FREQ_ANALOG_2|FREQ_ANALOG_3|FREQ_ANALOG_4)

/// PWM frequency in Hz.
#define PWM_FREQUENCY               2000

/// Maximum duty cycle value.
#define MAX_DUTY_CYCLE              1000
#define MIN_DUTY_CYCLE          	1

#define ADC_CHANNEL_MIN				0
#define ADC_CHANNEL_MAX				7

#define TIMER_CHANNEL_MIN			0
#define TIMER_CHANNEL_MAX			2

#define GPIO_CHANNEL_MIN			0
#define GPIO_CHANNEL_MAX			2

#define PWM_CHANNEL_MIN				0
#define PWM_CHANNEL_MAX				3

//define PIOs for GPIO ports
#define GPIO_1 (1<<8)
#define GPIO_2 (1<<16)
#define GPIO_3 (1<<27)
#define PIO_PUSHBUTTON_SWITCH (1 << 4)
#define GPIO_MASK (GPIO_1|GPIO_2|GPIO_3 | PIO_PUSHBUTTON_SWITCH)
#define ENABLED_GPIO_PINS GPIO_MASK
#define PUSHBUTTON_INTERRUPT_LEVEL	4

//define PIOs for status LEDs
#define LED1 (1<<1)
#define LED2 (1<<2)
#define LED_MASK        (LED1|LED2)

#endif
