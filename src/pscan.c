#include <stdio.h>
#include <threads.h>

#include "pcp.h"
#include "reader.h"
#include "str.h"
#include "string.h"

static bool stub_is_empty(void* k_)
{
    int k = *(int*)k_;
    printf("container::is_empty -> %u\n", k);

    if (k <= 0) {
        return true;
    }
    return false;
}

static bool stub_is_full(void* k_)
{
    int k = *(int*)k_;
    printf("container::is_full -> %u\n", k);

    if (k >= 10) {
        return true;
    }
    return false;
}

static void stub_container_add(int* k, Str str)
{
    ++*k;
    printf("container::add -> \"%s\"\n", str.data);
}

static Str stub_container_get(int* k)
{
    --*k;
    printf("container::get -> TODO \n");

    return (Str) { 0 };
}

typedef struct {
    Pcp* pcp;
    PcpContainerVirt* virt;
} ReaderArgs;

static int thread_reader(void* arg)
{
    ReaderArgs reader_args = *(ReaderArgs*)arg;
    Pcp* pcp = reader_args.pcp;
    PcpContainerVirt* virt = reader_args.virt;

    while (true) {
        pcp_producer_section_begin(pcp, virt);
        if (pcp_exits(pcp)) {
            mtx_unlock(&pcp->mutex);
            break;
        }

        Str proc_stat = read_file("/proc/stat");
        stub_container_add(virt->container, proc_stat);

        pcp_producer_section_end(pcp);
    }

    return 0;
}

typedef struct {
    Pcp* pcp;
    PcpContainerVirt* virt;
} AnalyzerArgs;

static int thread_analyzer(void* arg)
{
    AnalyzerArgs analyzer_args = *(AnalyzerArgs*)arg;
    Pcp* pcp = analyzer_args.pcp;
    PcpContainerVirt* virt = analyzer_args.virt;

    while (true) {
        pcp_consumer_section_begin(pcp, virt);
        if (pcp_exits(pcp)) {
            mtx_unlock(&pcp->mutex);
            break;
        }

        Str str = stub_container_get(virt->container);
        

        pcp_consumer_section_end(pcp);
    }

    return 0;
}

#include <unistd.h> // do testowania stopu TODO usun

int main(void /*int argc, char* argv[]*/)
{

    Pcp* pcp = pcp_create();

    int k = 0;
    PcpContainerVirt virt = {
        .container = &k,
        .is_empty = stub_is_empty,
        .is_full = stub_is_full
    };

    ReaderArgs reader_args = {
        .pcp = pcp,
        .virt = &virt
    };
    AnalyzerArgs analyzer_args = {
        .pcp = pcp,
        .virt = &virt
    };

    thrd_t reader;
    thrd_create(&reader, thread_reader, &reader_args);

    thrd_t consumer;
    thrd_create(&consumer, thread_analyzer, &analyzer_args);

    usleep(20000);
    printf("stop... ");
    pcp_stop(pcp);
    printf("OK\n");

    thrd_join(reader, NULL);
    thrd_join(consumer, NULL);

    return 0;
}
