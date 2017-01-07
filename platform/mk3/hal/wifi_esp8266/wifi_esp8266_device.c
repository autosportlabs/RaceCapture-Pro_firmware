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

#include "wifi_device.h"
#include <stdbool.h>
#include "taskUtil.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/* How long we hold the reset line for the ESP8266 */
#define ESP8266_RESET_DURATION_MS 1

/* How long we wait after reset.
 * Approximately the boot time for the ESP8266
 */
#define ESP8266_BOOT_DURATION_MS 50

/* I/O definitions for ESP8266 BOOT (PE12)*/
#define ESP8266_BOOT_PIN GPIO_Pin_12
#define ESP8266_BOOT_GPIO GPIOE

/* I/O definitions for ESP8266 RESET (PE13)*/
#define ESP8266_RESET_PIN GPIO_Pin_13
#define ESP8266_RESET_GPIO GPIOE

/**
 * Sets up GPIO ports for controlling the ESP8266:
 * * Enable boot mode from flash
 * * Enable control of reset line
 */
static void esp8266_init_io(void)
{

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

        GPIO_InitTypeDef gpio_conf;
        GPIO_StructInit(&gpio_conf);

        /* Configure the ESP8266 BOOT and RESET pins */
        gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
        gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
        gpio_conf.GPIO_OType = GPIO_OType_PP;
        gpio_conf.GPIO_Pin = ESP8266_BOOT_PIN | ESP8266_RESET_PIN;
        GPIO_Init(ESP8266_BOOT_GPIO, &gpio_conf);
}

/**
 * Enables flash boot mode for ESP8266
 */
static void esp8266_enable_flash_boot(void)
{
        GPIO_SetBits(ESP8266_BOOT_GPIO, ESP8266_BOOT_PIN);
}

/**
 * Perform a hard reset of the ESP8266 module
 */
bool wifi_device_reset()
{
        GPIO_ResetBits(ESP8266_RESET_GPIO, ESP8266_RESET_PIN);
        delayMs(ESP8266_RESET_DURATION_MS);
        GPIO_SetBits(ESP8266_RESET_GPIO, ESP8266_RESET_PIN);
        delayMs(ESP8266_BOOT_DURATION_MS);
        return true;
}

/**
 * Initialize the ESP8266 GPIO control lines
 */
bool wifi_device_init()
{
        esp8266_init_io();
        esp8266_enable_flash_boot();
        return wifi_device_reset();
}
