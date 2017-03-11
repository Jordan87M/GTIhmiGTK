#ifndef XMLLIB_H
#define XMLLIB_H

#define formatvmajor     0
#define formatvminor     1


#include "xmllib.h"
#include "gtihmi.h"
#include "logconf.h"

#define NOT_YET_VALIDATED           -1
#define REACHED_EOF_UNEXPECTEDLY    -2
#define UNEXPECTED_ATTRIBUTE        -3



typedef struct readstatus_t{
    unsigned char xmlverified;
    unsigned char xmlversionverified;
    unsigned char encodingverified;
    unsigned char formatversionverified;
    unsigned char readingconfig;
    unsigned char readinginverters;
    unsigned char readinganinverter;
    unsigned char invertersdone;
    unsigned char readingmessagecomps;
    unsigned char messagecompsdone;
    unsigned char readingamessagecomp;
    unsigned char done;
    int lineno;

}readstatus;

int saveinverterconfig(GTIinfo *gtilist, chosenmsg *chosenperm);
int writeinverterconfig(FILE *fp,char *name,char *ipaddr,char *macaddr);
int writemessageconfigs(FILE *fp, chosenmsg *chosenperm);
int loadinverterconfig(char *filename);
int processline(const char *line, readstatus *status, FILE *fpdebug);
int readintbetweentags(char *line, const char *tag);
double readdoublebetweentags(char *line, const char *tag);
char *readstringbetweentags(char *line, const char *tag);



#endif // XMLLIB_H

