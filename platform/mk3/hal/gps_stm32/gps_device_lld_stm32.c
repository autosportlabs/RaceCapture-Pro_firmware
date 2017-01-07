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

/**
 * How long we hold the reset line for the GPS module
 */
#define GPS_RESET_DURATION_MS 1

/**
 *  How long we wait after reset.
 */
#define GPS_BOOT_DURATION_MS 50

/**
 *  I/O definitions for GPS RESET (PE1)
 */
#define GPS_RESET_PIN GPIO_Pin_1
#define GPS_RESET_GPIO GPIOE

/**
 * Sets up GPIO ports for controlling GPS reset line
 */
static void gps_init_io(void)
{

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

        GPIO_InitTypeDef gpio_conf;
        GPIO_StructInit(&gpio_conf);

        /* Configure the GPS RESET pins */
        gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
        gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
        gpio_conf.GPIO_OType = GPIO_OType_PP;
        gpio_conf.GPIO_Pin = GPS_RESET_PIN;
        GPIO_Init(GPS_RESET_GPIO, &gpio_conf);
}

/**
 * Perform a hard reset of the GPS module
 */
bool gps_device_lld_reset()
{
        GPIO_ResetBits(GPS_RESET_GPIO, GPS_RESET_PIN);
        delayMs(GPS_RESET_DURATION_MS);
        GPIO_SetBits(GPS_RESET_GPIO, GPS_RESET_PIN);
        delayMs(GPS_BOOT_DURATION_MS);
        return true;
}

/**
 * Initialize the GPS GPIO control lines
 */
bool gps_device_lld_init()
{
        gps_init_io();
        return gps_device_lld_reset();
}
