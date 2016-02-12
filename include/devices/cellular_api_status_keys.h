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

/*
 * This file keep the conversion methods that we use to represent
 * keys to the app and RCL platform.  The idea behind using keys
 * is to be able to send human readable strings that are able to
 * be displayed in a pinch (ie when no pretty string exists on the
 * app), but are also standardized for a field so that we are
 * able to improve the language as needed.
 */

#ifndef _CELLULAR_API_STATUS_KEYS_H_
#define _CELLULAR_API_STATUS_KEYS_H_

#include "cpp_guard.h"
#include "cellular.h"

CPP_GUARD_BEGIN

/**
 * Returns the status key of cellular_net_status enum for API consumption.
 */
const char* cellular_net_status_api_key(const enum cellular_net_status s);

/**
 * Returns the status key of a telemetry_status_type enum for API consumption.
 */
const char* cellular_telemetry_status_api_key(const telemetry_status_t s);

CPP_GUARD_END

#endif /* _CELLULAR_API_STATUS_KEYS_H_ */
