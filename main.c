#include <stdlib.h>
#include <stdio.h>
#include <gtk.h>

#include "gpdefs.h"
#include "gtihmi.h"


FILE *debuglog;
FILE *datalog;

int main(int argc, char *argv[])
{
    int retval;
    char debugbuffer[128];

    char debugfilename[64];
    char datafilename[64];
    char namebuffer[64];
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    snprintf(namebuffer,64,"%s",asctime(timeinfo));
    replaceandclean(namebuffer,' ','_');
    snprintf(debugfilename,64,"debuglog%s",namebuffer);
    snprintf(datafilename,64,"datalog%s.csv",namebuffer);


    debuglog = fopen(debugfilename,"w");
    datalog = fopen(datafilename,"w");

    //create arrays of signal structures
    setupsignals();
    makefullsignallist();

    //print signal list for debugging
    printfullsignallist(debuglog);

    retval = g_application_run(G_APPLICATION(gtihmi_app_new()), argc, argv);
    printf("process closed with retval: %d",retval);

    snprintf(debugbuffer, 128, "GTK application finishes with retval: %d",retval);
    logwriteln(debuglog,debugbuffer);

    retval = fclose(datalog);
    snprintf(debugbuffer,128,"data log file closed with return value: %d",retval);
    logwriteln(debuglog,debugbuffer);

    logwriteln(debuglog,"getting ready to close debug log file... bye!");
    fclose(debuglog);

}
