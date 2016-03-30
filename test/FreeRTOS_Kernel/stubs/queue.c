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
#include "mem_mang.h"
#include "queue.h"
#include "ring_buffer.h"

#include <stddef.h>

struct mock_queue {
        size_t item_size;
        struct ring_buff rb;
};

signed portBASE_TYPE xQueueGenericSend(xQueueHandle pxQueue,
                                       const void * const pvBuffer,
                                       portTickType xTicksToWait,
                                       portBASE_TYPE xCopyPosition )
{
        struct mock_queue *mc = pxQueue;
        return !!put_data(&mc->rb, pvBuffer, mc->item_size);
}

signed portBASE_TYPE xQueueGenericReceive(
        xQueueHandle pxQueue, void *
        const pvBuffer, portTickType xTicksToWait,
        portBASE_TYPE xJustPeeking )
{
        struct mock_queue *mc = pxQueue;
        return !!get_data(&mc->rb, pvBuffer, mc->item_size);
}

void vQueueDelete(xQueueHandle pxQueue)
{
        struct mock_queue *mc = pxQueue;
        free_ring_buffer(&mc->rb);
        portFree(mc);
}

xQueueHandle xQueueCreate(unsigned portBASE_TYPE uxQueueLength,
                          unsigned portBASE_TYPE uxItemSize)
{
        struct mock_queue *mc = portMalloc(sizeof(struct mock_queue));
        mc->item_size = (size_t) uxItemSize;
        create_ring_buffer(&mc->rb, uxQueueLength * uxItemSize);
        return mc;
}
