/*********************/
/* errmsg.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "errmsg.h"  /* Makes sure we're consistent with the declarations. */


// char errmsg[163];

// const char * const outofmem = "Out of memory.\n";

static char *errmsg = "\0";

/**
 * @brief  Set an error indication, with a specified error message.
 * @param msg Pointer to the error message.  The string passed by the caller
 * will be copied.
 */
void set_error(char *msg){
	char *space = strdup(msg);
	if(space)
		errmsg = space;
}

/**
 * @brief  Test whether there is currently an error indication.
 * @return 1 if an error indication currently exists, 0 otherwise.
 */
int is_error(){
	if(*errmsg != '\0')
		return 1;
	else
		return 0;
}

/**
 * @brief  Issue any existing error message to the specified output stream.
 * @param file  Stream to which the error message is to be issued.
 * @return 0 if either there was no existing error message, or else there
 * was an existing error message and it was successfully output.
 * Return non-zero if the attempt to output an existing error message
 * failed.
 */
int report_error(FILE *file){
	int e = is_error();
	if(e == 0)
		return 0;

	int res = fprintf(file, "%s\n", errmsg);
	if(res < 0)
		return -1;
	else
		return 0;
}

/**
 * Clear any existing error indication and free storage occupied by
 * any existing error message.
 */
void clear_error(){
	if(errmsg) free(errmsg);
}
