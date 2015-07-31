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
bool has_data(struct ring_buff *rb);

#endif /* __RING_BUFFER_H__ */
