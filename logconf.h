#ifndef LOGCONF_H
#define LOGCONF_H

#include <stdio.h>

#define LOG_BUFFER_SIZE     512

//extern FILE *debuglog;
//extern FILE *datalog;

#include <time.h>

#include "logconf.h"


int logwriteln(char *filename, const char *line);
int logprintfstring(char *format, char *string);
int logprintfint(char *format, int data);
int replaceandclean(char *strptr, char from, char to);
int replace(char *strptr, char from, char to);
int debugprinthex(unsigned char *hexstring, int numbytes);

#endif
