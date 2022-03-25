#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_malloc(960);
    sf_show_heap();

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
