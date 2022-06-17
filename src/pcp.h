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
    bool (*is_full)(const void* container);
    bool (*is_empty)(const void* container);
    
} PcpContainerVirt;


Pcp* pcp_create(void);
bool pcp_section_exits(Pcp* pcp);
void pcp_stop(Pcp* pcp);

void pcp_section_producer_begin(Pcp* pcp, const PcpContainerVirt* virt);
void pcp_section_producer_end(Pcp* pcp);
void pcp_section_consumer_begin(Pcp* pcp, const PcpContainerVirt* virt);
void pcp_section_consumer_end(Pcp* pcp);


