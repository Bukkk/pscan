#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} Str;

Str str_create(size_t capacity);
void str_destroy(Str* str);
bool str_resize(Str* str, size_t new_capacity);
