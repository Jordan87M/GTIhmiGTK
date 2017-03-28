
#include <time.h>
#include <stdio.h>
#include <gtk.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

//#include <fcntl-linux.h>

#include "gpdefs.h"
#include "gtihmi.h"
#include "logconf.h"




//global gui pointers
GtkWidget *savedialog;
GtkWidget *loaddialog;

GtkListStore *store;
GtkTreeIter iter;
GtkCellRenderer *renderer;
GtkTreeModel *model;

GtkListStore *sigstore;
GtkTreeIter sigiter;
GtkCellRenderer *sigrenderer;
GtkTreeModel *sigmodel;

GtkListStore *recstore;
GtkTreeIter reciter;
GtkCellRenderer *recrenderer;
GtkTreeModel *recmodel;

GtkWidget *window;
GtkWidget *fixed;

GtkWidget *menubar;
GtkWidget *filedrop;
GtkWidget *filemenubutton;
GtkWidget *filemenuvbox;
GtkWidget *fromfile;
GtkWidget *tofile;

GtkWidget *signallistbox;
GtkWidget *signalscroll;
GtkWidget *chosensignaltreeview;
GtkWidget *chosenscroll;
GtkWidget *choosesignalbutton;
GtkWidget *rejectsignalbutton;
GtkWidget *newinverterframe;
GtkWidget *newinverterbox;
GtkWidget *newmacbox;
GtkWidget *newipbox;
GtkWidget *createbutton;
GtkWidget *removebutton;
GtkWidget *activetree;
GtkWidget *sendbutton;
GtkWidget *startcollectionbutton;
GtkWidget *stopcollectionbutton;
GtkWidget *recscroll;
GtkWidget *rectreeview;

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
char debugfilename[64];
char datafilename[64];
char debugbuffer[LOG_BUFFER_SIZE];



//other globals
extern int errno;
GTIinfo gtilist[MAX_N_INVERTERS];
int selected[MAX_N_INVERTERS];
int chosen[MAX_MSG_COMP];
//int socket_desc;
int timersetupflag = 0;
int timerteardownflag = 0;
int socketsetupflag = 0;
int chosenrecipient;
chosenmsg *chosenperm;
chosenmsg *schedmsgperm;
gpheader globalhead = {.magic = GP_MAGIC,
                        .seqnum = INITIAL_SEQNUM,
                        .context = INVERTER_CONTEXT
                        };
//guint timersource;
timer_t schedtimer;
pthread_t listenerthread;
pthread_mutex_t sendrecprotector[MAX_N_INVERTERS];
pthread_mutex_t sendmsgprotector;
pthread_mutex_t dispaktprotector;
pthread_mutex_t logprotector;
pthread_mutex_t llprotector;
void (*sendandreceive[MAX_N_INVERTERS])(void);



int inverterinsertindex = 0;
enum
{
    COL_NAME = 0,
    COL_IPADDR,
    COL_MACADDR,
    COL_VRMS,
    COL_IRMS,
    COL_POWREAC,
    COL_POWREAL,
    COL_PHASE,
    COL_FREQ
};

enum
{
    COL_INDEX =0,
    COL_SIGNAME,
    COL_VALUE
};

enum
{
    COL_RECNAME = 0,
    COL_RECCODE,
    COL_RECVALUE,
};

static void activate(GtkApplication *app, gpointer user_data);
int opensocket(void);
int choosesignalcallback(GtkWidget *widget, gpointer data);
int unchoosesignalcallback(GtkWidget *widget, gpointer data);
int addnewinverter(GtkWidget *widget, gpointer data);
int removeinverter(GtkWidget *wdiget, gpointer data);
int getindexfromip(char* ipaddr);
int sendoneoff(GtkWidget *widget, gpointer data);
//gboolean sendregularcollectionmessage(void);
void sendregularcollectionmessage(void);
void checkrecbufferspawner(int sig);
void sendregularcollectionmessagespawner(int sig);
void value_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *newtext, gpointer user_data);
int saveconfigtofile(void);
int loadconfigfromfile(void);
int startcollection(void);
int stopcollection(void);
void checkrecbuffer(int);
void disassemblepacket(unsigned char *buffer, int index);
void listenerthreadroutine(void);
gboolean displayupdate(gpointer nothing);

void messagehandler(void);
void messagehandler0(void);
void messagehandler1(void);
void messagehandler2(void);
void messagehandler3(void);
void messagehandler4(void);
void messagehandler5(void);
void messagehandler6(void);
void messagehandler7(void);
void messagehandler8(void);
void messagehandler9(void);

