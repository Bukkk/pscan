#include <stdio.h>
#include <threads.h>

#include "reader.h"
#include "pcp.h"
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
    printf("container::is_empty -> %u\n", k);

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

static int thread_reader(void* arg)
{
    Pcp* pcp = *(Pcp**)arg;

    int k = 0;
    PcpContainerVirt virt = {
        .container = &k,
        .is_empty = stub_is_empty,
        .is_full = stub_is_full
    };

    while (true) {
        pcp_producer_section_begin(pcp, &virt);
        if (pcp_exits(pcp)){
            break;
        }

        Str proc_stat = read_file("/proc/stat");
        stub_container_add(virt.container, proc_stat);

        pcp_producer_section_end(pcp);
    }

    return 0;
}

int main(void/*int argc, char* argv[]*/) {

    Pcp* pcp = pcp_create();

    thrd_t reader;
    thrd_create(&reader, thread_reader, &pcp);

    // thrd_t consumer;
    // thrd_create(&consumer, thread_consumer, &pcp);

    thrd_join(reader, NULL);
    // thrd_join(consumer, NULL);

    return 0;
}


