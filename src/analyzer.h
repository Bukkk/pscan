#pragma once

#include "str.h"

#include <stddef.h>

typedef struct {
    size_t cores;
    double procent[];
} AnalyzedData;

AnalyzedData* an_create(size_t cores);
void an_destroy(AnalyzedData* an);
double an_get(AnalyzedData* an, size_t cpu_id);
void an_set(AnalyzedData* an, size_t cpu_id, double val);

AnalyzedData* analyze_data(const Str* str);
