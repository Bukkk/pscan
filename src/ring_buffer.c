#include "ring_buffer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct RingBuffer {
    size_t capacity;
    size_t elem_size;

    size_t size;

    size_t take;
    size_t put;

    char data[];
};

static size_t rb_sizeof(size_t elem_size, size_t size)
{
    return sizeof(RingBuffer) + sizeof(char) * elem_size * size;
}

static void* rb_data_at_ind(RingBuffer* rb, size_t ind)
{
    return rb->data + rb->elem_size * ind;
}

RingBuffer* rb_create(size_t capacity, size_t elem_size)
{
    RingBuffer* ptr = malloc(rb_sizeof(elem_size, capacity));
    if (ptr == NULL) {
        return NULL;
    }

    *ptr = (RingBuffer) {
        .capacity = capacity,
        .elem_size = elem_size
    };

    memset(ptr->data, 0, elem_size * capacity);

    return ptr;
}

void rb_destroy(RingBuffer* rb)
{
    free(rb);
}

bool rb_is_full(const RingBuffer* rb)
{
    return !(rb->size < rb->capacity);
}

bool rb_is_empty(const RingBuffer* rb)
{
    return rb->size == 0;
}

void rb_add(RingBuffer* rb, void* elem)
{
    memcpy(rb_data_at_ind(rb, rb->put), elem, rb->elem_size);
    rb->size++;
    rb->put++;
    if (rb->put == rb->capacity) {
        rb->put = 0;
    }
}

void* rb_get(RingBuffer* rb)
{
    return rb_data_at_ind(rb, rb->take);
}

void rb_remove(RingBuffer* rb)
{
    rb->size--;
    rb->take++;
    if (rb->take == rb->capacity) {
        rb->take = 0;
    }
}
