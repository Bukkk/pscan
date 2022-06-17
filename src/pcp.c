#include "pcp.h"

#include <malloc.h>
#include <stdbool.h>

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

bool pcp_exits(const Pcp* pcp)
{
    return pcp->exits;
}

void pcp_producer_section_begin(Pcp* pcp, const PcpContainerVirt* virt)
{
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
