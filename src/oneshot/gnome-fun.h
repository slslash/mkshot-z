// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GNOME_FUN_H
#define GNOME_FUN_H

#include <cstddef>

/* Copy-paste of necessary prototypes and definitions from gtk/gtk.h
   So, to build this code GTK itself is not needed. */

#ifndef __GTK_H__
typedef char gchar;
typedef short gshort;
typedef long glong;
typedef int gint;
typedef size_t gsize;
typedef gint gboolean;
typedef unsigned char guchar;
typedef unsigned short gushort;
typedef unsigned long gulong;
typedef unsigned int guint;
typedef int gint32;
typedef unsigned int guint32;
typedef float gfloat;
typedef double gdouble;

typedef void *gpointer;
typedef const void *gconstpointer;
typedef gboolean (*GSourceFunc)(gpointer user_data);

typedef guint32 GQuark;

typedef gsize GType;
typedef struct _GTypeInstance GTypeInstance;

typedef struct _GVariant GVariant;
typedef struct _GError GError;

typedef struct _GSettings GSettings;
typedef struct _GSettingsSchema GSettingsSchema;
typedef struct _GSettingsSchemaSource GSettingsSchemaSource;

typedef struct _GtkWindow GtkWindow;
typedef struct _GtkDialog GtkDialog;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkButton GtkButton;

typedef struct _GdkDisplay GdkDisplay;
typedef struct _GdkMonitor GdkMonitor;
typedef struct _GdkRGBA GdkRGBA;

#ifndef	FALSE
#define	FALSE (0)
#endif
#ifndef	TRUE
#define	TRUE (!FALSE)
#endif

struct _GError
{
	GQuark domain;
	gint code;
	gchar *message;
};

struct _GdkRGBA
{
	float red;
	float green;
	float blue;
	float alpha;
};

typedef enum
{
	GTK_MESSAGE_INFO,
	GTK_MESSAGE_WARNING,
	GTK_MESSAGE_QUESTION,
	GTK_MESSAGE_ERROR,
	GTK_MESSAGE_OTHER
} GtkMessageType;

typedef enum
{
	GTK_DIALOG_MODAL               = 1 << 0,
	GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1,
	GTK_DIALOG_USE_HEADER_BAR      = 1 << 2
} GtkDialogFlags;

typedef enum
{
	GTK_RESPONSE_NONE         = -1,
	GTK_RESPONSE_REJECT       = -2,
	GTK_RESPONSE_ACCEPT       = -3,
	GTK_RESPONSE_DELETE_EVENT = -4,
	GTK_RESPONSE_OK           = -5,
	GTK_RESPONSE_CANCEL       = -6,
	GTK_RESPONSE_CLOSE        = -7,
	GTK_RESPONSE_YES          = -8,
	GTK_RESPONSE_NO           = -9,
	GTK_RESPONSE_APPLY        = -10,
	GTK_RESPONSE_HELP         = -11
} GtkResponseType;

typedef enum
{
	GTK_BUTTONS_NONE,
	GTK_BUTTONS_OK,
	GTK_BUTTONS_CLOSE,
	GTK_BUTTONS_CANCEL,
	GTK_BUTTONS_YES_NO,
	GTK_BUTTONS_OK_CANCEL
} GtkButtonsType;
#endif // __GTK_H__

/* Glib prototypes */
typedef guint (*GIDLEADDPROC)(GSourceFunc function, gpointer data);
typedef GTypeInstance *(*GTYPECHECKINSTANCECASTPROC)(GTypeInstance *instance, GType iface_type);
typedef void (*GERRORFREEPROC)(GError *error);

