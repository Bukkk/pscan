#include "pcp.h"

#include <malloc.h>
#include <stdbool.h>
#include <threads.h>

Pcp* pcp_create()
{
    Pcp* pcp = malloc(sizeof(*pcp));
    if (pcp == NULL) {
        return NULL;
    }

    *pcp = (Pcp) { 0 };

    if (mtx_init(&pcp->mutex, mtx_plain) != thrd_success || cnd_init(&pcp->can_consume) != thrd_success || cnd_init(&pcp->can_produce) != thrd_success) {
        free(pcp);
        return NULL;
    }

    return pcp;
}

bool pcp_section_exits(Pcp* pcp)
{
    if (pcp->exits) {
        mtx_unlock(&pcp->mutex);
        return true;
    }

    return false;
}

void pcp_section_producer_begin(Pcp* pcp, const PcpContainerVirt* virt)
{
    if (pcp_section_exits(pcp)) {
        return;
    }

    mtx_lock(&pcp->mutex);
    if (virt->is_full(virt->container)) {
        cnd_wait(&pcp->can_produce, &pcp->mutex);
    }
}

void pcp_producer_section_end(Pcp* pcp)
{
    cnd_signal(&pcp->can_consume);
    mtx_unlock(&pcp->mutex);
}

void pcp_section_consumer_begin(Pcp* pcp, const PcpContainerVirt* virt)
{
    if (pcp_section_exits(pcp)) {
        return;
    }
    
    mtx_lock(&pcp->mutex);
    if (virt->is_empty(virt->container)) {
        cnd_wait(&pcp->can_consume, &pcp->mutex);
    }
}

void pcp_consumer_section_end(Pcp* pcp)
{
    cnd_signal(&pcp->can_produce);
    mtx_unlock(&pcp->mutex);
}

void pcp_stop(Pcp* pcp)
{
    mtx_lock(&pcp->mutex);
    pcp->exits = true;
    cnd_signal(&pcp->can_produce);
    cnd_signal(&pcp->can_consume);
    mtx_unlock(&pcp->mutex);
}
