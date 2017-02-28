#include <stdlib.h>
#include <stdio.h>
#include <gtk.h>

#include "gpdefs.h"
#include "gtihmi.h"


//global gui pointers

GtkWidget *window;
GtkWidget *fixed;

GtkWidget *signallistbox;
GtkWidget *signalscroll;
GtkWidget *chosensignallistbox;
GtkWidget *chosenscroll;
GtkWidget *choosesignalbutton;
GtkWidget *rejectsignalbutton;
GtkWidget *newinverterframe;
GtkWidget *newinverterbox;
GtkWidget *newmacbox;
GtkWidget *newipbox;
GtkWidget *createbutton;

GtkWidget *newnameentry;
GtkWidget *newnamelabel;
GtkWidget *newmaclabel;
GtkWidget *dashlabel1;
GtkWidget *dashlabel2;
GtkWidget *dashlabel3;
GtkWidget *dashlabel4;
GtkWidget *dashlabel5;
GtkWidget *dotlabel1;
GtkWidget *dotlabel2;
GtkWidget *dotlabel3;
GtkWidget *newiplabel;
GtkWidget *newmacbyte1;
GtkWidget *newmacbyte2;
GtkWidget *newmacbyte3;
GtkWidget *newmacbyte4;
GtkWidget *newmacbyte5;
GtkWidget *newmacbyte6;
GtkWidget *newipoctet1;
GtkWidget *newipoctet2;
GtkWidget *newipoctet3;
GtkWidget *newipoctet4;

GtkWidget *signalname;

//debugging globals
FILE *debuglog;
FILE *datalog;
char debugbuffer[128];


static void activate(GtkApplication *app, gpointer user_data);
int choosesignalcallback(GtkWidget *widget, gpointer data);
int unchoosesignalcallback(GtkWidget *widget, gpointer data);
int addnewinverter(GtkWidget *widget, gpointer data);


int main(int argc, char *argv[])
{
    GtkApplication *app;

    //gui dimensional variables



    int retval;


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

    app = gtk_application_new("sevelevlabs.gti.hmi",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(activate), NULL);
    retval = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    printf("process closed with retval: %d",retval);

    snprintf(debugbuffer, 128, "GTK application finishes with retval: %d",retval);
    logwriteln(debuglog,debugbuffer);

    retval = fclose(datalog);
    snprintf(debugbuffer,128,"data log file closed with return value: %d",retval);
    logwriteln(debuglog,debugbuffer);

    logwriteln(debuglog,"getting ready to close debug log file... bye!");
    fclose(debuglog);

}

