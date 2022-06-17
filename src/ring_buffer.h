#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct RingBuffer RingBuffer;

RingBuffer* rb_create(size_t size, size_t elem_size);
void rb_destroy(RingBuffer* rb);

bool rb_is_full(const RingBuffer* rb);
bool rb_is_empty(const RingBuffer* rb);

// jako ze maja return void, nie moga sie niepowiesc 
// -> nalezy najpierw sprawdzic czy da sie dodac lub usunac
void rb_add(RingBuffer* rb, void* elem);
void rb_remove(RingBuffer* rb);

// podobnie get nie ma optionala -> if !is_empty get
void* rb_get(RingBuffer* rb);

