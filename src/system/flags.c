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

#include "capabilities.h"
#include "flags.h"

#define FEATURE_FLAG(flag)	flag,

/**
 * Denotes features supported on the given unit.  These features flags
 * help the app determine what windows it should display in the config.
 * Flags are always returned in alphabetical order and the list is always
 * NULL terminated.
 * Meanings:
 * - activetrack Support setting the active track in non-volatile fashion
 * - adc Analog to digital converter support
 * - bt Bluetooth support
 * - can CAN bus support
 * - cell Cellular support
 * - gpio General purose input/output support
 * - gps Global Positioning Satellite support
 * - imu Inertia Measurement Unit support
 * - lua Lua scripting support
 * - pwm Pulsw width modulation generation output support
 * - telemstream Supports telemetry streaming API
 * - timer Timed pulse frequency measurement support
 * - tracks Track DB support
 * - usb USB connectivity support
 * - wifi WiFi support
 */
static const char* feature_flags[] = {
        FEATURE_FLAG("activetrack")
#if ANALOG_CHANNELS > 0
        FEATURE_FLAG("adc")
#endif
#if BLUETOOTH_SUPPORT > 0
        FEATURE_FLAG("bt")
#endif
#if CAN_CHANNELS > 0
        FEATURE_FLAG("can")
#endif
#if CELLULAR_SUPPORT
        FEATURE_FLAG("cell")
#endif
#if GPIO_CHANNELS > 0
        FEATURE_FLAG("gpio")
#endif
        FEATURE_FLAG("gps")
        FEATURE_FLAG("imu")
#if LUA_SUPPORT > 0
        FEATURE_FLAG("lua")
#endif
#if CAN_CHANNELS > 0
        FEATURE_FLAG("obd2")
#endif
#if PWM_CHANNELS > 0
        FEATURE_FLAG("pwm")
#endif
        FEATURE_FLAG("telemstream")
#if TIMER_CHANNELS > 0
        FEATURE_FLAG("timer")
#endif
#if MAX_TRACKS > 0
        FEATURE_FLAG("tracks")
#endif
#if USB_SERIAL_SUPPORT > 0
        FEATURE_FLAG("usb")
#endif
#if WIFI_SUPPORT > 0
        FEATURE_FLAG("wifi")
#endif
        NULL, /* Always NULL terminated! */
};

const char** flags_get_features()
{
        return feature_flags;
}