int main(int argc, char *argv[])
{
    GtkApplication *app;

    //gui dimensional variables
    int retval;
    int i;


    sprintf(debugfilename,"debuglog");
    logwriteln(debugfilename,"\n**********************\nBEGINNING NEW SESSION!\n********************\n");

    //create arrays of signal structures
    setupsignals();
    makefullsignallist();

    //create message for regular data collection
    schedmsgperm = createnewchosendllist();
    insertchosenmsg(schedmsgperm, 20, 0.0);
    insertchosenmsg(schedmsgperm, 25, 0.0);
    insertchosenmsg(schedmsgperm, 26, 0.0);
    insertchosenmsg(schedmsgperm, 27, 0.0);
    insertchosenmsg(schedmsgperm, 28, 0.0);
    insertchosenmsg(schedmsgperm, 29, 0.0);

    //print signal list for debugging
    printfullsignallist(debugfilename);

    //initialize structure for message components to be sent
    chosenperm = createnewchosendllist();
    debugprintnodeinfo(chosenperm);
    for(i = 0; i< MAX_N_INVERTERS; i++)
    {
        gtilist[i].msgtypelistperm = createnewchosendllist();
    }

    //spawn thread to listen to signals -- must be done before opensocket()
    logwriteln(debugfilename,"about to launch listener thread");
    pthread_create(&listenerthread,NULL,listenerthreadroutine, NULL);

    sendandreceive[0] = messagehandler0;
    sendandreceive[1] = messagehandler1;
    sendandreceive[2] = messagehandler2;
    sendandreceive[3] = messagehandler3;
    sendandreceive[4] = messagehandler4;
    sendandreceive[5] = messagehandler5;
    sendandreceive[6] = messagehandler6;
    sendandreceive[7] = messagehandler7;
    sendandreceive[8] = messagehandler8;
    sendandreceive[9] = messagehandler9;


    app = gtk_application_new("sevelevlabs.gti.hmi",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(activate), NULL);
    retval = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    printf("process closed with retval: %d",retval);


    snprintf(debugbuffer, 128, "GTK application finishes with retval: %d",retval);
    logwriteln(debugfilename,debugbuffer);

    logwriteln(debugfilename,"bye!");

    pthread_exit(NULL);
    return;
}

static void activate(GtkApplication* app, gpointer user_data)
{
    int i;

    //variables to help coordinate widget positioning
    int horz_marg = 20;
    int top_marg = 25;
    int horz_dim = 900;
    int vert_dim = 620;
    int frame_width = horz_dim-2*horz_marg;
    int frame_height = 60;
    int sigbox_width = 180;
    int sigbox_height = 240;
    int sigbox_space = 20;
    int sigview_height = sigbox_height;
    int sigview_width = 240;
    int std_but_width = 80;
    int std_but_height = 30;
    int std_but_space_y = 5;
    int std_but_space_x = 10;
    int sm_but_width = 20;
    int sm_but_height = 10;
    int sm_but_space_y = 5;
    int active_view_height = 260;
    int active_view_width = horz_dim - 2*horz_marg;

    int sig_row_y = top_marg + frame_height + 10;
    int chosen_box_xpos = horz_marg + sigbox_width + std_but_width + 2*sigbox_space;
    int chosen_but_xpos = horz_marg + sigbox_width + sigbox_space;
    int chosen_top_ypos = sig_row_y + 20;
    int chosen_bot_ypos = chosen_top_ypos + std_but_height + 2*std_but_space_y;
    int active_view_ypos = sig_row_y + sigbox_height + 20;
    int remove_but_xpos = horz_dim - std_but_width - horz_marg;
    int remove_but_ypos = active_view_ypos - std_but_height - std_but_space_y;
    int send_but_xpos = chosen_box_xpos + sigview_width + sigbox_space;
    int send_but_ypos = remove_but_ypos - std_but_height - std_but_space_y;
    int start_but_xpos = send_but_xpos + std_but_width + std_but_space_x;
    int start_but_ypos = send_but_ypos;
    int stop_but_xpos = start_but_xpos + std_but_width + std_but_space_x;
    int stop_but_ypos = send_but_ypos;
    int recscroll_ypos = sig_row_y;
    int recscroll_xpos = send_but_xpos;
    int recscroll_width = horz_dim - horz_marg - recscroll_xpos;
    int recscroll_height = 180;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTI interface");
    gtk_window_set_default_size(GTK_WINDOW(window), horz_dim, vert_dim);

    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    filemenuvbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(fixed),filemenuvbox);

    menubar = gtk_menu_bar_new();
    filedrop = gtk_menu_new();
    filemenubutton = gtk_menu_item_new_with_label("File");
    fromfile = gtk_menu_item_new_with_label("Load");
    tofile = gtk_menu_item_new_with_label("Save");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(filemenubutton),filedrop);
    gtk_menu_shell_append(GTK_MENU_SHELL(filedrop),tofile);
    gtk_menu_shell_append(GTK_MENU_SHELL(filedrop),fromfile);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),filemenubutton);
    gtk_box_pack_start(GTK_BOX(filemenuvbox),menubar,FALSE,FALSE, 0);

    g_signal_connect(G_OBJECT(tofile),"activate",G_CALLBACK(saveconfigtofile), NULL);
    g_signal_connect(G_OBJECT(fromfile),"activate",G_CALLBACK(loadconfigfromfile),NULL);


    signalscroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_fixed_put(GTK_FIXED(fixed),signalscroll, horz_marg, sig_row_y);
    gtk_widget_set_size_request(signalscroll,sigbox_width,sigbox_height);

    chosenscroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_fixed_put(GTK_FIXED(fixed),chosenscroll,chosen_box_xpos, sig_row_y);
    gtk_widget_set_size_request(chosenscroll,sigview_width,sigview_height);

    recscroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_fixed_put(GTK_FIXED(fixed),recscroll,recscroll_xpos,recscroll_ypos);
    gtk_widget_set_size_request(recscroll,recscroll_width,recscroll_height);

    signallistbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(signalscroll),signallistbox);

    //build signal list
    for(i = 0; i < sizeof(signallist)/sizeof(signallist[0]); i++)
    {
        signalname = gtk_label_new(signallist[i].name);
        gtk_list_box_insert(signallistbox,signalname,i);
    }

    chosensignaltreeview = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(chosenscroll),chosensignaltreeview);

    rectreeview = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(recscroll),rectreeview);

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
    gtk_entry_set_max_length(newnameentry,MAX_NAME_LENGTH);

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

    removebutton = gtk_button_new_with_label("REMOVE");
    gtk_fixed_put(fixed, removebutton, remove_but_xpos, remove_but_ypos);
    gtk_widget_set_size_request(removebutton,std_but_width, std_but_height);
    g_signal_connect(removebutton,"clicked",G_CALLBACK(removeinverter),NULL);

    sendbutton = gtk_button_new_with_label("SEND");
    gtk_fixed_put(fixed, sendbutton, send_but_xpos, send_but_ypos);
    gtk_widget_set_size_request(sendbutton,std_but_width, std_but_height);
    g_signal_connect(sendbutton,"clicked",G_CALLBACK(sendoneoff),NULL);

    startcollectionbutton = gtk_button_new_with_label("START");
    gtk_fixed_put(fixed, startcollectionbutton, start_but_xpos, start_but_ypos);
    gtk_widget_set_size_request(startcollectionbutton, std_but_width, std_but_height);
    g_signal_connect(startcollectionbutton, "clicked",G_CALLBACK(startcollection),NULL);

    stopcollectionbutton = gtk_button_new_with_label("STOP");
    gtk_fixed_put(fixed, stopcollectionbutton, stop_but_xpos, stop_but_ypos);
    gtk_widget_set_size_request(stopcollectionbutton, std_but_width, std_but_height);
    g_signal_connect(stopcollectionbutton, "clicked", G_CALLBACK(stopcollection),NULL);

    //store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    store = gtk_list_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
    activetree = gtk_tree_view_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 0, "Name", renderer, "text", COL_NAME, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 1, "IP Address", renderer, "text", COL_IPADDR, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 2, "MAC Address", renderer,"text", COL_MACADDR, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 3, "Phase Offset", renderer,"text", COL_PHASE, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 4, "Voltage", renderer, "text", COL_VRMS, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 5, "Current", renderer, "text", COL_IRMS, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 6, "Real Power", renderer, "text", COL_POWREAL, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 7, "Reactive Power", renderer,"text", COL_POWREAC, NULL);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 8, "Frequency", renderer,"text", COL_FREQ, NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);

    gtk_fixed_put(fixed,activetree,horz_marg, active_view_ypos);
    gtk_widget_set_size_request(activetree, active_view_width, active_view_height);

    sigstore = gtk_list_store_new(3,G_TYPE_INT,G_TYPE_STRING,G_TYPE_DOUBLE);
    sigrenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(chosensignaltreeview),0,"Index",sigrenderer,"text",COL_INDEX,NULL);
    sigrenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(chosensignaltreeview),1,"Name",sigrenderer,"text",COL_SIGNAME,NULL);
    sigrenderer = gtk_cell_renderer_text_new();
    g_object_set(sigrenderer,"editable",TRUE,NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(chosensignaltreeview),2,"Value",sigrenderer,"text",COL_VALUE,NULL);
    g_signal_connect(sigrenderer,"edited",value_edited_callback, NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(chosensignaltreeview),sigmodel);

    recstore = gtk_list_store_new(3,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_DOUBLE);
    recrenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rectreeview),0,"Name",recrenderer,"text",COL_RECNAME,NULL);
    recrenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rectreeview),1,"Type",recrenderer,"text",COL_RECCODE, NULL);
    recrenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rectreeview),2,"Value",recrenderer,"text",COL_RECVALUE, NULL);

    gdk_threads_add_idle(displayupdate,NULL);

    gtk_widget_show_all(window);

}

