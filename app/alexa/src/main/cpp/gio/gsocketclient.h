#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_CLIENT_H__
#define __G_SOCKET_CLIENT_H__

#include "../gobject/gobject.h"
#include "giotypes.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_CLIENT  (g_socket_client_get_type())
#define G_SOCKET_CLIENT(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET_CLIENT, GSocketClient))
#define G_SOCKET_CLIENT_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET_CLIENT, GSocketClientClass))
#define G_IS_SOCKET_CLIENT(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET_CLIENT))
#define G_IS_SOCKET_CLIENT_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET_CLIENT))
#define G_SOCKET_CLIENT_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET_CLIENT, GSocketClientClass))
typedef struct _GSocketClientPrivate GSocketClientPrivate;
typedef struct _GSocketClientClass GSocketClientClass;
struct _GSocketClientClass {
  GObjectClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
struct _GSocketClient {
  GObject parent_instance;
  GSocketClientPrivate *priv;
};
GType g_socket_client_get_type(void) G_GNUC_CONST;
GSocketClient *g_socket_client_new(void);
GSocketFamily g_socket_client_get_family(GSocketClient *client);
void g_socket_client_set_family(GSocketClient *client, GSocketFamily family);
GSocketType g_socket_client_get_socket_type(GSocketClient *client);
void g_socket_client_set_socket_type(GSocketClient *client, GSocketType type);
GSocketProtocol g_socket_client_get_protocol(GSocketClient *client);
void g_socket_client_set_protocol(GSocketClient *client, GSocketProtocol protocol);
GSocketAddress *g_socket_client_get_local_address(GSocketClient *client);
void g_socket_client_set_local_address(GSocketClient *client, GSocketAddress *address);
guint g_socket_client_get_timeout(GSocketClient *client);
void g_socket_client_set_timeout(GSocketClient *client, guint timeout);
gboolean g_socket_client_get_enable_proxy(GSocketClient *client);
void g_socket_client_set_enable_proxy(GSocketClient *client, gboolean enable);
gboolean g_socket_client_get_tls(GSocketClient *client);
void g_socket_client_set_tls(GSocketClient *client, gboolean tls);
GTlsCertificateFlags g_socket_client_get_tls_validation_flags(GSocketClient *client);
void g_socket_client_set_tls_validation_flags(GSocketClient *client, GTlsCertificateFlags flags);
GSocketConnection *g_socket_client_connect(GSocketClient *client, GSocketConnectable *connectable, GCancellable *cancellable, GError **error);
GSocketConnection *g_socket_client_connect_to_host(GSocketClient *client, const gchar *host_and_port, guint16 default_port, GCancellable *cancellable, GError **error);
GSocketConnection *g_socket_client_connect_to_service(GSocketClient *client, const gchar *domain, const gchar *service, GCancellable *cancellable, GError **error);
GSocketConnection *g_socket_client_connect_to_uri(GSocketClient *client, const gchar *uri, guint16 default_port, GCancellable *cancellable, GError **error);
void g_socket_client_connect_async(GSocketClient *client, GSocketConnectable *connectable, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GSocketConnection *g_socket_client_connect_finish(GSocketClient *client, GAsyncResult *result, GError **error);
void g_socket_client_connect_to_host_async(GSocketClient *client, const gchar *host_and_port, guint16 default_port, GCancellable *cancellable, GAsyncReadyCallback callback,
                                           gpointer user_data);
GSocketConnection *g_socket_client_connect_to_host_finish(GSocketClient *client, GAsyncResult *result, GError **error);
void g_socket_client_connect_to_service_async(GSocketClient *client, const gchar *domain, const gchar *service, GCancellable *cancellable, GAsyncReadyCallback callback,
                                              gpointer user_data);
GSocketConnection *g_socket_client_connect_to_service_finish(GSocketClient *client, GAsyncResult *result, GError **error);
void g_socket_client_connect_to_uri_async(GSocketClient *client, const gchar *uri, guint16 default_port, GCancellable *cancellable, GAsyncReadyCallback callback,
                                          gpointer user_data);
GSocketConnection *g_socket_client_connect_to_uri_finish(GSocketClient *client, GAsyncResult *result, GError **error);
void g_socket_client_add_application_proxy(GSocketClient *client, const gchar *protocol);
G_END_DECLS

#endif