/* Gtk prototypes */
typedef void (*GTKINITPROC)(int *argc, char ***argv);
typedef gboolean (*GTKINITCHECKPROC)(int *argc, char ***argv);
typedef void (*GTKMAINPROC)(void);
typedef void (*GTKMAINQUITPROC)(void);
typedef GType (*GTKWINDOWGETTYPEPROC)(void);
typedef void (*GTKWINDOWSETTITLEPROC)(GtkWindow *window, const gchar *title);
typedef gint (*GTKDIALOGRUNPROC)(GtkDialog *dialog);
typedef GType (*GTKDIALOGGETTYPEPROC)(void);
typedef GtkWidget *(*GTKDIALOGGETWIDGETFORRESPONSEPROC)(GtkDialog *dialog, gint response_id);
typedef void (*GTKWIDGETDESTROYPROC)(GtkWidget *widget);
typedef GType (*GTKBUTTONGETTYPEPROC)(void);
typedef void (*GTKBUTTONSETLABELPROC)(GtkButton *button, const gchar *label);
typedef GtkWidget *(*GTKMESSAGEDIALOGNEWPROC)(GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...);

/* Gio prototypes */
typedef GSettings *(*GIOSETTINGSNEWPROC)(const gchar *schema_id);
typedef gchar *(*GIOSETTINGSGETSTRINGPROC)(GSettings *settings, const gchar *key);
typedef gboolean (*GIOSETTINGSSETSTRINGPROC)(GSettings *settings, const gchar *key, const gchar *value);
typedef GSettingsSchemaSource *(*GIOSETTINGSSCHEMASOURCEGETDEFAULTPROC)(void);
typedef GSettingsSchema *(*GIOSETTINGSSCHEMASOURCELOOKUPPROC)(GSettingsSchemaSource *source, const gchar *schema_id, gboolean recursive);
typedef gboolean (*GIOSETTINGSSCHEMAHASKEY)(GSettingsSchema *schema, const gchar *name);

/* Gdk prototypes */
typedef GdkDisplay *(*GDKDISPLAYOPENPROC)(const gchar *display_name);
typedef GType (*GDKDISPLAYGETTYPEPROC)(void);
typedef GdkMonitor *(*GDKDISPLAYGETMONITORPROC)(GdkDisplay *display, int monitor_num);
typedef int (*GDKDISPLAYGETNMONITORSPROC)(GdkDisplay *display);
typedef const char *(*GDKMONITORGETMODELPROC)(GdkMonitor *monitor);

/* Implementation bits */
#define DYN_G_TYPE_FUNDAMENTAL_SHIFT (2)

#define	DYN_G_TYPE_MAKE_FUNDAMENTAL(x) ((GType)((x) << DYN_G_TYPE_FUNDAMENTAL_SHIFT))

#define DYN_G_TYPE_INVALID DYN_G_TYPE_MAKE_FUNDAMENTAL(0)
#define DYN_G_TYPE_DOUBLE DYN_G_TYPE_MAKE_FUNDAMENTAL(15)

#ifndef __G_TYPE_H__
#define G_TYPE_INVALID DYN_G_TYPE_INVALID
#define G_TYPE_DOUBLE DYN_G_TYPE_DOUBLE
#endif // __G_TYPE_H__

#ifndef G_DISABLE_CAST_CHECKS
#define _DYN_G_TYPE_CIC(ip, gt, ct) ((ct *)dynGnome.g_type_check_instance_cast((GTypeInstance *)ip, gt))
#else
#define _DYN_G_TYPE_CIC(ip, gt, ct) ((ct *)ip)
#endif

#define DYN_G_TYPE_CHECK_INSTANCE_CAST(instance, g_type, c_type) (_DYN_G_TYPE_CIC((instance), (g_type), c_type))

#define DYN_GTK_TYPE_WINDOW (dynGnome.gtk_window_get_type())
#define DYN_GTK_TYPE_DIALOG (dynGnome.gtk_dialog_get_type())
#define DYN_GTK_TYPE_BUTTON (dynGnome.gtk_button_get_type())
#define DYN_GDK_TYPE_DISPLAY (dynGdk.gdk_display_get_type())

#define DYN_GTK_WINDOW(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_WINDOW, GtkWindow))
#define DYN_GTK_DIALOG(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_DIALOG, GtkDialog))
#define DYN_GTK_BUTTON(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GTK_TYPE_BUTTON, GtkButton))

#define DYN_GDK_DISPLAY(obj) (DYN_G_TYPE_CHECK_INSTANCE_CAST((obj), DYN_GDK_TYPE_DISPLAY, GdkDisplay))

