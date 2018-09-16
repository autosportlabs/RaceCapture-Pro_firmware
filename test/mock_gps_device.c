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


#include "gps.h"
#include "gps_device.h"
#include "serial.h"

gps_status_t GPS_device_init(uint8_t targetSampleRate, struct Serial *serial)
{
    return GPS_STATUS_PROVISIONED;
}

gps_msg_result_t GPS_device_get_update(GpsSample *gpsSample, struct Serial *serial)
{
    return GPS_MSG_SUCCESS;
}

void GPS_set_UTC_time(millis_t time)
{
        /* Do nothing, this driver always sets UTC time on it's own */
}

millis_t GPS_get_UTC_time()
{
        return 0;
}
