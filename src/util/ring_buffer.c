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
 * This ring buffer is designed to be as optimal as possible for the
 * FreeRTOS environment.  Using memcpy allows underlying layer to use
 * native register copying to speed up operations.  Also allowing the
 * user to put in more data than free space is available allows for
 * maximum flexibility at the cost of inheriently being SPSC threadsafe.
 * The buffer still meets that requirement if the user DOES NOT put in
 * more data than the buffer has free space available.  That choice is
 * intentionally left to the user to decide.
 */

#include "macros.h"
#include "mem_mang.h"
#include "ring_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct ring_buff {
        char *buff;
        size_t size;
        char *head;
        char *tail;
};

/**
 * Destroys the given buffer.  Note that the given pointer will no longer
 * be valid.
 */
void ring_buffer_destroy(struct ring_buff *rb)
{
        if (rb->buff)
                portFree(rb->buff);

        portFree(rb);
}

/**
 * Removes all data from the buffer.
 */
void ring_buffer_clear(struct ring_buff *rb)
{
        rb->tail = rb->head = rb->buff;
}

/**
 * Creates a new ring buffer.
 * @param cap The capacity of the ring buffer.  Note that this method
 * will always allocate 1 extra byte due to limits of ring buffers.
 * @return An opaque pointer to the ring buffer struct.
 */
struct ring_buff* ring_buffer_create(const size_t cap)
{
        struct ring_buff *rb = portMalloc(sizeof(struct ring_buff));
        if (!rb)
                return NULL;

        rb->size = cap + 1;
        rb->buff = portMalloc(rb->size);
        if (!rb->buff) {
                ring_buffer_destroy(rb);
                return NULL;
        }

        ring_buffer_clear(rb);
        return rb;
}

size_t ring_buffer_capacity(struct ring_buff *rb)
{
        return rb->size - 1;
}

size_t ring_buffer_bytes_free(struct ring_buff *rb)
{
        if (rb->tail <= rb->head)
                return ring_buffer_capacity(rb) - (rb->head - rb->tail);
        else
                return rb->tail - rb->head - 1;
}

size_t ring_buffer_bytes_used(struct ring_buff *rb)
{
        return ring_buffer_capacity(rb) - ring_buffer_bytes_free(rb);
}

/**
 * Calculates the new pointer value for a position within the ring
 * buffer.  Is able to handle offset values larger than buffer size
 * so we can do relative offsets if needed.
 * @param ptr The current pointer value.
 * @param offset Number of spots forward you want to off-set it.
 * @return The new pointer value, wrapped if needed.
 */
static char* get_new_ptr_val(struct ring_buff *rb, void *ptr,
                             const size_t offset)
{
        return rb->buff + ((char*) ptr - rb->buff + offset) % rb->size;
}

/**
 * Read up to size bytes from the ring buffer and puts the data into
 * the given buffer without modifying the tail pointer.
 * @param buff Buffer to put the data copied out.  If NULL, then no
 * data copying will happen but the amount of bytes that would have been
 * copied is returned.
 * @param size The maximum amount of data to copy out.  Note that if
 * the data is a string it will not be NULL terminated.  Caller is
 * responsible for that.
 * @return The amount of the data that was (or would have been if
 * buffer was not NULL) copied out.
 */
size_t ring_buffer_peek(struct ring_buff *rb, void *buffer,
                        size_t size)
{
        const size_t used = ring_buffer_bytes_used(rb);
        if (used < size)
                size = used;

        if (!buffer)
                return size;

        const size_t dist = rb->size + rb->buff - rb->tail;
        if (size < dist) {
                memcpy(buffer, rb->tail, size);
        } else {
                void *offset = (char*) buffer + dist;
                memcpy(buffer, rb->tail, dist);
                memcpy(offset, rb->buff, size - dist);
        }

        return size;
}

/**
 * Gets up to size bytes from the ring buffer and puts the data into
 * the given buffer.
 * @param buff Buffer to put the data copied out.  If NULL, the data is
 * simply discarded from the buffer.
 * @param size The maximum amount of data to copy out.  Note that if
 * the data is a string it will not be NULL terminated.  Caller is
 * responsible for that.
 */
size_t ring_buffer_get(struct ring_buff *rb, void *buff,
                       const size_t size)
{
        const size_t bytes = ring_buffer_peek(rb, buff, size);
        rb->tail = get_new_ptr_val(rb, rb->tail, bytes);
        return bytes;
}

/**
 * Puts data into the ring buffer.  If there isn't enough free space
 * then the ring buffer will drop the data in FIFO fashion.  If you
 * attempt to put in more data than the ring buffer has capacity, it
 * will treat the data in FIFO fashion and will keep the latter portion
 * of the data given up to its capacity.
 * @param data The data to put into the buffer.
 * @param size The amount of data to put in from the buffer.
 */
size_t ring_buffer_put(struct ring_buff *rb, const void *data,
                       size_t size)
{
        /* If buffer size > capacity of ring buffer */
        const size_t cap = ring_buffer_capacity(rb);
        if (size > cap) {
                data += size - cap;
                size = cap;
        }

        /* If data is getting overwritten, update tail. */
        if (size > ring_buffer_bytes_free(rb))
                rb->tail = get_new_ptr_val(rb, rb->tail, size + 1);

        const size_t dist = rb->buff - rb->head + rb->size;
        if (size < dist) {
                memcpy(rb->head, data, size);
        } else {
                void *offset = (char*) data + dist;
                memcpy(rb->head, data, dist);
                memcpy(rb->buff, offset, size - dist);
        }

        rb->head = get_new_ptr_val(rb, rb->head, size);
        return size;
}

/**
 * Writes data into the ring buffer without clobbering.  If there isn't
 * enough free space then this method will only write what it can and
 * will report back what it was able to write.
 * @param data The data to put into the buffer.
 * @param size The amount of data to put in from the buffer.
 * @return The amount of data actually written to the buffer.
 */
size_t ring_buffer_write(struct ring_buff *rb, const void *data,
                         size_t size)
{
        const size_t avail = ring_buffer_bytes_free(rb);
        if (avail < size)
                size = avail;

        return size ? ring_buffer_put(rb, data, size) : 0;
}

/**
 * Enables a form of poor man's DMA access to the ring buffer by
 * exposing the tail pointer and how much space is available until
 * either the end of the buffer or the location of the head pointer.
 * Use this method to figure out where to start the copy operation at
 * and then report how much was actually copied by invoking the
 * #ring_buffer_dma_read_fini command.
 * @param rb Ring buffer structure which has all the state.
 * @param avail Pointer to a size_t variable where we set how much space
 * is available to read.
 * @return pointer on where to start reading.
 */
const void* ring_buffer_dma_read_init(struct ring_buff* rb, size_t* avail)
{
	const size_t used = ring_buffer_bytes_used(rb);
        const size_t dist = rb->size + rb->buff - rb->tail;
	*avail = MIN(used, dist);

	return rb->tail;
}

/**
 * The counterpart of the #ring_buffer_dma_read_init method, this gets called
 * after a buffer DMA operation has completed.  Internally it updates the state
 * of the buffer as if bits were copied out.
 * @param rb Ring buffer structure which has all the state.
 * @param read The amount of data that was read from the buffer.
 */
void ring_buffer_dma_read_fini(struct ring_buff* rb, const size_t read)
{
	rb->tail = get_new_ptr_val(rb, rb->tail, read);
}
