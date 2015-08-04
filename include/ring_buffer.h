/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include <stdbool.h>
#include <stddef.h>

struct ring_buff {
    char *buf;
    size_t size;
    char *head;
    char *tail;
};

/**
 * Copies size bytes from the ring buffer into data.
 * @return The number of bytes copied.
 */
size_t get_data(struct ring_buff *rb, void *data, size_t size);
size_t get_space(struct ring_buff *rb);
size_t get_used(struct ring_buff *rb);
bool have_space(struct ring_buff *rb, size_t size);
size_t put_data(struct ring_buff *rb, const void *data, size_t size);
size_t dump_data(struct ring_buff *rb, size_t size);

/**
 * Clears all data from the ring buffer.  Doesn't actually wipe it,
 * but the data is forgotten about, and that is good enough.
 * @param rb The ring_buff structure.
 * @return Number of bytes cleared.
 */
size_t clear_data(struct ring_buff *rb);

bool has_data(struct ring_buff *rb);

/**
 * Initializes a ring_buff structure with the data given.  Sets all
 * other appropriate variables as needed.
 * @param rb The ring_buff structure to setup.
 * @param buff Pointer to the buffer to use.
 * @param size Size of the buffer
 * @return The size of the buffer.
 */
size_t init_ring_buffer(struct ring_buff *rb, char *buff,
                        const size_t size);

/**
 * Allocates a Ring Buffer and assigns relevant data to the ring_buff
 * structure provided.
 * @param rb The ring_buff structure to populate.
 * @param size The size of the buffer to allocate.
 * @return The size of the buffer allocated.
 */
size_t create_ring_buffer(struct ring_buff *rb, size_t size);

/**
 * Frees a populated ring_buf struct.  Sets all values to 0.
 */
void free_ring_buffer(struct ring_buff *rb);

/**
 * Puts a string onto the ring_buffer.  This method is a bit more
 * optimized than put_data as it avoids an extra iteration over the string
 * @param rb The ring buffer structure we are adding to.
 * @param str The string to append.
 * @return The pointer where we are in the string when the buffer became
 * full, otherwise NULL if successful.
 */
const char * put_string(struct ring_buff *rb, const char *str);

#endif /* __RING_BUFFER_H__ */
