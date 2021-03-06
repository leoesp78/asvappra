#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_LISTENER_H__
#define __G_SOCKET_LISTENER_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_LISTENER  (g_socket_listener_get_type())
#define G_SOCKET_LISTENER(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET_LISTENER, GSocketListener))
#define G_SOCKET_LISTENER_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET_LISTENER, GSocketListenerClass))
#define G_IS_SOCKET_LISTENER(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET_LISTENER))
#define G_IS_SOCKET_LISTENER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET_LISTENER))
#define G_SOCKET_LISTENER_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET_LISTENER, GSocketListenerClass))
typedef struct _GSocketListenerPrivate GSocketListenerPrivate;
typedef struct _GSocketListenerClass GSocketListenerClass;
struct _GSocketListenerClass {
  GObjectClass parent_class;
  void (* changed)(GSocketListener *listener);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
};
struct _GSocketListener {
  GObject parent_instance;
  GSocketListenerPrivate *priv;
};
GType g_socket_listener_get_type(void) G_GNUC_CONST;
GSocketListener *g_socket_listener_new(void);
void g_socket_listener_set_backlog(GSocketListener *listener, int listen_backlog);
gboolean g_socket_listener_add_socket(GSocketListener *listener, GSocket *socket, GObject *source_object, GError **error);
gboolean g_socket_listener_add_address(GSocketListener *listener, GSocketAddress *address, GSocketType type, GSocketProtocol protocol,
									 GObject *source_object, GSocketAddress **effective_address, GError **error);
gboolean g_socket_listener_add_inet_port(GSocketListener *listener, guint16 port, GObject *source_object, GError **error);
guint16 g_socket_listener_add_any_inet_port(GSocketListener *listener, GObject *source_object, GError **error);
GSocket *g_socket_listener_accept_socket(GSocketListener *listener, GObject **source_object, GCancellable *cancellable, GError **error);
void g_socket_listener_accept_socket_async(GSocketListener *listener, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GSocket *g_socket_listener_accept_socket_finish(GSocketListener *listener, GAsyncResult *result, GObject **source_object, GError **error);
GSocketConnection *g_socket_listener_accept(GSocketListener *listener, GObject **source_object, GCancellable *cancellable, GError **error);
void g_socket_listener_accept_async(GSocketListener *listener, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GSocketConnection *g_socket_listener_accept_finish(GSocketListener *listener, GAsyncResult *result, GObject **source_object, GError **error);
void g_socket_listener_close(GSocketListener *listener);
G_END_DECLS

#endif