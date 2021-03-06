#ifndef __XDG_MIME_ALIAS_H__
#define __XDG_MIME_ALIAS_H__

#include "xdgmime.h"

typedef struct XdgAliasList XdgAliasList;
#ifdef XDG_PREFIX
#define _xdg_mime_alias_read_from_file  XDG_RESERVED_ENTRY(alias_read_from_file)
#define _xdg_mime_alias_list_new  XDG_RESERVED_ENTRY(alias_list_new)
#define _xdg_mime_alias_list_free  XDG_RESERVED_ENTRY(alias_list_free)
#define _xdg_mime_alias_list_lookup  XDG_RESERVED_ENTRY(alias_list_lookup)
#define _xdg_mime_alias_list_dump  XDG_RESERVED_ENTRY(alias_list_dump)
#endif
void _xdg_mime_alias_read_from_file(XdgAliasList *list, const char *file_name);
XdgAliasList *_xdg_mime_alias_list_new(void);
void _xdg_mime_alias_list_free(XdgAliasList *list);
const char *_xdg_mime_alias_list_lookup(XdgAliasList *list, const char *alias);
#ifdef NOT_USED_IN_GIO
void _xdg_mime_alias_list_dump(XdgAliasList *list);
#endif
#endif