gboolean displayupdate(gpointer nothing)
{
    int i;
    double phase;
    double powreal;
    double powreac;
    double freq;
    double vrms;
    double irms;
    //update one off message display from list
    gtk_list_store_clear(recstore);


    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            pthread_mutex_lock(&llprotector);
            chosenmsg *current = gtilist[i].reclistperm;
            pthread_mutex_unlock(&llprotector);
            do{
                current = moveright(current);
                if(current->data != -1)
                {
                    gtk_list_store_append(recstore,&reciter);
                    gtk_list_store_set(recstore,&reciter,COL_RECNAME, gtilist[i].name, COL_RECCODE, signallist[current->data].name, COL_RECVALUE, current->value, -1);
                    recmodel = GTK_TREE_MODEL(recstore);
                    gtk_tree_view_set_model(GTK_TREE_VIEW(rectreeview),recmodel);

                    gtk_widget_show_all(rectreeview);
                }
            }while(current != gtilist[i].reclistperm);
        }
    }


    //update scheduled message values
    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {

           chosenmsg *current = gtilist[i].reclistperm;

            do{
                pthread_mutex_lock(&llprotector);
                current = moveright(current);
                pthread_mutex_unlock(&llprotector);
                if(signallist[current->data].code == RESPONSE_PHASE_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_PHASE,current->value,-1);
                    phase = current->value;
                }
                else if(signallist[current->data].code == RESPONSE_REACTIVE_POWER_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_POWREAC,current->value,-1);
                    powreac = current->value;
                }
                else if(signallist[current->data].code == RESPONSE_REAL_POWER_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_POWREAL,current->value,-1);
                    powreal = current-> value;
                }
                else if(signallist[current->data].code == RESPONSE_FREQUENCY_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_FREQ,current->value,-1);
                    freq = current->value;
                }
                else if(signallist[current->data].code == RESPONSE_OUTPUT_CURRENT_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_IRMS,current->value,-1);
                    irms = current->value;
                }
                else if(signallist[current->data].code == RESPONSE_OUTPUT_VOLTAGE_CODE)
                {
                    gtk_list_store_set(store,gtilist[i].inviter,COL_VRMS,current->value,-1);
                    vrms = current->value;
                }
            }while(current != gtilist[i].reclistperm);

            model = GTK_TREE_MODEL(store);
            gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);
            gtk_widget_show_all(window);

            //sprintf(fnbuffer,"inv%dlog",i);
            //fp = fopen(fnbuffer,"a");
            //fprintf(fp,"%f,%f,%f,%f,%f,%f\n",phase,vrms,irms,powreal,powreac,freq);
            //fclose(fp);
        }
    }


    return TRUE;
}

