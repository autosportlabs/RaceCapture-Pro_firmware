#include "LED_device.h"
#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>

struct led {
    GPIO_TypeDef *port;
    uint16_t mask;
    uint8_t level;
};

#define LED_COUNT 3
static struct led leds[] = {
    {GPIOA, GPIO_Pin_0, 0},
    {GPIOA, GPIO_Pin_1, 0},
    {GPIOA, GPIO_Pin_2, 0}
};

int LED_device_init(void)
{
    int i;
    GPIO_InitTypeDef gpio_conf;

    /* Clear the GPIO Structure */
    GPIO_StructInit(&gpio_conf);

    /* turn on debug port and clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
    gpio_conf.GPIO_OType = GPIO_OType_PP;

    for (i = 0; i < LED_COUNT; ++i) {
        gpio_conf.GPIO_Pin = leds[i].mask;
        GPIO_Init(leds[i].port, &gpio_conf);
        LED_device_disable(i);
    }
    return 1;
}

void LED_device_enable(unsigned int led)
{
    GPIO_SetBits(leds[led].port, leds[led].mask);
}

void LED_device_disable(unsigned int led)
{
    GPIO_ResetBits(leds[led].port, leds[led].mask);
}

void LED_device_toggle(unsigned int led)
{
    if (led >= LED_COUNT)
        return;

    if (leds[led].level == 0) {
        leds[led].level = 1;
        GPIO_ResetBits(leds[led].port, leds[led].mask);
    } else {
        leds[led].level = 0;
        GPIO_SetBits(leds[led].port, leds[led].mask);
    }
}
