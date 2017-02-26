#include <time.h>

#include "logconf.h"


int logwriteln(FILE *fp, const char *line)
{
    time_t now;
    struct tm *timeinfo;
    char timebuff[64];
    time(&now);
    timeinfo = localtime(&now);
    snprintf(timebuff, 64, "%s",asctime(timeinfo));
    replaceandclean(timebuff,' ','_');

    return fprintf(fp, "\n%s: %s",timebuff, line);
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
