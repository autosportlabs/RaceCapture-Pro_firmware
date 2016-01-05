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


#include "LED.h"
#include "LED_device.h"

int LED_init(void)
{
    return LED_device_init();
}

void LED_enable(unsigned int Led)
{
    LED_device_enable(Led);
}

void LED_disable(unsigned int Led)
{
    LED_device_disable(Led);
}

void LED_toggle(unsigned int Led)
{
    LED_device_toggle(Led);
}
