#include "str.h"

#include <malloc.h>

Str str_create(size_t capacity)
{
    char* ptr = malloc(capacity);
    if (ptr == NULL) {
        return (Str) {0};
    }

    return (Str) {
        .data = ptr,
        .capacity = capacity
    };
}

void str_destroy(Str* str)
{
    if (str->data != NULL) {
        free(str->data);
    }

    *str = (Str) {0};
}

bool str_resize(Str* str, size_t new_capacity)
{
    char* ptr = realloc(str->data, new_capacity);
    if (ptr == NULL) {
        return false;
    }

    str->data = ptr;
    str->capacity = new_capacity;
    return true;
}
