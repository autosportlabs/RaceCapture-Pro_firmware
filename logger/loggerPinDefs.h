#ifndef LOGGERPINDEFS_H_
#define LOGGERPINDEFS_H_

//define PIOs for analog/frequency outputs
#define FREQ_ANALOG_1 (1<<7)
#define FREQ_ANALOG_2 (1<<23)
#define FREQ_ANALOG_3 (1<<24)
#define FREQ_ANALOG_4 (1<<25)
#define FREQ_ANALOG_MASK (FREQ_ANALOG_1|FREQ_ANALOG_2|FREQ_ANALOG_3|FREQ_ANALOG_4)

//define PIOs for GPIO ports
#define GPIO_1 (1 << 8)
#define GPIO_2 (1 << 31)
#define GPIO_3 (1 << 27)
#define SD_WRITE_PROTECT (1 << 28)
#define SD_CARD_DETECT (1 << 29)
#define PIO_PUSHBUTTON_SWITCH (1 << 4)
#define GPIO_MASK (GPIO_1 | GPIO_2 | GPIO_3 )
#define ENABLED_GPIO_PINS GPIO_MASK
#define PUSHBUTTON_INTERRUPT_LEVEL	4

//define PIOs for status LEDs
#define LED1 			(1 << 1)
#define LED2 			(1 << 2)
#define LED_MASK        (LED1 | LED2)

#endif
