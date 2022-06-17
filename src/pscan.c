#include <stdio.h>
#include <threads.h>

#include "reader.h"
#include "pcp.h"

void pcp_stub_(void)
{
    printf("container::create\n");
}

int thread_reader(void* arg)
{
    Pcp* pcp = *(Pcp**)arg;

    while (true) {
        mtx_lock(&pcp->mutex);
        if (pcp->abstr.is_full(pcp->abstr.container)) {
            cnd_wait(&pcp->can_produce, &pcp->mutex);
        }

        Str proc_stat = read_file("/proc/stat");
        pcp->abstr.push(pcp->abstr.container, &proc_stat);

        cnd_signal(&pcp->can_consume);
        mtx_unlock(&pcp->mutex);
    }

    return 0;
}

int main(void) {

    Pcp* pcp = pcp_create();

    thrd_t reader;
    thrd_create(&reader, thread_reader, &pcp);

    thrd_t consumer;
    thrd_create(&consumer, thread_consumer, &pcp);

    thrd_join(reader, NULL);
    thrd_join(consumer, NULL);

    return 0;
}
