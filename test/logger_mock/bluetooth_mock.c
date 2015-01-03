/**
 * AutoSport Labs - Race Capture Pro Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture Pro firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "bluetooth.h"
#include "FreeRTOS.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "task.h"
#include <stdio.h>
#define COMMAND_WAIT 	600

static int configureBt(DeviceConfig *config, unsigned int targetBaud, const char * deviceName) {
    return 0;
}

static int bt_probe_config(unsigned int probeBaud, unsigned int targetBaud, const char * deviceName,
        DeviceConfig *config) {
   return DEVICE_INIT_SUCCESS;
}

int bt_init_connection(DeviceConfig *config) {
    printf("\r\nbt_init_connection");
    return DEVICE_INIT_SUCCESS;
}

int bt_check_connection_status(DeviceConfig *config) {
    return DEVICE_STATUS_NO_ERROR;
}
