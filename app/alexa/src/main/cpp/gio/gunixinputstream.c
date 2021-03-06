#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include "../glib/glib.h"
#include "../glib/gstdio.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gioerror.h"
#include "gsimpleasyncresult.h"
#include "gunixinputstream.h"
#include "gcancellable.h"
#include "gasynchelper.h"

enum {
  PROP_0,
  PROP_FD,
  PROP_CLOSE_FD
};
static void g_unix_input_stream_pollable_iface_init (GPollableInputStreamInterface *iface);
G_DEFINE_TYPE_WITH_CODE(GUnixInputStream, g_unix_input_stream, G_TYPE_INPUT_STREAM,G_IMPLEMENT_INTERFACE(G_TYPE_POLLABLE_INPUT_STREAM,
						g_unix_input_stream_pollable_iface_init));
struct _GUnixInputStreamPrivate {
  int fd;
  gboolean close_fd;
};
static void g_unix_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_unix_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static gssize g_unix_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_unix_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error);
static void g_unix_input_stream_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                           gpointer data);
static gssize g_unix_input_stream_read_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static void g_unix_input_stream_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                           gpointer data);
static gssize g_unix_input_stream_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static void g_unix_input_stream_close_async(GInputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
static gboolean g_unix_input_stream_close_finish(GInputStream *stream, GAsyncResult *result, GError **error);
static gboolean g_unix_input_stream_pollable_is_readable(GPollableInputStream *stream);
static GSource *g_unix_input_stream_pollable_create_source(GPollableInputStream *stream, GCancellable *cancellable);
static void g_unix_input_stream_finalize(GObject *object) {
  G_OBJECT_CLASS(g_unix_input_stream_parent_class)->finalize(object);
}
static void g_unix_input_stream_class_init(GUnixInputStreamClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GInputStreamClass *stream_class = G_INPUT_STREAM_CLASS(klass);
  g_type_class_add_private(klass, sizeof(GUnixInputStreamPrivate));
  gobject_class->get_property = g_unix_input_stream_get_property;
  gobject_class->set_property = g_unix_input_stream_set_property;
  gobject_class->finalize = g_unix_input_stream_finalize;
  stream_class->read_fn = g_unix_input_stream_read;
  stream_class->close_fn = g_unix_input_stream_close;
  stream_class->read_async = g_unix_input_stream_read_async;
  stream_class->read_finish = g_unix_input_stream_read_finish;
  if (0) {
      stream_class->skip_async = g_unix_input_stream_skip_async;
      stream_class->skip_finish = g_unix_input_stream_skip_finish;
  }
  stream_class->close_async = g_unix_input_stream_close_async;
  stream_class->close_finish = g_unix_input_stream_close_finish;
  g_object_class_install_property(gobject_class,PROP_FD,g_param_spec_int("fd", P_("File descriptor"), P_("The file descriptor"
                                  " to read from"),G_MININT, G_MAXINT,-1,G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY |
                                  G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
  g_object_class_install_property(gobject_class,PROP_CLOSE_FD,g_param_spec_boolean("close-fd", P_("Close file descriptor"),
							      P_("Whether to close the file descriptor when the stream is closed"), TRUE,G_PARAM_READABLE | G_PARAM_WRITABLE |
							      G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
}
static void g_unix_input_stream_pollable_iface_init(GPollableInputStreamInterface *iface) {
  iface->is_readable = g_unix_input_stream_pollable_is_readable;
  iface->create_source = g_unix_input_stream_pollable_create_source;
}
static void g_unix_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GUnixInputStream *unix_stream;
  unix_stream = G_UNIX_INPUT_STREAM(object);
  switch(prop_id) {
      case PROP_FD: unix_stream->priv->fd = g_value_get_int(value); break;
      case PROP_CLOSE_FD: unix_stream->priv->close_fd = g_value_get_boolean(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_unix_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GUnixInputStream *unix_stream;
  unix_stream = G_UNIX_INPUT_STREAM(object);
  switch(prop_id) {
      case PROP_FD: g_value_set_int(value, unix_stream->priv->fd); break;
      case PROP_CLOSE_FD: g_value_set_boolean(value, unix_stream->priv->close_fd); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_unix_input_stream_init(GUnixInputStream *unix_stream) {
  unix_stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(unix_stream, G_TYPE_UNIX_INPUT_STREAM, GUnixInputStreamPrivate);
  unix_stream->priv->fd = -1;
  unix_stream->priv->close_fd = TRUE;
}
GInputStream *g_unix_input_stream_new(gint fd, gboolean close_fd) {
  GUnixInputStream *stream;
  g_return_val_if_fail(fd != -1, NULL);
  stream = g_object_new(G_TYPE_UNIX_INPUT_STREAM,"fd", fd, "close-fd", close_fd, NULL);
  return G_INPUT_STREAM(stream);
}
void g_unix_input_stream_set_close_fd(GUnixInputStream *stream, gboolean close_fd) {
  g_return_if_fail(G_IS_UNIX_INPUT_STREAM(stream));
  close_fd = close_fd != FALSE;
  if (stream->priv->close_fd != close_fd) {
      stream->priv->close_fd = close_fd;
      g_object_notify(G_OBJECT(stream), "close-fd");
  }
}
gboolean g_unix_input_stream_get_close_fd(GUnixInputStream *stream) {
  g_return_val_if_fail(G_IS_UNIX_INPUT_STREAM(stream), FALSE);
  return stream->priv->close_fd;
}
gint g_unix_input_stream_get_fd(GUnixInputStream *stream) {
  g_return_val_if_fail(G_IS_UNIX_INPUT_STREAM(stream), -1);
  return stream->priv->fd;
}
static gssize g_unix_input_stream_read(GInputStream *stream, void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GUnixInputStream *unix_stream;
  gssize res;
  GPollFD poll_fds[2];
  int poll_ret;
  unix_stream = G_UNIX_INPUT_STREAM(stream);
  if (g_cancellable_make_pollfd(cancellable, &poll_fds[1])) {
      poll_fds[0].fd = unix_stream->priv->fd;
      poll_fds[0].events = G_IO_IN;
      do {
          poll_ret = g_poll(poll_fds, 2, -1);
      } while(poll_ret == -1 && errno == EINTR);
      g_cancellable_release_fd(cancellable);
      if (poll_ret == -1) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error reading from unix: %s"), g_strerror(errsv));
          return -1;
	  }
  }
  while(1) {
      if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
      res = read(unix_stream->priv->fd, buffer, count);
      if (res == -1) {
          int errsv = errno;
          if (errsv == EINTR) continue;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error reading from unix: %s"), g_strerror(errsv));
	  }
      break;
  }
  return res;
}
static gboolean g_unix_input_stream_close(GInputStream *stream, GCancellable *cancellable, GError **error) {
  GUnixInputStream *unix_stream;
  int res;
  unix_stream = G_UNIX_INPUT_STREAM(stream);
  if (!unix_stream->priv->close_fd) return TRUE;
  while(1) {
      res = close(unix_stream->priv->fd);
      if (res == -1) {
          int errsv = errno;
          g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error closing unix: %s"), g_strerror(errsv));
      }
      break;
  }
  return res != -1;
}
typedef struct {
  gsize count;
  void *buffer;
  GAsyncReadyCallback callback;
  gpointer user_data;
  GCancellable *cancellable;
  GUnixInputStream *stream;
} ReadAsyncData;
static gboolean read_async_cb(int fd, GIOCondition condition, ReadAsyncData *data) {
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  gssize count_read;
  while(1) {
      if (g_cancellable_set_error_if_cancelled(data->cancellable, &error)) {
          count_read = -1;
          break;
	  }
      count_read = read(data->stream->priv->fd, data->buffer, data->count);
      if (count_read == -1) {
          int errsv = errno;
          if (errsv == EINTR) continue;
          g_set_error(&error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error reading from unix: %s"), g_strerror(errsv));
	  }
      break;
  }
  simple = g_simple_async_result_new(G_OBJECT(data->stream), data->callback, data->user_data, g_unix_input_stream_read_async);
  g_simple_async_result_set_op_res_gssize(simple, count_read);
  if (count_read == -1) g_simple_async_result_take_error(simple, error);
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
  return FALSE;
}
static void g_unix_input_stream_read_async(GInputStream *stream, void *buffer, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                           gpointer user_data) {
  GSource *source;
  GUnixInputStream *unix_stream;
  ReadAsyncData *data;
  unix_stream = G_UNIX_INPUT_STREAM(stream);
  data = g_new0(ReadAsyncData, 1);
  data->count = count;
  data->buffer = buffer;
  data->callback = callback;
  data->user_data = user_data;
  data->cancellable = cancellable;
  data->stream = unix_stream;
  source = _g_fd_source_new(unix_stream->priv->fd,G_IO_IN, cancellable);
  g_source_set_name(source, "GUnixInputStream");
  g_source_set_callback(source, (GSourceFunc)read_async_cb, data, g_free);
  g_source_attach(source, g_main_context_get_thread_default());
  g_source_unref(source);
}
static gssize g_unix_input_stream_read_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  gssize nread;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_unix_input_stream_read_async);
  nread = g_simple_async_result_get_op_res_gssize(simple);
  return nread;
}
static void g_unix_input_stream_skip_async(GInputStream *stream, gsize count, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
				                           gpointer data) {}
static gssize g_unix_input_stream_skip_finish(GInputStream *stream, GAsyncResult *result, GError **error) {
  g_warn_if_reached();
  return 0;
}
typedef struct {
  GInputStream *stream;
  GAsyncReadyCallback callback;
  gpointer user_data;
} CloseAsyncData;
static void close_async_data_free(gpointer _data) {
  CloseAsyncData *data = _data;
  g_free(data);
}
static gboolean close_async_cb(CloseAsyncData *data) {
  GUnixInputStream *unix_stream;
  GSimpleAsyncResult *simple;
  GError *error = NULL;
  gboolean result;
  int res;
  unix_stream = G_UNIX_INPUT_STREAM(data->stream);
  if (!unix_stream->priv->close_fd) {
      result = TRUE;
      goto out;
  }
  while(1) {
      res = close(unix_stream->priv->fd);
      if (res == -1) {
          int errsv = errno;
          g_set_error(&error, G_IO_ERROR, g_io_error_from_errno(errsv), _("Error closing unix: %s"), g_strerror(errsv));
      }
      break;
  }
  result = res != -1;
out:
  simple = g_simple_async_result_new(G_OBJECT(data->stream), data->callback, data->user_data, g_unix_input_stream_close_async);
  if (!result) g_simple_async_result_take_error(simple, error);
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
  return FALSE;
}
static void g_unix_input_stream_close_async(GInputStream *stream, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GSource *idle;
  CloseAsyncData *data;
  data = g_new0(CloseAsyncData, 1);
  data->stream = stream;
  data->callback = callback;
  data->user_data = user_data;
  idle = g_idle_source_new();
  g_source_set_callback(idle, (GSourceFunc)close_async_cb, data, close_async_data_free);
  g_source_attach(idle, g_main_context_get_thread_default());
  g_source_unref(idle);
}
static gboolean g_unix_input_stream_close_finish(GInputStream  *stream, GAsyncResult *result, GError **error) {
  return TRUE;
}
static gboolean g_unix_input_stream_pollable_is_readable(GPollableInputStream *stream) {
  GUnixInputStream *unix_stream = G_UNIX_INPUT_STREAM(stream);
  GPollFD poll_fd;
  gint result;
  poll_fd.fd = unix_stream->priv->fd;
  poll_fd.events = G_IO_IN;
  do {
      result = g_poll(&poll_fd, 1, 0);
  } while(result == -1 && errno == EINTR);
  return poll_fd.revents != 0;
}
static GSource *g_unix_input_stream_pollable_create_source(GPollableInputStream *stream, GCancellable *cancellable) {
  GUnixInputStream *unix_stream = G_UNIX_INPUT_STREAM(stream);
  GSource *inner_source, *pollable_source;
  pollable_source = g_pollable_source_new(G_OBJECT(stream));
  inner_source = _g_fd_source_new(unix_stream->priv->fd, G_IO_IN, cancellable);
  g_source_set_dummy_callback(inner_source);
  g_source_add_child_source(pollable_source, inner_source);
  g_source_unref(inner_source);
  return pollable_source;
}