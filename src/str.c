#include "str.h"

#include <malloc.h>
#include <stdbool.h>
#include <string.h>

Str str_create(size_t capacity)
{
    char* ptr = malloc(capacity);
    if (ptr == NULL) {
        return (Str) { 0 };
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

    *str = (Str) { 0 };
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

StrView strv_from_str(Str str)
{
    return (StrView) {
        .data = str.data,
        .size = str.size
    };
}

StrView strv_disjoin(StrView* str, char disjoin_on)
{
    for (size_t i = 0; i < str->size; ++i) {
        char character = str->data[i];
        if (character == disjoin_on) {

            StrView ret = (StrView) {
                .data = str->data,
                .size = i
            };

            if (str->size > i) {
                str->data = &str->data[i + 1];
                str->size -= i + 1;
            } else {
                str->data = NULL;
                str->size = 0;
            }

            return ret;
        }
    }

    return (StrView) { 0 };
}

bool strv_same_until_end(StrView a, StrView b)
{
    size_t size = (a.size > b.size) ? b.size : a.size;

    for(size_t i = 0; i < size; ++i) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }

    return true;
}

StrView strv_from_cstr(const char data[])
{
    size_t size = strlen(data);
    return (StrView) {
        .data = data,
        .size = size
    };
}

