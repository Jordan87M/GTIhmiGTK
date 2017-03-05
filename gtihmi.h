#ifndef __GTIHMIAPP_H
#define __GTIHMIAPP_H

#include <gtk.h>


#define MAX_N_INVERTERS         10
#define MAX_NAME_LENGTH         48
#define MAX_MSG_COMP            5


typedef struct chosenmsg_t{
    int data;
    void *llink;
    void *rlink;
} chosenmsg;

typedef struct GTIinfo_t{
    char name[MAX_NAME_LENGTH];
    char ipaddr[15];
    char macaddr[17];
    int oneoffenable;
    int schedenable;
    int extant;
    chosenmsg *msgtypelistperm;

} GTIinfo;

//extern chosenmsg *chosenperm;

extern char debugfilename[64];
extern char datafilename[64];

#include <math.h>

#include "gtihmi.h"

chosenmsg *insertchosenmsg(chosenmsg *entry1, int newdata);
int removechosenmsg(chosenmsg *entry);
chosenmsg *moveright(chosenmsg *start);
void traverseright(chosenmsg *start,void(*doforeach)(chosenmsg*));
void traverserightandremove(chosenmsg *start,void(*doforeach)(chosenmsg*),int remindex);
void debugprintnodeinfo(chosenmsg *node);
void debugprintnodedata(chosenmsg *node);
chosenmsg *createnewchosendllist(void);




#endif
