#ifndef __GTIHMIAPP_H
#define __GTIHMIAPP_H

#include <gtk.h>

#define GTIHMI_APP_TYPE (gtihmi_app_get_type ())
G_DECLARE_FINAL_TYPE(GtihmiApp, gtihmi_app, GTIHMI, APP, GtkApplication)
GtihmiApp *gtihmi_app_new(void);

#endif
