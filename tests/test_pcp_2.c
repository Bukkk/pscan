#include "pcp.c"
#include "pcp.h"

#include <assert.h>
#include <threads.h>
#include <stdio.h>

static bool stub_is_empty(int* k)
{
    printf("container::is_empty -> %u\n", *k);

    if (*k <= 0) {
        return true;
    }
    return false;
}

static bool stub_is_full(int* k)
{
    printf("container::is_full -> %u\n", *k);

    if (*k >= 10) {
        return true;
    }
    return false;
}

static void stub_container_add(int* k)
{
    ++*k;
    printf("container::add -> \"%u\"\n", *k);
}

static void stub_container_get(int* k)
{
    --*k;
    printf("container::get ->  \"%u\"\n", *k);
}

int g_k;

static int thread_producer(void* arg)
{
    Pcp* pcp = (Pcp*)arg;
    PcpContainerVirt virt = {
        .container = &g_k,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    while (true) {
        pcp_section_producer_begin(pcp, &virt);
        if (pcp_section_exits(pcp)) {
            break;
        }

        assert(!virt.is_full(virt.container));
        stub_container_add(&g_k);

        pcp_section_producer_end(pcp);
    }

    return 0;
}

static int thread_consumer(void* arg)
{
    Pcp* pcp = (Pcp*)arg;
    PcpContainerVirt virt = {
        .container = &g_k,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    while (true) {
        pcp_section_consumer_begin(pcp, &virt);
        if (pcp_section_exits(pcp)) {
            break;
        }

        assert(!virt.is_empty(virt.container));
        stub_container_get(&g_k);

        pcp_section_consumer_end(pcp);
    }

    return 0;
}

int main()
{
    Pcp pcp = {0};
    pcp_init(&pcp);

    thrd_t producer;
    thrd_create(&producer, thread_producer, &pcp);

    thrd_t consumer;
    thrd_create(&consumer, thread_consumer, &pcp);

    thrd_join(producer, NULL);
    thrd_join(consumer, NULL);

    return 0;
}
