/*
 * Race Capture
 */

#include <stdbool.h>
#include <stddef.h>

#include <mod_string.h>
#include <ring_buffer.h>

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
        // Check if we have the space.  If not, only write what we can
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

size_t dump_data(struct ring_buff *rb, size_t size) {

        size_t dist = get_used(rb);
        if (size > dist)
                size = dist;

        dist = get_end_dist(rb, rb->tail);
        if (size < dist) {
                rb->tail += size;
        } else {
                rb->tail = rb->buf + size - dist;
        }

        return size;
}

bool has_data(struct ring_buff *rb) {
        return rb->head != rb->tail;
}
