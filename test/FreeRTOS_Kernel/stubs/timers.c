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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "StackMacros.h"

xTimerHandle xTimerCreate(const signed char* const pcTimerName,
                          portTickType xTimerPeriodInTicks,
                          unsigned portBASE_TYPE uxAutoReload,
                          void *pvTimerID,
                          tmrTIMER_CALLBACK pxCallbackFunction )
{
        return NULL;
}

portBASE_TYPE xTimerGenericCommand(xTimerHandle xTimer,
                                   portBASE_TYPE xCommandID,
                                   portTickType xOptionalValue,
                                   signed portBASE_TYPE *pxHigherPriorityTaskWoken,
                                   portTickType xBlockTime )
{
        return pdTRUE;
}
