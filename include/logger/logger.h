/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
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
#ifndef LOGGER_H_
#define LOGGER_H_
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LOGGING_STATUS_IDLE = 0,
    LOGGING_STATUS_WRITING,
    LOGGING_STATUS_ERROR_WRITING
} logging_status_t;

bool logging_is_active();
logging_status_t logging_get_status( void );
void logging_set_status( logging_status_t status );

int32_t logging_active_time( void );
void logging_set_logging_start( int32_t start );

#endif /* LOGGER_H_ */
