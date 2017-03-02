#ifndef LOGCONF_H
#define LOGCONF_H

#include <stdio.h>

#define LOG_BUFFER_SIZE     512

extern FILE *debuglog;
extern FILE *datalog;

int logwriteln(FILE *fp, const char *line);
int replaceandclean(char *strptr, char from, char to);
int replace(char *strptr, char from, char to);

#endif
