#include "pcp.h"

#include <malloc.h>

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
