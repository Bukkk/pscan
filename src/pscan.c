#include <stdbool.h>
#include <stdio.h>
#include <threads.h>
#include <time.h>

#include "analyzer.h"
#include "printer.h"
#include "reader.h"

#include "pcp.h"
#include "ring_buffer.h"
#include "signals.h"
#include "str.h"
#include "string.h"

#include "assert.h"

typedef struct {
    Pcp* pcp_reader_analyzer;
    PcpContainerVirt* virt;
} ReaderArgs;

static int thread_reader(void* arg)
{
    ReaderArgs* a = (ReaderArgs*)arg;

    Str proc_stat = { 0 };
    while (true) {
        proc_stat = read_file("/proc/stat");

        pcp_section_producer_begin(a->pcp_reader_analyzer, a->virt);
        if (pcp_section_exits(a->pcp_reader_analyzer)) {
            break;
        }

        assert(!a->virt->is_full(a->virt->container));
        rb_add(a->virt->container, &proc_stat);

        pcp_section_producer_end(a->pcp_reader_analyzer);
    }

    str_destroy(&proc_stat);
    return 0;
}

typedef struct {
    Pcp* pcp_reader_analyzer;
    Pcp* pcp_analyzer_printer;
    PcpContainerVirt* virt_string;
    PcpContainerVirt* virt_analized;
} AnalyzerArgs;

static int thread_analyzer(void* arg)
{
    AnalyzerArgs* a = (AnalyzerArgs*)arg;

    AnalyzedData* analyzed = { 0 };
    while (true) {
        pcp_section_consumer_begin(a->pcp_reader_analyzer, a->virt_string);
        if (pcp_section_exits(a->pcp_reader_analyzer)) {
            break;
        }

        assert(!a->virt_string->is_empty(a->virt_string->container));
        Str str = *(Str*)rb_get(a->virt_string->container);
        rb_remove(a->virt_string->container);

        pcp_section_consumer_end(a->pcp_reader_analyzer);

        analyzed = analyze_data(&str);
        str_destroy(&str);

        pcp_section_producer_begin(a->pcp_analyzer_printer, a->virt_analized);
        if (pcp_section_exits(a->pcp_analyzer_printer)) {
            break;
        }

        assert(!a->virt_analized->is_full(a->virt_analized->container));
        rb_add(a->virt_analized->container, &analyzed);

        pcp_section_producer_end(a->pcp_analyzer_printer);
    }

    an_destroy(analyzed);

    return 0;
}

typedef struct {
    Pcp* pcp_analyzer_printer;
    PcpContainerVirt* virt;
} PrinterArgs;

static int thread_printer(void* arg)
{
    static const time_t sleep_time_ms = 200;

    PrinterArgs* a = (PrinterArgs*)arg;

    while (true) {
        pcp_section_consumer_begin(a->pcp_analyzer_printer, a->virt);
        if (pcp_section_exits(a->pcp_analyzer_printer)) {
            break;
        }

        assert(!a->virt->is_empty(a->virt->container));
        AnalyzedData* analyzed = *(AnalyzedData**)rb_get(a->virt->container);
        rb_remove(a->virt->container);

        pcp_section_consumer_end(a->pcp_analyzer_printer);

        if (analyzed != NULL) {
            print_analized_data(analyzed);
            thrd_sleep(&(struct timespec) { .tv_nsec = 1000 * 1000 * sleep_time_ms }, NULL);
        }
        an_destroy(analyzed);
    }

    return 0;
}

extern Pcp g_pcp_reader_analyzer;
extern Pcp g_pcp_analyzer_printer;
Pcp g_pcp_reader_analyzer;
Pcp g_pcp_analyzer_printer;

int main(void)
{
    signals_init();

    pcp_init(&g_pcp_reader_analyzer);
    pcp_init(&g_pcp_analyzer_printer);

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
        .pcp_reader_analyzer = &g_pcp_reader_analyzer,
        .virt = &virt_string
    };
    AnalyzerArgs analyzer_args = {
        .pcp_reader_analyzer = &g_pcp_reader_analyzer,
        .pcp_analyzer_printer = &g_pcp_analyzer_printer,
        .virt_string = &virt_string,
        .virt_analized = &virt_analyzed_ptr
    };
    PrinterArgs printer_args = {
        .pcp_analyzer_printer = &g_pcp_analyzer_printer,
        .virt = &virt_analyzed_ptr
    };

    thrd_t reader;
    thrd_create(&reader, thread_reader, &reader_args);

    thrd_t analyzer;
    thrd_create(&analyzer, thread_analyzer, &analyzer_args);

    thrd_t printer;
    thrd_create(&printer, thread_printer, &printer_args);

    thrd_join(reader, NULL);
    thrd_join(analyzer, NULL);
    thrd_join(printer, NULL);

    while (!rb_is_empty(rb_string)) {
        Str str = *(Str*)rb_get(rb_string);
        rb_remove(rb_string);
        str_destroy(&str);
    }
    while (!rb_is_empty(rb_analyzed_ptr)) {
        AnalyzedData* data = *(AnalyzedData**)rb_get(rb_analyzed_ptr);
        rb_remove(rb_analyzed_ptr);
        an_destroy(data);
    }
    rb_destroy(rb_string);
    rb_destroy(rb_analyzed_ptr);

    return 0;
}
