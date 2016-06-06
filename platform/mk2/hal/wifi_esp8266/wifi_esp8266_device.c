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

/**
 * Perform a hard reset of the ESP8266 module
 */
bool wifi_device_reset()
{
        /*No ability to hard reset the ESP8266 on MK2, so this is NOOP */
        return false;
}

/**
 * Initialize the ESP8266 GPIO control lines
 */
bool wifi_device_init()
{
        /*ESP8266 hardware configuration is fixed on MK2, so this is NOOP */
        return true;
}
