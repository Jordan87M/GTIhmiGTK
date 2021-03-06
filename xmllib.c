#include <stdio.h>

#include "xmllib.h"
#include "gtihmi.h"
#include "logconf.h"
#include "gpdefs.h"


int saveinverterconfig(GTIinfo *gtilist, chosenmsg *chosenperm,char *filename)
{
    int i;
    FILE *fp;


    fp = fopen(filename,"w");
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

    //open message components
    fprintf(fp,"\n    <msgs>");


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
    retval = fprintf(fp,"\n        <inverter>\n            <name>%s </name>\n            <ipaddr>%s </ipaddr>\n            <macaddr>%s </macaddr>\n        </inverter>",name,ipaddr,macaddr);
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

void loadinverterconfig(char *filename, chosenmsg *loadedmsgs, GTIinfo *loadedgtilist)
{
    FILE *fp;
    FILE *fpdebug;
    readstatus status;
    long int filesize;
    long int position = 0;

    char linebuffer[256];

    char parsedebugfilename[] = "parse.log";
    char parsedebugbuffer[256];
    int retval;
    int i;




    //initialize counter for new inverters
    int invcreateindex = 0;
    //before proceeding, clear current conifguration, if any
    //first remove inverters
    for(i = 0; i< MAX_N_INVERTERS; i++)
    {
        removeinverterfromgtilist(i);
    }
    //now remove message components
    clearlist(loadedmsgs);
    //initialize pointer to current message element
    chosenmsg *currentmsgptr = loadedmsgs;

    logwriteln(debugfilename,"cleared old message list, see?");
    traverseright(loadedmsgs,debugprintnodeinfo);

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
    logwriteln(parsedebugfilename,"***************************************NEW");

    //fprintf(fpdebug,"\n\n***************************************\n NEW RUN\n*****************************");
    //fpdebug = fopen(parsedebugfilename,"a");


    fp = fopen(filename,"r");
    fseek(fp,0L,SEEK_END);
    filesize = ftell(fp);

    rewind(fp);
    sprintf(debugbuffer,"reading a file that is %d bytes long",filesize);
    logwriteln(debugfilename,debugbuffer);

    while(position < filesize)
    {
        fpdebug = fopen(parsedebugfilename,"a");
        //read a line
        fgets(linebuffer,256,fp);
        status.lineno++;

        retval = processline(linebuffer, &status, loadedmsgs, loadedgtilist, currentmsgptr, &currentmsgptr, &invcreateindex, fpdebug);

        position = ftell(fp);

        //fprintf(fpdebug,"\nposition: %d, lineno: %d",position, status.lineno);
        //sprintf(parsedebugbuffer,"\nposition: %d, lineno: %d",position, status.lineno);
        //logwriteln(parsedebugfilename,parsedebugbuffer);
        //fprintf(fpdebug," processline() return code: %d", retval);
        fclose(fpdebug);

    }

    return;
}

int processline(const char *line, readstatus *status, chosenmsg *loadedmsgs, GTIinfo *loadedgtilist, chosenmsg *currentmsgptr, chosenmsg **currentptrptr, int *invcreateindex, FILE *fpdebug)
{
    int len = strlen(line);
    int i;
    int notspace = 0;
    int xmlopenpresent = 0;
    int xmlquestionpresent = 0;
    char *retval = 0;
    char parsedebugbuffer[256];
    char retstring[256];
    chosenmsg *tempmsgptr;

    //printf("status : %d", status->lineno);
    //fprintf(fpdebug,"line: %s",line);

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
        //fprintf(fpdebug,"\n%c : %d compared to %c : %d", *(line + i), (int) *(line + i), '<', (int) '<');

        if(*(line+i) != ' ')
        {

            notspace = 1;
        }


        if(*(line + i) == '<')
        {
            xmlopenpresent = 1;
            //fprintf(fpdebug,"found <");
        }

        if(xmlopenpresent == i)
        {
            if(*(line+i) == '?')
            {
                xmlquestionpresent = 1;
            }
        }
    }

    if(xmlopenpresent == 1)
    {
        retval = strstr(line,"<format>0.1</format>");
        if(retval != NULL)
        {
            status->formatversionverified = 1;
            fprintf(fpdebug,"\n format version 0.1");
            return 0;
        }

        if(xmlquestionpresent == 1)
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
            return 0;
        }

    }

    if(notspace == 0)
    {
        fprintf(fpdebug,"\n line is just spaces");
        return 0;
    }

    if(xmlopenpresent != 1)
    {
        return NOT_XML_LINE;
    }

    if(status->xmlverified == 0 || status->xmlversionverified == 0 || status->encodingverified == 0)
    {
        fprintf(fpdebug,"\nignoring line, file not xml");
        return NOT_YET_VALIDATED;
    }

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
            return SHOULD_HAVE_QUIT;
        }
    }
    else
    {
        //are we looking at inverters...
        if(status->readinginverters == 1)
        {
            //are we already looking at a specific inverter?
            if(status->readinganinverter == 1)
            {
                retval = strstr(line,"<name>");
                if(retval != NULL)
                {
                    readstringbetweentags(line,"name",retstring);
                    strcpy(loadedgtilist[*invcreateindex].name,retstring);
                    fprintf(fpdebug,"\nadding inverter name attribute at line %d to %d: %s",status->lineno, *invcreateindex, loadedgtilist[*invcreateindex].name);
                    return 0;
                }

                retval = strstr(line,"<ipaddr>");
                if(retval != NULL)
                {
                    readstringbetweentags(line,"ipaddr",retstring);
                    strcpy(loadedgtilist[*invcreateindex].ipaddr,retstring);
                    loadedgtilist[*invcreateindex].server.sin_addr.s_addr = inet_addr(retstring);
                    loadedgtilist[*invcreateindex].server.sin_family = AF_INET;
                    loadedgtilist[*invcreateindex].server.sin_port = htons(GP_PORT);
                    fprintf(fpdebug,"\ninverter ipaddr attribute at line %d to %d: %s",status->lineno, *invcreateindex, loadedgtilist[*invcreateindex].ipaddr);
                    return 0;
                }

                retval = strstr(line,"<macaddr>");
                if(retval != NULL)
                {
                    readstringbetweentags(line,"macaddr",retstring);
                    strcpy(loadedgtilist[*invcreateindex].macaddr, retstring);
                    fprintf(fpdebug,"\ninverter macaddr attribute at line %d to %d: %s",status->lineno, *invcreateindex, loadedgtilist[*invcreateindex].macaddr);
                    return 0;
                }

                retval = strstr(line,"</inverter>");
                if(retval != NULL)
                {
                    fprintf(fpdebug,"\nclosing an inverter");
                    status->readinganinverter = 0;
                    loadedgtilist[*invcreateindex].newdata = 0;
                    loadedgtilist[*invcreateindex].extant = 1;
                    fprintf(fpdebug,"\nmarking inverter %d as active", *invcreateindex);
                    //increment counter for loaded inverters
                    int newinvindex = (*invcreateindex) + 1;
                    fprintf(fpdebug,"\nnextindex : %d",newinvindex);
                    memcpy(invcreateindex,&newinvindex, sizeof(int));
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
                    loadedgtilist[*invcreateindex].msgtypelistperm = createnewchosendllist();
                    loadedgtilist[*invcreateindex].reclistperm = createnewchosendllist();
                    fprintf(fpdebug,"\nopening a new inverter");
                    return 0;
                }

                retval = strstr(line,"</inverters>");
                if(retval != NULL)
                {
                    status->readinginverters = 0;
                    fprintf(fpdebug,"\nfinished reading inverters");
                    return 0;
                }
                return MISPLACED_ITEM;

            }
        }//or messages...
        else if(status->readingmessagecomps == 1)
        {
            //are we already looking at a specific message component
            if(status->readingamessagecomp == 1)
            {
                retval = strstr(line,"<index>");
                if(retval != NULL)
                {
                    sprintf(debugbuffer,"writing to msgptr: %d",(int) currentmsgptr);
                    logwriteln(debugfilename,debugbuffer);
                    currentmsgptr->data = readintbetweentags(line,"index");
                    fprintf(fpdebug,"\nreading component index at line %d : %d",status->lineno, currentmsgptr->data);
                    return 0;
                }

                retval = strstr(line,"<value>");
                if(retval != NULL)
                {
                    currentmsgptr->value = readdoublebetweentags(line,"value");
                    fprintf(fpdebug,"\nreading component value at line %d : %f",status->lineno, currentmsgptr->value);
                    return 0;
                }

                retval = strstr(line,"</cmp>");
                if(retval != NULL)
                {
                    fprintf(fpdebug,"\nclosing a message component");
                    //logwriteln(debugfilename,"building message component list... here it is now:");
                    //traverseright(currentmsgptr,debugprintnodeinfo);
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
                    tempmsgptr = insertchosenmsg(loadedmsgs,0,0);
                    //sprintf(debugbuffer,"current value of tempmsgptr: %d", (int) tempmsgptr);
                    //logwriteln(debugfilename,debugbuffer);

                    memcpy(currentptrptr,&tempmsgptr,sizeof(void*));
                    //sprintf(debugbuffer,"current value of msgptr: %d", (int) currentmsgptr);
                    //logwriteln(debugfilename,debugbuffer);

                    fprintf(fpdebug,"\nopening a new message");
                    return 0;
                }

                //is this the end of the messages portion?
                retval = strstr(line,"</msgs>");
                if(retval != NULL)
                {
                    fprintf(fpdebug,"\ndone reading messages");
                    status->readingmessagecomps = 0;
                    return 0;
                }

                return MISPLACED_ITEM;
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
            return MISSING_SECTION;
        }
    }
}

