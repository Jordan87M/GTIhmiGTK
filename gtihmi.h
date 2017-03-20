#ifndef __GTIHMIAPP_H
#define __GTIHMIAPP_H

#include <gtk.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "gtihmi.h"
#include "logconf.h"

#define MAX_N_INVERTERS             10
#define MAX_NAME_LENGTH             48
#define MAX_MSG_COMP                5

#define DATA_COLLECTION_INTERVAL    1000 //in ms

typedef struct chosenmsg_t{
    int data;
    double value;
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
    short int lastseqnum;
    chosenmsg *msgtypelistperm;
    chosenmsg *reclistperm;
    struct sockaddr_in server;

} GTIinfo;

typedef struct configstore_t{
    chosenmsg *loadedmsgs;
    GTIinfo loadedgtilist[MAX_N_INVERTERS];
}configstore;

//extern chosenmsg *chosenperm;

extern char debugfilename[64];
extern char datafilename[64];


extern char debugbuffer[LOG_BUFFER_SIZE];


chosenmsg *insertchosenmsg(chosenmsg *entry1, int newdata, double newvalue);
int removechosenmsg(chosenmsg *entry);
chosenmsg *moveright(chosenmsg *start);
void traverseright(chosenmsg *start,void(*doforeach)(chosenmsg*));
void traverserightandremove(chosenmsg *start,void(*doforeach)(chosenmsg*),int remindex);
void debugprintnodeinfo(chosenmsg *node);
void debugprintnodedata(chosenmsg *node);
chosenmsg *createnewchosendllist(void);
int dlllength(chosenmsg *start);
chosenmsg *findbydata(chosenmsg *start, int searchfor);
chosenmsg *setvalue(chosenmsg *start, int searchfor, double newvalue);
void flipbytes(void *ptr, int length);
int removeinverterfromgtilist(int index);
int makeinverterlistfromstruct(GTIinfo *gtilist);
void addmsgfromstruct(chosenmsg *chosen);



#endif