void listenerthreadroutine(void)
{
    sprintf(debugbuffer,"listener thread running with id %d",(unsigned int) listenerthread);
    logwriteln(debugfilename,debugbuffer);
    //just wait for signals
    while(1)
    {
        if(timersetupflag == 1)
        {
            struct sigevent sevp;
            struct sigaction sa;
            struct itimerspec new_value;

            sa.sa_handler = sendregularcollectionmessagespawner;
            //sa.sa_flags =

            new_value.it_interval.tv_nsec = 500000000;
            time_t sec = (time_t) 0;
            new_value.it_interval.tv_sec = sec;
            new_value.it_value.tv_nsec = 500000000;
            new_value.it_value.tv_sec = sec;

            #ifdef SYS_gettid
            pid_t tid = syscall(SYS_gettid);
            #else
            #error "SYS_gettid unavailable on this system"
            #endif

            sevp.sigev_notify = 4;         //SIGEV_THREAD_ID defined to 4 SIGEV_SIGNAL defined to 0 in siginfo.h
            sevp.sigev_signo = SIGRTMIN;
            //sevp.sigev_value.sigev_notify_thread_id = listenerthread;
            sevp._sigev_un._tid = tid;

            //see if using POSIX timers helps
            timer_create(1, &sevp, &schedtimer);    //CLOCK_MONOTONIC defined to 1 in uapi/linux/time.h

            //designate handler for SIGEV
            sigaction(SIGRTMIN,&sa,NULL);
            timer_settime(schedtimer,0,&new_value,NULL);


            timersetupflag = 0;
        }

        if(timerteardownflag == 1)
        {

            struct itimerspec disarm_value;

            struct itimerspec curr_time;

            timer_gettime(schedtimer, &curr_time);

            pthread_mutex_lock(&logprotector);
            sprintf(debugbuffer,"time left: %ds and %dns \nnumber of overruns: %d",curr_time.it_value.tv_sec, curr_time.it_value.tv_nsec, timer_getoverrun(schedtimer));
            logwriteln(debugfilename,debugbuffer);
            pthread_mutex_unlock(&logprotector);

            disarm_value.it_value.tv_nsec = 0;
            disarm_value.it_value.tv_sec = 0;

            timer_settime(schedtimer,0,&disarm_value,NULL);
            pthread_mutex_lock(&logprotector);
            logwriteln(debugfilename,"stopping regular data collection");
            pthread_mutex_unlock(&logprotector);

            timerteardownflag = 0;
        }

    }
}



void value_edited_callback(GtkCellRendererText *cell, gchar *path_string, gchar *newtext, gpointer user_data)
{
    GtkTreeIter activeiter;
    chosenmsg *chosenptr;
    int index;
    double newval;

    newval = strtod(newtext,NULL);
    sprintf(debugbuffer,"cell edited - path: %s, new value: %s",path_string, newtext);
    logwriteln(debugfilename,debugbuffer);

    gtk_tree_model_get_iter_from_string(sigmodel,&activeiter,path_string);
    gtk_list_store_set(sigmodel,&activeiter,COL_VALUE,newval,-1);
    sigmodel = GTK_TREE_MODEL(sigstore);
    gtk_tree_model_get(sigmodel,&activeiter,COL_INDEX,&index,-1);

    logwriteln(debugfilename,"updating message component struct value");

    chosenptr = setvalue(chosenperm,index,newval);
    debugprintnodeinfo(chosenptr);
    return;
}

int sendoneoff(GtkWidget *widget, gpointer data)
{
    //first find out which inverter is selected
    int index;
    int retval;
    int i;
    int listempty = 1;
    pthread_t newthread;

    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            listempty = 0;
        }
    }
    if(listempty == 1)
    {
        return -1;
    }

    index = getselectedactive();
    sprintf(debugbuffer,"selected index: %d", index);
    logwriteln(debugfilename,debugbuffer);
    if(index < 0)
    {
        logwriteln(debugfilename,"no inverter has been selected to receive the message");
        return -1;
    }
    //retval = sendmessagetoinverter(index, chosenperm);
    chosenrecipient = index;

    pthread_create(&newthread,NULL,messagehandler,NULL);

    return 0;
}

int startcollection(void)
{
    timerteardownflag = 0;
    timersetupflag = 1;

    return 0;
}



int stopcollection(void)
{
    //g_source_remove(timersource);

    timersetupflag = 0;
    timerteardownflag = 1;
    return 0;
}

void sendregularcollectionmessagespawner(int sig)
{

    int i;
    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            pthread_t newthread;
            pthread_create(&newthread,NULL,sendandreceive[i],NULL);

            pthread_mutex_lock(&logprotector);
            sprintf(debugbuffer,"send scheduled - new thread with id: %d",(unsigned int) newthread);
            logwriteln(debugfilename,debugbuffer);
            pthread_mutex_unlock(&logprotector);

        }
    }

    return;
}


