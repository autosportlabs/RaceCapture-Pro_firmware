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

#include "cellular.h"
#include "printk.h"

#include <stdbool.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

#define EXT_POWER_CONTROL_PIN GPIO_Pin_10
#define EXT_POWER_CONTROL_PORT GPIOE

static void cell_pwr_btn_init()
{
        static bool configured = false;

        if (configured)
                return;

        GPIO_InitTypeDef gpio_conf;

        /* Clear the GPIO Structure */
        GPIO_StructInit(&gpio_conf);

        /* turn on debug port and clock */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

        /*
         * This configuration should work on both the u-blox sara u280 and
         * the sim900 without issue.
         */
        gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
        gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
        gpio_conf.GPIO_Pin = EXT_POWER_CONTROL_PIN;
        gpio_conf.GPIO_OType = GPIO_OType_OD;
        gpio_conf.GPIO_PuPd = GPIO_PuPd_UP;

        GPIO_Init(EXT_POWER_CONTROL_PORT, &gpio_conf);
        configured = true;
}

void cell_pwr_btn(const bool pressed)
{
        cell_pwr_btn_init();

        if (pressed) {
                GPIO_ResetBits(EXT_POWER_CONTROL_PORT, EXT_POWER_CONTROL_PIN);
        } else {
                GPIO_SetBits(EXT_POWER_CONTROL_PORT, EXT_POWER_CONTROL_PIN);
        }
}
