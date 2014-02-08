#ifndef GPIO_DEVICE_AT91_PIN_MAP_H_
#define GPIO_DEVICE_AT91_PIN_MAP_H_

//define PIOs for GPIO ports
#define GPIO_1 (1 << 8)
#define GPIO_2 (1 << 30)
#define GPIO_3 (1 << 27)
#define SD_WRITE_PROTECT (1 << 28)
#define SD_CARD_DETECT (1 << 29)
#define PIO_PUSHBUTTON_SWITCH (1 << 4)
#define GPIO_MASK (GPIO_1 | GPIO_2 | GPIO_3 )
#define ENABLED_GPIO_PINS GPIO_MASK
#define PUSHBUTTON_INTERRUPT_LEVEL	4

#endif /* GPIO_DEVICE_AT91_PIN_MAP_H_ */
