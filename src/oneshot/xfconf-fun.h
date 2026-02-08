// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XFCONF_FUN_H
#define XFCONF_FUN_H

#include <cstddef>

#include "gnome-fun.h"

#ifndef __XFCONF_CHANNEL_H__
typedef struct _XfconfChannel XfconfChannel;
#endif // __XFCONF_CHANNEL_H__

/* Xfconf prototypes */
typedef gboolean (*XFCONFINITPROC)(GError **error);
typedef void (*XFCONFSHUTDOWNPROC)(void);
typedef XfconfChannel *(*XFCONFCHANNELGETPROC)(const gchar *channel_name);
typedef gchar *(*XFCONFCHANNELGETSTRINGPROC)(XfconfChannel *channel, const gchar *property, const gchar *default_value);
typedef gboolean (*XFCONFCHANNELSETSTRINGPROC)(XfconfChannel *channel, const gchar *property, const gchar *value);
typedef gint32 (*XFCONFCHANNELGETINTPROC)(XfconfChannel *channel, const gchar *property, gint32 default_value);
typedef gboolean (*XFCONFCHANNELSETINTPROC)(XfconfChannel *channel, const gchar *property, gint32 value);
typedef gboolean (*XFCONFCHANNELGETARRAYPROC)(XfconfChannel *channel, const gchar *property, GType first_value_type, ...);
typedef gboolean (*XFCONFCHANNELSETARRAYPROC)(XfconfChannel *channel, const gchar *property, GType first_value_type, ...);
typedef void (*XFCONFCHANNELRESETPROPERTYPROC)(XfconfChannel *channel, const gchar *property_base, gboolean recursive);

#define XFCONF_FUNCS \
	XFCONF_FUNC(init, XFCONFINITPROC) \
	XFCONF_FUNC(shutdown, XFCONFSHUTDOWNPROC) \
	XFCONF_FUNC(channel_get, XFCONFCHANNELGETPROC) \
	XFCONF_FUNC(channel_get_string, XFCONFCHANNELGETSTRINGPROC) \
	XFCONF_FUNC(channel_set_string, XFCONFCHANNELSETSTRINGPROC) \
	XFCONF_FUNC(channel_get_int, XFCONFCHANNELGETINTPROC) \
	XFCONF_FUNC(channel_set_int, XFCONFCHANNELSETINTPROC) \
	XFCONF_FUNC(channel_get_array, XFCONFCHANNELGETARRAYPROC) \
	XFCONF_FUNC(channel_set_array, XFCONFCHANNELSETARRAYPROC) \
	XFCONF_FUNC(channel_reset_property, XFCONFCHANNELRESETPROPERTYPROC)

struct XfconfFunctions
{
#define XFCONF_FUNC(name, type) type name;
	XFCONF_FUNCS
#undef XFCONF_FUNC
};

#define HAVE_XFCONF dynXfconf.init

extern XfconfFunctions dynXfconf;

void initXfconfFunctions();

#endif // XFCONF_FUN_H
