#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    // sf_set_magic(0x0);
    // sf_malloc(40);
    sf_malloc(1000);
    // void *ptr1 = sf_malloc(8);
    // sf_show_blocks();
    // // sf_free(ptr1);
    // void *ptr2 = sf_malloc(200);
    // // sf_malloc(5);
    // void *ptr3 = sf_malloc(300);
    // // sf_malloc(5);
    // // void *ptr4 = sf_malloc(4);
    // sf_malloc(4);
    // // void *ptr5 = sf_malloc(40);
    // // void *ptr6 = sf_malloc(20);
    // // sf_malloc(40);
    // // sf_free(ptr1);
    // sf_free(ptr3);
    // sf_free(ptr2);
    // // sf_free(ptr4);
    // // sf_free(ptr5);
    // // sf_free(ptr6);
    sf_show_heap();
    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