int sendmessagetoinverter(int index, chosenmsg* compptr, int socket_desc)
{
    int i;
    unsigned char count = 0;
    int keepgoing = 1;
    int retval = 1;
    char sendbuffer[1024];
    chosenmsg *currentptr;
    int payloadsize;

    unsigned short int magic;
    unsigned char context;
    unsigned char ncomps;
    unsigned short int seqnum;
    unsigned short typecode;
    double val;


    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"preparing to send message to inverter: %s",gtilist[index].name);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    magic = htons(globalhead.magic);
    context = globalhead.context;
    seqnum = htons(globalhead.seqnum);


    //build message packet
    memcpy(sendbuffer + MAGIC_OFFSET,&magic,2);
    memcpy(sendbuffer + CONTEXT_OFFSET,&context,1);
    memcpy(sendbuffer + SEQN_OFFSET,&seqnum,2);

    payloadsize = 6;

    pthread_mutex_lock(&llprotector);
    pthread_mutex_lock(&logprotector);
    currentptr = moveright(chosenperm);
    pthread_mutex_unlock(&llprotector);
    pthread_mutex_unlock(&logprotector);

    while(keepgoing == 1)
    {
        //debugprintnodeinfo(currentptr);
        if(currentptr->data == -1)
        {
            pthread_mutex_lock(&logprotector);
            logwriteln(debugfilename,"exhausted message type list...");
            pthread_mutex_unlock(&logprotector);
            keepgoing = 0;
            break;
        }
        if(count > 10)
        {
            pthread_mutex_lock(&logprotector);
            logwriteln(debugfilename, "problem: too many message types in list, truncating message");
            pthread_mutex_unlock(&logprotector);
            keepgoing = 0;
            break;
        }
        //fill out message component related fields
        typecode = htons(signallist[currentptr->data].code);
        memcpy(sendbuffer + TYPE_OFFSET + count*MULTICOMP_OFFSET,&typecode,2);

        memcpy(&val,&(currentptr->value),8);
        flipbytes(&val,sizeof(val));

        //val = htobe64(val);
        //flipbytes(&val,8);
        memcpy(sendbuffer + VALUE_OFFSET + count*MULTICOMP_OFFSET,&val,8);
        count++;
        pthread_mutex_lock(&llprotector);
        pthread_mutex_lock(&logprotector);
        currentptr = moveright(currentptr);
        pthread_mutex_unlock(&llprotector);
        pthread_mutex_unlock(&logprotector);
        payloadsize += 10;
    }
    //now that all message components have been added, fill in the number of components in the header
    memcpy(sendbuffer + COMPN_OFFSET, &count,1);

    //print message in buffer as hex bytestream
    pthread_mutex_lock(&logprotector);
    debugprinthex(sendbuffer,payloadsize);
    pthread_mutex_unlock(&logprotector);

    //server.sin_addr.s_addr = inet_addr(gtilist[index].ipaddr);
    //server.sin_family = AF_INET;
    //server.sin_port = htons(GP_PORT);
    retval = sendto(socket_desc, sendbuffer, payloadsize,0,(struct sockaddr *)&gtilist[index].server,sizeof(gtilist[index].server));
    if(retval  == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"sendto() failed : %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"sendto() returns with value %d",retval);
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }

    gtilist[index].lastseqnum = seqnum;
    globalhead.seqnum++;

    return 0;
}


void disassemblepacket(unsigned char *buffer, int index)
{
    unsigned short int magic;
    unsigned char ncomps;
    unsigned char context;
    unsigned short int seqnum;

    unsigned short int responsetype;
    double responsevalue;
    int responsetypeindex;

    int i;

    memcpy(&magic,buffer+ MAGIC_OFFSET,2);
    magic = ntohs(magic);

    memcpy(&ncomps,buffer + COMPN_OFFSET,1);
    memcpy(&context,buffer + CONTEXT_OFFSET,1);

    memcpy(&seqnum,buffer + SEQN_OFFSET,2);

    //clear old received message components
    pthread_mutex_lock(&llprotector);
    pthread_mutex_lock(&logprotector);
    clearlist(gtilist[index].reclistperm);
    pthread_mutex_unlock(&llprotector);
    pthread_mutex_unlock(&logprotector);
    for(i = 0; i < ncomps; i++)
    {
        memcpy(&responsetype,buffer + TYPE_OFFSET + i*MULTICOMP_OFFSET,2);
        memcpy(&responsevalue,buffer + VALUE_OFFSET + i*MULTICOMP_OFFSET,8);

        responsetype = ntohs(responsetype);
        flipbytes(&responsevalue,8);

        pthread_mutex_lock(&llprotector);
        pthread_mutex_lock(&logprotector);
        responsetypeindex = lookupbycode(responsetype);
        insertchosenmsg(gtilist[index].reclistperm,responsetypeindex,responsevalue);
        pthread_mutex_unlock(&llprotector);
        pthread_mutex_unlock(&logprotector);
    }
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"processed message - about to exit thread %d",(unsigned int) pthread_self());
    logwriteln(debugfilename, debugbuffer);
    pthread_mutex_unlock(&logprotector);

    return;
}


int choosesignalcallback(GtkWidget *widget, gpointer data)
{

    GtkWidget *transferrow;
    int index;

    transferrow = gtk_list_box_get_selected_row(signallistbox);
    index = (int) gtk_list_box_row_get_index(transferrow);


    gtk_list_store_append(sigstore, &sigiter);
    gtk_list_store_set(sigstore,&sigiter, COL_INDEX, index, COL_SIGNAME, signallist[index].name, COL_VALUE, 0.0, -1);
    sigmodel = GTK_TREE_MODEL(sigstore);
    gtk_tree_view_set_model(GTK_TREE_VIEW(chosensignaltreeview),sigmodel);


    //maintain log
    snprintf(debugbuffer,128,"adding component to message: %s",signallist[index].name);
    logwriteln(debugfilename,debugbuffer);
    //make new line visible
    gtk_widget_show_all(window);

    //add to list of chosen messages
    insertchosenmsg(chosenperm,index,0);
    logwriteln(debugfilename,"here's our list of message types now:");
    traverseright(chosenperm,debugprintnodedata);


    return 0;
}



