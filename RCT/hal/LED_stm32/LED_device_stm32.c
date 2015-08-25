/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * LED outputs are on GPIOA group.  Pins 0 - 2
 * 0 - Blue
 * 1 - Green
 * 2 - Red
 */

#include "LED_device.h"

#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>

struct led {
    uint16_t mask;
    uint8_t level;
};

static struct led leds[] = {
        {GPIO_Pin_0, 0},
        {GPIO_Pin_1, 0},
        {GPIO_Pin_2, 0}
};

#define LED_COUNT (sizeof(leds) / sizeof(struct led))

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

    for (i = 0; i < LED_COUNT; ++i)
        gpio_conf.GPIO_Pin |= leds[i].mask;

    GPIO_Init(GPIOA, &gpio_conf);

    for (i = 0; i < LED_COUNT; ++i)
            LED_device_disable(i);

    return 1;
}

void LED_device_enable(unsigned int led)
{
    if (led >= LED_COUNT)
        return;

    leds[led].level = 1;
    GPIO_SetBits(GPIOA, leds[led].mask);
}

void LED_device_disable(unsigned int led)
{
    if (led >= LED_COUNT)
        return;

    leds[led].level = 0;
    GPIO_ResetBits(GPIOA, leds[led].mask);
}

void LED_device_toggle(unsigned int led)
{
    if (led >= LED_COUNT)
        return;

    leds[led].level ? LED_device_disable(led) : LED_device_enable(led);
}
