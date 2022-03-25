#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    size_t a = 333;
    size_t b = 87;
    size_t c = 260;
    size_t d = 9;
    size_t e = 126;
    void *ptr1 = sf_malloc(b);
    sf_malloc(c);
    void *ptr2 = sf_malloc(e);
    void *ptr3 = sf_malloc(a);
    sf_malloc(d);

    sf_free(ptr1);
    sf_free(ptr2);
    sf_free(ptr3);
    sf_show_heap();
    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