int unchoosesignalcallback(GtkWidget *widget, gpointer data)
{
    GtkTreeSelection *activesel;
    GtkTreeRowReference *activeselref;
    GtkTreePath *activeselpath;
    GtkTreeIter activeseliter;
    GtkTreeModel *activemodel;
    GList *activesellist;

    int index;
    char *signame;

    //find out which signal, if any, has been selected
    activesel = gtk_tree_view_get_selection(chosensignaltreeview);
    activesellist = gtk_tree_selection_get_selected(activesel, &activemodel, &activeseliter);
    gtk_tree_model_get(activemodel,&activeseliter,COL_INDEX,&index,COL_SIGNAME,&signame,-1);

    sprintf(debugbuffer,"data from signal selected for removal: %d %s",index, signame);
    logwriteln(debugfilename,debugbuffer);

    gtk_list_store_remove(sigstore, &activeseliter);

    //maintain log
    logwriteln(debugfilename,"here's what the message list looks like now");
    traverserightandremove(chosenperm,debugprintnodedata,index);

    return 0;
}

int addnewinverter(GtkWidget *widget, gpointer data)
{
    if(inverterinsertindex >= 0)
    {
        int i;
        int approval;

        FILE *fp;
        char retval[1024];

        char name[48];

        char ip1[3];
        char ip2[3];
        char ip3[3];
        char ip4[3];
        char ipaddr[15];

        char mac1[2];
        char mac2[2];
        char mac3[2];
        char mac4[2];
        char mac5[2];
        char mac6[2];
        char macaddr[17];

        char arpcommand[48];

        strcpy(name,gtk_entry_get_text(newnameentry));

        strcpy(ip1,gtk_entry_get_text(newipoctet1));
        strcpy(ip2,gtk_entry_get_text(newipoctet2));
        strcpy(ip3,gtk_entry_get_text(newipoctet3));
        strcpy(ip4,gtk_entry_get_text(newipoctet4));

        strcpy(mac1,gtk_entry_get_text(newmacbyte1));
        strcpy(mac2,gtk_entry_get_text(newmacbyte2));
        strcpy(mac3,gtk_entry_get_text(newmacbyte3));
        strcpy(mac4,gtk_entry_get_text(newmacbyte4));
        strcpy(mac5,gtk_entry_get_text(newmacbyte5));
        strcpy(mac6,gtk_entry_get_text(newmacbyte6));


        //make full ip address
        snprintf(ipaddr,16,"%s.%s.%s.%s",ip1,ip2,ip3,ip4);
        //make full mac address
        snprintf(macaddr,18,"%s:%s:%s:%s:%s:%s",mac1,mac2,mac3,mac4,mac5,mac6);

        //check to see if the new entry is valid
        approval = 1;
        for(i = 0; i < MAX_N_INVERTERS; i++)
        {
            //for all inverters that currently exist
            if(gtilist[i].extant == 1)
            {
                sprintf(debugbuffer,"comparing new ip: %s to existing ip: %s",ipaddr, gtilist[i].ipaddr);
                logwriteln(debugfilename,debugbuffer);
                //check for duplicate ip address or mac addresses
                if(strcmp(gtilist[i].ipaddr,ipaddr) == 0 )
                {
                    approval = 0;
                    sprintf(debugbuffer,"found duplicate IP address: %s",ipaddr);
                    logwriteln(debugfilename,debugbuffer);
                }
                else if(strcmp(gtilist[i].macaddr,macaddr) == 0)
                {
                    approval = 0;
                    sprintf(debugbuffer,"found duplicate MAC address: %s",macaddr);
                    logwriteln(debugfilename,debugbuffer);
                }
            }
        }

        if(approval == 1)
        {
            snprintf(debugbuffer,256,"attempting to create static arp cache entry for inverter %s (%s), at %s", name, macaddr, ipaddr);
            logwriteln(debugfilename,debugbuffer);
            //form command to add inverter as static arp entry
            snprintf(arpcommand,48,"arp -s %s %s 2>&1", ipaddr, macaddr);

            //add inverter info to active tree
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, COL_NAME, name, COL_IPADDR, ipaddr, COL_MACADDR, macaddr,-1);
            model = GTK_TREE_MODEL(store);
            gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);
            //save handle for data display
            gtilist[i].inviter = &iter;

            gtk_widget_show_all(window);

            //update GTIlist array
            strcpy(gtilist[inverterinsertindex].name,name);
            strcpy(gtilist[inverterinsertindex].ipaddr,ipaddr);
            strcpy(gtilist[inverterinsertindex].macaddr,macaddr);
            gtilist[inverterinsertindex].extant = 1;

            //make sockaddr_in element
            gtilist[inverterinsertindex].server.sin_addr.s_addr = inet_addr(gtilist[inverterinsertindex].ipaddr);
            gtilist[inverterinsertindex].server.sin_family = AF_INET;
            gtilist[inverterinsertindex].server.sin_port = htons(GP_PORT);

            //initialize recieved message list pointer
            gtilist[inverterinsertindex].reclistperm = createnewchosendllist();

            sprintf(debugbuffer,"added inverter %s (%s) at %s in index %d",name,macaddr,ipaddr,inverterinsertindex);
            logwriteln(debugfilename,debugbuffer);

            //find a place to put the next inverter
            for(i = 0; i< MAX_N_INVERTERS; i++)
            {
                if(gtilist[i].extant == 0)
                {
                    inverterinsertindex = i;
                    break;
                }
                if(i == MAX_N_INVERTERS-1)
                {
                    printf("\ninverter list is full");
                    logwriteln(debugfilename,"inverter list is full");
                    inverterinsertindex = -1;
                }
            }

            //write command to stdin
            fp = popen(arpcommand,"r");
            if(fp == NULL)
            {
                printf("\ndidn't run command: %s",arpcommand);
                sprintf(debugbuffer,"failed to run command: %s",arpcommand);
                logwriteln(debugfilename,debugbuffer);
            }

            //read output
            while(fgets(retval,sizeof(retval)-1, fp) != NULL)
            {
                logwriteln(debugfilename,retval);
            }
            pclose(fp);


            return 0;
        }
        else
        {
            logwriteln(debugfilename,"failed to create new entry in inverter list");
            return -2;
        }
    }
    else
    {
        printf("\ntried to add inverter to full list");
        logwriteln(debugfilename,"tried to add inverter to full list");
        return -1;
    }
}

