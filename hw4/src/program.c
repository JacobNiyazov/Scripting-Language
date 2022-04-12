#include <stdlib.h>
#include <stdio.h>

#include "mush.h"
#include "debug.h"

/*
 * This is the "program store" module for Mush.
 * It maintains a set of numbered statements, along with a "program counter"
 * that indicates the current point of execution, which is either before all
 * statements, after all statements, or in between two statements.
 * There should be no fixed limit on the number of statements that the program
 * store can hold.
 */

typedef struct node{
    STMT *stmt;
    struct node *nextNode;
} node;

typedef struct p_store{
    int counter;
    int storeLen;
    node *head;
} p_store;

p_store prog_store = { .head = NULL, .storeLen = 0, .counter = 0 };

/**
 * @brief  Output a listing of the current contents of the program store.
 * @details  This function outputs a listing of the current contents of the
 * program store.  Statements are listed in increasing order of their line
 * number.  The current position of the program counter is indicated by
 * a line containing only the string "-->" at the current program counter
 * position.
 *
 * @param out  The stream to which to output the listing.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_list(FILE *out) {
    printf("counter = %d\n", prog_store.counter);
    int is_pc_printed = 0;
    node *temp = prog_store.head;
    int pos = 0;
    if(temp == NULL){
        int res = fprintf(out, "-->\n");
        is_pc_printed = 1;
        if(res < 0){
            return -1;
        }
    }
    while(temp){
        if(pos == prog_store.counter){
            int res = fprintf(out, "-->\n");
            is_pc_printed = 1;
            if(res < 0){
                return -1;
            }
        }
        show_stmt(out, temp->stmt);
        pos++;
        temp = temp->nextNode;
    }
    if(!is_pc_printed){
        int res = fprintf(out, "-->\n");
        is_pc_printed = 1;
        if(res < 0){
            return -1;
        }
    }

    return 0;
}

/**
 * @brief  Insert a new statement into the program store.
 * @details  This function inserts a new statement into the program store.
 * The statement must have a line number.  If the line number is the same as
 * that of an existing statement, that statement is replaced.
 * The program store assumes the responsibility for ultimately freeing any
 * statement that is inserted using this function.
 * Insertion of new statements preserves the value of the program counter:
 * if the position of the program counter was just before a particular statement
 * before insertion of a new statement, it will still be before that statement
 * after insertion, and if the position of the program counter was after all
 * statements before insertion of a new statement, then it will still be after
 * all statements after insertion.
 *
 * @param stmt  The statement to be inserted.
 * @return  0 if successful, -1 if any error occurred.
 */
int prog_insert(STMT *stmt) {
    int lineno = stmt->lineno;

    //check existing lineNums
    node *curr = prog_store.head;

    while(curr != NULL){
        //there already exists a stmt w the lineno - replace it
        if(curr->stmt->lineno == lineno){
            free_stmt(curr->stmt);
            curr->stmt = stmt;

            return 0;
        }

        curr = curr -> nextNode;
    }

    int pos = 0;
    curr = prog_store.head;
    //if insert at head
    // int x = (lineno < prog_store.head->stmt->lineno) || (prog_store.storeLen == 0);
    // printf("x = %d\n", x);
    if((prog_store.storeLen == 0) || (lineno < prog_store.head->stmt->lineno)){
        node *new = malloc(sizeof(node));
        if(new == NULL){
            free_stmt(stmt);
            return -1;
        }
        new->stmt = stmt;
        new->nextNode = curr;
        prog_store.head = new;
    }
    else{
        pos++;
        while(curr->nextNode && curr->nextNode->stmt->lineno < lineno){
            curr = curr->nextNode;
            pos++;
        }

        node *new = malloc(sizeof(node));
        if(new == NULL){
            free_stmt(stmt);
            return -1;
        }
        new->stmt = stmt;

        new->nextNode = curr->nextNode;
        curr->nextNode = new;
    }


    if(pos <= prog_store.counter){
        prog_store.counter = (prog_store.counter) + 1;
    }

    prog_store.storeLen = prog_store.storeLen + 1;


    return 0;
}

