#include <stdio.h>
#include <threads.h>

#include "analyzer.h"
#include "pcp.h"
#include "reader.h"
#include "ring_buffer.h"
#include "str.h"
#include "string.h"

// static bool stub_is_empty(void* k_)
// {
//     int k = *(int*)k_;
//     printf("container::is_empty -> %u\n", k);

//     if (k <= 0) {
//         return true;
//     }
//     return false;
// }

// static bool stub_is_full(void* k_)
// {
//     int k = *(int*)k_;
//     printf("container::is_full -> %u\n", k);

//     if (k >= 10) {
//         return true;
//     }
//     return false;
// }

// static void stub_container_add(int* k, Str str)
// {
//     ++*k;
//     printf("container::add -> \"%s\"\n", str.data);
// }

// static Str stub_container_get(int* k)
// {
//     --*k;
//     printf("container::get -> TODO \n");

//     return (Str) { 0 };
// }

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
        pcp_section_producer_begin(pcp, virt);
        if (pcp_section_exits(pcp)) {
            break;
        }

        Str proc_stat = read_file("/proc/stat");
        rb_add(virt->container, &proc_stat);

        pcp_section_producer_end(pcp);
    }

    return 0;
}

typedef struct {
    Pcp* pcp_reader_analyzer;
    PcpContainerVirt* virt;
} AnalyzerArgs;

static int thread_analyzer(void* arg)
{
    AnalyzerArgs analyzer_args = *(AnalyzerArgs*)arg;
    Pcp* pcp_reader_analyzer = analyzer_args.pcp_reader_analyzer;
    PcpContainerVirt* virt = analyzer_args.virt;

    while (true) {
        pcp_section_consumer_begin(pcp_reader_analyzer, virt);
        if (pcp_section_exits(pcp_reader_analyzer)) {
            break;
        }

        Str str = *(Str*)rb_get(virt->container);
        rb_remove(virt->container);

        pcp_section_consumer_end(pcp_reader_analyzer);

        AnalyzedData* analyzed = analyze_data(&str);
        str_destroy(&str);
        an_destroy(analyzed);
    }

    return 0;
}

// #include <unistd.h> // do testowania stopu TODO usun

int main(void /*int argc, char* argv[]*/)
{

    Pcp* pcp = pcp_create();

    RingBuffer* rb = rb_create(5, sizeof(Str));
    PcpContainerVirt virt = {
        .container = rb,
        .is_empty = (bool (*)(const void*))rb_is_empty,
        .is_full = (bool (*)(const void*))rb_is_full
    };

    ReaderArgs reader_args = {
        .pcp = pcp,
        .virt = &virt
    };
    AnalyzerArgs analyzer_args = {
        .pcp_reader_analyzer = pcp,
        .virt = &virt
    };

    thrd_t reader;
    thrd_create(&reader, thread_reader, &reader_args);

    thrd_t consumer;
    thrd_create(&consumer, thread_analyzer, &analyzer_args);

    // usleep(1000000);
    // printf("stop... ");
    // pcp_stop(pcp);
    // printf("OK\n");

    thrd_join(reader, NULL);
    thrd_join(consumer, NULL);

    return 0;
}
