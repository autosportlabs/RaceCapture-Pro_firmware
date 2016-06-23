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

#include "macros.h"
#include <string.h>
#include "led.h"
#include "led_device.h"

/*
 * LED names are the same strings as the enums in code.  These are the
 * names used by certain APIs to refer to the LEDs.  DO NOT CHANGE
 * THEM.
 */
#define LED_NAME_ENTRY(str, enm)	{.name = str, .led = enm}
static struct led_names {
        const char *name;
        const enum led led;
} led_names[] = {
        LED_NAME_ENTRY("error", LED_ERROR),
        LED_NAME_ENTRY("logger", LED_LOGGER),
        LED_NAME_ENTRY("gps", LED_GPS),
        LED_NAME_ENTRY("telemetry", LED_TELEMETRY),
};

bool led_init(void)
{
        return led_device_init();
}

bool led_set_index(const size_t i, const bool on)
{
        return led_device_set_index(i, on);
}

bool led_set(const enum led l, const bool on)
{
        return led_device_set(l, on);
}

bool led_enable(const enum led l)
{
        return led_set(l, true);
}

bool led_disable(const enum led l)
{
        return led_set(l, false);
}

bool led_toggle(const enum led l)
{
        return led_device_toggle(l);
}

enum led get_led_enum(const char *name)
{
        if (NULL == name)
                return LED_UNKNOWN;

        for (size_t i = 0; i < ARRAY_LEN(led_names); ++i)
                if (0 == strcmp(led_names[i].name, name))
                        return led_names[i].led;

        return LED_UNKNOWN;
}
