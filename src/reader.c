#include "reader.h"

#include "str.h"

#include <stdbool.h>
#include <stdio.h>

Str read_file(const char path[const])
{
    static const size_t try_size = 1024 << 4; // ponad 4k na moim cpu
    
    Str string = str_create(try_size);
    if (string.data == NULL) {
        return string;
    }

    FILE* file = fopen(path, "rb");

    bool can_stop = false;
    size_t offset = 0;
    while (!can_stop) {

        size_t count = fread(string.data + offset, sizeof(string.data[0]), try_size, file);
        string.size += count;

        if (count < try_size) {
            can_stop = true;
        } else {
            offset += try_size;

            bool succ = str_resize(&string, string.capacity + try_size);
            if (!succ) {
                can_stop = true;
            }
        }
    }

    fclose(file);
    return string;
}
