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

#include "led.h"
#include "stm32f30x_iwdg.h"
#include "stm32f30x_rcc.h"
#include "watchdog_device.h"

#define LSI_FREQUENCY 42000

static int flags;

inline void watchdog_device_reset()
{
        IWDG_ReloadCounter();
}

void watchdog_device_init(int timeoutMs)
{
        /*
         * Using a pre-scaler (divider) of 64.  This means with a 12
         * bit counter we can support up to about 6 seconds for a
         * watchdog timer length.  Factor that into reload val
         */
        const int rVal = (LSI_FREQUENCY * timeoutMs) / (64 * 1000);

        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
        IWDG_SetPrescaler(IWDG_Prescaler_64);
        IWDG_SetReload(rVal);
        IWDG_ReloadCounter();
        IWDG_Enable();
}

/*
 * FIXME: It would seem that the IWDGRST and PORRST flags are being
 * set during a watchdog reset event and a poweron event.  This is
 * not what we want... but I'm not going to fix this now.  Watchdog
 * works and that is what matters.
 */
enum rcc_flag {
        STATUS_FLAG,
        WATCHDOG_FLAG,
        POWERON_FLAG
};

static int read_flags(enum rcc_flag flag) {
        if (flags || STATUS_FLAG == flag)
                return flags & 1 << STATUS_FLAG;

        flags |= 1 << STATUS_FLAG;
        flags |= RCC_GetFlagStatus(RCC_FLAG_IWDGRST) << WATCHDOG_FLAG;
        flags |= RCC_GetFlagStatus(RCC_FLAG_PORRST) << POWERON_FLAG;

        RCC_ClearFlag();
        return read_flags(flag);
}

int watchdog_device_is_watchdog_reset()
{
        return read_flags(WATCHDOG_FLAG);
}

int watchdog_device_is_poweron_reset()
{
        return read_flags(POWERON_FLAG);
}
