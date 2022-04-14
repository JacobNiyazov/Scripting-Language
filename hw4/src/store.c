#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"


/*
 * This is the "data store" module for Mush.
 * It maintains a mapping from variable names to values.
 * The values of variables are stored as strings.
 * However, the module provides functions for setting and retrieving
 * the value of a variable as an integer.  Setting a variable to
 * an integer value causes the value of the variable to be set to
 * a string representation of that integer.  Retrieving the value of
 * a variable as an integer is possible if the current value of the
 * variable is the string representation of an integer.
 */

typedef struct node{
    char *key;
    char *value;
    struct node *nextNode;
} node;

typedef struct d_store{
    int dStoreLen;
    node *head;
} d_store;

d_store data_store = { .head = NULL, .dStoreLen = 0 };

/**
 * @brief  Get the current value of a variable as a string.
 * @details  This function retrieves the current value of a variable
 * as a string.  If the variable has no value, then NULL is returned.
 * Any string returned remains "owned" by the data store module;
 * the caller should not attempt to free the string or to use it
 * after any subsequent call that would modify the value of the variable
 * whose value was retrieved.  If the caller needs to use the string for
 * an indefinite period, a copy should be made immediately.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @return  A string that is the current value of the variable, if any,
 * otherwise NULL.
 */
char *store_get_string(char *var) {
    node *temp = data_store.head;
    while(temp){
        if(strcmp(var, temp->key) == 0){
            return temp->value;
        }
        temp = temp->nextNode;
    }
    return NULL;
}

/**
 * @brief  Get the current value of a variable as an integer.
 * @details  This retrieves the current value of a variable and
 * attempts to interpret it as an integer.  If this is possible,
 * then the integer value is stored at the pointer provided by
 * the caller.
 *
 * @param  var  The variable whose value is to be retrieved.
 * @param  valp  Pointer at which the returned value is to be stored.
 * @return  If the specified variable has no value or the value
 * cannot be interpreted as an integer, then -1 is returned,
 * otherwise 0 is returned.
 */
int store_get_int(char *var, long *valp) {
    node *temp = data_store.head;
    while(temp){
        if(strcmp(var, temp->key) == 0){
            char **end = NULL;
            long res = strtol(temp->value, end, 10);
            if(res){
                *valp = res;
                return 0;
            }
            else{
                if(errno != 0)
                    return -1;
                else if((temp->value) == *end)
                    return -1;
                else{
                    *valp = res;
                    return 0;
                }

            }
        }
        temp = temp->nextNode;
    }
    return -1;
}

/**
 * @brief  Set the value of a variable as a string.
 * @details  This function sets the current value of a specified
 * variable to be a specified string.  If the variable already
 * has a value, then that value is replaced.  If the specified
 * value is NULL, then any existing value of the variable is removed
 * and the variable becomes un-set.  Ownership of the variable and
 * the value strings is not transferred to the data store module as
 * a result of this call; the data store module makes such copies of
 * these strings as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set, or NULL if the variable is to become
 * un-set.
 */
int store_set_string(char *var, char *val) {
    node *temp = data_store.head;
    int is_null_val = 0;
    if(val == NULL){
        is_null_val = 1;
    }
    while(temp){
        if(strcmp(var, temp->key) == 0){
            if(is_null_val){
                if(temp == data_store.head){
                    if(temp->value != NULL)
                        free(temp->value);
                    free(temp->key);
                    data_store.head = temp->nextNode;
                    free(temp);
                    return 0;

                }
                else{
                    node *temptwo = data_store.head;
                    while(temptwo->nextNode && temptwo->nextNode != temp){
                        temptwo = temptwo->nextNode;
                    }
                    if(temp->value != NULL)
                        free(temp->value);
                    free(temp->key);
                    temptwo->nextNode = temp->nextNode;
                    free(temp);
                    return 0;
                }
            }
            else{ // val is not null
                if(temp->value == NULL){
                    temp->value = strdup(val);
                    if(temp->value == NULL)
                        return -1;
                    return 0;
                }
                else{
                    free(temp->value);
                    temp->value = strdup(val);
                    if(temp->value == NULL)
                        return -1;
                    return 0;
                }
            }
        }
        temp = temp->nextNode;
    }
    if(is_null_val)
        return 0;
    else{ //create node
        temp = data_store.head;
        while(temp && temp->nextNode){
            temp = temp->nextNode;
        }
        node *new = malloc(sizeof(node));
        if(new == NULL)
            return -1;
        new->key = malloc(sizeof(var));
        if(new->key == NULL){
            free(new);
            return -1;
        }
        new->value = malloc(sizeof(val));
        if(new->value == NULL){
            free(new);
            free(new->key);
            return -1;
        }
        char *res = strcpy(new->key, var);
        if(res == NULL){
            free(new);
            free(new->key);
            free(new->value);
            return -1;
        }
        res = strcpy(new->value, val);
        if(res == NULL){
            free(new);
            free(new->key);
            free(new->value);
            return -1;
        }
        new->nextNode = NULL;
        if(temp)
            temp->nextNode = new;
        else
            data_store.head = new;
        return 0;
    }
}

/**
 * @brief  Set the value of a variable as an integer.
 * @details  This function sets the current value of a specified
 * variable to be a specified integer.  If the variable already
 * has a value, then that value is replaced.  Ownership of the variable
 * string is not transferred to the data store module as a result of
 * this call; the data store module makes such copies of this string
 * as it may require.
 *
 * @param  var  The variable whose value is to be set.
 * @param  val  The value to set.
 */
int store_set_int(char *var, long val) {
    node *temp = data_store.head;
    while(temp){
        if(strcmp(var, temp->key) == 0){
            if(temp->value == NULL){
                char buff[21];
                sprintf(buff, "%ld", val);
                temp->value = strdup(buff);
                if(temp->key == NULL){
                    return -1;
                }

                return 0;
            }
            else{
                free(temp->value);
                char buff[21];
                sprintf(buff, "%ld", val);
                temp->value = strdup(buff);
                if(temp->key == NULL){
                    return -1;
                }

                return 0;
            }
        }
        temp = temp->nextNode;
    }

    //create node
    temp = data_store.head;
    while(temp->nextNode){
        temp = temp->nextNode;
    }
    node *new = malloc(sizeof(node));
    if(new == NULL)
        return -1;
    new->key = strdup(var);
    if(new->key == NULL){
        free(new);
        return -1;
    }

    char buff[21];
    sprintf(buff, "%ld", val);  // ld ???
    new->value = strdup(buff);
    if(new->key == NULL){
        free(new);
        free(new->key);
        return -1;
    }
    new->nextNode = NULL;
    temp->nextNode = new;
    return 0;
}

/**
 * @brief  Print the current contents of the data store.
 * @details  This function prints the current contents of the data store
 * to the specified output stream.  The format is not specified; this
 * function is intended to be used for debugging purposes.
 *
 * @param f  The stream to which the store contents are to be printed.
 */
void store_show(FILE *f) {
    node *temp = data_store.head;
    while(temp){
        fprintf(f, "%s::%s\n", temp->key, temp->value);
        temp = temp->nextNode;
    }
}

