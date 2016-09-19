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


#include "watchdog.h"
#include "watchdog_device.h"
#include "printk.h"

#define LOG_PFX	"[watchdog] "

void watchdog_reset(void)
{
    watchdog_device_reset();
}

void watchdog_init(int timeoutMs)
{
	pr_info(LOG_PFX "Initializing...\r\n");
	watchdog_device_init(timeoutMs);

	if (watchdog_is_watchdog_reset())
		pr_warning(LOG_PFX "Detected watchdog reset!\r\n");
}

bool watchdog_is_watchdog_reset(void)
{
    return watchdog_device_is_watchdog_reset();
}

bool watchdog_is_poweron_reset(void)
{
    return watchdog_device_is_poweron_reset();
}
