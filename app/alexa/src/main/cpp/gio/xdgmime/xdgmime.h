#ifndef __XDG_MIME_H__
#define __XDG_MIME_H__

#include <stdlib.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef XDG_PREFIX
#define XDG_ENTRY(func) _XDG_ENTRY2(XDG_PREFIX,func)
#define _XDG_ENTRY2(prefix,func) _XDG_ENTRY3(prefix,func)
#define _XDG_ENTRY3(prefix,func) prefix##_##func
#define XDG_RESERVED_ENTRY(func) _XDG_RESERVED_ENTRY2(XDG_PREFIX,func)
#define _XDG_RESERVED_ENTRY2(prefix,func) _XDG_RESERVED_ENTRY3(prefix,func)
#define _XDG_RESERVED_ENTRY3(prefix,func) _##prefix##_##func
#endif
typedef void (*XdgMimeCallback)(void *user_data);
typedef void (*XdgMimeDestroy)(void *user_data);
typedef unsigned int size_t;
#ifdef XDG_PREFIX
#define xdg_mime_get_mime_type_for_data   XDG_ENTRY(get_mime_type_for_data)
#define xdg_mime_get_mime_type_for_file   XDG_ENTRY(get_mime_type_for_file)
#define xdg_mime_get_mime_type_from_file_name  XDG_ENTRY(get_mime_type_from_file_name)
#define xdg_mime_get_mime_types_from_file_name  XDG_ENTRY(get_mime_types_from_file_name)
#define xdg_mime_is_valid_mime_type  XDG_ENTRY(is_valid_mime_type)
#define xdg_mime_mime_type_equal  XDG_ENTRY(mime_type_equal)
#define xdg_mime_media_type_equal  XDG_ENTRY(media_type_equal)
#define xdg_mime_mime_type_subclass  XDG_ENTRY(mime_type_subclass)
#define xdg_mime_get_mime_parents  XDG_ENTRY(get_mime_parents)
#define xdg_mime_list_mime_parents  XDG_ENTRY(list_mime_parents)
#define xdg_mime_unalias_mime_type  XDG_ENTRY(unalias_mime_type)
#define xdg_mime_get_max_buffer_extents  XDG_ENTRY(get_max_buffer_extents)
#define xdg_mime_shutdown  XDG_ENTRY(shutdown)
#define xdg_mime_dump  XDG_ENTRY(dump)
#define xdg_mime_register_reload_callback  XDG_ENTRY(register_reload_callback)
#define xdg_mime_remove_callback  XDG_ENTRY(remove_callback)
#define xdg_mime_type_unknown  XDG_ENTRY(type_unknown)
#define xdg_mime_get_icon  XDG_ENTRY(get_icon)
#define xdg_mime_get_generic_icon  XDG_ENTRY(get_generic_icon)
#define _xdg_mime_mime_type_subclass  XDG_RESERVED_ENTRY(mime_type_subclass)
#define _xdg_mime_mime_type_equal  XDG_RESERVED_ENTRY(mime_type_equal)
#define _xdg_mime_unalias_mime_type  XDG_RESERVED_ENTRY(unalias_mime_type)
#endif
extern const char xdg_mime_type_unknown[];
#define XDG_MIME_TYPE_UNKNOWN xdg_mime_type_unknown
const char *xdg_mime_get_mime_type_for_data(const void *data, size_t len, int *result_prio);
#ifdef NOT_USED_IN_GIO
const char *xdg_mime_get_mime_type_for_file(const char *file_name, struct stat *statbuf);
const char *xdg_mime_get_mime_type_from_file_name(const char *file_name);
#endif
int xdg_mime_get_mime_types_from_file_name(const char *file_name, const char *mime_types[], int n_mime_types);
#ifdef NOT_USED_IN_GIO
int xdg_mime_is_valid_mime_type(const char *mime_type);
#endif
int xdg_mime_mime_type_equal(const char *mime_a, const char *mime_b);
int xdg_mime_media_type_equal(const char *mime_a, const char *mime_b);
int xdg_mime_mime_type_subclass(const char *mime_a, const char *mime_b);
#ifdef NOT_USED_IN_GIO
const char **xdg_mime_get_mime_parents(const char *mime);
#endif
char **xdg_mime_list_mime_parents(const char *mime);
const char *xdg_mime_unalias_mime_type(const char *mime);
const char *xdg_mime_get_icon(const char *mime);
const char *xdg_mime_get_generic_icon(const char *mime);
int xdg_mime_get_max_buffer_extents(void);
void xdg_mime_shutdown(void);
#ifdef NOT_USED_IN_GIO
void xdg_mime_dump(void);
#endif
int xdg_mime_register_reload_callback(XdgMimeCallback callback, void *data, XdgMimeDestroy destroy);
#ifdef NOT_USED_IN_GIO
void xdg_mime_remove_callback(int callback_id);
#endif
int _xdg_mime_mime_type_equal(const char *mime_a, const char *mime_b);
int _xdg_mime_mime_type_subclass(const char *mime, const char *base);
const char *_xdg_mime_unalias_mime_type(const char *mime);
#ifdef __cplusplus
}
#endif
#endif