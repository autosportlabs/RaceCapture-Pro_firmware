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

#ifndef __TS_RING_BUFFER_H__
#define __TS_RING_BUFFER_H__

#include "cpp_guard.h"

#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

struct ts_ring_buff;

struct ts_ring_buff* ts_ring_buff_create(const size_t cap);
void ts_ring_buff_clear(struct ts_ring_buff *rb);
size_t ts_ring_buff_capacity(struct ts_ring_buff *rb);
size_t ts_ring_buff_bytes_free(struct ts_ring_buff *rb);
size_t ts_ring_buff_bytes_used(struct ts_ring_buff *rb);
size_t ts_ring_buff_get(struct ts_ring_buff *rb, void *buff,
                        size_t size);
size_t ts_ring_buff_peek(struct ts_ring_buff *rb, void *data,
                         size_t size);
size_t ts_ring_buff_put(struct ts_ring_buff *rb, const void *data,
                        size_t size);
size_t ts_ring_buff_write(struct ts_ring_buff *rb, const void *data,
                          size_t size);

CPP_GUARD_END

#endif /* __TS_RING_BUFFER_H__ */
