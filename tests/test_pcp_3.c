#include "pcp.c"
#include "pcp.h"

#include <assert.h>
#include <threads.h>
#include <stdio.h>

static bool stub_is_empty(int* k)
{
    if (*k <= 0) {
        return true;
    }
    return false;
}

static bool stub_is_full(int* k)
{
    if (*k >= 10) {
        return true;
    }
    return false;
}

static void stub_container_add(int* k, char s[])
{
    ++*k;
    printf("container::add -> %s \"%u\"\n", s, *k);
}

static void stub_container_get(int* k, char s[])
{
    --*k;
    printf("container::get -> %s \"%u\"\n", s, *k);
}

int g_k;
int g_kk;

Pcp g_pcp_k;
Pcp g_pcp_kk;

static int thread_producer(void* arg)
{
    (void)arg;
    PcpContainerVirt virt = {
        .container = &g_k,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    while (true) {
        pcp_section_producer_begin(&g_pcp_k, &virt);
        if (pcp_section_exits(&g_pcp_k)) {
            break;
        }

        assert(!virt.is_full(virt.container));
        stub_container_add(&g_k, "producer");

        pcp_section_producer_end(&g_pcp_k);
    }

    return 0;
}

static int thread_middleman(void* arg)
{
    (void)arg;
    PcpContainerVirt virt_k = {
        .container = &g_k,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    PcpContainerVirt virt_kk = {
        .container = &g_kk,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    while (true) {
        pcp_section_consumer_begin(&g_pcp_k, &virt_k);
        if (pcp_section_exits(&g_pcp_k)) {
            break;
        }

        assert(!virt_k.is_empty(virt_k.container));
        stub_container_get(&g_k, "middleman");

        pcp_section_consumer_end(&g_pcp_k);

        pcp_section_producer_begin(&g_pcp_kk, &virt_kk);
        if (pcp_section_exits(&g_pcp_kk)) {
            break;
        }

        assert(!virt_kk.is_full(virt_kk.container));
        stub_container_add(&g_kk, "middleman");

        pcp_section_producer_end(&g_pcp_kk);
    }

    return 0;
}

static int thread_consumer(void* arg)
{
    (void)arg;
    PcpContainerVirt virt = {
        .container = &g_kk,
        .is_empty = (bool (*)(const void*))stub_is_empty,
        .is_full = (bool (*)(const void*))stub_is_full
    };

    while (true) {
        pcp_section_consumer_begin(&g_pcp_kk, &virt);
        if (pcp_section_exits(&g_pcp_kk)) {
            break;
        }

        assert(!virt.is_empty(virt.container));
        stub_container_get(&g_kk, "consumer");

        pcp_section_consumer_end(&g_pcp_kk);
    }

    return 0;
}

int main()
{
    pcp_init(&g_pcp_k);
    pcp_init(&g_pcp_kk);

    thrd_t producer;
    thrd_create(&producer, thread_producer, NULL);

    thrd_t middleman;
    thrd_create(&middleman, thread_middleman, NULL);

    thrd_t consumer;
    thrd_create(&consumer, thread_consumer, NULL);

    thrd_join(producer, NULL);
    thrd_join(middleman, NULL);
    thrd_join(consumer, NULL);

    return 0;
}
