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
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_rcc.h"
#include "watchdog_device.h"

#define LSI_FREQUENCY 32000

inline void watchdog_device_reset()
{
    IWDG_ReloadCounter();
}

void watchdog_device_init(int timeoutMs)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(timeoutMs);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

int watchdog_device_is_watchdog_reset()
{
    return (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) ? 1 : 0;
}

int watchdog_device_is_poweron_reset()
{
    return (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) ? 1 : 0;
}
