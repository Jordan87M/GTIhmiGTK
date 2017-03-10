#ifndef XMLLIB_H
#define XMLLIB_H

#define formatvmajor     0
#define formatvminor     1


#include "xmllib.h"
#include "gtihmi.h"
#include "logconf.h"


int saveinverterconfig(GTIinfo *gtilist, chosenmsg *chosenperm);
int writeinverterconfig(FILE *fp,char *name,char *ipaddr,char *macaddr);
int writemessageconfigs(FILE *fp, chosenmsg *chosenperm);


#endif // XMLLIB_H