/**
 * @brief  Delete statements from the program store.
 * @details  This function deletes from the program store statements whose
 * line numbers fall in a specified range.  Any deleted statements are freed.
 * Deletion of statements preserves the value of the program counter:
 * if before deletion the program counter pointed to a position just before
 * a statement that was not among those to be deleted, then after deletion the
 * program counter will still point the position just before that same statement.
 * If before deletion the program counter pointed to a position just before
 * a statement that was among those to be deleted, then after deletion the
 * program counter will point to the first statement beyond those deleted,
 * if such a statement exists, otherwise the program counter will point to
 * the end of the program.
 *
 * @param min  Lower end of the range of line numbers to be deleted.
 * @param max  Upper end of the range of line numbers to be deleted.
 */
int prog_delete(int min, int max) {
    node *temp = prog_store.head;
    //handle head
    while(temp && temp->stmt->lineno >= min && temp->stmt->lineno <= max){
        prog_store.head = temp->nextNode;
        free_stmt(temp->stmt);
        free(temp);
        prog_store.storeLen = prog_store.storeLen - 1;
        prog_store.counter = prog_store.counter - 1;
        temp = prog_store.head;
    }
    if(prog_store.counter < 0)
        prog_store.counter = 0;
    while(temp && temp->nextNode){
        int lineno = temp->nextNode->stmt->lineno;
        if(lineno >= min && lineno <= max){
            node *n = temp->nextNode;
            prog_store.storeLen = prog_store.storeLen - 1;
            if(prog_store.counter > 1)
                prog_store.counter = prog_store.counter - 1;
            temp->nextNode = temp->nextNode->nextNode;
            free_stmt(n->stmt);
            free(n);
        }
        else
            temp = temp->nextNode;
    }
    if(prog_store.storeLen == 1)
        prog_store.counter = 0;

    return 0;
}

/**
 * @brief  Reset the program counter to the beginning of the program.
 * @details  This function resets the program counter to point just
 * before the first statement in the program.
 */
void prog_reset(void) {
    prog_store.counter = 0;
}

/**
 * @brief  Fetch the next program statement.
 * @details  This function fetches and returns the first program
 * statement after the current program counter position.  The program
 * counter position is not modified.  The returned pointer should not
 * be used after any subsequent call to prog_delete that deletes the
 * statement from the program store.
 *
 * @return  The first program statement after the current program
 * counter position, if any, otherwise NULL.
 */
STMT *prog_fetch(void) {
    int pos = 0;
    node *temp = prog_store.head;
    while(temp){
        if(pos == prog_store.counter){
            return temp->stmt;
        }
        temp = temp->nextNode;
        pos++;
    }
    return NULL;
}

/**
 * @brief  Advance the program counter to the next existing statement.
 * @details  This function advances the program counter by one statement
 * from its original position and returns the statement just after the
 * new position.  The returned pointer should not be used after any
 * subsequent call to prog_delete that deletes the statement from the
 * program store.
 *
 * @return The first program statement after the new program counter
 * position, if any, otherwise NULL.
 */
STMT *prog_next() {
    prog_store.counter = prog_store.counter + 1;
    int pos = 0;
    node *temp = prog_store.head;
    while(temp){
        if(pos == prog_store.counter){
            return temp->stmt;
        }
        temp = temp->nextNode;
        pos++;
    }
    return NULL;
}

/**
 * @brief  Perform a "go to" operation on the program store.
 * @details  This function performs a "go to" operation on the program
 * store, by resetting the program counter to point to the position just
 * before the statement with the specified line number.
 * The statement pointed at by the new program counter is returned.
 * If there is no statement with the specified line number, then no
 * change is made to the program counter and NULL is returned.
 * Any returned statement should only be regarded as valid as long
 * as no calls to prog_delete are made that delete that statement from
 * the program store.
 *
 * @return  The statement having the specified line number, if such a
 * statement exists, otherwise NULL.
 */
STMT *prog_goto(int lineno) {
    int pos = 0;
    node *temp = prog_store.head;
    while(temp){
        int lineNum = temp->stmt->lineno;
        if(lineNum == lineno){
            prog_store.counter = pos;
            return temp->stmt;
        }
        temp = temp->nextNode;
        pos++;
    }
    return NULL;
}