#ifndef __GTK_H__
#define GTK_WINDOW DYN_GTK_WINDOW
#define GTK_DIALOG DYN_GTK_DIALOG
#define GTK_BUTTON DYN_GTK_BUTTON
#endif // __GTK_H__

#ifndef __GDK_H_INSIDE__
#define GDK_DISPLAY DYN_GDK_DISPLAY
#endif // __GDK_H_INSIDE__

#define GLIB_FUNCS \
	GNOME_FUNC(g_idle_add, GIDLEADDPROC) \
	GNOME_FUNC(g_type_check_instance_cast, GTYPECHECKINSTANCECASTPROC) \
	GNOME_FUNC(g_error_free, GERRORFREEPROC)

#define GTK_FUNCS \
	GNOME_FUNC(gtk_init, GTKINITPROC) \
	GNOME_FUNC(gtk_init_check, GTKINITCHECKPROC) \
	GNOME_FUNC(gtk_main, GTKMAINPROC) \
	GNOME_FUNC(gtk_main_quit, GTKMAINQUITPROC) \
	GNOME_FUNC(gtk_window_get_type, GTKWINDOWGETTYPEPROC) \
	GNOME_FUNC(gtk_window_set_title, GTKWINDOWSETTITLEPROC) \
	GNOME_FUNC(gtk_dialog_run, GTKDIALOGRUNPROC) \
	GNOME_FUNC(gtk_dialog_get_type, GTKDIALOGGETTYPEPROC) \
	GNOME_FUNC(gtk_dialog_get_widget_for_response, GTKDIALOGGETWIDGETFORRESPONSEPROC) \
	GNOME_FUNC(gtk_widget_destroy, GTKWIDGETDESTROYPROC) \
	GNOME_FUNC(gtk_button_get_type, GTKBUTTONGETTYPEPROC) \
	GNOME_FUNC(gtk_button_set_label, GTKBUTTONSETLABELPROC) \
	GNOME_FUNC(gtk_message_dialog_new, GTKMESSAGEDIALOGNEWPROC)

#define GIO_FUNCS \
	GIO_FUNC(g_settings_new, GIOSETTINGSNEWPROC) \
	GIO_FUNC(g_settings_get_string, GIOSETTINGSGETSTRINGPROC) \
	GIO_FUNC(g_settings_set_string, GIOSETTINGSSETSTRINGPROC) \
	GIO_FUNC(g_settings_schema_source_get_default, GIOSETTINGSSCHEMASOURCEGETDEFAULTPROC) \
	GIO_FUNC(g_settings_schema_source_lookup, GIOSETTINGSSCHEMASOURCELOOKUPPROC) \
	GIO_FUNC(g_settings_schema_has_key, GIOSETTINGSSCHEMAHASKEY)

#define GDK_FUNCS \
	GDK_FUNC(gdk_display_open, GDKDISPLAYOPENPROC) \
	GDK_FUNC(gdk_display_get_type, GDKDISPLAYGETTYPEPROC) \
	GDK_FUNC(gdk_display_get_monitor, GDKDISPLAYGETMONITORPROC) \
	GDK_FUNC(gdk_display_get_n_monitors, GDKDISPLAYGETNMONITORSPROC) \
	GDK_FUNC(gdk_monitor_get_model, GDKMONITORGETMODELPROC)

struct GnomeFunctions
{
#define GNOME_FUNC(name, type) type name;
	GLIB_FUNCS
	GTK_FUNCS
#undef GNOME_FUNC
};

struct GioFunctions
{
#define GIO_FUNC(name, type) type name;
	GIO_FUNCS
#undef GIO_FUNC
};

struct GdkFunctions
{
#define GDK_FUNC(name, type) type name;
	GDK_FUNCS
#undef GDK_FUNC
};

#define HAVE_GTK dynGnome.gtk_init
#define HAVE_GIO dynGio.g_settings_new
#define HAVE_GDK dynGdk.gdk_display_open

extern GnomeFunctions dynGnome;
extern GioFunctions dynGio;
extern GdkFunctions dynGdk;

void initGnomeFunctions();
void initGioFunctions();
void initGdkFunctions();

#endif // GNOME_FUN_H
