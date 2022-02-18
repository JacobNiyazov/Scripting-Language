#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "argo.h"
#include "global.h"
#include "debug.h"


void dec_to_hex(int c, FILE *f);
int hex_to_dec(int c);
int argo_read_value_obj(ARGO_OBJECT *obj,FILE *f);
int argo_read_value_array(ARGO_ARRAY *arr,FILE *f);

/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
ARGO_VALUE *argo_read_value(FILE *f) {
    // Return value
    ARGO_VALUE *output = argo_value_storage + argo_next_value;

    // Verify not out of bounds
    if(argo_next_value >= NUM_ARGO_VALUES){
        fprintf(stderr, "ERROR");
        return NULL;
    }

    // Iterate through input until an argo value is found
    int file_i = fgetc(f);
    while(file_i != EOF){
        // Argo object found
        if(file_i == '{'){
            (argo_value_storage + argo_next_value) -> type = ARGO_OBJECT_TYPE;
            int ro = argo_read_value_obj(&((argo_value_storage + argo_next_value) -> content.object), f);
            if(ro){
                fprintf(stderr, "ERROR");
                return NULL;
            }

        }
        // argo array found
        else if(file_i == '['){
            (argo_value_storage + argo_next_value) -> type = ARGO_ARRAY_TYPE;
            int ra = argo_read_value_array(&((argo_value_storage + argo_next_value) -> content.array), f);
            if(ra){
                fprintf(stderr, "ERROR");
                return NULL;
            }
        }
        //argo string found
        else if(file_i == '"'){
            (argo_value_storage + argo_next_value) -> type = ARGO_STRING_TYPE;
            ARGO_STRING *str = &((argo_value_storage + argo_next_value) -> content.string);
            int ars = argo_read_string(str, f);
            if(ars){
                fprintf(stderr, "ERROR");
                return NULL;
            }
            argo_next_value++;

        }
        //argo num found
        else if(file_i >= '0' && file_i <= '9'){
            int ug = ungetc(file_i, f);
            if(ug == EOF){
                fprintf(stderr, "ERROR");
                return NULL;
            }
            int arn = argo_read_number(&((argo_value_storage + argo_next_value) -> content.number), f);
            if(arn){
                fprintf(stderr, "ERROR");
                return NULL;
            }
            (argo_value_storage + argo_next_value) -> type = ARGO_NUMBER_TYPE;
            argo_next_value++;
        }
        // argo basic true found
        else if(file_i == 't'){
            int basicChar = fgetc(f);
            if(basicChar == 'r'){
                basicChar = fgetc(f);
                if(basicChar == 'u'){
                    basicChar = fgetc(f);
                    if(basicChar == 'e'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_TRUE;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return NULL;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return NULL;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return NULL;
            }

            argo_next_value++;
        }
        // argo basic false found
        else if(file_i == 'f'){
            int basicChar = fgetc(f);
            if(basicChar == 'a'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 's'){
                        basicChar = fgetc(f);
                        if(basicChar == 'e'){
                            (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                            (argo_value_storage + argo_next_value) -> content.basic = ARGO_FALSE;
                            argo_next_value++;
                        }
                        else{
                            fprintf(stderr, "ERROR");
                            return NULL;
                        }
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return NULL;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return NULL;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return NULL;
            }
            argo_next_value++;
        }
        //argo basic null found
        else if(file_i == 'n'){
            int basicChar = fgetc(f);
            if(basicChar == 'u'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 'l'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_NULL;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return NULL;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return NULL;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return NULL;
            }
            argo_next_value++;
        }
        // get next chararcter
        file_i = fgetc(f);

    }
    return output;
}

// Helper function for reading argo arrays
int argo_read_value_array(ARGO_ARRAY *arr,FILE *f){
    // Set argo value type and sentinel of element list
    (argo_value_storage + argo_next_value) -> type = ARGO_ARRAY_TYPE;
    ARGO_VALUE snt;
    snt.type = ARGO_NO_TYPE;
    (argo_value_storage + argo_next_value)->content.array.element_list = &snt;
    argo_next_value++;

    int file_i = fgetc(f);


    snt.next = (argo_value_storage + argo_next_value);
    (argo_value_storage + argo_next_value) -> prev =  &snt;

    // iterate until end of array or end of file
    while(file_i != EOF && file_i != ']'){
        if(argo_next_value >= NUM_ARGO_VALUES){
            fprintf(stderr, "ERROR");
            return -1;
        }

        file_i = fgetc(f);

        // skip to an argo value indicator
        while(file_i != EOF && file_i != '{' && file_i != '[' && file_i != '"' && file_i != 't' && file_i != 'f' && file_i != 'n' && (file_i < '0' || file_i > '9')){
            file_i = fgetc(f);
        }
        if(file_i == EOF){
            fprintf(stderr, "ERROR");
            return -1;
        }

        // Read argo value based on type

        if(file_i == '{'){
            int arvo = argo_read_value_obj(&((argo_value_storage + argo_next_value) -> content.object), f);
            if(arvo){
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        else if(file_i == '['){
            (argo_value_storage + argo_next_value) -> type = ARGO_ARRAY_TYPE;
            int ra = argo_read_value_array(&((argo_value_storage + argo_next_value) -> content.array), f);
            if(ra){
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        else if(file_i == '"'){
            (argo_value_storage + argo_next_value) -> type = ARGO_STRING_TYPE;
            ARGO_STRING *str = &((argo_value_storage + argo_next_value) -> content.string);
            int ars = argo_read_string(str, f);
            if(ars){
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;

        }
        else if(file_i >= '0' && file_i <= '9'){
            int ug = ungetc(file_i, f);
            if(ug == EOF){
                fprintf(stderr, "ERROR");
                return -1;
            }
            int arn = argo_read_number(&((argo_value_storage + argo_next_value) -> content.number), f);
            if(arn){
                fprintf(stderr, "ERROR");
                return -1;
            }
            (argo_value_storage + argo_next_value) -> type = ARGO_NUMBER_TYPE;
            argo_next_value++;
        }
        else if(file_i == 't'){
            int basicChar = fgetc(f);
            if(basicChar == 'r'){
                basicChar = fgetc(f);
                if(basicChar == 'u'){
                    basicChar = fgetc(f);
                    if(basicChar == 'e'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_TRUE;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }
        else if(file_i == 'f'){
            int basicChar = fgetc(f);
            if(basicChar == 'a'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 's'){
                        basicChar = fgetc(f);
                        if(basicChar == 'e'){
                            (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                            (argo_value_storage + argo_next_value) -> content.basic = ARGO_FALSE;
                            argo_next_value++;
                        }
                        else{
                            fprintf(stderr, "ERROR");
                            return -1;
                        }
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }
        else if(file_i == 'n'){
            int basicChar = fgetc(f);
            if(basicChar == 'u'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 'l'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_NULL;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }

        file_i = fgetc(f);

    }
    argo_next_value--;
    snt.prev = (argo_value_storage + argo_next_value);
    (argo_value_storage + argo_next_value) -> next =  &snt;
    return 0;
}

// Helper functino for reading argo objects
int argo_read_value_obj(ARGO_OBJECT *obj, FILE *f){
    // Set argo type and sentinel of member list
    (argo_value_storage + argo_next_value) -> type = ARGO_OBJECT_TYPE;
    ARGO_VALUE *snt = NULL;
    snt->type = ARGO_NO_TYPE;
    (argo_value_storage + argo_next_value)->content.object.member_list = snt;
    argo_next_value++;
    //printf("first");
    int file_i = fgetc(f);


    snt->next = (argo_value_storage + argo_next_value);
    (argo_value_storage + argo_next_value) -> prev =  snt;

    // Iterate through until enf of file or end of object
    while(file_i != EOF && file_i != '}'){
        if(argo_next_value >= NUM_ARGO_VALUES){
            fprintf(stderr, "ERROR");
            return -1;
        }


        // skip to member name
        while(file_i != EOF && file_i != '"'){
            file_i = fgetc(f);
        }
        if(file_i == EOF){
            fprintf(stderr, "ERROR");
            return -1;
        }

        // read name
        int ars = argo_read_string(&((argo_value_storage + argo_next_value) -> name), f);
        if(ars){
            fprintf(stderr, "ERROR");
            return -1;
        }

        file_i = fgetc(f);
        if(file_i != ':'){
            fprintf(stderr, "ERROR");
            return -1;
        }
        file_i = fgetc(f);

        // skip to arrgo value
        while(file_i != EOF && file_i != '{' && file_i != '[' && file_i != '"' && file_i != 't' && file_i != 'f' && file_i != 'n' && (file_i < '0' || file_i > '9')){
            file_i = fgetc(f);
        }
        if(file_i == EOF){
            fprintf(stderr, "ERROR");
            return -1;
        }

        // read argo value based on type
        if(file_i == '{'){
            int arvo = argo_read_value_obj(obj, f);
            if(arvo){
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        else if(file_i == '['){
            (argo_value_storage + argo_next_value) -> type = ARGO_ARRAY_TYPE;
            int ra = argo_read_value_array(&((argo_value_storage + argo_next_value) -> content.array), f);
            if(ra){
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        else if(file_i == '"'){
            (argo_value_storage + argo_next_value) -> type = ARGO_STRING_TYPE;
            ARGO_STRING *str = &((argo_value_storage + argo_next_value) -> content.string);
            int ars = argo_read_string(str, f);
            if(ars){
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;

        }
        else if(file_i >= '0' && file_i <= '9'){
            int ug = ungetc(file_i, f);
            if(ug == EOF){
                fprintf(stderr, "ERROR");
                return -1;
            }
            int arn = argo_read_number(&((argo_value_storage + argo_next_value) -> content.number), f);
            if(arn){
                fprintf(stderr, "ERROR");
                return -1;
            }
            (argo_value_storage + argo_next_value) -> type = ARGO_NUMBER_TYPE;
            argo_next_value++;
            //printf("%d\n", *((argo_value_storage + argo_next_value-1) -> name.content));
        }
        else if(file_i == 't'){
            int basicChar = fgetc(f);
            if(basicChar == 'r'){
                basicChar = fgetc(f);
                if(basicChar == 'u'){
                    basicChar = fgetc(f);
                    if(basicChar == 'e'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_TRUE;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }
        else if(file_i == 'f'){
            int basicChar = fgetc(f);
            if(basicChar == 'a'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 's'){
                        basicChar = fgetc(f);
                        if(basicChar == 'e'){
                            (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                            (argo_value_storage + argo_next_value) -> content.basic = ARGO_FALSE;
                            argo_next_value++;
                        }
                        else{
                            fprintf(stderr, "ERROR");
                            return -1;
                        }
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }
        else if(file_i == 'n'){
            int basicChar = fgetc(f);
            if(basicChar == 'u'){
                basicChar = fgetc(f);
                if(basicChar == 'l'){
                    basicChar = fgetc(f);
                    if(basicChar == 'l'){
                        (argo_value_storage + argo_next_value) -> type = ARGO_BASIC_TYPE;
                        (argo_value_storage + argo_next_value) -> content.basic = ARGO_NULL;
                        argo_next_value++;
                    }
                    else{
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                }
                else{
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
            argo_next_value++;
        }

        //(argo_value_storage + argo_next_value-1) -> next = (argo_value_storage + argo_next_value);
        //(argo_value_storage + argo_next_value) -> prev = (argo_value_storage + argo_next_value-1);
        file_i = fgetc(f);

    }
    argo_next_value--;
    snt->prev = (argo_value_storage + argo_next_value);
    (argo_value_storage + argo_next_value) -> next =  snt;
    //printf("{{{%d\n", ((argo_value_storage + argo_next_value-1) ->content.object.member_list->next->type));
    //printf("type = %c\n", *((argo_value_storage + argo_next_value-1) ->content.object.member_list->next->name.content));
    return 0;
}


/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f) {
    int file_i = fgetc(f);

    // Iterate until end of string
    while(file_i != '"'){
        if(file_i == EOF){
            fprintf(stderr, "ERROR");
            return -1;
        }

        // Handle escape sequences and control characters
        if(file_i == '\\'){
            int tempChar = fgetc(f);
            if(tempChar == EOF){
                fprintf(stderr, "ERROR");
                return -1;
            }

            if(tempChar == '"'){
                int ac = argo_append_char(s, tempChar);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == '\\'){
                int ac = argo_append_char(s, ARGO_BSLASH);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == '/'){
                int ac = argo_append_char(s, '/');
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 'r'){
                int ac = argo_append_char(s, ARGO_CR);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 'b'){
                int ac = argo_append_char(s, ARGO_BS);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 'f'){
                int ac = argo_append_char(s, ARGO_FF);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 'n'){
                int ac = argo_append_char(s, ARGO_LF);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 't'){
                int ac = argo_append_char(s, ARGO_HT);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else if(tempChar == 'u'){
                long sum = 0;
                for(int i = 0; i < 4; i++){
                    tempChar = fgetc(f);
                    if(tempChar == EOF){
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                    else{
                        int htd = hex_to_dec(tempChar);
                        if(htd == -1){
                            fprintf(stderr, "ERROR");
                            return -1;
                        }
                        sum += htd;
                    }
                }
                int ac = argo_append_char(s, sum);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        // read regular characters
        else{
            int ac = argo_append_char(s, file_i);
            if(ac){
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        file_i = fgetc(f);
    }
    return 0;
}

// Helper function to convert from hexadecimal representation to decimal representation
int hex_to_dec(int c){
    if(c >= '0' && c <= '9')
        return (c - 48);
    else if(c >= 'a' && c <= 'f')
        return (c - 87);
    else if(c >= 'A' && c <= 'F')
        return (c - 55);
    else{
        fprintf(stderr, "ERROR");
        return -1;
    }
}


/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    // Set local variables
    bool has_decimal = false;
    ARGO_STRING *as = &(n -> string_value);
    long num = 0;
    double dnum = 0.0;
    long exp = 0;
    double multTen = 1;

    int c = fgetc(f);

    // read integer
    while(c != EOF && c >= '0' && c <= '9'){
        num = num * 10;
        num = num + (c - 48);

        int ac = argo_append_char(as, c);
        if(ac){
            fprintf(stderr, "ERROR");
            return -1;
        }
        c = fgetc(f);

    // If number has a . read the rest of the num
    }
    if(c == '.'){
        has_decimal = true;

        dnum = num;
        int divisor = 10;

        int ac = argo_append_char(as, '.');
        if(ac){
            fprintf(stderr, "ERROR");
            return -1;
        }
        c = fgetc(f);
        // construct the double value as the loop iterates through each char
        while(c != EOF && c >= '0' && c <= '9'){
            dnum = dnum + ((c - 48.0)/divisor);
            divisor = divisor * 10;
            int ac = argo_append_char(as, c);
            if(ac){
                fprintf(stderr, "ERROR");
                return -1;
            }
            c = fgetc(f);
        }
    }
    // If there's an exponent
    if(c == 'e' || c == 'E'){
        int ac = argo_append_char(as, c);
        if(ac){
            fprintf(stderr, "ERROR");
            return -1;
        }

        c = fgetc(f);
        if(c == EOF){
            fprintf(stderr, "ERROR");
            return -1;
        }

        // If exponent is negative
        if(c == '-'){
            int ac = argo_append_char(as, c);
            if(ac){
                fprintf(stderr, "ERROR");
                return -1;
            }

            c = fgetc(f);
            if(c == EOF){
                fprintf(stderr, "ERROR");
                return -1;
            }

            // read exponent
            if(c >= '0' && c <= '9'){
                while(c != EOF && c >= '0' && c <= '9'){
                    exp = exp * 10;
                    exp = exp + (c - 48);
                    int ac = argo_append_char(as, c);
                    if(ac){
                        fprintf(stderr, "ERROR");
                        return -1;
                    }
                    c = fgetc(f);
                }

                // get power of 10 of exponent
                for(int i = 0; i < exp; i++)
                    multTen = multTen / 10;

                dnum = dnum * multTen;
            }
            else{
                fprintf(stderr, "ERROR");
                return -1;
            }
        }
        // positive exponent
        else if(c >= '0' && c <= '9'){
            // read exponent
            while(c != EOF && c >= '0' && c <= '9'){
                exp = exp * 10;
                exp = exp + (c - 48);
                int ac = argo_append_char(as, c);
                if(ac){
                    fprintf(stderr, "ERROR");
                    return -1;
                }
                c = fgetc(f);
            }
            //get power of 10 of exponent
            for(int i = 0; i < exp; i++)
                multTen = multTen * 10;

            dnum = dnum * multTen;
        }
        else{
            fprintf(stderr, "ERROR");
            return -1;
        }
    }

    // set argo num attributes
    ungetc(c, f);
    n -> valid_string = 1;
    if(has_decimal){
        n -> valid_float = 1;
        n -> valid_int = 0;
        n -> float_value = dnum;
    }
    else{
        n -> valid_float = 1;
        n -> valid_int = 1;
        n -> int_value = num;
    }
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_value(ARGO_VALUE *v, FILE *f) {
    // write argo number
    if(v->type == ARGO_NUMBER_TYPE){
        if(v->name.content != NULL){
            int fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR1");
                return -1;
            }
            int *temp = v->name.content;
            for(int i = 0; i < v->name.length; i++){
                fpc = fputc(*temp, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR2");
                    return -1;
                }
                ++temp;
            }
            fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR3");
                return -1;
            }
            fpc = fputc(ARGO_COLON, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR4");
                return -1;
            }
            if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR5");
                    return -1;
                }
            }
        }
        int awn = argo_write_number(&(v->content.number), f);
        if(awn){
            fprintf(stderr, "ERROR6");
            return -1;
        }
    }
    //write argo strings
    else if(v->type == ARGO_STRING_TYPE){
        if(v->name.content != NULL){
            int fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR7");
                return -1;
            }
            int *temp = v->name.content;
            for(int i = 0; i < v->name.length; i++){
                fpc = fputc(*temp, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR8");
                    return -1;
                }
                ++temp;
            }
            fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR9");
                return -1;}
            fpc = fputc(ARGO_COLON, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR10");
                return -1;}
            if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR11");
                    return -1;}
            }
        }
        int aws = argo_write_string(&(v->content.string), f);
        if(aws){
            fprintf(stderr, "ERROR12");
            return -1;}

    }
    // write argo object
    else if(v->type == ARGO_OBJECT_TYPE){
        ARGO_VALUE *av = v->content.object.member_list;
        if(v->name.content != NULL){
            int fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR13");
                return -1;}
            int *temp = v->name.content;
            for(int i = 0; i < v->name.length; i++){
                fpc = fputc(*temp, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR14");
                    return -1;}
                ++temp;
            }
            fpc = fputc(ARGO_QUOTE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR15");
                return -1;}
            fpc = fputc(ARGO_COLON, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR16");
                return -1;}
            if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR17");
                    return -1;}
            }
        }
        int fpc = fputc(ARGO_LBRACE, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR18");
            return -1;}

        // Write indents
        if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
            fpc = fputc(ARGO_LF, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR19");
                return -1;}
            for(int i = 0; i < indent_level; i++){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR20");
                    return -1;}
            }
        }

        // Iterate through object member list
        if(av != v->content.object.member_list->prev || av != v->content.object.member_list->next){
            do{
                int temp_i = 0;
                if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)
                    temp_i = global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
                indent_level = indent_level + temp_i;
                int awv = argo_write_value(av, f);
                if(awv){
                    fprintf(stderr, "ERROR21");
                    return -1;}
                indent_level = indent_level - temp_i;
                if(av->type != ARGO_NO_TYPE && av->next && av->next->type != ARGO_NO_TYPE){
                    int fpc = fputc(ARGO_COMMA, f);
                    if(fpc == EOF){
                        fprintf(stderr, "ERROR22");
                        return -1;}
                    if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
                        fpc = fputc(ARGO_LF, f);
                        if(fpc == EOF){
                            fprintf(stderr, "ERROR23");
                            return -1;}
                        for(int i = 0; i < indent_level; i++){
                            fpc = fputc(ARGO_SPACE, f);
                            if(fpc == EOF){
                                fprintf(stderr, "ERROR24");
                                return -1;}
                        }
                    }
                }
                av=av->next;
            }while(av != v->content.object.member_list);
        }

        // handle pretty printing and indent level
        if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
            int fpc = fputc(ARGO_LF, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR25");
                return -1;}
            int temp_i = 0;
            if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)
                temp_i = global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
            indent_level = indent_level - temp_i;
            for(int i = 0; i < indent_level; i++){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR26");
                    return -1;}
            }

            indent_level = indent_level + temp_i;
        }
        fpc = fputc(ARGO_RBRACE, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR27");
            return -1;}

    }

    // write argo basic
    else if(v->type == ARGO_BASIC_TYPE){
        int n = v->content.basic;
        int fpc = fputc(ARGO_QUOTE, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR28");
            return -1;}
        int *temp = v->name.content;
        for(int i = 0; i < v->name.length; i++){
            fpc = fputc(*temp, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR29");
                return -1;}
            ++temp;
        }
        fpc = fputc(ARGO_QUOTE, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR30");
            return -1;}
        fpc = fputc(ARGO_COLON, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR31");
            return -1;}
        if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
            fpc = fputc(ARGO_SPACE, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR32");
                return -1;}
        }
        if(n == 0){
            fpc = fputs(ARGO_NULL_TOKEN, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR33");
                return -1;}
        }
        else if(n == 1){
            fpc = fputs(ARGO_TRUE_TOKEN, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR34");
                return -1;}
        }
        else if(n == 2){
            fpc = fputs(ARGO_FALSE_TOKEN, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR35");
                return -1;}
        }

    }

    // write argo array
    else if(v->type == ARGO_ARRAY_TYPE){
        ARGO_VALUE *av = v->content.array.element_list;
        int fpc = fputc(ARGO_LBRACK, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR36");
            return -1;}
        if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
            fpc = fputc(ARGO_LF, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR37");
                return -1;}
            for(int i = 0; i < indent_level; i++){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR38");
                    return -1;}
            }
        }

        // iterate through element list
        if(av != v->content.array.element_list->prev || av != v->content.array.element_list->next){
            do{
                int temp_i = 0;
                if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)
                    temp_i = global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
                indent_level = indent_level + temp_i;
                int awv = argo_write_value(av, f);
                if(awv){
                    fprintf(stderr, "ERROR39");
                }
                indent_level = indent_level - temp_i;
                if(av->type != ARGO_NO_TYPE && av->next && av->next->type != ARGO_NO_TYPE){
                    int fpc = fputc(ARGO_COMMA, f);
                    if(fpc == EOF){
                        fprintf(stderr, "ERROR40");
                        return -1;}
                    if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
                        fpc = fputc(ARGO_LF, f);
                        if(fpc == EOF){
                            fprintf(stderr, "ERROR41");
                            return -1;}
                        for(int i = 0; i < indent_level; i++){
                            fpc = fputc(ARGO_SPACE, f);
                            if(fpc == EOF){
                                fprintf(stderr, "ERROR42");
                                return -1;}
                        }
                    }
                }
                av=av->next;
            }while(av != v->content.array.element_list);
        }

        // pretty printing and indent level
        if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION){
            int fpc = fputc(ARGO_LF, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR43");
                return -1;}
            int temp_i = 0;
            if(global_options >= CANONICALIZE_OPTION + PRETTY_PRINT_OPTION)
                temp_i = global_options - CANONICALIZE_OPTION - PRETTY_PRINT_OPTION;
            indent_level = indent_level - temp_i;
            for(int i = 0; i < indent_level; i++){
                fpc = fputc(ARGO_SPACE, f);
                if(fpc == EOF){
                    fprintf(stderr, "ERROR44");
                    return -1;}
            }
            indent_level = indent_level + temp_i;
        }
        fpc = fputc(ARGO_RBRACK, f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR45");
            return -1;}
    }

    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f) {
    int fpc = fputc(ARGO_QUOTE, f);
    if(fpc == EOF){
        fprintf(stderr, "ERROR47");
        return -1;}
    int *temp = s->content;
    // iterate through entire argo string
    for(int i = 0; i < s->length; i++){
        // write escape sequences
        if(*temp == ARGO_QUOTE){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR48");
                return -1;}
            fpc = fputc('"', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR49");
                return -1;}
            ++temp;
        }
        else if(*temp == '\\'){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR50");
                return -1;}
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR51");
                return -1;}
            ++temp;
        }
        else if(*temp == ARGO_BS){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR52");
                return -1;}
            fpc = fputc('b', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR53");
                return -1;}
            ++temp;
        }
        else if(*temp == ARGO_FF){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR54");
                return -1;}
            fpc = fputc('f', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR55");
                return -1;}
            ++temp;
        }
        else if(*temp == ARGO_LF){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR56");
                return -1;}
            fpc = fputc('n', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR57");
                return -1;}
            ++temp;
        }
        else if(*temp == ARGO_CR){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR58");
                return -1;}
            fpc = fputc('r', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR59");
                return -1;}
            ++temp;
        }
        else if(*temp == ARGO_HT){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR60");
                return -1;}
            fpc = fputc('t', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR61");
                return -1;}
            ++temp;
        }
        // write unicode
        else if(*temp > 255 || *temp < 32){
            fpc = fputc('\\', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR62");
                return -1;}
            fpc = fputc('u', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR63");
                return -1;}
            dec_to_hex(*temp, f);
            ++temp;
        }
        // write regular characters
        else{
            fpc = fputc(*temp, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR64");
                return -1;}
            ++temp;
        }

    }
    fpc = fputc(ARGO_QUOTE, f);
    if(fpc == EOF){
        fprintf(stderr, "ERROR65");
        return -1;}
    return 0;
}