int readintbetweentags(char *line, char *tag)
{
    char format[128];
    char clean[128];
    const char type[] = "%d";
    int retval;

    sprintf(format,"<%s>%s</%s>",tag,type,tag);

    stripleadingspaces(line,clean);
    sscanf(clean,format,&retval);
    //sprintf(debugbuffer,"%d",retval);
    //logwriteln(debugfilename,debugbuffer);
    return retval;
}

double readdoublebetweentags(char *line, char *tag)
{
    char format[128];
    char clean[128];
    const char type[] = "%lf";
    double retval;


    //logwriteln(debugfilename,line);
    //logwriteln(debugfilename,tag);
    sprintf(format,"<%s>%s</%s>\n",tag,type,tag);
    logwriteln(debugfilename,format);
    stripleadingspaces(line,clean);

    //sprintf(debugbuffer,"%s",clean);
    //logwriteln(debugfilename,debugbuffer);

    sscanf(clean,format,&retval);
    //sprintf(debugbuffer,"double returned %lf",retval);
    //logwriteln(debugfilename,debugbuffer);
    return retval;
}

void readstringbetweentags(char *line, char *tag, char *retstring)
{
    char format[128];
    char clean[128];
    const char type[] = "%s";
    char retval[128];

    sprintf(format,"<%s>%s</%s>",tag,type,tag);
    stripleadingspaces(line,clean);

    sscanf(clean,format,retstring);
    //sprintf(debugbuffer," returned string: %s",retstring);
    //logwriteln(debugfilename,debugbuffer);
}

void stripleadingspaces(char *line, char *clean)
{
    int i = 0;
    int j = 0;
    int shouldcopy = 0;

    //sprintf(debugbuffer,"length of input: %d", strlen(line));
    //logwriteln(debugfilename,debugbuffer);


    for(i = 0; i<strlen(line); i++)
    {
        if(*(line + i) != ' ')
        {
            strcpy(clean,line + i);
            //logwriteln(debugfilename,clean);
            return;
        }

    }

}
