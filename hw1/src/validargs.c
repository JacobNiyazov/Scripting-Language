#include <stdlib.h>
#include <stdbool.h>

#include "argo.h"
#include "global.h"
#include "debug.h"


bool is_valid_command_arg(char **arg);
bool is_valid_int(char **arg);
bool is_command_h(char **arg);
bool is_command_v(char **arg);
bool is_command_c(char **arg);
bool is_command_p(char **arg);
int str_to_int(char **arg);
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    // Booleans for keeping track what arg appeared
    bool is_c = false;
    bool is_v = false;
    bool is_p = false;
    int indnt = 4;

    // If no args, exit
    if(argc <= 1){
        global_options = 0;
        return -1;
    }

    // If -h set global options, exit
    char **temp = argv+1;
    if(is_command_h(temp)){
        global_options = HELP_OPTION;
        return EXIT_SUCCESS;
    }

    // Iterate through each arg
    for(char **a = argv+1; *a; ++a){
        // Check if arg is invalid
        if(!is_valid_command_arg(a)){
            // Check if arg isnt an int, exit
            if(!is_valid_int(a)){
                global_options = 0;
                return -1;
            }
            // Int with -v not allowed, exit
            else if(is_v || !is_p || !is_c){
                global_options = 0;
                return -1;
            }
            // Indent > 127 not allowed, exit
            else{
                //is_indent = true;
                indnt = str_to_int(a);
                if(indnt > 127){
                    global_options = 0;
                    return -1;
                }
            }
        }
        else if(is_command_v(a)){
            is_v = true;
            if(is_c || is_p){
                global_options = 0;
                return -1;
            }
        }
        // -c not allowed with -v or after -p
        else if(is_command_c(a)){
            is_c = true;
            if(is_v || is_p){
                global_options = 0;
                return -1;
            }
        }
        // -p not allowed with -v or before -c
        else if(is_command_p(a)){
            is_p = true;
            if(is_v || !is_c){
                global_options = 0;
                return -1;
            }
        }
    }
    // Set global options and return success
    if(is_v){
        global_options = VALIDATE_OPTION;
    }
    else if(is_p){
        global_options = CANONICALIZE_OPTION + PRETTY_PRINT_OPTION + indnt;
    }
    else if(is_c){
        global_options = CANONICALIZE_OPTION;
    }
    return EXIT_SUCCESS;
}

// Helper function: check if given arg is valid
bool is_valid_command_arg(char **arg){
    if(**arg != '-'){
        return false;
    }
    if(*(*arg+1) != 'c' && *(*arg+1) != 'v' && *(*arg+1) != 'p'){
        return false;
    }
    if(*(*arg+2) != '\0'){
        return false;
    }
    return true;
}
// Helper function: check if given arg is an int
bool is_valid_int(char **arg){
    for(char *c = *arg; *c; ++c) {
        if(*c < '0' || *c > '9'){
            return false;
        }
    }
    return true;
}
// Helper function: check if given arg is -h
bool is_command_h(char **arg){
    if(**arg != '-'){
        return false;
    }
    if(*(*arg+1) != 'h'){
        return false;
    }
    if(*(*arg+2) != '\0'){
        return false;
    }
    return true;
}
// Helper function: check if given arg is -v
bool is_command_v(char **arg){
    if(**arg != '-'){
        return false;
    }
    if(*(*arg+1) != 'v'){
        return false;
    }
    return true;
}
// Helper function: check if given arg is -c
bool is_command_c(char **arg){
    if(**arg != '-'){
        return false;
    }
    if(*(*arg+1) != 'c'){
        return false;
    }
    return true;
}
// Helper function: check if given arg is -p
bool is_command_p(char **arg){
    if(**arg != '-'){
        return false;
    }
    if(*(*arg+1) != 'p'){
        return false;
    }
    return true;
}
// Helper function: convert given string arg to int
int str_to_int(char **arg){
    int num = 0;

    for(char *c = *arg; *c; ++c) {
        if(*c > '0' || *c < '9'){
            num = num * 10 + (*c - '0');
        }
    }
    return num;
}