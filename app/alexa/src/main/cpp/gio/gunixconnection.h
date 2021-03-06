#ifndef __G_UNIX_CONNECTION_H__
#define __G_UNIX_CONNECTION_H__

#include "../gobject/gobject.h"
#include "gsocketconnection.h"

G_BEGIN_DECLS
#define G_TYPE_UNIX_CONNECTION  (g_unix_connection_get_type ())
#define G_UNIX_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_CAST ((inst), G_TYPE_UNIX_CONNECTION, GUnixConnection))
#define G_UNIX_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST ((class), G_TYPE_UNIX_CONNECTION, GUnixConnectionClass))
#define G_IS_UNIX_CONNECTION(inst)  (G_TYPE_CHECK_INSTANCE_TYPE ((inst), G_TYPE_UNIX_CONNECTION))
#define G_IS_UNIX_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), G_TYPE_UNIX_CONNECTION))
#define G_UNIX_CONNECTION_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), G_TYPE_UNIX_CONNECTION, GUnixConnectionClass))
typedef struct _GUnixConnection GUnixConnection;
typedef struct _GUnixConnectionPrivate GUnixConnectionPrivate;
typedef struct _GUnixConnectionClass GUnixConnectionClass;
struct _GUnixConnectionClass {
  GSocketConnectionClass parent_class;
};
struct _GUnixConnection {
  GSocketConnection parent_instance;
  GUnixConnectionPrivate *priv;
};
GType g_unix_connection_get_type(void);
gboolean g_unix_connection_send_fd(GUnixConnection *connection, gint fd, GCancellable *cancellable, GError **error);
gint g_unix_connection_receive_fd(GUnixConnection *connection, GCancellable *cancellable, GError **error);
gboolean g_unix_connection_send_credentials(GUnixConnection *connection, GCancellable *cancellable, GError **error);
GCredentials *g_unix_connection_receive_credentials(GUnixConnection *connection, GCancellable *cancellable, GError **error);
G_END_DECLS

#endif