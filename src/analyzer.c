#include "analyzer.h"
#include "str.h"

#include <malloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static size_t an_sizeof(size_t cores)
{
    return sizeof(AnalyzedData) + cores * sizeof(double);
}

AnalyzedData* an_create(size_t cores)
{
    AnalyzedData* ptr = malloc(an_sizeof(cores));
    if (ptr == NULL) {
        return NULL;
    }

    *ptr = (AnalyzedData) {
        .cores = cores
    };

    memset(ptr->procent, 0, sizeof(double) * cores);

    return ptr;
}

void an_destroy(AnalyzedData* an)
{
    free(an);
}

double an_get(AnalyzedData* an, size_t cpu_id)
{
    return an->procent[cpu_id];
}

void an_set(AnalyzedData* an, size_t cpu_id, double val)
{
    an->procent[cpu_id] = val;
}

typedef struct {
    unsigned int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
} CpuStats;

static size_t calculate_cpu_amount(Str str)
{
    size_t cores = 0;
    {
        StrView rest = strv_from_str(str);
        strv_disjoin(&rest, '\n'); // pomin pierwsza linijke
        while (true) {
            StrView line = strv_disjoin(&rest, '\n');

            StrView cpu_num = strv_disjoin(&line, ' ');
            StrView cpu_cstr = strv_from_cstr("cpu");
            if (cpu_num.size == 0 || !strv_same_until_end(cpu_num, cpu_cstr)) {
                break; // w tej linijce juz nie ma cpu
            }

            ++cores;
        }
    }

    return cores;
}

AnalyzedData* analyze_data(const Str* str)
{
    static size_t cores = 0;
    if (cores == 0) {
        cores = calculate_cpu_amount(*str);
        if (cores > 128) {
            cores = 128;
        }
    }

    static CpuStats prevs[128] = { 0 };
    static bool is_first = true;

    AnalyzedData* an = an_create(cores);
    if (an == NULL || is_first) {
        return NULL;
    }

    StrView rest = strv_from_str(*str);
    strv_disjoin(&rest, '\n'); // pomin pierwsza linijke
    for (size_t id = 0; id < cores; ++id) {
        // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
        StrView line = rest;
        strv_disjoin(&rest, '\n');

        strv_disjoin(&line, ' ');

        CpuStats stats = { 0 };
        sscanf(line.data, " %u %u %u %u %u %u %u %u %u %u",
            &stats.user, &stats.nice, &stats.system, &stats.idle, &stats.iowait, &stats.irq, &stats.softirq, &stats.steal, &stats.guest, &stats.guest_nice);

        unsigned int prev_idle = prevs[id].idle + prevs[id].iowait;
        unsigned int idle = stats.idle + stats.iowait;
        unsigned int prev_non_idle = prevs[id].user + prevs[id].nice + prevs[id].system + prevs[id].irq + prevs[id].softirq + prevs[id].steal;
        unsigned int non_idle = stats.user + stats.nice + stats.system + stats.irq + stats.softirq + stats.steal;

        unsigned int prev_total = prev_idle + prev_non_idle;
        unsigned int total = idle + non_idle;

        unsigned int total_diff = total - prev_total;
        unsigned int idle_diff = idle - prev_idle;

        prevs[id] = stats;

        double proc = ((double)(total_diff - idle_diff)) / total_diff;
        an_set(an, id, proc);
    }

    is_first = false;

    return an;
}
