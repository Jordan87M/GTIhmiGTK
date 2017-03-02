#ifndef __GTIHMIAPP_H
#define __GTIHMIAPP_H

#include <gtk.h>

#define MAX_N_INVERTERS         10
#define MAX_NAME_LENGTH         48
#define MAX_MSG_COMP            5

typedef struct GTIinfo_t{
    char name[MAX_NAME_LENGTH];
    char ipaddr[15];
    char macaddr[17];
    int oneoffenable;
    int schedenable;
    int extant;

} GTIinfo;

#endif
