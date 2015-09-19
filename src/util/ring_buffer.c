/**
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

#include "mem_mang.h"
#include "mod_string.h"
#include "ring_buffer.h"

#include <stdbool.h>
#include <stddef.h>

static size_t get_end_dist(struct ring_buff *rb, const char *p)
{
        return rb->buf + rb->size - p;
}

size_t get_space(struct ring_buff *rb)
{
        int diff = rb->tail - rb->head;
        if (diff <= 0)
                diff += rb->size;

        return diff - 1;
}

bool have_space(struct ring_buff *rb, size_t size)
{
        return size <= get_space(rb);
}

size_t put_data(struct ring_buff *rb, const void *data, size_t size)
{
        /* Check if we have the space.  If not, only write what we can */
        size_t dist = get_space(rb);
        if (dist < size)
                size = dist;

        dist = get_end_dist(rb, rb->head);
        if (size < dist) {
                memcpy(rb->head, data, size);
                rb->head += size;
        } else {
                size_t wrap_size = size - dist;
                memcpy(rb->head, data, dist);
                memcpy(rb->buf, dist + (char *)data, wrap_size);
                rb->head = rb->buf + wrap_size;
        }

        return size;
}

const char * put_string(struct ring_buff *rb, const char *str)
{
        if (!str)
                return NULL;

        while (0 < get_space(rb) && *str) {
                *rb->head = *str;

                rb->head += 1;
                if (0 == get_end_dist(rb, rb->head))
                        rb->head = rb->buf;
                ++str;
        }

        return *str ? str : NULL;
}


size_t get_used(struct ring_buff *rb)
{
        return rb->size - get_space(rb) - 1;
}

size_t get_data(struct ring_buff *rb, void *data, size_t size)
{
        size_t dist = get_used(rb);
        if (size > dist)
                size = dist;

        dist = get_end_dist(rb, rb->tail);
        if (size < dist) {
                memcpy(data, rb->tail, size);
                rb->tail += size;
        } else {
                size_t wrap_size = size - dist;
                memcpy(data, rb->tail, dist);
                memcpy((char *)data + dist, rb->buf, wrap_size);
                rb->tail = rb->buf + wrap_size;
        }

        return size;
}

size_t dump_data(struct ring_buff *rb, size_t size)
{
        const size_t used = get_used(rb);
        if (size > used)
                size = used;

        const size_t dist = get_end_dist(rb, rb->tail);
        if (size < dist) {
                rb->tail += size;
        } else {
                rb->tail = rb->buf + size - dist;
        }

        return size;
}

size_t clear_data(struct ring_buff *rb)
{
        return dump_data(rb, rb->size);
}

bool has_data(struct ring_buff *rb)
{
        return rb->head != rb->tail;
}

size_t init_ring_buffer(struct ring_buff *rb, char *buff,
                        const size_t size)
{
        rb->head = rb->tail = rb->buf = buff;
        rb->size = size;
        return size - 1;
}

size_t create_ring_buffer(struct ring_buff *rb, size_t size)
{
        char *buff = (char *) portMalloc(size);
        return NULL == buff ? 0 : init_ring_buffer(rb, buff, size);
}

void free_ring_buffer(struct ring_buff *rb)
{
        portFree(rb->buf);
        rb->size = 0;
        rb->buf = rb->head = rb->tail = NULL;
}
