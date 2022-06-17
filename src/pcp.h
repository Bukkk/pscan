#pragma once

#include <threads.h>
#include <stdbool.h>

typedef struct {
    void* container;
    bool (*is_full)(void* container);
    bool (*is_empty)(void* container);
    void (*push)(void* container, void* elem);
    void* (*pop)(void* container);
} ContainerAbstraction;

typedef struct {
    mtx_t mutex;
    cnd_t can_consume;
    cnd_t can_produce;
} Pcp;


Pcp* pcp_create();
