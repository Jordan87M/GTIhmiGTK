
#include "xmllib.h"
#include "gtihmi.h"
#include "logconf.h"


int saveinverterconfig(GTIinfo *gtilist, chosenmsg *chosenperm)
{
    int i;
    FILE *fp;

    fp = fopen("tempname.config","w");
    fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    fprintf(fp,"\n<format>%d.%d</format>",formatvmajor,formatvminor);

    //open config
    fprintf(fp,"\n<config>");
    //open inverters
    fprintf(fp,"\n    <inverters>");

    for(i = 0; i<MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            writeinverterconfig(fp,gtilist[i].name,gtilist[i].ipaddr,gtilist[i].macaddr);
        }
    }
    //close inverters
    fprintf(fp,"\n    </inverters>");
    writemessageconfigs(fp,chosenperm);
    //open message components
    fprintf(fp,"\n    <msgs>");


    //close message components
    fprintf(fp,"\n    </msgs>");
    //close config
    fprintf(fp,"\n</config>");
    fclose(fp);
    return 0;
}


int writeinverterconfig(FILE *fp,char *name,char *ipaddr,char *macaddr)
{
    int retval;
    retval = fprintf(fp,"\n        <inverter>\n            <name>%s</name>\n            <ipaddr>%s</ipaddr>\n            <macaddr>%s</macaddr>\n        </inverter>\n");
    return retval;
}

int writemessageconfigs(FILE *fp, chosenmsg *chosenperm)
{
    chosenmsg *current = chosenperm;
    do{
        current = moveright(current);
        if(current->data != -1)
        {
            fprintf(fp,"\n        <cmp>\n            <index>%d</index>\n            <value>%f</value>\n        </cmp>", current->data, current->value);
        }
    }while(current != chosenperm);
}
