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


#include "GPIO_device.h"

#define GPIO_CHANNELS 3

static int g_isCardPresent = 0;
static int g_isCardWritable = 0;
static int g_isButtonPressed = 0;
static int g_gpio[GPIO_CHANNELS] = {0,0,0};


int GPIO_device_init(LoggerConfig *loggerConifg)
{
    return 1;
}

void GPIO_device_init_port(unsigned int port, unsigned int mode)
{

}
void GPIO_device_init_SD_card_IO(void)
{

}

void GPIO_device_init_pushbutton(void)
{

}

unsigned int GPIO_device_get(unsigned int port)
{
    return g_gpio[port];
}

void GPIO_device_set(unsigned int port, unsigned int state)
{
    g_gpio[port] = state;
}

int GPIO_device_is_SD_card_present(void)
{
    return g_isCardPresent;
}

int GPIO_device_is_SD_card_writable(void)
{
    return g_isCardWritable;
}

int GPIO_device_is_button_pressed(void)
{
    return g_isButtonPressed;
}

void mock_setIsCardPresent(int present)
{
    g_isCardPresent = present;
}

void mock_setIsCardWritable(int writable)
{
    g_isCardWritable = writable;
}

void mock_setIsButtonPressed(int pressed)
{
    g_isButtonPressed = pressed;
}
