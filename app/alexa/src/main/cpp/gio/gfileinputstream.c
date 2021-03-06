#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "gfileinputstream.h"
#include "gseekable.h"
#include "config.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gasyncresult.h"
#include "gioerror.h"

static void g_file_input_stream_seekable_iface_init(GSeekableIface *iface);
static goffset g_file_input_stream_seekable_tell(GSeekable *seekable);
static gboolean g_file_input_stream_seekable_can_seek(GSeekable *seekable);
static gboolean g_file_input_stream_seekable_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error);
static gboolean g_file_input_stream_seekable_can_truncate(GSeekable *seekable);
static gboolean g_file_input_stream_seekable_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error);
static void g_file_input_stream_real_query_info_async(GFileInputStream *stream, const char *attributes, int io_priority, GCancellable *cancellable,
							                          GAsyncReadyCallback callback, gpointer user_data);
static GFileInfo *g_file_input_stream_real_query_info_finish(GFileInputStream *stream, GAsyncResult *result, GError **error);
G_DEFINE_TYPE_WITH_CODE(GFileInputStream, g_file_input_stream, G_TYPE_INPUT_STREAM,G_IMPLEMENT_INTERFACE(G_TYPE_SEEKABLE, g_file_input_stream_seekable_iface_init));
struct _GFileInputStreamPrivate {
  GAsyncReadyCallback outstanding_callback;
};
static void g_file_input_stream_class_init(GFileInputStreamClass *klass) {
  g_type_class_add_private(klass, sizeof(GFileInputStreamPrivate));
  klass->query_info_async = g_file_input_stream_real_query_info_async;
  klass->query_info_finish = g_file_input_stream_real_query_info_finish;
}
static void g_file_input_stream_seekable_iface_init(GSeekableIface *iface) {
  iface->tell = g_file_input_stream_seekable_tell;
  iface->can_seek = g_file_input_stream_seekable_can_seek;
  iface->seek = g_file_input_stream_seekable_seek;
  iface->can_truncate = g_file_input_stream_seekable_can_truncate;
  iface->truncate_fn = g_file_input_stream_seekable_truncate;
}
static void g_file_input_stream_init(GFileInputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_FILE_INPUT_STREAM, GFileInputStreamPrivate);
}
GFileInfo *g_file_input_stream_query_info(GFileInputStream *stream, const char *attributes, GCancellable *cancellable, GError **error) {
  GFileInputStreamClass *class;
  GInputStream *input_stream;
  GFileInfo *info;
  g_return_val_if_fail(G_IS_FILE_INPUT_STREAM(stream), NULL);
  input_stream = G_INPUT_STREAM(stream);
  if (!g_input_stream_set_pending(input_stream, error)) return NULL;
  info = NULL;
  if (cancellable) g_cancellable_push_current(cancellable);
  class = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  if (class->query_info) info = class->query_info(stream, attributes, cancellable, error);
  else g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Stream doesn't support query_info"));
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(input_stream);
  return info;
}
static void async_ready_callback_wrapper(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GFileInputStream *stream = G_FILE_INPUT_STREAM(source_object);
  g_input_stream_clear_pending(G_INPUT_STREAM(stream));
  if (stream->priv->outstanding_callback)(*stream->priv->outstanding_callback)(source_object, res, user_data);
  g_object_unref(stream);
}
void g_file_input_stream_query_info_async(GFileInputStream *stream, const char *attributes, int io_priority, GCancellable *cancellable, GAsyncReadyCallback callback,
                                          gpointer user_data) {
  GFileInputStreamClass *klass;
  GInputStream *input_stream;
  GError *error = NULL;
  g_return_if_fail(G_IS_FILE_INPUT_STREAM(stream));
  input_stream = G_INPUT_STREAM(stream);
  if (!g_input_stream_set_pending(input_stream, &error)) {
      g_simple_async_report_take_gerror_in_idle(G_OBJECT(stream), callback, user_data, error);
      return;
  }
  klass = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  stream->priv->outstanding_callback = callback;
  g_object_ref(stream);
  klass->query_info_async(stream, attributes, io_priority, cancellable, async_ready_callback_wrapper, user_data);
}
GFileInfo *g_file_input_stream_query_info_finish(GFileInputStream *stream, GAsyncResult *result, GError **error) {
  GSimpleAsyncResult *simple;
  GFileInputStreamClass *class;
  g_return_val_if_fail(G_IS_FILE_INPUT_STREAM(stream), NULL);
  g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);
  if (G_IS_SIMPLE_ASYNC_RESULT(result)) {
      simple = G_SIMPLE_ASYNC_RESULT(result);
      if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  }
  class = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  return class->query_info_finish(stream, result, error);
}
static goffset g_file_input_stream_tell(GFileInputStream *stream) {
  GFileInputStreamClass *class;
  goffset offset;
  g_return_val_if_fail(G_IS_FILE_INPUT_STREAM(stream), 0);
  class = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  offset = 0;
  if (class->tell) offset = class->tell(stream);
  return offset;
}
static goffset g_file_input_stream_seekable_tell(GSeekable *seekable) {
  return g_file_input_stream_tell(G_FILE_INPUT_STREAM(seekable));
}
static gboolean g_file_input_stream_can_seek(GFileInputStream *stream) {
  GFileInputStreamClass *class;
  gboolean can_seek;
  g_return_val_if_fail(G_IS_FILE_INPUT_STREAM(stream), FALSE);
  class = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  can_seek = FALSE;
  if (class->seek) {
      can_seek = TRUE;
      if (class->can_seek) can_seek = class->can_seek(stream);
  }
  return can_seek;
}
static gboolean g_file_input_stream_seekable_can_seek(GSeekable *seekable) {
  return g_file_input_stream_can_seek(G_FILE_INPUT_STREAM (seekable));
}
static gboolean g_file_input_stream_seek(GFileInputStream  *stream, goffset offset, GSeekType type, GCancellable *cancellable, GError **error) {
  GFileInputStreamClass *class;
  GInputStream *input_stream;
  gboolean res;
  g_return_val_if_fail(G_IS_FILE_INPUT_STREAM(stream), FALSE);
  input_stream = G_INPUT_STREAM(stream);
  class = G_FILE_INPUT_STREAM_GET_CLASS(stream);
  if (!class->seek) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Seek not supported on stream"));
      return FALSE;
  }
  if (!g_input_stream_set_pending(input_stream, error)) return FALSE;
  if (cancellable) g_cancellable_push_current(cancellable);
  res = class->seek(stream, offset, type, cancellable, error);
  if (cancellable) g_cancellable_pop_current(cancellable);
  g_input_stream_clear_pending(input_stream);
  return res;
}
static gboolean g_file_input_stream_seekable_seek(GSeekable *seekable, goffset offset, GSeekType type, GCancellable *cancellable, GError **error) {
  return g_file_input_stream_seek(G_FILE_INPUT_STREAM(seekable), offset, type, cancellable, error);
}
static gboolean g_file_input_stream_seekable_can_truncate(GSeekable *seekable) {
  return FALSE;
}
static gboolean g_file_input_stream_seekable_truncate(GSeekable *seekable, goffset offset, GCancellable *cancellable, GError **error) {
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Truncate not allowed on input stream"));
  return FALSE;
}
typedef struct {
  char *attributes;
  GFileInfo *info;
} QueryInfoAsyncData;
static void query_info_data_free(QueryInfoAsyncData *data) {
  if (data->info) g_object_unref(data->info);
  g_free(data->attributes);
  g_free(data);
}
static void query_info_async_thread(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable) {
  GFileInputStreamClass *class;
  GError *error = NULL;
  QueryInfoAsyncData *data;
  GFileInfo *info;
  data = g_simple_async_result_get_op_res_gpointer(res);
  info = NULL;
  class = G_FILE_INPUT_STREAM_GET_CLASS(object);
  if (class->query_info) info = class->query_info(G_FILE_INPUT_STREAM(object), data->attributes, cancellable, &error);
  else g_set_error_literal(&error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Stream doesn't support query_info"));
  if (info == NULL) g_simple_async_result_take_error(res, error);
  else data->info = info;
}
static void g_file_input_stream_real_query_info_async(GFileInputStream *stream, const char *attributes, int io_priority, GCancellable *cancellable,
                                                      GAsyncReadyCallback callback, gpointer user_data) {
  GSimpleAsyncResult *res;
  QueryInfoAsyncData *data;
  data = g_new0(QueryInfoAsyncData, 1);
  data->attributes = g_strdup(attributes);
  res = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, g_file_input_stream_real_query_info_async);
  g_simple_async_result_set_op_res_gpointer(res, data, (GDestroyNotify)query_info_data_free);
  g_simple_async_result_run_in_thread(res, query_info_async_thread, io_priority, cancellable);
  g_object_unref(res);
}
static GFileInfo *g_file_input_stream_real_query_info_finish(GFileInputStream *stream, GAsyncResult *res, GError **error) {
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT(res);
  QueryInfoAsyncData *data;
  g_warn_if_fail(g_simple_async_result_get_source_tag(simple) == g_file_input_stream_real_query_info_async);
  data = g_simple_async_result_get_op_res_gpointer(simple);
  if (data->info) return g_object_ref(data->info);
  return NULL;
}