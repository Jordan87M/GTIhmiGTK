#include <stdlib.h>
#include <gtk.h>

#include "gpdefs.h"
#include "gtihmi.h"
#include "gtihmiwin.h"

struct _GtihmiAppWindow
{
    GtkApplicationWindow parent;
};

G_DEFINE_TYPE(GtihmiAppWindow, gtihmi_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void gtihmi_app_window_init(GtihmiAppWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void gtihmi_app_window_class_init(GtihmiAppWindowClass* class)
{
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/sevelevlabs/gti/hmi/builder.ui");
}

GtihmiAppWindow * gtihmi_app_window_new(GtihmiApp *app)
{
    return g_object_new(GTIHMI_APP_WINDOW_TYPE, "application", app, NULL);
}

void gtihmi_app_window_open(GtihmiAppWindow *win, GFile *file)
{
    //populate signallistbox
    newrow = gtk_list_box_row_new();
    gtk_list_box_insert(signallistbox,newrow,0);
}
