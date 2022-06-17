#include "printer.h"

#include <stdio.h>

void print_analized_data(AnalyzedData* analyzed)
{
    for (size_t core = 0; core < analyzed->cores; ++core) {
        double usage = analyzed->procent[core];

        printf("core%zu: %lf", core, usage);
    }
}

