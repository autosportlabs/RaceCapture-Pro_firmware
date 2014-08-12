#include "GPIO_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>


//#define	MODE_INPUT	0
//#define MODE_OUTPUT	1

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

struct gpio {
	GPIO_TypeDef *port;
	uint16_t mask;
	uint8_t level;
};

static struct gpio gpios[] = {
	{ GPIOE, GPIO_Pin_40, 0 },
	{ GPIOE, GPIO_Pin_39, 0 },
	{ GPIOE, GPIO_Pin_38, 0 },
	{ GPIOC, GPIO_Pin_13, 0 },
	{ GPIOC, GPIO_Pin_14, 0 },
	{ GPIOC, GPIO_Pin_15, 0 },
	{ GPIOE, GPIO_Pin_10, 0 },
	{ GPIOA, GPIO_Pin_8, 0 }
};


static void GPIO_port_enable(unsigned int led){
	GPIO_ResetBits(gpios[led].port, gpios[led].mask);
}

static void GPIO_port_disable(unsigned int led){
	GPIO_SetBits(gpios[led].port, gpios[led].mask);
}

void gpio_irq_handler ( void );

static unsigned int GetGPIOBits(void){
	return 0;

}

static void clear_GPIO_bits(unsigned int portBits){

}

static void set_GPIO_bits(unsigned int portBits){

}

void GPIO_device_init_port(unsigned int port, unsigned int mode){


}

void GPIO_device_init_base(void){

}

void GPIO_device_init_SD_card_IO(void){
}

int GPIO_device_is_SD_card_present(void){
	return 1;
}

int GPIO_device_is_SD_card_writable(void){
	return 1;
}

int GPIO_device_is_button_pressed(void){
	return 0;
}

void GPIO_device_set(unsigned int port, unsigned int state){

}

unsigned int GPIO_device_get(unsigned int port){
	return 0;
}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){
}


void GPIO_device_init_pushbutton(void){
}

int LED_device_init(void){
	int i;
	GPIO_InitTypeDef gpio_conf;

	/* Clear the GPIO Structure */
	GPIO_StructInit(&gpio_conf);

	/* turn on debug port and clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
	gpio_conf.GPIO_OType = GPIO_OType_PP;

	for (i = 0; i < 4; ++i){
		gpio_conf.GPIO_Pin = leds[i].mask;
		GPIO_Init(leds[i].port, &gpio_conf);
		LED_device_disable(i);
	}
	return 1;
}

