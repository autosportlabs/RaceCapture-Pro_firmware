#include "GPIO_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerConfig.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>


//#define	MODE_INPUT	0
//#define MODE_OUTPUT	1

struct gpio {
	uint32_t rcc_ahb1;
	GPIO_TypeDef *port;
	uint16_t mask;
	GPIOMode_TypeDef mode;
	uint8_t level;

};

static struct gpio gpios[] = {
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_9, GPIO_Mode_IN, 0 },
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_8, GPIO_Mode_IN, 0 },
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_7, GPIO_Mode_IN, 0 },
	{ RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_13, GPIO_Mode_OUT, 0 },
	{ RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_14, GPIO_Mode_OUT, 0 },
	{ RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_15, GPIO_Mode_OUT, 0 },
	{ RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_10, GPIO_Mode_OUT, 0 },
	{ RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_8, 	GPIO_Mode_IN, 0 }
};

typedef enum{
	GPI0_ID = 0,
	GPI1_ID,
	GPI2_ID,
	GPO0_ID,
	GPO1_ID,
	GPO2_ID,
	EXT_CTRL_ID,
	PUSHBUTTON_ID,
} GPIO_IDs;

#define GPIO_COUNT 8


void GPIO_device_set(unsigned int port, unsigned int state){
	int gpioIndex = -1;
	switch(port){
	case 0:
		gpioIndex = GPO0_ID;
		break;
	case 1:
		gpioIndex = GPO1_ID;
		break;
	case 2:
		gpioIndex = GPO1_ID;
		break;
	default:
		break;
	}
	if (gpioIndex >=0){
		if (state){
			GPIO_SetBits(gpios[gpioIndex].port, gpios[gpioIndex].mask);
		}
		else{
			GPIO_ResetBits(gpios[gpioIndex].port, gpios[gpioIndex].mask);
		}
	}

}

int GPIO_device_is_button_pressed(void){
	return 0;
}

unsigned int GPIO_device_get(unsigned int port){
	return 0;
}

int GPIO_device_init(LoggerConfig *loggerConfig){
	int i;
	GPIO_InitTypeDef gpio_conf;

	/* Clear the GPIO Structure */
	GPIO_StructInit(&gpio_conf);
	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;

	for (i = 0; i < GPIO_COUNT; ++i){
		RCC_AHB1PeriphClockCmd(gpios[i].rcc_ahb1, ENABLE);
		gpio_conf.GPIO_Pin = gpios[i].mask;
		GPIO_Init(gpios[i].port, &gpio_conf);
		if (gpio_conf.GPIO_Mode == GPIO_Mode_OUT){
			gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
			gpio_conf.GPIO_OType = GPIO_OType_PP;
			GPIO_ResetBits(gpios[i].port, gpios[i].mask);
		}
		else{
			gpio_conf.GPIO_Mode = GPIO_Mode_IN;
		}

	}
	return 1;
}
