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

#ifndef SD_LOG_H_
#define SD_LOG_H_

#include "capabilities.h"
#include "cpp_guard.h"
#include "sdcard.h"

CPP_GUARD_BEGIN

#ifdef SDCARD_SUPPORT
#define sd_log( x ) _sd_log_write( x )
bool _sd_log_write( const char* str );
bool sd_log_int( const int value);
bool sd_log_uint( const unsigned int value);
bool sd_log_float( const float value);
#else // ! SD_CARD_SUPPORT
#define sd_log( x )
#define sd_log_int( x )
#define sd_log_uint( x )
#define sd_log_float( x )
#endif // SDCARD_SUPPORT

CPP_GUARD_END

#endif /*SD_LOG_H_*/
