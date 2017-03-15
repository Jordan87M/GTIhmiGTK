#ifndef XMLLIB_H
#define XMLLIB_H

#define formatvmajor     0
#define formatvminor     1


#include "xmllib.h"
#include "gtihmi.h"
#include "logconf.h"

#define NOT_YET_VALIDATED           -101
#define REACHED_EOF_UNEXPECTEDLY    -201
#define UNEXPECTED_ATTRIBUTE        -301
#define NOT_XML_LINE                -401
#define SHOULD_HAVE_QUIT            -501
#define MISPLACED_ITEM              -601
#define MISSING_SECTION             -701



#define PARSEDEBUGFILENAME          "parse.log"



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

int saveinverterconfig(GTIinfo *gtilist, chosenmsg *chosenperm, char *filename);
int writeinverterconfig(FILE *fp,char *name,char *ipaddr,char *macaddr);
int writemessageconfigs(FILE *fp, chosenmsg *chosenperm);
void loadinverterconfig(char *filename,chosenmsg *chosenperm, GTIinfo *gtilist);
int processline(const char *line, readstatus *status, chosenmsg *loadedmsgs, GTIinfo *loadedgtilist, chosenmsg *currentmsgptr, chosenmsg **currentptrptr, int *invcreateindex, FILE *fpdebug);
int readintbetweentags(char *line, char *tag);
double readdoublebetweentags(char *line, char *tag);
void readstringbetweentags(char *line, char *tag, char *retstring);
void stripleadingspaces(char *line, char *clean);



#endif // XMLLIB_H

