/*********************/
/* par.c             */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"
#include "buffer.h"    /* Also includes <stddef.h>. */
#include "reformat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>


#undef NULL
#define NULL ((void *) 0)
#define OPTIONAL_ARGUMENT \
    ((optarg == NULL && optind < argc && opt[optind][0] != '-') \
     ? (bool) (optarg = opt[optind++]) \
     : (optarg != NULL))

const char * const progname = "par";
const char * const version = "3.20";


static int digtoint(char c)

/* Returns the value represented by the digit c,   */
/* or -1 if c is not a digit. Does not use errmsg. */
{
  return c == '0' ? 0 :
         c == '1' ? 1 :
         c == '2' ? 2 :
         c == '3' ? 3 :
         c == '4' ? 4 :
         c == '5' ? 5 :
         c == '6' ? 6 :
         c == '7' ? 7 :
         c == '8' ? 8 :
         c == '9' ? 9 :
         -1;

  /* We can't simply return c - '0' because this is ANSI  */
  /* C code, so it has to work for any character set, not */
  /* just ones which put the digits together in order.    */
}


static int strtoudec(const char *s, int *pn)

/* Puts the decimal value of the string s into *pn, returning */
/* 1 on success. If s is empty, or contains non-digits,       */
/* or represents an integer greater than 9999, then *pn       */
/* is not changed and 0 is returned. Does not use errmsg.     */
{
  int n = 0;

  if (!*s) return 0;

  do {
    if (n >= 1000 || !isdigit(*s)) return 0;
    n = 10 * n + digtoint(*s);
  } while (*++s);

  *pn = n;

  return 1;
}


