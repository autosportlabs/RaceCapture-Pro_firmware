#include "sim900_device.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

#define EXT_POWER_CONTROL_PIN GPIO_Pin_10
#define EXT_POWER_CONTROL_PORT GPIOE

static uint32_t gpio_is_init = 0;

static void sim900_gpio_init(void)
{

    GPIO_InitTypeDef gpio_conf;

    /* Clear the GPIO Structure */
    GPIO_StructInit(&gpio_conf);

    /* turn on debug port and clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
	gpio_conf.GPIO_OType = GPIO_OType_OD;
	gpio_conf.GPIO_PuPd = GPIO_PuPd_UP;
	gpio_conf.GPIO_Pin = EXT_POWER_CONTROL_PIN;
	GPIO_Init(EXT_POWER_CONTROL_PORT, &gpio_conf);
	gpio_is_init = 1;
}

void sim900_device_power_button(uint32_t pressed)
{
    sim900_gpio_init();

    if (pressed) {
        GPIO_ResetBits(EXT_POWER_CONTROL_PORT, EXT_POWER_CONTROL_PIN);
    } else {
        GPIO_SetBits(EXT_POWER_CONTROL_PORT, EXT_POWER_CONTROL_PIN);
    }
}
