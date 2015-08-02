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
#include "logger.h"
#include "dateTime.h"

static logging_status_t g_logging_status = LOGGING_STATUS_IDLE;
static int g_logging_since = 0;

void logging_set_status(logging_status_t status)
{
    g_logging_status = status;
}

logging_status_t logging_get_status( void )
{
    return g_logging_status;
}

void logging_set_logging_start( int32_t start )
{
    g_logging_since = start;
}

int32_t logging_active_time( void )
{
    if (g_logging_since) {
        int uptime = getUptimeAsInt();
        int duration = uptime - g_logging_since;
        return duration;
    }
    return 0;
}

bool logging_is_active()
{
    return g_logging_since > 0;
}