static void activate(GtkApplication* app, gpointer user_data)
{
    int i;

    int horz_marg = 20;
    int top_marg = 5;
    int horz_dim = 900;
    int vert_dim = 600;
    int frame_width = horz_dim-2*horz_marg;
    int frame_height = 80;
    int sigbox_width = 180;
    int sigbox_height = 300;
    int sigbox_space = 20;
    int std_but_width = 80;
    int std_but_height = 10;
    int std_but_space_y = 10;
    int sm_but_width = 20;
    int sm_but_height = 10;
    int sm_but_space_y = 5;

    int sig_row_y = top_marg + frame_height + 5;
    int chosen_box_xpos = horz_marg + sigbox_width + std_but_width + 2*sigbox_space;
    int chosen_but_xpos = horz_marg + sigbox_width + sigbox_space;
    int chosen_top_ypos = sig_row_y + 20;
    int chosen_bot_ypos = chosen_top_ypos + std_but_height + 2*std_but_space_y;



    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTI interface");
    gtk_window_set_default_size(GTK_WINDOW(window), horz_dim, vert_dim);

    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    signalscroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_fixed_put(GTK_FIXED(fixed),signalscroll, horz_marg, sig_row_y);
    gtk_widget_set_size_request(signalscroll,sigbox_width,sigbox_height);

    chosenscroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_fixed_put(GTK_FIXED(fixed),chosenscroll,chosen_box_xpos, sig_row_y);
    gtk_widget_set_size_request(chosenscroll,sigbox_width,sigbox_height);

    signallistbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(signalscroll),signallistbox);

    //build signal list
    for(i = 0; i < sizeof(signallist)/sizeof(signallist[0]); i++)
    {
        signalname = gtk_label_new(signallist[i].name);
        gtk_list_box_insert(signallistbox,signalname,i);
    }

    chosensignallistbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(chosenscroll),chosensignallistbox);

    choosesignalbutton = gtk_button_new_with_label(">>");
    gtk_fixed_put(GTK_FIXED(fixed),choosesignalbutton,chosen_but_xpos,chosen_top_ypos);
    gtk_widget_set_size_request(choosesignalbutton,std_but_width,std_but_height);
    g_signal_connect(choosesignalbutton,"clicked",G_CALLBACK(choosesignalcallback), NULL);

    rejectsignalbutton = gtk_button_new_with_label("<<");
    gtk_fixed_put(GTK_FIXED(fixed),rejectsignalbutton,chosen_but_xpos,chosen_bot_ypos);
    gtk_widget_set_size_request(rejectsignalbutton,std_but_width,std_but_height);
    g_signal_connect(rejectsignalbutton,"clicked",G_CALLBACK(unchoosesignalcallback),NULL);

    newinverterframe = gtk_frame_new("New Inverter");
    gtk_fixed_put(GTK_FIXED(fixed),newinverterframe,horz_marg,top_marg);
    gtk_widget_set_size_request(newinverterframe,frame_width,frame_height);

    newinverterbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,15);
    gtk_container_add(GTK_CONTAINER(newinverterframe),newinverterbox);

    newipbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,3);
    gtk_container_add(GTK_CONTAINER(newinverterbox),newipbox);
    newmacbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,3);
    gtk_container_add(GTK_CONTAINER(newinverterbox),newmacbox);

    newnamelabel = gtk_label_new("Name:");
    gtk_container_add(GTK_CONTAINER(newinverterbox),newnamelabel);
    //gtk_widget_set_size_request(newnamelabel,std_but_width,std_but_height);

    newnameentry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newinverterbox),newnameentry);
    gtk_widget_set_size_request(newnameentry,std_but_width,std_but_height);

    newmaclabel = gtk_label_new("MAC:");
    gtk_container_add(GTK_CONTAINER(newmacbox),newmaclabel);
    dashlabel1 = gtk_label_new("-");
    dashlabel2 = gtk_label_new("-");
    dashlabel3 = gtk_label_new("-");
    dashlabel4 = gtk_label_new("-");
    dashlabel5 = gtk_label_new("-");
    dotlabel1 = gtk_label_new(".");
    dotlabel2 = gtk_label_new(".");
    dotlabel3 = gtk_label_new(".");

    newmacbyte1 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte1);
    gtk_entry_set_width_chars(newmacbyte1,2);
    gtk_entry_set_max_length(newmacbyte1,2);
    gtk_container_add(GTK_CONTAINER(newmacbox),dashlabel1);
    newmacbyte2 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte2);
    gtk_entry_set_width_chars(newmacbyte2,2);
    gtk_entry_set_max_length(newmacbyte2,2);
    gtk_container_add(GTK_CONTAINER(newmacbox),dashlabel2);
    newmacbyte3 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte3);
    gtk_entry_set_width_chars(newmacbyte3,2);
    gtk_entry_set_max_length(newmacbyte3,2);
    gtk_container_add(GTK_CONTAINER(newmacbox),dashlabel3);
    newmacbyte4 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte4);
    gtk_entry_set_width_chars(newmacbyte4,2);
    gtk_entry_set_max_length(newmacbyte4,2);
    gtk_container_add(GTK_CONTAINER(newmacbox),dashlabel4);
    newmacbyte5 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte5);
    gtk_entry_set_width_chars(newmacbyte5,2);
    gtk_entry_set_max_length(newmacbyte5,2);
    gtk_container_add(GTK_CONTAINER(newmacbox),dashlabel5);
    newmacbyte6 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newmacbox),newmacbyte6);
    gtk_entry_set_width_chars(newmacbyte6,2);
    gtk_entry_set_max_length(newmacbyte6,2);

    newiplabel = gtk_label_new("IP:");
    gtk_container_add(GTK_CONTAINER(newipbox),newiplabel);
    newipoctet1 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newipbox),newipoctet1);
    gtk_entry_set_width_chars(newipoctet1,3);
    gtk_entry_set_max_length(newipoctet1,3);
    gtk_container_add(GTK_CONTAINER(newipbox),dotlabel1);
    newipoctet2 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newipbox),newipoctet2);
    gtk_entry_set_width_chars(newipoctet2,3);
    gtk_entry_set_max_length(newipoctet2,3);
    gtk_container_add(GTK_CONTAINER(newipbox),dotlabel2);
    newipoctet3 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newipbox),newipoctet3);
    gtk_entry_set_width_chars(newipoctet3,3);
    gtk_entry_set_max_length(newipoctet3,3);
    gtk_container_add(GTK_CONTAINER(newipbox),dotlabel3);
    newipoctet4 = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(newipbox),newipoctet4);
    gtk_entry_set_width_chars(newipoctet4,3);
    gtk_entry_set_max_length(newipoctet4,3);

    createbutton = gtk_button_new_with_label("CREATE");
    gtk_container_add(GTK_CONTAINER(newinverterbox),createbutton);
    gtk_widget_set_size_request(createbutton,std_but_width,std_but_height);
    g_signal_connect(createbutton,"clicked",G_CALLBACK(addnewinverter),NULL);

    gtk_widget_show_all(window);
}

int choosesignalcallback(GtkWidget *widget, gpointer data)
{
    GtkWidget *transferrow;
    GtkWidget *addrow;
    int index;

    transferrow = gtk_list_box_get_selected_row(signallistbox);
    index = (int) gtk_list_box_row_get_index(transferrow);

    addrow = gtk_label_new(signallist[index].name);
    gtk_list_box_insert(chosensignallistbox,addrow,0);

    //maintain log
    snprintf(debugbuffer,128,"adding component to message: %s",signallist[index].name);
    logwriteln(debuglog,debugbuffer);
    //make new line visible
    gtk_widget_show_all(chosensignallistbox);
    return 0;
}

int unchoosesignalcallback(GtkWidget *widget, gpointer data)
{
    GtkWidget *removerow;
    int index;

    removerow = gtk_list_box_get_selected_row(chosensignallistbox);
    index = (int) gtk_list_box_row_get_index(removerow);
    gtk_container_remove(chosensignallistbox,removerow);

    //maintain log
    snprintf(debugbuffer,128,"removing component from message: %d",index);
    logwriteln(debuglog,debugbuffer);

    return 0;
}

int addnewinverter(GtkWidget *widget, gpointer data)
{
    int ipt1;
    int ip2;
    int ip3;
    int ip4;

    char mac1[2];
    char mac2[2];
    char mac3[2];
    char mac4[2];
    char mac5[2];
    char mac6[2];

    return 0;
}
