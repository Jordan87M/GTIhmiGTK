#include <stdlib.h>
#include <gtk.h>

#include "gpdefs.h"
#include "gtihmi.h"
#include "gtihmiwin.h"

struct _GtihmiApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(GtihmiApp, gtihmi_app, GTK_TYPE_APPLICATION);

static void gtihmi_app_init(GtihmiApp *app)
{
}

static void gtihmi_app_activate(GApplication *app)
{
    GtihmiAppWindow *win;

    win = gtihmi_app_window_new (GTIHMI_APP (app));
    gtk_window_present(GTK_WINDOW (win));
}

static void gtihmi_app_open(GApplication *app, GFile **file, gint n_files, const gchar *hint)
{
    GList *windows;
    GtihmiApp *win;
    int i;

    windows = gtk_application_get_windows(GTK_APPLICATION (app));
    if(windows)
        win = GTIHMI_APP_WINDOW(windows->data);
    else
        win = gtihmi_app_window_new(GTIHMI_APP (app));

    gtk_window_present(GTK_WINDOW (win));
}

static void gtihmi_app_class_init(GtihmiAppClass *class)
{
    G_APPLICATION_CLASS(class)->activate = gtihmi_app_activate;
    G_APPLICATION_CLASS(class)->open = gtihmi_app_open;
}

GtihmiApp * gtihmi_app_new(void)
{
    return g_object_new(GTIHMI_APP_TYPE, "application-id","sevelevlabs.gti.hmi","flags", G_APPLICATION_HANDLES_OPEN, NULL);
}
