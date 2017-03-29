#include <math.h>

#include "gtihmi.h"
#include "gpdefs.h"
#include "logconf.h"

chosenmsg *insertchosenmsg(chosenmsg *entry1, int newdata, double newvalue)
{
    chosenmsg *newlyinserted;
    newlyinserted = (chosenmsg*) malloc(sizeof(chosenmsg));

    newlyinserted->rlink = entry1->rlink;
    newlyinserted->llink = entry1;
    newlyinserted->data = newdata;
    newlyinserted->value = newvalue;

    entry1->rlink = (void*) newlyinserted;
    ((chosenmsg*) newlyinserted->rlink)->llink =(void*) newlyinserted;
    //logwriteln(debugfilename,"creating a new node: ");
    //debugprintnodeinfo(newlyinserted);

    return newlyinserted;
}

chosenmsg *findbydata(chosenmsg *start, int searchfor)
{
    sprintf(debugbuffer,"searching linked list for data: %d", searchfor);
    logwriteln(debugfilename,debugbuffer);

    chosenmsg *current = start;
    do{
        current = moveright(current);
        if(current->data == searchfor)
        {
            logprintfint("found match returning pointer to %d", (int) current);
            return current;
        }
    }while(current != start);
    logwriteln(debugfilename,"couldn't find a match");
    return start;
}

chosenmsg *setvalue(chosenmsg *start, int searchfor, double newvalue)
{
    chosenmsg *searchptr;

    searchptr = findbydata(start, searchfor);

    if(searchptr == start)
    {
        logwriteln(debugfilename,"unable to update value");
        return start;
    }
    sprintf(debugbuffer,"preparing to write data %f to node at %d",newvalue,(int) searchptr);
    logwriteln(debugfilename,debugbuffer);
    searchptr->value = newvalue;
    return searchptr;
}

int removechosenmsg(chosenmsg *entry)
{
    ((chosenmsg*) entry->llink)->rlink = entry->rlink;
    ((chosenmsg*) entry->rlink)->llink = entry->llink;
    free(entry);

    return 0;
}

chosenmsg *moveright(chosenmsg *start)
{
    return (chosenmsg*) (start->rlink);
}

void traverseright(chosenmsg *start,void(*doforeach)(chosenmsg*))
{
    chosenmsg *current = start;
    do{
        current = moveright(current);
        doforeach(current);
    }while(current != start);

}

void clearlist(chosenmsg *perm)
{
    chosenmsg *current;
    current = moveright(perm);
    while(current->data != -1)
    {
        removechosenmsg(current);
        current = moveright(perm);
    }
}

int dlllength(chosenmsg *start)
{
    int count = 0;
    chosenmsg *current = start;
    do{
        current = moveright(current);
        if(current->data != -1)
        {
            count++;
        }
    }while(current != start);

    return count;
}

void traverserightandremove(chosenmsg *start,void(*doforeach)(chosenmsg*),int remindex)
{
    chosenmsg *current = start;
    chosenmsg *temp;
    do{
        current = moveright(current);
        if((remindex >= 0) && (current->data == remindex))
        {
            temp = moveright(current);
            removechosenmsg(current);
            current = temp;
        }
        doforeach(current);
    }while(current != start);
}

void debugprintnodeinfo(chosenmsg *node)
{
    char dbgbuffer[1024];
    snprintf(dbgbuffer, 1024,"STARTNEW NODE\n    data: %d\n    value: %f\n    loc: %d\n    rlink: %d\n    llink: %d",node->data, node->value,(int) node, (int) node->rlink, (int) node->llink);
    logwriteln(debugfilename,dbgbuffer);
}

void debugprintnodedata(chosenmsg *node)
{
    char dbgbuffer[1024];
    snprintf(dbgbuffer, 1024, "data: %d", node->data);
    logwriteln(debugfilename,dbgbuffer);
}

chosenmsg *createnewchosendllist(void)
{
    chosenmsg *permentry;
    permentry = malloc(sizeof(chosenmsg));

    permentry->data = -1;
    permentry->rlink = permentry;
    permentry->llink = permentry;

    return permentry;
}

void flipbytes(void *ptr, int length)
{
    unsigned char *buffer;
    int i;

    for(i=0;i < (int)length/2; i++)
    {
        memcpy(&buffer,ptr+i,1);
        memcpy(ptr+i,ptr+length-1-i,1);
        memcpy(ptr+length-1-i,&buffer,1);
    }
}

int lookupbycode(unsigned short int code)
{
    int i;
    for(i = 0; i < sizeof(signallist)/sizeof(signallist[0]); i++)
    {
        if(code == signallist[i].code)
        {
            return i;
        }
    }
    return -2;
}
