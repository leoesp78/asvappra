#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "goutputstream.h"
#include "gsocketoutputstream.h"
#include "gsocket.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gpollableinputstream.h"
#include "gpollableoutputstream.h"
#include "gioerror.h"

static void g_socket_output_stream_pollable_iface_init (GPollableOutputStreamInterface *iface);
#define g_socket_output_stream_get_type _g_socket_output_stream_get_type
G_DEFINE_TYPE_WITH_CODE(GSocketOutputStream, g_socket_output_stream, G_TYPE_OUTPUT_STREAM,G_IMPLEMENT_INTERFACE(G_TYPE_POLLABLE_OUTPUT_STREAM, g_socket_output_stream_pollable_iface_init));
enum {
  PROP_0,
  PROP_SOCKET
};
struct _GSocketOutputStreamPrivate {
  GSocket *socket;
  GSimpleAsyncResult *result;
  GCancellable *cancellable;
  gconstpointer buffer;
  gsize count;
};
static void g_socket_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocketOutputStream *stream = G_SOCKET_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_SOCKET: g_value_set_object(value, stream->priv->socket); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSocketOutputStream *stream = G_SOCKET_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_SOCKET: stream->priv->socket = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_output_stream_finalize(GObject *object) {
  GSocketOutputStream *stream = G_SOCKET_OUTPUT_STREAM(object);
  if (stream->priv->socket) g_object_unref(stream->priv->socket);
  if (G_OBJECT_CLASS(g_socket_output_stream_parent_class)->finalize) (*G_OBJECT_CLASS(g_socket_output_stream_parent_class)->finalize)(object);
}
static gssize g_socket_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GSocketOutputStream *onput_stream = G_SOCKET_OUTPUT_STREAM(stream);
  return g_socket_send_with_blocking(onput_stream->priv->socket, buffer, count, TRUE, cancellable, error);
}
static gboolean g_socket_output_stream_write_ready(GSocket *socket, GIOCondition condition, GSocketOutputStream *stream) {
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  gssize result;
  result = g_socket_send_with_blocking(stream->priv->socket, stream->priv->buffer, stream->priv->count, FALSE, stream->priv->cancellable, &error);
  if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) return TRUE;
  simple = stream->priv->result;
  stream->priv->result = NULL;
  if (result >= 0) g_simple_async_result_set_op_res_gssize(simple, result);
  if (error) g_simple_async_result_take_error(simple, error);
  if (stream->priv->cancellable) g_object_unref(stream->priv->cancellable);
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
  return FALSE;
}
static void g_socket_output_stream_write_async(GOutputStream *stream, const void *buffer, gsize count, gint io_priority, GCancellable *cancellable,
                                               GAsyncReadyCallback callback, gpointer user_data) {
  GSocketOutputStream *output_stream = G_SOCKET_OUTPUT_STREAM(stream);
  GSource *source;
  g_assert(output_stream->priv->result == NULL);
  output_stream->priv->result = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_socket_output_stream_write_async);
  if (cancellable) g_object_ref(cancellable);
  output_stream->priv->cancellable = cancellable;
  output_stream->priv->buffer = buffer;
  output_stream->priv->count = count;
  source = g_socket_create_source(output_stream->priv->socket,G_IO_OUT | G_IO_HUP | G_IO_ERR, cancellable);
  g_source_set_callback(source, (GSourceFunc)g_socket_output_stream_write_ready, g_object_ref(output_stream), g_object_unref);
  g_source_attach(source, g_main_context_get_thread_default());
  g_source_unref(source);
}
static gssize g_socket_output_stream_write_finish(GOutputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  gssize count;
  g_return_val_if_fail(G_IS_SOCKET_OUTPUT_STREAM(stream), -1);
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_socket_output_stream_write_async);
  count = g_simple_async_result_get_op_res_gssize(simple);
  return count;
}
static gboolean g_socket_output_stream_pollable_is_writable(GPollableOutputStream *pollable) {
  GSocketOutputStream *output_stream = G_SOCKET_OUTPUT_STREAM(pollable);
  return g_socket_condition_check(output_stream->priv->socket, G_IO_OUT);
}
static GSource *g_socket_output_stream_pollable_create_source(GPollableOutputStream *pollable, GCancellable *cancellable) {
  GSocketOutputStream *output_stream = G_SOCKET_OUTPUT_STREAM(pollable);
  GSource *socket_source, *pollable_source;
  pollable_source = g_pollable_source_new(G_OBJECT(output_stream));
  socket_source = g_socket_create_source(output_stream->priv->socket,G_IO_OUT, cancellable);
  g_source_set_dummy_callback(socket_source);
  g_source_add_child_source(pollable_source, socket_source);
  g_source_unref(socket_source);
  return pollable_source;
}
static gssize g_socket_output_stream_pollable_write_nonblocking(GPollableOutputStream *pollable, const void *buffer, gsize size, GError **error) {
  GSocketOutputStream *output_stream = G_SOCKET_OUTPUT_STREAM(pollable);
  return g_socket_send_with_blocking(output_stream->priv->socket, buffer, size, FALSE,NULL, error);
}
static void g_socket_output_stream_class_init(GSocketOutputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GOutputStreamClass *goutputstream_class = G_OUTPUT_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GSocketOutputStreamPrivate));
  gobject_class->finalize = g_socket_output_stream_finalize;
  gobject_class->get_property = g_socket_output_stream_get_property;
  gobject_class->set_property = g_socket_output_stream_set_property;
  goutputstream_class->write_fn = g_socket_output_stream_write;
  goutputstream_class->write_async = g_socket_output_stream_write_async;
  goutputstream_class->write_finish = g_socket_output_stream_write_finish;
  g_object_class_install_property(gobject_class, PROP_SOCKET, g_param_spec_object("socket", P_("socket"), P_("The socket that this stream wraps"),
							      G_TYPE_SOCKET, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_socket_output_stream_pollable_iface_init(GPollableOutputStreamInterface *iface) {
  iface->is_writable = g_socket_output_stream_pollable_is_writable;
  iface->create_source = g_socket_output_stream_pollable_create_source;
  iface->write_nonblocking = g_socket_output_stream_pollable_write_nonblocking;
}
static void g_socket_output_stream_init(GSocketOutputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_SOCKET_OUTPUT_STREAM, GSocketOutputStreamPrivate);
}
GSocketOutputStream *_g_socket_output_stream_new(GSocket *socket) {
  return G_SOCKET_OUTPUT_STREAM(g_object_new(G_TYPE_SOCKET_OUTPUT_STREAM, "socket", socket, NULL));
}