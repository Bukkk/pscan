#include "printer.h"
#include "esc_def.h"

#include <stdio.h>

#define WIDTH 32

void print_analized_data(AnalyzedData* analyzed)
{
    printf(ESC_CLEAR_SCREEN);
    printf(ESC_CURS_POS(0, 0));
    for (size_t core = 0; core < analyzed->cores; ++core) {
        double usage = analyzed->procent[core];

        printf("core%3zu: ", core);

        size_t dots = (size_t)(WIDTH * usage);
        printf("[");
        if (usage < 0.3) {
            printf(ESC_SGR_FGR_COLOR_RGB(50, 255, 50));
        } else if (usage < 0.7){
            printf(ESC_SGR_FGR_COLOR_RGB(255, 255, 50));
        } else {
            printf(ESC_SGR_FGR_COLOR_RGB(255, 50, 50));
        }
        for (size_t i = 0; i < WIDTH; ++i) {
            if (i < dots) {
                printf("#");
            }else {
                printf(" ");
            }
        }
        printf(ESC_SGR_CLEAR);
        printf("]");

        printf("%lf%%", usage * 100);

        printf("\n");
    }
}
