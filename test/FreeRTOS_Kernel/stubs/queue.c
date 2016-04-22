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


#include "FreeRTOS.h"
#include "queue.h"

signed portBASE_TYPE xQueueGenericSend( xQueueHandle xQueue,
                                        const void * const pvItemToQueue,
                                        portTickType xTicksToWait,
                                        portBASE_TYPE xCopyPosition )
{
        return 0;
}

signed portBASE_TYPE xQueueGenericReceive(
        xQueueHandle pxQueue, void *
        const pvBuffer, portTickType xTicksToWait,
        portBASE_TYPE xJustPeeking )
{
        return 0;
}


xQueueHandle xQueueCreate(
        unsigned portBASE_TYPE uxQueueLength,
        unsigned portBASE_TYPE uxItemSize)
{
        return NULL;
}

xQueueHandle xQueueCreateMutex()
{
        /* Can't return NULL b/c failure checks.  Wing it */
        return (xQueueHandle) 1;
}
