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

/**
 * This is a thread-safe wrapper method around our ring_buff
 * object.  This gets us thread safety in our ring buff object
 * as needed without implementing new code.
 */

#include "FreeRTOS.h"
#include "mem_mang.h"
#include "ring_buffer.h"
#include "ts_ring_buff.h"
#include "semphr.h"
#include <stdbool.h>
#include <stddef.h>

struct ts_ring_buff {
        struct ring_buff *rb;
        xSemaphoreHandle mutex;
};

static void take_mutex(struct ts_ring_buff *tsrb)
{
        xSemaphoreTake(tsrb->mutex, portMAX_DELAY);
}

static void give_mutex(struct ts_ring_buff *tsrb)
{
        xSemaphoreGive(tsrb->mutex);
}

void ts_ring_buff_clear(struct ts_ring_buff *tsrb)
{
        take_mutex(tsrb);
        ring_buffer_clear(tsrb->rb);
        give_mutex(tsrb);
}

struct ts_ring_buff* ts_ring_buff_create(const size_t cap)
{
        struct ts_ring_buff *tsrb = portMalloc(sizeof(struct ts_ring_buff));
        if (!tsrb)
                return NULL;

        tsrb->rb = ring_buffer_create(cap);
        if (!tsrb->rb)
                goto fail_clean_obj;

        tsrb->mutex = xSemaphoreCreateMutex();
        if (!tsrb->mutex)
                goto fail_clean_obj;

        return tsrb;

        /* Failure code here to avoid duplication of code */
fail_clean_obj:
        portFree(tsrb);
        return NULL;
}

size_t ts_ring_buff_capacity(struct ts_ring_buff *tsrb)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_capacity(tsrb->rb);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_bytes_free(struct ts_ring_buff *tsrb)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_bytes_free(tsrb->rb);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_bytes_used(struct ts_ring_buff *tsrb)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_bytes_used(tsrb->rb);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_peek(struct ts_ring_buff *tsrb, void *buff,
                         size_t size)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_peek(tsrb->rb, buff, size);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_get(struct ts_ring_buff *tsrb, void *buff,
                        const size_t size)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_get(tsrb->rb, buff, size);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_put(struct ts_ring_buff *tsrb, const void *data,
                        size_t size)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_put(tsrb->rb, data, size);
        give_mutex(tsrb);
        return rv;
}

size_t ts_ring_buff_write(struct ts_ring_buff *tsrb, const void *data,
                          size_t size)
{
        take_mutex(tsrb);
        const size_t rv = ring_buffer_write(tsrb->rb, data, size);
        give_mutex(tsrb);
        return rv;
}
