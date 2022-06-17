#pragma once

#include <threads.h>
#include <stdbool.h>

typedef struct {
    mtx_t mutex;
    cnd_t can_consume;
    cnd_t can_produce;

    bool exits;
    char padding[7];
} Pcp;

typedef struct {
    void* container;
    bool (*is_full)(void* container);
    bool (*is_empty)(void* container);
    
} PcpContainerVirt;


Pcp* pcp_create(void);

bool pcp_exits(Pcp* pcp);
void pcp_producer_section_begin(Pcp* pcp, PcpContainerVirt* virt);
void pcp_producer_section_end(Pcp* pcp);


