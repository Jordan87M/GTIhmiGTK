#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>

#include "logconf.h"
#include "gtihmi.h"


int logwriteln(char *filename, const char *line)
{
    FILE *fp;
    fp = fopen(filename,"a");
    int retval;
    double us;

    time_t now;
    struct tm *timeinfo;
    char timebuff[64];
    struct timespec tsnow;
    struct timespec tselapsed;



    time(&now);
    clock_gettime(0,&tsnow);
    timeinfo = localtime(&now);
    snprintf(timebuff, 64, "%s",asctime(timeinfo));
    replaceandclean(timebuff,' ','_');
    elapsedtime(&tsnow,&tselapsed);
    us = tselapsed.tv_sec*1000000 + tselapsed.tv_nsec*.001;
    retval = fprintf(fp, "\n%s (%f): %s",timebuff, us, line);
    fclose(fp);

    return retval;
}

int datawriteln(char *filename, const char *line)
{
    FILE *fp;
    fp = fopen(filename,"a");
    int retval;

    retval = fprintf(fp, "\n%s",line);
    fclose(fp);

    return retval;
}

int timestampfilename(char *tbuffer)
{
    time_t now;
    struct tm *timeinfo;
    char timebuff[64];
    int retval;

    time(&now);
    timeinfo = localtime(&now);
    retval = snprintf(timebuff, 64, "%s",asctime(timeinfo));
    replaceandclean(timebuff,' ','_');
    replaceandclean(timebuff,':','-');

    return retval;
}


int logprintfstring(char *format, char *string)
{
    int retval;
    snprintf(debugbuffer,1024,format,string);
    retval = logwriteln(debugfilename,debugbuffer);
    return retval;
}

int logprintfint(char *format, int data)
{
    int retval;
    snprintf(debugbuffer,1024,format,data);
    retval = logwriteln(debugfilename,debugbuffer);
    return retval;
}

int replaceandclean(char *strptr, char from, char to)
{
    int i = 0;
    for(i = 0; i< strlen(strptr); i++)
    {
        if(*(strptr + i) == from)
        {
            *(strptr + i) = to;
        }

        if(*(strptr + i) == '\n' || *(strptr + i) == '/r')
        {
            *(strptr + i) = '\0';
        }
    }
    return 0;
}

int replace(char *strptr, char from, char to)
{
    int i = 0;
    for(i = 0; i< strlen(strptr); i++)
    {
        if(*(strptr + i) == from)
        {
            *(strptr + i) = to;
        }
    }
    return 0;
}

int debugprinthex(unsigned char *hexstring, int numbytes)
{
    int i;
    char outbuffer[512];
    for(i = 0; i < numbytes; i++)
    {
        sprintf(outbuffer, "%#04x ", hexstring[i]);
        strcat(debugbuffer,outbuffer);
        //logwriteln(debugfilename,debugbuffer);
    }
    logwriteln(debugfilename,debugbuffer);
}
