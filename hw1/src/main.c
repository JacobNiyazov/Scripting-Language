#include <stdio.h>
#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv)){
        USAGE(*argv, EXIT_FAILURE);
    }
    if(global_options == HELP_OPTION){
        USAGE(*argv, EXIT_SUCCESS);
    }

    if(global_options > CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)
        indent_level = global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
    else
        indent_level = 0;
    ARGO_VALUE *p = argo_read_value(stdin);
    //printf("%i",p->type);
    int res = argo_write_value(p, stdout);
    if(res)
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
