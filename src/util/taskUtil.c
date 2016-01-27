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


#include "taskUtil.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

size_t getCurrentTicks()
{
    return xTaskGetTickCount();
}

int isTimeoutMs(unsigned int startTicks, unsigned int timeoutMs)
{
    return ((xTaskGetTickCount() - startTicks) * portTICK_RATE_MS) >= timeoutMs;
}

void delayMs(unsigned int delay)
{
    vTaskDelay(msToTicks(delay));
}

void delayTicks(size_t ticks)
{
    vTaskDelay(ticks);
}

size_t msToTicks(size_t ms)
{
    return ms / portTICK_RATE_MS;
}

size_t ticksToMs(size_t ticks)
{
    return ticks * portTICK_RATE_MS;
}

void yield()
{
        taskYIELD();
}
