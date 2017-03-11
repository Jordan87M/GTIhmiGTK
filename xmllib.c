#include <stdio.h>

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

    //open message components
    fprintf(fp,"\n    <msgs>");

    //close inverters
    fprintf(fp,"\n    </inverters>");
    writemessageconfigs(fp,chosenperm);

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
    retval = fprintf(fp,"\n        <inverter>\n            <name>%s</name>\n            <ipaddr>%s</ipaddr>\n            <macaddr>%s</macaddr>\n        </inverter>\n",name,ipaddr,macaddr);
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

int loadinverterconfig(char *filename)
{
    FILE *fp;
    FILE *fpdebug;
    readstatus status;
    long int filesize;
    long int position;

    char linebuffer[256];

    //state tracking variables
    status.xmlverified = 0;
    status.xmlversionverified = 0;
    status.encodingverified = 0;
    status.formatversionverified = 0;
    status.readingconfig = 0;
    status.readinginverters = 0;
    status.readinganinverter = 0;
    status.invertersdone = 0;
    status.readingmessagecomps = 0;
    status.messagecompsdone = 0;
    status.readingamessagecomp = 0;
    status.done = 0;
    status.lineno = 0;

    //debugging file to test parser
    fpdebug = fopen("parsedebug.log","a");
    fprintf(fpdebug,"\n\n***************************************\n NEW RUN\n*****************************");

    fp = fopen(filename,"r");
    fseek(fp,0L,SEEK_END);
    filesize = ftell(fp);

    rewind(fp);

    while(position < filesize)
    {
        //read a line
        fgets(linebuffer,256,fp);
        status.lineno++;

        processline(linebuffer, &status, fpdebug);

        position = ftell(fp);

    }

}