// Helper function: convert int from decimal to hexadecimal
void dec_to_hex(int c, FILE *f){
    int first = -1;
    int second = -1;
    int third = -1;
    int fourth = -1;
    int q = c;
    int var;
    while(q != 0){
        var = q % 16;
        if(var < 10){
            var += 48;
            if(fourth > 0){
                if(third > 0){
                    if(second > 0)
                    {
                        first = var;
                    }
                    else
                        second = var;
                }
                else
                    third = var;
            }
            else
                fourth = var;
        }
        else{
            var += 87;
            if(fourth > 0){
                if(third > 0){
                    if(second > 0)
                    {
                        first = var;
                    }
                    else
                        second = var;
                }
                else
                    third = var;
            }
            else
                fourth = var;
        }
        q = q / 16;
    }
    if(first < 0)
        fputc(48, f);
    else
        fputc(first, f);
    if(second < 0)
        fputc(48, f);
    else
        fputc(second, f);
    if(third < 0)
        fputc(48, f);
    else
        fputc(third, f);
    if(fourth < 0)
        fputc(48, f);
    else
        fputc(fourth, f);
}


/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    // Write int
    if(n->valid_int){
        long temp_num = n->int_value;

        // if num is negative, write a '-' and make positive for operations
        if(temp_num < 0){
            int fpc = fputc('-', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR67");
                return -1;}
            temp_num = temp_num * -1;
        }
        long pow = 1;
        while(pow * 10 <= temp_num){
            pow = pow * 10;
        }

        while(pow > 0){
            int i = (temp_num / pow) % 10;
            char c = i + 48;
            int fpc = fputc(c, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR68");
                return -1;}
            pow = pow / 10;
        }
    }

    // Write float
    else if(n -> valid_float){
        bool is_neg = false;
        int exp = 0;
        double temp_num = n -> float_value;

        // if num is negative, write a '-' and make positive for operations
        if(temp_num < 0){
            is_neg = true;
            temp_num = temp_num * -1;
        }

        if(temp_num >= 1){
            while(temp_num >= 1){
                temp_num = temp_num / 10;
                exp++;
            }
        }
        else if(temp_num < 0.1 && temp_num > 0){
            while(temp_num < 0.1){
                temp_num = temp_num * 10;
                exp--;
            }
        }

        if(is_neg){
            int fpc = fputc('-', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR69");
                return -1;}
        }
        int fpc = fputc('0', f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR70");
            return -1;}
        fpc = fputc('.', f);
        if(fpc == EOF){
            fprintf(stderr, "ERROR71");
            return -1;}

        // calculate and write matissa
        for(int i = 0; i < ARGO_PRECISION; i++){
            temp_num = temp_num * 10;
            long leading = (long)temp_num;
            char digi = leading + '0';
            fpc = fputc(digi, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR72");
                return -1;}
            temp_num = temp_num - leading;
        }

        // write e and exponent value
        if(exp > 0){
            int fpc = fputc('e', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR73");
                return -1;}
            fpc = fputc((exp + '0'), f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR74");
                return -1;}
        }
        else if(exp < 0){
            int fpc = fputc('e', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR75");
                return -1;}
            fpc = fputc('-', f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR76");
                return -1;}
            fpc = fputc(((exp *-1) + '0'), f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR77");
                return -1;}
        }

    }
    else if(n -> valid_string){
        ARGO_CHAR *temp_s = n -> string_value.content;
        for(int i = 0; i < n -> string_value.length; i++){
            int fpc = fputc(*temp_s, f);
            if(fpc == EOF){
                fprintf(stderr, "ERROR78");
                return -1;}
            temp_s++;
        }
    }
    else{
        fprintf(stderr, "ERROR79");
        return -1;
    }
    return 0;
}
