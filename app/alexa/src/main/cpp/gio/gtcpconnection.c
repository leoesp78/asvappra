#include "../glib/glibintl.h"
#include "config.h"
#include "gtcpconnection.h"
#include "gasyncresult.h"
#include "gsimpleasyncresult.h"
#include "giostream.h"

G_DEFINE_TYPE_WITH_CODE(GTcpConnection, g_tcp_connection, G_TYPE_SOCKET_CONNECTION,g_socket_connection_factory_register_type(g_define_type_id,
					    G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT); g_socket_connection_factory_register_type(g_define_type_id,
					    G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT); g_socket_connection_factory_register_type(g_define_type_id,
					    G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP); g_socket_connection_factory_register_type(g_define_type_id,
					    G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP););
static gboolean g_tcp_connection_close(GIOStream *stream, GCancellable *cancellable, GError **error);
static void g_tcp_connection_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
struct _GTcpConnectionPrivate {
  guint graceful_disconnect : 1;
};
enum {
  PROP_0,
  PROP_GRACEFUL_DISCONNECT
};
static void g_tcp_connection_init(GTcpConnection *connection) {
  connection->priv = G_TYPE_INSTANCE_GET_PRIVATE(connection, G_TYPE_TCP_CONNECTION, GTcpConnectionPrivate);
  connection->priv->graceful_disconnect = FALSE;
}
static void g_tcp_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GTcpConnection *connection = G_TCP_CONNECTION(object);
  switch(prop_id) {
      case PROP_GRACEFUL_DISCONNECT: g_value_set_boolean(value, connection->priv->graceful_disconnect); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_tcp_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GTcpConnection *connection = G_TCP_CONNECTION(object);
  switch(prop_id) {
      case PROP_GRACEFUL_DISCONNECT: g_tcp_connection_set_graceful_disconnect(connection, g_value_get_boolean(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_tcp_connection_class_init(GTcpConnectionClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GIOStreamClass *stream_class = G_IO_STREAM_CLASS(class);
  g_type_class_add_private(class, sizeof(GTcpConnectionPrivate));
  gobject_class->set_property = g_tcp_connection_set_property;
  gobject_class->get_property = g_tcp_connection_get_property;
  stream_class->close_fn = g_tcp_connection_close;
  stream_class->close_async = g_tcp_connection_close_async;
  g_object_class_install_property(gobject_class, PROP_GRACEFUL_DISCONNECT,g_param_spec_boolean("graceful-disconnect",
							      P_("Graceful Disconnect"), P_("Whether or not close does a graceful disconnect"),FALSE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static gboolean g_tcp_connection_close(GIOStream *stream, GCancellable *cancellable, GError **error) {
  GTcpConnection *connection = G_TCP_CONNECTION(stream);
  GSocket *socket;
  char buffer[1024];
  gssize ret;
  GError *my_error;
  gboolean had_error;
  socket = g_socket_connection_get_socket(G_SOCKET_CONNECTION(stream));
  had_error = FALSE;
  if (connection->priv->graceful_disconnect && !g_cancellable_is_cancelled(cancellable)) {
      if (!g_socket_shutdown(socket, FALSE, TRUE, error)) {
          error = NULL;
          had_error = TRUE;
	  } else {
          while(TRUE) {
              my_error = NULL;
              ret = g_socket_receive(socket,  buffer, sizeof(buffer), cancellable, &my_error);
              if (ret < 0) {
                  if (g_error_matches(my_error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) g_error_free(my_error);
                  else {
                      had_error = TRUE;
                      g_propagate_error(error, my_error);
                      error = NULL;
                      break;
                  }
              }
              if (ret == 0) break;
          }
	  }
  }
  return G_IO_STREAM_CLASS(g_tcp_connection_parent_class)->close_fn (stream, cancellable, error) && !had_error;
}
typedef struct {
  GSimpleAsyncResult *res;
  GCancellable *cancellable;
} CloseAsyncData;
static void close_async_data_free(CloseAsyncData *data) {
  g_object_unref(data->res);
  if (data->cancellable) g_object_unref(data->cancellable);
  g_free(data);
}
static void async_close_finish(CloseAsyncData *data, GError *error, gboolean in_mainloop) {
  GIOStreamClass *parent = G_IO_STREAM_CLASS(g_tcp_connection_parent_class);
  GIOStream *stream;
  GError *my_error;
  stream = G_IO_STREAM(g_async_result_get_source_object(G_ASYNC_RESULT(data->res)));
  if (error) {
      parent->close_fn(stream, data->cancellable, NULL);
      g_simple_async_result_take_error(data->res, error);
  } else {
      my_error = NULL;
      parent->close_fn(stream, data->cancellable, &my_error);
      if (my_error) g_simple_async_result_take_error(data->res, my_error);
  }
  if (in_mainloop) g_simple_async_result_complete(data->res);
  else g_simple_async_result_complete_in_idle(data->res);
}
static gboolean close_read_ready(GSocket *socket, GIOCondition condition, CloseAsyncData *data) {
  GError *error = NULL;
  char buffer[1024];
  gssize ret;
  ret = g_socket_receive(socket,  buffer, sizeof(buffer), data->cancellable, &error);
  if (ret < 0) {
      if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) g_error_free(error);
      else {
          async_close_finish(data, error, TRUE);
          return FALSE;
	  }
  }
  if (ret == 0) {
      async_close_finish(data, NULL, TRUE);
      return FALSE;
  }
  return TRUE;
}
static void g_tcp_connection_close_async(GIOStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GTcpConnection *connection = G_TCP_CONNECTION(stream);
  CloseAsyncData *data;
  GSocket *socket;
  GSource *source;
  GError *error;
  if (connection->priv->graceful_disconnect && !g_cancellable_is_cancelled(cancellable)) {
      data = g_new(CloseAsyncData, 1);
      data->res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_tcp_connection_close_async);
      if (cancellable) data->cancellable = g_object_ref(cancellable);
      else data->cancellable = NULL;
      socket = g_socket_connection_get_socket(G_SOCKET_CONNECTION(stream));
      error = NULL;
      if (!g_socket_shutdown(socket, FALSE, TRUE, &error)) {
          async_close_finish(data, error, FALSE);
          close_async_data_free(data);
          return;
	  }
      source = g_socket_create_source(socket, G_IO_IN, cancellable);
      g_source_set_callback(source, (GSourceFunc)close_read_ready, data, (GDestroyNotify)close_async_data_free);
      g_source_attach(source, g_main_context_get_thread_default());
      g_source_unref(source);
      return;
  }
  G_IO_STREAM_CLASS(g_tcp_connection_parent_class)->close_async(stream, io_priority, cancellable, callback, user_data);
}
void g_tcp_connection_set_graceful_disconnect(GTcpConnection *connection, gboolean graceful_disconnect) {
  graceful_disconnect = !!graceful_disconnect;
  if (graceful_disconnect != connection->priv->graceful_disconnect) {
      connection->priv->graceful_disconnect = graceful_disconnect;
      g_object_notify(G_OBJECT(connection), "graceful-disconnect");
  }
}
gboolean g_tcp_connection_get_graceful_disconnect(GTcpConnection *connection) {
  return connection->priv->graceful_disconnect;
}