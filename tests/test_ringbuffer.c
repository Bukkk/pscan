#include "ring_buffer.h"
#include "ring_buffer.c"

#include <assert.h>
#include <stdint.h>

int main()
{
    RingBuffer* rb = rb_create(5, sizeof(uint32_t));

    assert(rb_is_empty(rb) == true);
    assert(rb_is_full(rb) == false);

    uint32_t k = 1;
    rb_add(rb, &k);

    assert(rb_is_empty(rb) == false);
    assert(rb_is_full(rb) == false);
    assert(*(uint32_t*)rb_get(rb) == k);

    rb_remove(rb);
    assert(rb_is_empty(rb) == true);
    assert(rb_is_full(rb) == false);

    uint32_t kk[5] = {11, 22, 33, 44, 55};
    rb_add(rb, &kk[0]);
    rb_add(rb, &kk[1]);
    rb_add(rb, &kk[2]);
    rb_add(rb, &kk[3]);
    rb_add(rb, &kk[4]);

    assert(rb_is_empty(rb) == false);
    assert(rb_is_full(rb) == true);

    rb_destroy(rb);
}