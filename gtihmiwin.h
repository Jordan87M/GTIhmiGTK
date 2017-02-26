#ifndef __GTIHMIAPPWIN_H
#define __GTIHMIAPPWIN_H

#include <gtk.h>
#include "gtihmi.h"

#define GTIHMI_APP_WINDOW_TYPE (gtihmi_app_window_get_type())
G_DECLARE_FINAL_TYPE(GtihmiAppWindow, gtihmi_app_window, GTIHMI, APP_WINDOW, GtkApplicationWindow)

GtihmiAppWindow *gtihmi_app_window_new(GtihmiApp *app);
void gtihmi_app_window_open(GtihmiAppWindow *win, GFile *file);

#endif // __GTIHMIAPPWIN_H
