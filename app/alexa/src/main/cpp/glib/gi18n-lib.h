#ifndef __G_I18N_LIB_H__
#define __G_I18N_LIB_H__

#include <string.h>
#include "../gio/config.h"
#include "glibintl.h"

#ifndef GETTEXT_PACKAGE
#error You must define GETTEXT_PACKAGE before including gi18n-lib.h.  Did you forget to include config.h?
#endif

#define  _(String) ((char*)g_dgettext (GETTEXT_PACKAGE, String))
#define Q_(String) g_dpgettext(GETTEXT_PACKAGE, String, 0)
#define N_(String) (String)
#define C_(Context,String) g_dpgettext(GETTEXT_PACKAGE, Context "\004" String, strlen(Context) + 1)
#define NC_(Context, String) (String)

#endif