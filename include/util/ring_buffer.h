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

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "cpp_guard.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

struct ring_buff;

void ring_buffer_destroy(struct ring_buff *rb);
struct ring_buff* ring_buffer_create(const size_t cap);
void ring_buffer_clear(struct ring_buff *rb);
size_t ring_buffer_capacity(struct ring_buff *rb);
size_t ring_buffer_bytes_free(struct ring_buff *rb);
size_t ring_buffer_bytes_used(struct ring_buff *rb);
size_t ring_buffer_get(struct ring_buff *rb, void *buff,
                       size_t size);
size_t ring_buffer_peek(struct ring_buff *rb, void *data,
                        size_t size);
size_t ring_buffer_put(struct ring_buff *rb, const void *data,
                       size_t size);
size_t ring_buffer_write(struct ring_buff *rb, const void *data,
                         size_t size);
const void* ring_buffer_dma_read_init(struct ring_buff* rb, size_t* avail);
void ring_buffer_dma_read_fini(struct ring_buff* rb, const size_t read);

CPP_GUARD_END

#endif /* __RING_BUFFER_H__ */
