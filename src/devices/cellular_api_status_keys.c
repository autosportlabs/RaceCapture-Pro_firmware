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

#include "cellular.h"
#include "cellular_api_status_keys.h"

const char* cellular_net_status_api_key(const enum cellular_net_status s)
{
        switch(s) {
        case CELLULAR_NETWORK_NOT_SEARCHING:
                return "stopped_searching";
        case CELLULAR_NETWORK_SEARCHING:
                return "searching";
        case CELLULAR_NETWORK_DENIED:
                return "denied";
        case CELLULAR_NETWORK_REGISTERED:
                return "registered";
        case CELLULAR_NETWORK_STATUS_UNKNOWN:
        default:
                return "unknown";
        }
}

const char* cellular_telemetry_status_api_key(const telemetry_status_t s)
{
        switch(s) {
        case TELEMETRY_STATUS_IDLE:
                return "idle";
        case TELEMETRY_STATUS_CONNECTED:
                return "connected";
        case TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED:
                return "terminated";
        case TELEMETRY_STATUS_REJECTED_DEVICE_ID:
                return "rejected";
        case TELEMETRY_STATUS_SERVER_CONNECTION_FAILED:
                return "connect_failed";
        case TELEMETRY_STATUS_INTERNET_CONFIG_FAILED:
                return "data_failed";
        case TELEMETRY_STATUS_CELL_REGISTRATION_FAILED:
                return "registration_failed";
        case TELEMETRY_STATUS_MODEM_INIT_FAILED:
                return "init_failed";
        case TELEMETRY_STATUS_DATA_PLAN_NOT_AVAILABLE: /* NOT USED */
        default:
                return "unknown";
        }
}
