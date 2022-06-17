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

typedef struct {
    const char* data;
    size_t size;
} StrView;

StrView strv_from_str(Str str);
StrView strv_from_cstr(const char data[]);

StrView strv_disjoin(StrView* str, char disjoin_on);
bool strv_same_until_end(StrView a, StrView b);
