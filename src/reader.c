#include "reader.h"

#include "str.h"

#include <stdbool.h>
#include <stdio.h>

Str read_file(const char path[const])
{
    enum { TRY_SIZE = 8196 }; // ponad 4k na moim cpu
    Str string = str_create(TRY_SIZE);
    if (string.data == NULL) {
        return string;
    }

    FILE* file = fopen(path, "rb");

    bool can_stop = false;
    size_t offset = 0;
    while (!can_stop) {

        size_t count = fread(string.data + offset, sizeof(string.data[0]), TRY_SIZE, file);
        string.size += count;

        if (count < TRY_SIZE) {
            can_stop = true;
        } else {
            offset += TRY_SIZE;

            bool succ = str_resize(&string, string.capacity + TRY_SIZE);
            if (!succ) {
                can_stop = true;
            }
        }
    }

    fclose(file);
    return string;
}
