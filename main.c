#include <stdlib.h>
#include <stdio.h>
#include <gtk.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "gpdefs.h"
#include "gtihmi.h"
#include "logconf.h"


//global gui pointers

GtkListStore *store;
GtkTreeIter iter;
GtkCellRenderer *renderer;
GtkTreeModel *model;

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
GtkWidget *activetree;

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
char debugbuffer[LOG_BUFFER_SIZE];

//socket globals
int socket_desc;


//other globals
GTIinfo gtilist[MAX_N_INVERTERS];
int selected[MAX_N_INVERTERS];
int chosen[MAX_MSG_COMP];
unsigned short int globalseqnum;

int inverterinsertindex;


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

    //variables to help coordinate widget positioning
    int horz_marg = 20;
    int top_marg = 5;
    int horz_dim = 900;
    int vert_dim = 600;
    int frame_width = horz_dim-2*horz_marg;
    int frame_height = 60;
    int sigbox_width = 180;
    int sigbox_height = 240;
    int sigbox_space = 20;
    int std_but_width = 80;
    int std_but_height = 10;
    int std_but_space_y = 10;
    int sm_but_width = 20;
    int sm_but_height = 10;
    int sm_but_space_y = 5;
    int active_view_height = 280;
    int active_view_width = horz_dim - 2*horz_marg;

    int sig_row_y = top_marg + frame_height + 10;
    int chosen_box_xpos = horz_marg + sigbox_width + std_but_width + 2*sigbox_space;
    int chosen_but_xpos = horz_marg + sigbox_width + sigbox_space;
    int chosen_top_ypos = sig_row_y + 20;
    int chosen_bot_ypos = chosen_top_ypos + std_but_height + 2*std_but_space_y;
    int active_view_ypos = sig_row_y + sigbox_height + 20;


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

    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    activetree = gtk_tree_view_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 0, "Name", renderer, "text", 0);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 1, "IP Address", renderer, "text", 1);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(activetree), 2, "MAC Address", renderer, "text", 2);


    gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);
    //g_object_unref(model);
    /*
    activetreenamecol = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(activetreenamecol,"Name");
    gtk_tree_view_column_set_min_width(activetreenamecol, MAX_NAME_LENGTH);
    activetreeipcol = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(activetreeipcol,"IP Address");
    activetreemaccol = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(activetreemaccol,"MAC Address");

    gtk_tree_view_append_column(activetree, activetreenamecol);
    gtk_tree_view_append_column(activetree, activetreemaccol);
    gtk_tree_view_append_column(activetree, activetreeipcol);
    */
    gtk_fixed_put(fixed,activetree,horz_marg, active_view_ypos);
    gtk_widget_set_size_request(activetree, active_view_width, active_view_height);

    gtk_widget_show_all(window);

}

int sendtoenabled(GtkWidget *widget, gpointer data)
{
    int i;
    for(i = 0; i < MAX_N_INVERTERS; i++)
    {
        if(selected[i] == 1)
        {
            sendmessagetoinverter(i);
        }
    }

}

int sendmessagetoinverter(int i)
{
    int socket_desc;
    struct sockaddr_in server;
    char sendbuffer[1024];
    char recbuffer[1024];

    gpheader head;

    head.magic = GP_MAGIC;
    head.context = INVERTER_CONTEXT;
    head.seqnum = globalseqnum;


    //build message packet


    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_desc == -1)
    {
        printf("could not create socket");
        logwriteln(debuglog,"coudl not open socket");
    }

    server.sin_addr.s_addr = inet_addr(gtilist[i].ipaddr);
    server.sin_family = AF_INET;
    server.sin_port = htons(GP_PORT);

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
    if(inverterinsertindex >= 0)
    {
        int i;

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
        snprintf(debugbuffer,256,"attempting to create static arp cache entry for inverter %s (%s), at %s", name, macaddr, ipaddr);
        logwriteln(debuglog,debugbuffer);
        //form command to add inverter as static arp entry
        snprintf(arpcommand,48,"arp -s %s %s 2>&1", ipaddr, macaddr);

        //add inverter info to active tree
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, name, 1, ipaddr, 2, macaddr,-1);
        model = GTK_TREE_MODEL(store);
        gtk_tree_view_set_model(GTK_TREE_VIEW(activetree), model);

        gtk_widget_show_all(window);

        //update GTIlist array
        strcpy(gtilist[inverterinsertindex].name,name);
        strcpy(gtilist[inverterinsertindex].ipaddr,ipaddr);
        strcpy(gtilist[inverterinsertindex].macaddr,macaddr);
        gtilist[inverterinsertindex].extant = 1;
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
                logwriteln(debuglog,"inverter list is full");
                inverterinsertindex = -1;
            }
        }

        //write command to stdin
        fp = popen(arpcommand,"r");
        if(fp == NULL)
        {
            printf("\ndidn't run command: %s",arpcommand);
            sprintf(debugbuffer,"failed to run command: %s",arpcommand);
            logwriteln(debuglog,debugbuffer);
        }

        //read output
        while(fgets(retval,sizeof(retval)-1, fp) != NULL)
        {
            logwriteln(debuglog,retval);
        }
        pclose(fp);


        return 0;
    }
    else
    {
        printf("\ntried to add inverter to full list");
        logwriteln(debuglog,"tried to add inverter to full list");
        return -1;
    }
}

int removeinverter(GtkWidget *widget, gpointer data)
{

    return 0;
}