int removeinverter(GtkWidget *widget, gpointer data)
{
    int removeindex;
    int i;
    int listempty = 1;

    char commandbuffer[256];
    char *name;
    char *ipaddr;
    char *macaddr;
    char retval[1024];
    FILE *fp;

    GtkTreeSelection *activesel;
    GtkTreeRowReference *activeselref;
    GtkTreePath *activeselpath;
    GtkTreeIter activeseliter;
    GtkTreeModel *activemodel;
    GList *activesellist;

    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            listempty = 0;
        }
    }
    if(listempty == 1)
    {
        return -1;
    }

    //find out which inverter, if any, has been selected
    activesel = gtk_tree_view_get_selection(activetree);
    if(activesel == NULL)
    {
        return -1;
    }
    if(gtilist[i].reclistperm == NULL)
    {
        return -2;
    }
    if(gtilist[i].msgtypelistperm == NULL)
    {
        return -3;
    }

    activesellist = gtk_tree_selection_get_selected(activesel, &activemodel, &activeseliter);
    gtk_tree_model_get(activemodel,&activeseliter,COL_NAME,&name,COL_IPADDR,&ipaddr,COL_MACADDR,&macaddr,-1);

    sprintf(debugbuffer,"data from item selected for removal: %s %s %s",name,ipaddr,macaddr);
    logwriteln(debugfilename,debugbuffer);

    gtk_list_store_remove(store, &activeseliter);

    //free memory devoted to received message list
    clearlist(gtilist[removeindex].reclistperm);

    //remove from gti structure list
    removeindex = getindexfromip(ipaddr);
    gtilist[removeindex].extant = 0;
    sprintf(debugbuffer,"freed gtilist index %d", removeindex);
    logwriteln(debugfilename,debugbuffer);

    sprintf(debugbuffer,"found inverter %d at %s using IP %s and removed it from the list",removeindex, gtilist[removeindex].ipaddr, ipaddr);
    logwriteln(debugfilename,debugbuffer);

    snprintf(commandbuffer,256,"arp -d %s 2>&1", ipaddr);

    snprintf(debugbuffer,1024,"attempting to remove static arp entry with command: %s",commandbuffer);
    logwriteln(debugfilename,debugbuffer);


    //write command to stdin
    fp = popen(commandbuffer,"r");
    if(fp == NULL)
    {
        printf("\ndidn't run command: %s",commandbuffer);
        sprintf(debugbuffer,"failed to run command: %s",commandbuffer);
        logwriteln(debugfilename,debugbuffer);
    }


    //read output
    while(fgets(retval,sizeof(retval)-1, fp) != NULL)
    {
        logwriteln(debugfilename,retval);
    }
    pclose(fp);

    return 0;
}

int getindexfromip(char *ipaddr)
{
    int i;
    for(i=0;i<MAX_N_INVERTERS;i++)
    {
        if(strcmp(gtilist[i].ipaddr,ipaddr) == 0)
        {
            return i;
        }
    }
    return -1;
}

int getselectedactive(void)
{

    int index;
    char *name;
    char *ipaddr;
    char *macaddr;

    GtkTreeSelection *activesel;
    GtkTreeRowReference *activeselref;
    GtkTreePath *activeselpath;
    GtkTreeIter activeseliter;
    GtkTreeModel *activemodel;
    GList *activesellist;
     //find out which inverter, if any, has been selected
    activesel = gtk_tree_view_get_selection(activetree);
    if(activesel == NULL)
    {
        return -1;
    }

    activesellist = gtk_tree_selection_get_selected(activesel, &activemodel, &activeseliter);
    gtk_tree_model_get(activemodel,&activeseliter,COL_NAME,&name,COL_IPADDR,&ipaddr,COL_MACADDR,&macaddr,-1);

    //remove from gti structure list
    index = getindexfromip(ipaddr);

    return index;
}

int saveconfigtofile(void)
{
    GtkFileChooser *chooser;
    gint res;

    logwriteln(debugfilename,"saving current configuration to file...");

    savedialog = gtk_file_chooser_dialog_new("Save configuration as...",window,GTK_FILE_CHOOSER_ACTION_SAVE,
                                            "Cancel",GTK_RESPONSE_CANCEL,
                                            "Save",GTK_RESPONSE_ACCEPT,NULL);
    chooser = GTK_FILE_CHOOSER(savedialog);

    gtk_file_chooser_set_do_overwrite_confirmation(chooser,TRUE);


    gtk_file_chooser_set_current_name(chooser, "Untitled document");


    res = gtk_dialog_run(GTK_DIALOG(savedialog));
    if(res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(savedialog);
        filename = gtk_file_chooser_get_filename(chooser);
        saveinverterconfig(&gtilist,chosenperm,filename);

    }

    gtk_widget_destroy(savedialog);
    return 0;
}

