#include <math.h>

#include "gtihmi.h"

chosenmsg *insertchosenmsg(chosenmsg *entry1, int newdata)
{
    chosenmsg *newlyinserted;
    newlyinserted = (chosenmsg*) malloc(sizeof(chosenmsg));

    newlyinserted->rlink = entry1->rlink;
    newlyinserted->llink = entry1;
    newlyinserted->data = newdata;

    entry1->rlink = (void*) newlyinserted;
    ((chosenmsg*) newlyinserted->rlink)->llink =(void*) newlyinserted;
    debugprintnodeinfo(newlyinserted);

    return newlyinserted;
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
    snprintf(dbgbuffer, 1024,"STARTNEW NODE\n    data: %d\n    loc: %d\n    rlink: %d\n    llink: %d",node->data, (int) node, (int) node->rlink, (int) node->llink);
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


