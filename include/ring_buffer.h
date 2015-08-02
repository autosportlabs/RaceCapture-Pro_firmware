/*
 * Race Capture
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

#endif /* __RING_BUFFER_H__ */