static void parseopt(int argc,
  char * const *opt, int *pwidth, int *pprefix,
  int *psuffix, int *phang, int *plast, int *pmin
)
/* Parses the single option in opt, setting *pwidth, *pprefix,     */
/* *psuffix, *phang, *plast, or *pmin as appropriate. Uses errmsg. */
{
  // bool isVersion = false;
  // bool isError = false;
  static int lastVal;
  static int minVal;

  static struct option long_options[] = {
    {"version", no_argument, 0, 'v'},
    {"width", required_argument, 0, 'w'},
    {"prefix", required_argument, 0, 'p'},
    {"suffix", required_argument, 0, 's'},
    {"hang", required_argument, 0, 'h'},
    {"last", no_argument, &lastVal, 1},
    {"no-last", no_argument, &lastVal, 0},
    {"min", no_argument, &minVal, 1},
    {"no-min", no_argument, &minVal, 0},
    {NULL, 0, 0, '\0'}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, opt, "w:p:s:h::l::m::", long_options, &option_index)) != -1){
    switch(c){
      case 'v':{
        char *str;
        size_t size_len;
        FILE *stream = open_memstream(&str, &size_len);
        if (stream == NULL){
          set_error("open_memstream error.\n");
          return;
        }
        fprintf(stream, "%s %s\n", progname, version);
        fflush(stream);
        fclose(stream);
        set_error(str);
        free(str);
        //sprintf(errmsg, "%s %s\n", progname, version);
        return;
      }
      case 'w':
        if(optarg){
          int res = strtoudec(optarg, pwidth);
          if(res)
            break;
          else{
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          set_error("Parsing Arguments Error.\n");
          return;
        }
      case 'p':
        if(optarg){
          int res = strtoudec(optarg, pprefix);
          if(res)
            break;
          else{
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          set_error("Parsing Arguments Error.\n");
          return;
        }
      case 's':
        if(optarg){
          int res = strtoudec(optarg, psuffix);
          if(res)
            break;
          else{
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          set_error("Parsing Arguments Error.\n");
          return;
        }
      case 'h':
        if(OPTIONAL_ARGUMENT){
          int res = strtoudec(optarg, phang);
          if(res)
            break;
          else{
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          *phang = 1;
        }
      case 'l':
        if(OPTIONAL_ARGUMENT){
          int *temp = malloc(sizeof(int));
          int res = strtoudec(optarg, temp);
          if(res){
            if(*temp == 0 || *temp == 1){
              *plast = *temp;
              free(temp);
              break;
            }
            else{
              free(temp);
              set_error("Parsing Arguments Error.\n");
              return;
            }
          }
          else{
            free(temp);
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          *plast = 1;
          break;
        }
      case 'm':
        if(OPTIONAL_ARGUMENT){
          int *temp = malloc(sizeof(int));
          int res = strtoudec(optarg, temp);
          if(res){
            if(*temp == 0 || *temp == 1){
              *pmin = *temp;
              free(temp);
              break;
            }
            else{
              free(temp);
              set_error("Parsing Arguments Error.\n");
              return;
            }
          }
          else{
            free(temp);
            set_error("Parsing Arguments Error.\n");
            return;
          }
        }
        else{
          *pmin = 1;
          break;
        }
      case 0:
        if (long_options[option_index].flag != 0){
          if(strcmp(long_options[option_index].name,"no-last") == 0 || strcmp(long_options[option_index].name,"last") == 0){
            *plast = lastVal;
            break;
          }
          else{
            *pmin = minVal;
            break;
          }
        }
        break;

      default:
        set_error("Parsing Arguments Error.\n");
        return;

    }

  }

  int *num = malloc(sizeof(int));
  while(optind < argc){
    char *arg = opt[optind];
    int res = strtoudec(arg, num);
    if(res){
      if(*num <= 8)
        *pprefix = *num;
      else if(*num >= 9)
        *pwidth = *num;
    }
    else{
      set_error("Parsing Arguments Error.\n");
      return;
    }
    optind++;
  }
  free(num);

  // if(isVersion){
  //   sprintf(errmsg, "%s %s\n", progname, version);
  //   return;
  // }
  // else if(isError){
  //   sprintf(errmsg, "%s ERROR\n", progname);
  //   return;
  // }

  return;
//   const char *saveopt = opt;
//   char oc;
//   int n, r;

//   if (*opt == '-') ++opt;

//   if (!strcmp(opt, "version")) {
//     sprintf(errmsg, "%s %s\n", progname, version);
//     return;
//   }

//   oc = *opt;

//   if (isdigit(oc)) {
//     if (!strtoudec(opt, &n)) goto badopt;
//     if (n <= 8) *pprefix = n;
//     else *pwidth = n;
//   }
//   else {
//     if (!oc) goto badopt;
//     n = 1;
//     r = strtoudec(opt + 1, &n);
//     if (opt[1] && !r) goto badopt;

//     if (oc == 'w' || oc == 'p' || oc == 's') {
//       if (!r) goto badopt;
//       if      (oc == 'w') *pwidth  = n;
//       else if (oc == 'p') *pprefix = n;
//       else                *psuffix = n;
//     }
//     else if (oc == 'h') *phang = n;
//     else if (n <= 1) {
//       if      (oc == 'l') *plast = n;
//       else if (oc == 'm') *pmin = n;
//     }
//     else goto badopt;
//   }

//   *errmsg = '\0';
//   return;

// badopt:
//   sprintf(errmsg, "Bad option: %.149s\n", saveopt);
}


static char **readlines(void)

/* Reads lines from stdin until EOF, or until a blank line is encountered, */
/* in which case the newline is pushed back onto the input stream. Returns */
/* a NULL-terminated array of pointers to individual lines, stripped of    */
/* their newline characters. Uses errmsg, and returns NULL on failure.     */
{
  struct buffer *cbuf = NULL, *pbuf = NULL;
  int c, blank;
  char ch, *ln, *nullline = NULL, nullchar = '\0', **lines = NULL;

  cbuf = newbuffer(sizeof (char));
  if (is_error()) goto rlcleanup;
  pbuf = newbuffer(sizeof (char *));
  if (is_error()) goto rlcleanup;

  for (blank = 1;  ; ) {
    c = getchar();
    if (c == EOF) break;
    if (c == '\n') {
      if (blank) {
        ungetc(c,stdin);
        break;
      }
      additem(cbuf, &nullchar);
      if (is_error()) goto rlcleanup;
      ln = copyitems(cbuf);
      if (is_error()) goto rlcleanup;
      additem(pbuf, &ln);
      if (is_error()) goto rlcleanup;
      clearbuffer(cbuf);
      blank = 1;
    }
    else {
      if (!isspace(c)) blank = 0;
      ch = c;
      additem(cbuf, &ch);
      if (is_error()) goto rlcleanup;
    }
  }

  if (!blank) {
    additem(cbuf, &nullchar);
    if (is_error()) goto rlcleanup;
    ln = copyitems(cbuf);
    if (is_error()) goto rlcleanup;
    additem(pbuf, &ln);
    if (is_error()) goto rlcleanup;
  }

  additem(pbuf, &nullline);
  if (is_error()) goto rlcleanup;
  lines = copyitems(pbuf);

rlcleanup:

  if (cbuf) freebuffer(cbuf);
  if (pbuf) {
    if (!lines)
      for (;;) {
        lines = nextitem(pbuf);
        if (!lines) break;
        free(*lines);
      }
    freebuffer(pbuf);
  }

  return lines;
}


static void setdefaults(
  const char * const *inlines, int *pwidth, int *pprefix,
  int *psuffix, int *phang, int *plast, int *pmin
)
/* If any of *pwidth, *pprefix, *psuffix, *phang, *plast, *pmin are     */
/* less than 0, sets them to default values based on inlines, according */
/* to "par.doc". Does not use errmsg because it always succeeds.        */
{
  int numlines;
  const char *start, *end, * const *line, *p1, *p2;

  if (*pwidth < 0) *pwidth = 72;
  if (*phang < 0) *phang = 0;
  if (*plast < 0) *plast = 0;
  if (*pmin < 0) *pmin = *plast;

  for (line = inlines;  *line;  ++line);
  numlines = line - inlines;

  if (*pprefix < 0){
    if (numlines <= *phang + 1)
      *pprefix = 0;
    else {
      start = inlines[*phang];
      for (end = start;  *end;  ++end);
      for (line = inlines + *phang + 1;  *line;  ++line) {
        for (p1 = start, p2 = *line;  p1 < end && *p1 == *p2;  ++p1, ++p2);
        end = p1;
      }
      *pprefix = end - start;
    }
  }

  if (*psuffix < 0){
    if (numlines <= 1)
      *psuffix = 0;
    else {
      start = *inlines;
      for (end = start;  *end;  ++end);
      for (line = inlines + 1;  *line;  ++line) {
        for (p2 = *line;  *p2;  ++p2);
        for (p1 = end;
             p1 > start && p2 > *line && p1[-1] == p2[-1];
             --p1, --p2);
        start = p1;
      }
      while (end - start >= 2 && isspace(*start) && isspace(start[1])) ++start;
      *psuffix = end - start;
    }
  }
}


static void freelines(char **lines)
/* Frees the strings pointed to in the NULL-terminated array lines, then */
/* frees the array. Does not use errmsg because it always succeeds.      */
{
  char **line;
  for (line = lines;  *line;  ++line){
    char *tl = *line;
    free(tl);
  }

  free(lines);
}


int original_main(int argc, char * const *argv)
{
  int width, widthbak = -1, prefix, prefixbak = -1, suffix, suffixbak = -1,
      hang, hangbak = -1, last, lastbak = -1, min, minbak = -1, c;
  char *parinit, *picopy = NULL, *opt, **inlines = NULL, **outlines = NULL,
       **line;
  const char * const whitechars = " \f\n\r\t\v";

  //putenv("PARINIT=bin/par -w 20 3 --last --min -h");
  char **otherArgv = malloc(0);
  parinit = getenv("PARINIT");
  if (parinit) {
    picopy = malloc((strlen(parinit) + 1) * sizeof (char));
    if (!picopy) {
      set_error("Out of memory.\n");
      goto parcleanup;
    }
    int numArgs = 0;
    strcpy(picopy,parinit);
    opt = strtok(picopy,whitechars);
    while (opt) {
      numArgs++;
      otherArgv = realloc(otherArgv, sizeof(char *)*numArgs);
      otherArgv[numArgs-1] = opt;

      // parseopt(argc, opt, &widthbak, &prefixbak,
      //          &suffixbak, &hangbak, &lastbak, &minbak);
      opt = strtok(NULL,whitechars);
    }
    parseopt(numArgs, otherArgv, &widthbak, &prefixbak,
               &suffixbak, &hangbak, &lastbak, &minbak);
    if (is_error()) goto parcleanup;
    free(picopy);
    free(otherArgv);
    picopy = NULL;
    otherArgv = NULL;
  }

  optind = 1;
  parseopt(argc, argv, &widthbak, &prefixbak,
           &suffixbak, &hangbak, &lastbak, &minbak);
  if (is_error()) goto parcleanup;
  // printf("widthbak = %d\n", widthbak);
  // printf("prefixbak = %d\n", prefixbak);
  // printf("suffixbak = %d\n", suffixbak);
  // printf("hangbak = %d\n", hangbak);
  // printf("lastbak = %d\n", lastbak);
  // printf("minbak = %d\n", minbak);


  for (;;) {
    for (;;) {
      c = getchar();
      if (c != '\n') break;
      putchar(c);
    }
    if(c == EOF)
      break;
    ungetc(c,stdin);

    inlines = readlines();
    if (is_error()) goto parcleanup;
    if (!*inlines) {
      free(inlines);
      inlines = NULL;
      continue;
    }

    width = widthbak;  prefix = prefixbak;  suffix = suffixbak;
    hang = hangbak;  last = lastbak;  min = minbak;
    setdefaults((const char * const *) inlines,
                &width, &prefix, &suffix, &hang, &last, &min);

    outlines = reformat((const char * const *) inlines,
                        width, prefix, suffix, hang, last, min);
    if (is_error()) goto parcleanup;

    freelines(inlines);
    inlines = NULL;

    for (line = outlines;  *line;  ++line)
      puts(*line);

    freelines(outlines);
    outlines = NULL;
  }

parcleanup:

  if (picopy) free(picopy);
  if (otherArgv) free(otherArgv);
  if (inlines) freelines(inlines);
  if (outlines) freelines(outlines);

  if (is_error()) {
    report_error(stderr);
    clear_error();
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