int processline(const char *line, readstatus *status, FILE *fpdebug)
{
    int len = strlen(line);
    int i;
    int notspace = 0;
    int xmlopenpresent = 0;
    char *retval = 0;


    //check for empty line
    if(len == 0)
    {
        fprintf(fpdebug,"\nempty line - just new line");
        //just skip to the next line
        return 0;
    }
    //preprocessing - scan line for formatting info
    for(i = 0; i < len; i++)
    {
        if(*(line+i) != ' ')
        {
            notspace = 1;
        }

        if(*(line + 1) == '<')
        {
            xmlopenpresent = 1;
        }

        if(xmlopenpresent == 1)
        {
            if(*(line+i) == '?')
            {
                retval = strstr(line,"xml");
                if(retval != NULL)
                {
                    status->xmlverified = 1;
                    fprintf(fpdebug,"\n file is an xml file");
                }

                retval = strstr(line,"version=\"1.0\"");
                if(retval != NULL)
                {
                    status->xmlversionverified = 1;
                    fprintf(fpdebug,"\n xml version 1.0");
                }

                retval = strstr(line,"encoding=\"UTF-8\"");
                if(retval != NULL)
                {
                    status->encodingverified = 1;
                    fprintf(fpdebug,"\n encoding is UTF-8");
                }
            }
        }
    }

    if(status->xmlverified == 0 || status->xmlversionverified == 0 || status->encodingverified == 0)
    {
        return NOT_YET_VALIDATED;
    }

    if(xmlopenpresent != 1)
    {
        return -1;
    }

    if(notspace == 1)
    {
        if(status->readingconfig == 0)
        {
            if(status->done == 0)
            {
                fprintf(fpdebug,"\nlooking for an config opening tag");
                retval = strstr(line,"<config>");
                if(retval == NULL)
                {
                    fprintf(fpdebug," - not here");
                }
                else
                {
                    fprintf(fpdebug," - and here it is");
                    status->readingconfig = 1;
                    return 0;
                }


            }
            else
            {
                fprintf(fpdebug,"\nwe're done, why haven't we quit?");
                //why are we here?
                return 0;
            }
        }
        else
        {
            //are we looking at inverters...
            if(status->readinginverters == 1)
            {
                //are we already looking at a specific inverter?
                if(status->readinganinverter)
                {
                    retval = strstr(line,"<name>");
                    if(retval != NULL)
                    {
                        fprintf(fpdebug,"\ninverter name attribute at line %d",status->lineno);
                        return 0;
                    }

                    retval = strstr(line,"<ipaddr>");
                    if(retval != NULL)
                    {
                        fprintf(fpdebug,"\ninverter ipaddr attribute at line %d",status->lineno);
                        return 0;
                    }

                    retval = strstr(line,"<macaddr>");
                    if(retval != NULL)
                    {
                        fprintf(fpdebug,"\ninverter macaddr attribute at line %d",status->lineno);
                        return 0;
                    }

                    retval = strstr(line,"</inverter>");
                    if(retval != NULL)
                    {
                        fprintf(fpdebug,"\nclosing an inverter");
                        status->readinganinverter = 0;
                        return 0;
                    }

                    fprintf(fpdebug,"\nreceived unexpected inverter attribute or didn't close inverter properly at line %d",status->lineno);
                    return UNEXPECTED_ATTRIBUTE;
                }
                else
                {
                    //does this line open an inverter
                    retval = strstr(line,"<inverter>");
                    if(retval != NULL)
                    {
                        status->readinganinverter = 1;
                        fprintf(fpdebug,"\nopening a new inverter");
                        return 0;
                    }
                    return -1;

                }
            }//or messages...
            else if(status->readingmessagecomps == 1)
            {
                //are we already looking at a specific message component
                if(status->readingamessagecomp)
                {
                    retval = strstr(line,"<index>");
                    if(retval != NULL)
                    {
                        readintbetweentags(line,"index");
                        fprintf(fpdebug,"\nreading component index at line %d",status->lineno);
                        return 0;
                    }

                    retval = strstr(line,"<value>");
                    if(retval != NULL)
                    {
                        readdoublebetweentags(line,"value");
                        fprintf(fpdebug,"\nreading component value at line %d",status->lineno);
                        return 0;
                    }

                    retval = strstr(line,"</cmp>");
                    if(retval != NULL)
                    {
                        fprintf(fpdebug,"\closing a message component");
                        status->readingamessagecomp = 0;
                        return 0;
                    }

                    fprintf(fpdebug,"\n received an unexpected component attribute or a component was not closed properly");
                    return UNEXPECTED_ATTRIBUTE;

                }
                else
                {
                    //does this line open a message?
                    retval = strstr(line,"<cmp>");
                    if(retval != NULL)
                    {
                        status->readingamessagecomp = 1;
                        fprintf(fpdebug,"\nopening a new message");
                        return 0;
                    }
                    return -1;
                }
            }//or nothing yet.
            else
            {
                //are we starting to look at inverters?
                retval = strstr(line,"<inverters>");
                if(retval != NULL)
                {
                    fprintf(fpdebug,"\nfound beginning of inverters at line %d",status->lineno);
                    status->readinginverters = 1;
                    return 0;
                }

                //or messages
                retval = strstr(line,"<msgs>");
                if(retval != NULL)
                {
                    status->readingmessagecomps = 1;
                    fprintf(fpdebug,"\nfound beginning of message components at line %d",status->lineno);
                    return 1;
                }

                //or are we done?
                retval = strstr(line,"</config>");
                if(retval != NULL)
                {
                    fprintf(fpdebug,"\nfound end of configs at line %d. \nNow we're done.",status->lineno);
                    status->done = 1;
                    return 2;
                }

                fprintf(fpdebug,"\nfailed to figure out what to do next");
                return -1;
            }

        }
    }
    else
    {
        fprintf(fpdebug,"\nempty line - just spaces");
        return 0;
    }


}

int readintbetweentags(char *line, const char *tag)
{
    char *format;
    const char *type = "%d";
    int retval;

    sprintf(format,"<%s>%s</%s>",tag,type,tag);

    sscanf(line,format,retval);
    return retval;
}

double readdoublebetweentags(char *line, const char *tag)
{
    char *format;
    const char *type = "%f";
    int retval;

    sprintf(format,"<%s>%s</%s>",tag,type,tag);

    sscanf(line,format,retval);
    return retval;
}

char *readstringbetweentags(char *line, const char *tag)
{
    char *format;
    const char *type = "%s";
    char *retval;

    sprintf(format,"<%s>%s</%s>",tag,type,tag);

    sscanf(line,format,retval);
    return retval;
}
