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

#ifndef _CELLULAR_H_
#define _CELLULAR_H_

#include "stddef.h"
#include "devices_common.h"

typedef enum {
    TELEMETRY_STATUS_IDLE = 0,
    TELEMETRY_STATUS_CONNECTED,
    TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED,
    TELEMETRY_STATUS_REJECTED_DEVICE_ID,
    TELEMETRY_STATUS_DATA_PLAN_NOT_AVAILABLE,
    TELEMETRY_STATUS_SERVER_CONNECTION_FAILED,
    TELEMETRY_STATUS_INTERNET_CONFIG_FAILED,
    TELEMETRY_STATUS_CELL_REGISTRATION_FAILED
} telemetry_status_t;

telemetry_status_t cellular_get_connection_status();
int32_t cellular_active_time();
int cellular_disconnect(DeviceConfig *config);
int cellular_init_connection(DeviceConfig *config);
int cellular_check_connection_status(DeviceConfig *config);

#endif /* _CELLULAR_H_ */
