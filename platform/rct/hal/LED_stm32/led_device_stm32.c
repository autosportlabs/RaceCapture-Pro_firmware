/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
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

#include "led_device.h"
#include "macros.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"

#include <stdbool.h>
#include <stdlib.h>

#define GPIO_PORT	GPIOA

/*
 * Order of the leds in this array matters!  Index order of LEDs (RGB).
 * Ensure you know what you are doing before you change it.
 *
 * LED outputs are on GPIOA group.  Pins 0 - 2
 * Blue  - A:0 -> LOGGING/Telemetry
 * Green - A:1 -> GPS
 * Red   - A:2 -> ERROR
 */
static struct led_data {
        const enum led led;
        uint16_t mask;
        bool level;
} leds[] = {
        {LED_ERROR, GPIO_Pin_0, 0},
        {LED_GPS,   GPIO_Pin_1, 0},
        {LED_WIFI,  GPIO_Pin_2, 0},
};

static bool led_set_level(struct led_data *ld, const bool on)
{
        if (NULL == ld)
                return false;

        ld->level = on;
        if (on) {
                GPIO_ResetBits(GPIO_PORT, ld->mask);
        } else {
                GPIO_SetBits(GPIO_PORT, ld->mask);
        }

        return true;
}

bool led_device_set_index(const size_t i, const bool on)
{
        return i < ARRAY_LEN(leds) ? led_set_level(leds + i, on) : false;
}

struct led_data* find_led_data(const enum led l)
{
        /* Use an unsigned value here to handle negative indicies */
        for (size_t i = 0; i < ARRAY_LEN(leds); ++i) {
                struct led_data *ld = leds + i;
                if (l == ld->led)
                        return ld;
        }

        return NULL;
}

bool led_device_set(const enum led l, const bool on)
{
        struct led_data *ld = find_led_data(l);
        return led_set_level(ld, on);
}


bool led_device_toggle(const enum led l)
{
        struct led_data *ld = find_led_data(l);
        return ld ? led_set_level(ld, !ld->level) : false;
}

void led_device_set_all(const bool on)
{
        for (size_t i = 0; i < ARRAY_LEN(leds); ++i) {
                struct led_data *ld = leds + i;
                led_set_level(ld, on);
        }
}

bool led_device_init(void)
{
        GPIO_InitTypeDef gpio_conf;

        /* Clear the GPIO Structure */
        GPIO_StructInit(&gpio_conf);

        /* turn on debug port and clock */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

        gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
        gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
        gpio_conf.GPIO_OType = GPIO_OType_PP;
        gpio_conf.GPIO_Pin = 0;

        for (size_t i = 0; i < ARRAY_LEN(leds); ++i) {
                struct led_data *ld = leds + i;
                gpio_conf.GPIO_Pin = ld->mask;
                GPIO_Init(GPIO_PORT, &gpio_conf);
        }

	led_device_set_all(false);
        return true;
}
