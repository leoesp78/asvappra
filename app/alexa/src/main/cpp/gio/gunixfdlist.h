#ifndef __G_UNIX_FD_LIST_H__
#define __G_UNIX_FD_LIST_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_FD_LIST  (g_unix_fd_list_get_type())
#define G_UNIX_FD_LIST(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_UNIX_FD_LIST, GUnixFDList))
#define G_UNIX_FD_LIST_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_UNIX_FD_LIST, GUnixFDListClass))
#define G_IS_UNIX_FD_LIST(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_UNIX_FD_LIST))
#define G_IS_UNIX_FD_LIST_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_UNIX_FD_LIST))
#define G_UNIX_FD_LIST_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_UNIX_FD_LIST, GUnixFDListClass))
typedef struct _GUnixFDListPrivate GUnixFDListPrivate;
typedef struct _GUnixFDListClass GUnixFDListClass;
struct _GUnixFDListClass {
  GObjectClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
struct _GUnixFDList {
  GObject parent_instance;
  GUnixFDListPrivate *priv;
};
GType g_unix_fd_list_get_type(void) G_GNUC_CONST;
GUnixFDList *g_unix_fd_list_new(void);
GUnixFDList *g_unix_fd_list_new_from_array(const gint *fds, gint n_fds);
gint g_unix_fd_list_append(GUnixFDList *list, gint fd, GError **error);
gint g_unix_fd_list_get_length(GUnixFDList *list);
gint g_unix_fd_list_get(GUnixFDList *list, gint index_, GError **error);
const gint *g_unix_fd_list_peek_fds(GUnixFDList *list, gint *length);
gint *g_unix_fd_list_steal_fds(GUnixFDList *list, gint *length);
G_END_DECLS

#endif