int loadconfigfromfile(void)
{
    int i;
    gint res;


    loaddialog = gtk_file_chooser_dialog_new("Load configuration from file...", window, GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "Cancel", GTK_RESPONSE_CANCEL,
                                        "Open", GTK_RESPONSE_ACCEPT,NULL);
    res = gtk_dialog_run(GTK_DIALOG(loaddialog));
    if(res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(loaddialog);
        filename = gtk_file_chooser_get_filename(chooser);

        sprintf(debugbuffer,"loading configuration from file %s",filename);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"node ptr: %d", (int) chosenperm);
        logwriteln(debugfilename,debugbuffer);
        loadinverterconfig(filename,chosenperm, &gtilist);

        makeinverterlistfromstruct(&gtilist);

        logwriteln(debugfilename,"here are the message components that we are loading: ");
        traverseright(chosenperm,debugprintnodeinfo);
        makesignallistfromstruct(chosenperm);
    }


    gtk_widget_destroy(loaddialog);
    return 0;
}

int removeinverterfromgtilist(int index)
{
    //free heap memory from message type list
    sprintf(debugbuffer,"listpointer: %d",(int) (gtilist[index].msgtypelistperm));
    if(gtilist[index].msgtypelistperm != NULL)
    {
        clearlist(gtilist[index].msgtypelistperm);
    }

    //deactivate inverter index
    gtilist[index].extant = 0;
    return 0;
}

int makeinverterlistfromstruct(GTIinfo *gtilist)
{
    int i;
    //add inverter info to active tree

    for(i = 0; i< MAX_N_INVERTERS; i++)
    {
        if(gtilist[i].extant == 1)
        {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, COL_NAME, gtilist[i].name, COL_IPADDR, gtilist[i].ipaddr, COL_MACADDR, gtilist[i].macaddr,-1);
            gtilist[i].inviter = &iter;
        }
    }
    model = GTK_TREE_MODEL(store);

    gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);

    gtk_widget_show_all(window);
    return 0;
}

int makesignallistfromstruct(chosenmsg *chosenperm)
{
    traverseright(chosenperm,addmsgfromstruct);

    //make new line visible
    gtk_widget_show_all(window);
    return 0;
}

void addmsgfromstruct(chosenmsg *chosen)
{
    if(chosen->data != -1)
    {
        gtk_list_store_append(sigstore, &sigiter);
        gtk_list_store_set(sigstore,&sigiter, COL_INDEX, chosen->data, COL_SIGNAME, signallist[chosen->data].name, COL_VALUE, chosen->value, -1);
        sigmodel = GTK_TREE_MODEL(sigstore);
        gtk_tree_view_set_model(GTK_TREE_VIEW(chosensignaltreeview),sigmodel);


        //maintain log
        snprintf(debugbuffer,128,"adding component to message: %s",signallist[chosen->data].name);
        logwriteln(debugfilename,debugbuffer);
    }

}

void messagehandler(void)
{
    pthread_mutex_lock(&sendrecprotector[chosenrecipient]);
    int index = chosenrecipient;
    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_lock(&logprotector);
        return -1;
    }
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,chosenperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }

    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"terminating thread %d", (int) pthread_self());
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);


    close(socket_desc);
    pthread_mutex_unlock(&sendrecprotector[chosenrecipient]);
    pthread_exit(NULL);
}

void messagehandler0(void)
{
    int index = 0;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler1(void)
{
    int index = 1;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler2(void)
{
    int index = 2;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler3(void)
{
    int index = 3;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler4(void)
{
    int index = 4;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler5(void)
{
    int index = 5;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler6(void)
{
    int index = 6;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler7(void)
{
    int index = 7;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler8(void)
{
    int index = 8;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

void messagehandler9(void)
{
    int index = 9;

    pthread_mutex_lock(&sendrecprotector[index]);

    int on = 1;
    int socket_desc;
    char recbuffer[1024];
    int datapresent;
    int slen;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(socket_desc,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    setsockopt(socket_desc,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));

    if(socket_desc == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"could not open socket: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
        return -1;
    }

    sprintf(debugbuffer, "opened socket with file description: %d", socket_desc);
    logwriteln(debugfilename,debugbuffer);

    pthread_mutex_lock(&sendmsgprotector);
    sendmessagetoinverter(index,gtilist[index].msgtypelistperm, socket_desc);
    pthread_mutex_unlock(&sendmsgprotector);

    memset(recbuffer,0, 1024);

    slen = sizeof(gtilist[index].server);
    datapresent = recvfrom(socket_desc,recbuffer, 1024, 0,(struct sockaddr *)&gtilist[index].server,&slen);
    if(datapresent == -1)
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"no data from inverter %d", index);
        logwriteln(debugfilename,debugbuffer);
        sprintf(debugbuffer,"here is the problem: %s",strerror(errno));
        logwriteln(debugfilename,debugbuffer);
        pthread_mutex_unlock(&logprotector);
    }
    else
    {
        pthread_mutex_lock(&logprotector);
        sprintf(debugbuffer,"received %d bytes of data from %d. here is the payload bytestream: ", datapresent, index);
        logwriteln(debugfilename,debugbuffer);
        debugprinthex(recbuffer, datapresent);
        pthread_mutex_unlock(&logprotector);

        pthread_mutex_lock(&dispaktprotector);
        disassemblepacket(recbuffer, index);
        pthread_mutex_unlock(&dispaktprotector);
    }
    close(socket_desc);
    pthread_mutex_lock(&logprotector);
    sprintf(debugbuffer,"closing socket with descriptor: %d",socket_desc);
    logwriteln(debugfilename,debugbuffer);
    pthread_mutex_unlock(&logprotector);

    pthread_mutex_unlock(&sendrecprotector[index]);
    pthread_exit(NULL);
}

