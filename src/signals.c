#include "signals.h"

#include "pcp.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

extern Pcp g_pcp_reader_analyzer;
extern Pcp g_pcp_analyzer_printer;

static void sigterm_handler(int signum)
{
    pcp_stop(&g_pcp_reader_analyzer);
    pcp_stop(&g_pcp_analyzer_printer);
    printf("sygnal %i -> grzecznie wychodze\n", signum);
}

void signals_init(void)
{
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
}
