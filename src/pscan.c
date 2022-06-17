#include <stdbool.h>
#include <stdio.h>
#include <threads.h>

#include "analyzer.h"
#include "printer.h"
#include "reader.h"

#include "pcp.h"
#include "ring_buffer.h"
#include "str.h"
#include "string.h"

#include "assert.h"

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
    Pcp* pcp_reader_analyzer;
    PcpContainerVirt* virt;
} ReaderArgs;

static int thread_reader(void* arg)
{
    ReaderArgs* a = (ReaderArgs*)arg;
    PcpContainerVirt* virt = a->virt;

    while (true) {
        Str proc_stat = read_file("/proc/stat");

        pcp_section_producer_begin(a->pcp_reader_analyzer, virt);
        if (pcp_section_exits(a->pcp_reader_analyzer)) {
            break;
        }

        assert(!virt->is_full(virt->container));
        rb_add(virt->container, &proc_stat);

        pcp_section_producer_end(a->pcp_reader_analyzer);
    }

    return 0;
}

typedef struct {
    Pcp* pcp_reader_analyzer;
    Pcp* pcp_analyzer_printer;
    PcpContainerVirt* virt;
} AnalyzerArgs;

static int thread_analyzer(void* arg)
{
    AnalyzerArgs* a = (AnalyzerArgs*)arg;
    PcpContainerVirt* virt = a->virt;

    while (true) {
        pcp_section_consumer_begin(a->pcp_reader_analyzer, virt);
        if (pcp_section_exits(a->pcp_reader_analyzer)) {
            break;
        }

        assert(!virt->is_empty(virt->container));
        Str str = *(Str*)rb_get(virt->container);
        rb_remove(virt->container);

        pcp_section_consumer_end(a->pcp_reader_analyzer);

        AnalyzedData* analyzed = analyze_data(&str);
        str_destroy(&str);

        pcp_section_producer_begin(a->pcp_analyzer_printer, virt);
        if (pcp_section_exits(a->pcp_analyzer_printer)) {
            break;
        }

        assert(!virt->is_full(virt->container));
        rb_add(virt->container, &analyzed);

        pcp_section_producer_end(a->pcp_analyzer_printer);
    }

    return 0;
}

typedef struct {
    Pcp* pcp_analyzer_printer;
    PcpContainerVirt* virt;
} PrinterArgs;

static int thread_printer(void* arg)
{
    PrinterArgs* a = (PrinterArgs*)arg;
    PcpContainerVirt* virt = a->virt;

    while (true) {
        pcp_section_consumer_begin(a->pcp_analyzer_printer, virt);
        if (pcp_section_exits(a->pcp_analyzer_printer)) {
            break;
        }

        assert(!virt->is_empty(virt->container));
        AnalyzedData* analyzed = *(AnalyzedData**)rb_get(virt->container);
        rb_remove(virt->container);

        pcp_section_consumer_end(a->pcp_analyzer_printer);

        if (analyzed != NULL) {
            print_analized_data(analyzed);
            thrd_sleep(&(struct timespec) { .tv_nsec = 1000 * 1000 * 20 }, NULL);
        }
        an_destroy(analyzed);
    }

    return 0;
}

// #include <unistd.h> // do testowania stopu TODO usun

int main(void)
{
    Pcp* pcp_reader_analyzer = pcp_create();
    Pcp* pcp_analyzer_printer = pcp_create();

    RingBuffer* rb_string = rb_create(3, sizeof(Str));
    PcpContainerVirt virt_string = {
        .container = rb_string,
        .is_empty = (bool (*)(const void*))rb_is_empty,
        .is_full = (bool (*)(const void*))rb_is_full
    };

    RingBuffer* rb_analyzed_ptr = rb_create(5, sizeof(AnalyzedData*));
    PcpContainerVirt virt_analyzed_ptr = {
        .container = rb_analyzed_ptr,
        .is_empty = (bool (*)(const void*))rb_is_empty,
        .is_full = (bool (*)(const void*))rb_is_full
    };

    ReaderArgs reader_args = {
        .pcp_reader_analyzer = pcp_reader_analyzer,
        .virt = &virt_string
    };
    AnalyzerArgs analyzer_args = {
        .pcp_reader_analyzer = pcp_reader_analyzer,
        .pcp_analyzer_printer = pcp_analyzer_printer,
        .virt = &virt_string
    };
    PrinterArgs printer_args = {
        .pcp_analyzer_printer = pcp_analyzer_printer,
        .virt = &virt_analyzed_ptr
    };

    thrd_t reader;
    thrd_create(&reader, thread_reader, &reader_args);

    thrd_t consumer;
    thrd_create(&consumer, thread_analyzer, &analyzer_args);

    thrd_t printer;
    thrd_create(&printer, thread_printer, &printer_args);

    thrd_join(reader, NULL);
    thrd_join(consumer, NULL);
    thrd_join(printer, NULL);

    while(!rb_is_empty(rb_string)) {
        Str str = *(Str*)rb_get(rb_string);
        rb_remove(rb_string);
        str_destroy(&str);
    }
    while(!rb_is_empty(rb_analyzed_ptr)) {
        AnalyzedData* data = *(AnalyzedData**)rb_get(rb_analyzed_ptr);
        rb_remove(rb_analyzed_ptr);
        an_destroy(data);
    }

    return 0;
}
