#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gdatainputstream.h"
#include "gsimpleasyncresult.h"
#include "gcancellable.h"
#include "gioenumtypes.h"
#include "gioerror.h"

struct _GDataInputStreamPrivate {
  GDataStreamByteOrder byte_order;
  GDataStreamNewlineType newline_type;
};
enum {
  PROP_0,
  PROP_BYTE_ORDER,
  PROP_NEWLINE_TYPE
};
static void g_data_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_data_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
G_DEFINE_TYPE(GDataInputStream, g_data_input_stream, G_TYPE_BUFFERED_INPUT_STREAM)
static void g_data_input_stream_class_init(GDataInputStreamClass *klass) {
  GObjectClass *object_class;
  g_type_class_add_private(klass, sizeof(GDataInputStreamPrivate));
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_data_input_stream_get_property;
  object_class->set_property = g_data_input_stream_set_property;
  g_object_class_install_property(object_class, PROP_BYTE_ORDER, g_param_spec_enum("byte-order", "Byte order", "The byte order",
                                  G_TYPE_DATA_STREAM_BYTE_ORDER, G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN, G_PARAM_READWRITE |
                                  G_PARAM_STATIC_NAME|G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class, PROP_NEWLINE_TYPE, g_param_spec_enum("newline-type", "Newline type",
                                  "The accepted types of line ending", G_TYPE_DATA_STREAM_NEWLINE_TYPE, G_DATA_STREAM_NEWLINE_TYPE_LF,
                                  G_PARAM_READWRITE | G_PARAM_STATIC_NAME|G_PARAM_STATIC_BLURB));
}
static void g_data_input_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GDataInputStream *dstream;
  dstream = G_DATA_INPUT_STREAM(object);
  switch(prop_id) {
      case PROP_BYTE_ORDER: g_data_input_stream_set_byte_order(dstream, g_value_get_enum(value)); break;
      case PROP_NEWLINE_TYPE: g_data_input_stream_set_newline_type (dstream, g_value_get_enum(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
  }
}
static void g_data_input_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GDataInputStreamPrivate *priv;
  GDataInputStream *dstream;
  dstream = G_DATA_INPUT_STREAM(object);
  priv = dstream->priv;
  switch(prop_id) {
      case PROP_BYTE_ORDER: g_value_set_enum(value, priv->byte_order); break;
      case PROP_NEWLINE_TYPE: g_value_set_enum(value, priv->newline_type); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_data_input_stream_init(GDataInputStream *stream) {
  stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream, G_TYPE_DATA_INPUT_STREAM, GDataInputStreamPrivate);
  stream->priv->byte_order = G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN;
  stream->priv->newline_type = G_DATA_STREAM_NEWLINE_TYPE_LF;
}
GDataInputStream *g_data_input_stream_new(GInputStream *base_stream) {
  GDataInputStream *stream;
  g_return_val_if_fail(G_IS_INPUT_STREAM(base_stream), NULL);
  stream = g_object_new(G_TYPE_DATA_INPUT_STREAM,"base-stream", base_stream, NULL);
  return stream;
}
void g_data_input_stream_set_byte_order(GDataInputStream *stream, GDataStreamByteOrder order) {
  GDataInputStreamPrivate *priv;
  g_return_if_fail(G_IS_DATA_INPUT_STREAM(stream));
  priv = stream->priv;
  if (priv->byte_order != order) {
      priv->byte_order = order;
      g_object_notify(G_OBJECT(stream), "byte-order");
  }
}
GDataStreamByteOrder g_data_input_stream_get_byte_order(GDataInputStream *stream) {
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN);
  return stream->priv->byte_order;
}
void g_data_input_stream_set_newline_type(GDataInputStream *stream, GDataStreamNewlineType type) {
  GDataInputStreamPrivate *priv;
  g_return_if_fail(G_IS_DATA_INPUT_STREAM(stream));
  priv = stream->priv;
  if (priv->newline_type != type) {
      priv->newline_type = type;
      g_object_notify(G_OBJECT(stream), "newline-type");
  }
}
GDataStreamNewlineType g_data_input_stream_get_newline_type(GDataInputStream *stream) {
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), G_DATA_STREAM_NEWLINE_TYPE_ANY);
  return stream->priv->newline_type;
}
static gboolean read_data(GDataInputStream *stream, void *buffer, gsize size, GCancellable *cancellable, GError **error) {
  gsize available;
  gssize res;
  while((available = g_buffered_input_stream_get_available(G_BUFFERED_INPUT_STREAM(stream))) < size) {
      res = g_buffered_input_stream_fill(G_BUFFERED_INPUT_STREAM(stream),size - available, cancellable, error);
      if (res < 0) return FALSE;
      if (res == 0) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Unexpected early end-of-stream");
          return FALSE;
	  }
  }
  res = g_input_stream_read(G_INPUT_STREAM(stream), buffer, size,NULL, NULL);
  g_warn_if_fail(res == size);
  return TRUE;
}
guchar g_data_input_stream_read_byte(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  guchar c;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), '\0');
  if (read_data(stream, &c, 1, cancellable, error)) return c;
  return 0;
}
gint16 g_data_input_stream_read_int16(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  gint16 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v, 2, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GINT16_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GINT16_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
guint16 g_data_input_stream_read_uint16(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  guint16 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v, 2, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GUINT16_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GUINT16_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
gint32 g_data_input_stream_read_int32(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  gint32 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v, 4, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GINT32_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GINT32_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
guint32 g_data_input_stream_read_uint32(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  guint32 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v, 4, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GUINT32_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GUINT32_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
gint64 g_data_input_stream_read_int64(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  gint64 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v,8, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GINT64_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GINT64_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
guint64 g_data_input_stream_read_uint64(GDataInputStream *stream, GCancellable *cancellable, GError **error) {
  guint64 v;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), 0);
  if (read_data(stream, &v, 8, cancellable, error)) {
      switch(stream->priv->byte_order) {
          case G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN: v = GUINT64_FROM_BE(v); break;
          case G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN: v = GUINT64_FROM_LE(v); break;
	  }
      return v;
  }
  return 0;
}
static gssize scan_for_newline(GDataInputStream *stream, gsize *checked_out, gboolean *last_saw_cr_out, int *newline_len_out) {
  GBufferedInputStream *bstream;
  GDataInputStreamPrivate *priv;
  const char *buffer;
  gsize start, end, peeked;
  int i;
  gssize found_pos;
  int newline_len;
  gsize available, checked;
  gboolean last_saw_cr;
  priv = stream->priv;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  checked = *checked_out;
  last_saw_cr = *last_saw_cr_out;
  found_pos = -1;
  newline_len = 0;
  start = checked;
  buffer = (const char*)g_buffered_input_stream_peek_buffer(bstream, &available) + start;
  end = available;
  peeked = end - start;
  for (i = 0; checked < available && i < peeked; i++) {
      switch (priv->newline_type) {
          case G_DATA_STREAM_NEWLINE_TYPE_LF:
              if (buffer[i] == 10) {
                  found_pos = start + i;
                  newline_len = 1;
              }
              break;
          case G_DATA_STREAM_NEWLINE_TYPE_CR:
              if (buffer[i] == 13) {
                  found_pos = start + i;
                  newline_len = 1;
              }
              break;
          case G_DATA_STREAM_NEWLINE_TYPE_CR_LF:
              if (last_saw_cr && buffer[i] == 10) {
                  found_pos = start + i - 1;
                  newline_len = 2;
              }
              break;
          default:
              case G_DATA_STREAM_NEWLINE_TYPE_ANY:
                  if (buffer[i] == 10) {
                      if (last_saw_cr) {
                          found_pos = start + i - 1;
                          newline_len = 2;
                      } else {
                          found_pos = start + i;
                          newline_len = 1;
                      }
                  } else if (last_saw_cr) {
                      found_pos = start + i - 1;
                      newline_len = 1;
                  }
	          break;
	  }
      last_saw_cr = (buffer[i] == 13);
      if (found_pos != -1) {
          *newline_len_out = newline_len;
          return found_pos;
	  }
  }
  checked = end;
  *checked_out = checked;
  *last_saw_cr_out = last_saw_cr;
  return -1;
}
char *g_data_input_stream_read_line(GDataInputStream *stream, gsize *length, GCancellable *cancellable, GError **error) {
  GBufferedInputStream *bstream;
  gsize checked;
  gboolean last_saw_cr;
  gssize found_pos;
  gssize res;
  int newline_len;
  char *line;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), NULL);
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  newline_len = 0;
  checked = 0;
  last_saw_cr = FALSE;
  while((found_pos = scan_for_newline (stream, &checked, &last_saw_cr, &newline_len)) == -1) {
      if (g_buffered_input_stream_get_available(bstream) == g_buffered_input_stream_get_buffer_size(bstream))
	      g_buffered_input_stream_set_buffer_size(bstream,2 * g_buffered_input_stream_get_buffer_size(bstream));
      res = g_buffered_input_stream_fill(bstream, -1, cancellable, error);
      if (res < 0) return NULL;
      if (res == 0) {
	  if (g_buffered_input_stream_get_available(bstream) == 0) {
	      if (length) *length = 0;
	      return NULL;
	  } else {
	      found_pos = checked;
	      newline_len = 0;
	      break;
	  }
	  }
  }
  line = g_malloc(found_pos + newline_len + 1);
  res = g_input_stream_read(G_INPUT_STREAM(stream), line,found_pos + newline_len,NULL, NULL);
  if (length) *length = (gsize)found_pos;
  g_warn_if_fail(res == found_pos + newline_len);
  line[found_pos] = 0;
  return line;
}
static gssize scan_for_chars(GDataInputStream *stream, gsize *checked_out, const char *stop_chars, gssize stop_chars_len) {
  GBufferedInputStream *bstream;
  const char *buffer;
  gsize start, end, peeked;
  int i;
  gsize available, checked;
  const char *stop_char;
  const char *stop_end;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  stop_end = stop_chars + stop_chars_len;
  checked = *checked_out;
  start = checked;
  buffer = (const char*)g_buffered_input_stream_peek_buffer(bstream, &available) + start;
  end = available;
  peeked = end - start;
  for (i = 0; checked < available && i < peeked; i++) {
      for (stop_char = stop_chars; stop_char != stop_end; stop_char++) {
	      if (buffer[i] == *stop_char) return (start + i);
	  }
  }
  checked = end;
  *checked_out = checked;
  return -1;
}
char *g_data_input_stream_read_until(GDataInputStream *stream, const gchar *stop_chars, gsize *length, GCancellable *cancellable, GError **error) {
  GBufferedInputStream *bstream;
  gchar *result;
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  result = g_data_input_stream_read_upto(stream, stop_chars, -1, length, cancellable, error);
  if (result != NULL && g_buffered_input_stream_get_available(bstream) > 0) {
      gsize res;
      gchar b;
      res = g_input_stream_read(G_INPUT_STREAM(stream), &b, 1, NULL, NULL);
      g_assert(res == 1);
  }
  return result;
}
typedef struct {
  GDataInputStream *stream;
  GSimpleAsyncResult *simple;
  gboolean last_saw_cr;
  gsize checked;
  gint io_priority;
  GCancellable *cancellable;
  gchar *stop_chars;
  gssize stop_chars_len;
  gchar *line;
  gsize length;
} GDataInputStreamReadData;
static void g_data_input_stream_read_complete(GDataInputStreamReadData *data, gsize read_length, gsize skip_length, gboolean need_idle_dispatch) {
  if (read_length || skip_length) {
      gssize bytes;
      data->length = read_length;
      data->line = g_malloc(read_length + 1);
      data->line[read_length] = '\0';
      bytes = g_input_stream_read(G_INPUT_STREAM(data->stream), data->line, read_length, NULL, NULL);
      g_assert_cmpint(bytes, ==, read_length);
      bytes = g_input_stream_skip(G_INPUT_STREAM(data->stream), skip_length, NULL, NULL);
      g_assert_cmpint(bytes, ==, skip_length);
  }
  if (need_idle_dispatch) g_simple_async_result_complete_in_idle(data->simple);
  else g_simple_async_result_complete(data->simple);
  g_object_unref(data->simple);
}
static void g_data_input_stream_read_line_ready(GObject *object, GAsyncResult *result, gpointer user_data) {
  GDataInputStreamReadData *data = user_data;
  gssize found_pos;
  gint newline_len;
  if (result) {
      GBufferedInputStream *buffer = G_BUFFERED_INPUT_STREAM(data->stream);
      GError *error = NULL;
      gssize bytes;
      bytes = g_buffered_input_stream_fill_finish(buffer, result, &error);
      if (bytes <= 0) {
          if (bytes < 0) {
              g_simple_async_result_take_error(data->simple, error);
              data->checked = 0;
          }
          g_data_input_stream_read_complete(data, data->checked, 0, FALSE);
          return;
      }
  }
  if (data->stop_chars) {
      found_pos = scan_for_chars(data->stream, &data->checked, data->stop_chars, data->stop_chars_len);
      newline_len = 0;
  } else found_pos = scan_for_newline (data->stream, &data->checked, &data->last_saw_cr, &newline_len);
  if (found_pos == -1) {
      GBufferedInputStream *buffer = G_BUFFERED_INPUT_STREAM(data->stream);
      gsize size;
      size = g_buffered_input_stream_get_buffer_size(buffer);
      if (g_buffered_input_stream_get_available(buffer) == size) g_buffered_input_stream_set_buffer_size(buffer, size * 2);
      g_buffered_input_stream_fill_async(buffer, -1, data->io_priority, data->cancellable, g_data_input_stream_read_line_ready, user_data);
  } else g_data_input_stream_read_complete(data, found_pos, newline_len, result == NULL);
}
static void g_data_input_stream_read_data_free(gpointer user_data) {
  GDataInputStreamReadData *data = user_data;
  g_free(data->stop_chars);
  if (data->cancellable) g_object_unref(data->cancellable);
  g_free(data->line);
  g_slice_free(GDataInputStreamReadData, data);
}
static void g_data_input_stream_read_async(GDataInputStream *stream, const gchar *stop_chars, gssize stop_chars_len, gint io_priority, GCancellable *cancellable,
                                           GAsyncReadyCallback callback, gpointer user_data, gpointer source_tag) {
  GDataInputStreamReadData *data;
  data = g_slice_new(GDataInputStreamReadData);
  data->stream = stream;
  if (cancellable) g_object_ref(cancellable);
  data->cancellable = cancellable;
  if (stop_chars_len == -1) stop_chars_len = strlen(stop_chars);
  data->stop_chars = g_memdup(stop_chars, stop_chars_len);
  data->stop_chars_len = stop_chars_len;
  data->io_priority = io_priority;
  data->last_saw_cr = FALSE;
  data->checked = 0;
  data->line = NULL;
  data->simple = g_simple_async_result_new(G_OBJECT(stream), callback, user_data, source_tag);
  g_simple_async_result_set_op_res_gpointer(data->simple, data, g_data_input_stream_read_data_free);
  g_data_input_stream_read_line_ready(NULL, NULL, data);
}
static gchar *g_data_input_stream_read_finish(GDataInputStream  *stream, GAsyncResult *result, gsize *length, GError **error) {
  GDataInputStreamReadData *data;
  GSimpleAsyncResult *simple;
  gchar *line;
  simple = G_SIMPLE_ASYNC_RESULT(result);
  if (g_simple_async_result_propagate_error(simple, error)) return NULL;
  data = g_simple_async_result_get_op_res_gpointer(simple);
  line = data->line;
  data->line = NULL;
  if (length && line) *length = data->length;
  return line;
}
void g_data_input_stream_read_line_async(GDataInputStream *stream, gint io_priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_DATA_INPUT_STREAM(stream));
  g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
  g_data_input_stream_read_async(stream, NULL, 0, io_priority, cancellable, callback, user_data, g_data_input_stream_read_line_async);
}
void g_data_input_stream_read_until_async(GDataInputStream *stream, const gchar *stop_chars, gint io_priority, GCancellable *cancellable,
                                          GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_DATA_INPUT_STREAM(stream));
  g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
  g_return_if_fail(stop_chars != NULL);
  g_data_input_stream_read_async(stream, stop_chars,-1, io_priority, cancellable, callback, user_data, g_data_input_stream_read_until_async);
}
gchar *g_data_input_stream_read_line_finish(GDataInputStream *stream, GAsyncResult *result, gsize *length, GError **error) {
  g_return_val_if_fail(g_simple_async_result_is_valid(result, G_OBJECT(stream), g_data_input_stream_read_line_async), NULL);
  return g_data_input_stream_read_finish(stream, result, length, error);
}
gchar *g_data_input_stream_read_until_finish(GDataInputStream *stream, GAsyncResult *result, gsize *length, GError **error) {
  g_return_val_if_fail(g_simple_async_result_is_valid(result, G_OBJECT(stream), g_data_input_stream_read_until_async), NULL);
  return g_data_input_stream_read_finish(stream, result, length, error);
}
char *g_data_input_stream_read_upto(GDataInputStream *stream, const gchar *stop_chars, gssize stop_chars_len, gsize *length, GCancellable *cancellable,
                                    GError **error) {
  GBufferedInputStream *bstream;
  gsize checked;
  gssize found_pos;
  gssize res;
  char *data_until;
  g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(stream), NULL);
  if (stop_chars_len < 0) stop_chars_len = strlen(stop_chars);
  bstream = G_BUFFERED_INPUT_STREAM(stream);
  checked = 0;
  while((found_pos = scan_for_chars(stream, &checked, stop_chars, stop_chars_len)) == -1) {
      if (g_buffered_input_stream_get_available(bstream) == g_buffered_input_stream_get_buffer_size(bstream))
          g_buffered_input_stream_set_buffer_size(bstream,2 * g_buffered_input_stream_get_buffer_size(bstream));
      res = g_buffered_input_stream_fill(bstream, -1, cancellable, error);
      if (res < 0) return NULL;
      if (res == 0) {
          if (g_buffered_input_stream_get_available(bstream) == 0) {
              if (length) *length = 0;
              return NULL;
          } else {
              found_pos = checked;
              break;
          }
      }
  }
  data_until = g_malloc(found_pos + 1);
  res = g_input_stream_read(G_INPUT_STREAM(stream), data_until, found_pos,NULL, NULL);
  if (length) *length = (gsize)found_pos;
  g_warn_if_fail(res == found_pos);
  data_until[found_pos] = 0;
  return data_until;
}
void g_data_input_stream_read_upto_async(GDataInputStream *stream, const gchar *stop_chars, gssize stop_chars_len, gint io_priority, GCancellable *cancellable,
                                         GAsyncReadyCallback callback, gpointer user_data) {
  g_return_if_fail(G_IS_DATA_INPUT_STREAM(stream));
  g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
  g_return_if_fail(stop_chars != NULL);
  g_data_input_stream_read_async(stream, stop_chars, stop_chars_len, io_priority, cancellable, callback, user_data, g_data_input_stream_read_upto_async);
}
gchar * g_data_input_stream_read_upto_finish(GDataInputStream *stream, GAsyncResult *result, gsize *length, GError **error) {
  g_return_val_if_fail(g_simple_async_result_is_valid(result, G_OBJECT (stream), g_data_input_stream_read_upto_async), NULL);
  return g_data_input_stream_read_finish(stream, result, length, error);
}