#include "../glib/glibintl.h"
#include "../gobject/gparam.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gfilteroutputstream.h"
#include "gsimpleasyncresult.h"
#include "goutputstream.h"

enum {
  PROP_0,
  PROP_BASE_STREAM,
  PROP_CLOSE_BASE
};
static void g_filter_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void g_filter_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void g_filter_output_stream_dispose(GObject *object);
static gssize g_filter_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error);
static gboolean g_filter_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error);
static gboolean g_filter_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error);
G_DEFINE_ABSTRACT_TYPE(GFilterOutputStream, g_filter_output_stream, G_TYPE_OUTPUT_STREAM);
#define GET_PRIVATE(inst) G_TYPE_INSTANCE_GET_PRIVATE(inst, G_TYPE_FILTER_OUTPUT_STREAM, GFilterOutputStreamPrivate)
typedef struct {
  gboolean close_base;
} GFilterOutputStreamPrivate;
static void g_filter_output_stream_class_init(GFilterOutputStreamClass *klass) {
  GObjectClass *object_class;
  GOutputStreamClass *ostream_class;
  object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = g_filter_output_stream_get_property;
  object_class->set_property = g_filter_output_stream_set_property;
  object_class->dispose = g_filter_output_stream_dispose;
  ostream_class = G_OUTPUT_STREAM_CLASS(klass);
  ostream_class->write_fn = g_filter_output_stream_write;
  ostream_class->flush = g_filter_output_stream_flush;
  ostream_class->close_fn = g_filter_output_stream_close;
  g_type_class_add_private(klass, sizeof(GFilterOutputStreamPrivate));
  g_object_class_install_property(object_class, PROP_BASE_STREAM, g_param_spec_object("base-stream","The Filter Base Stream",
                                  "The underlying base stream on which the io ops will be done.", G_TYPE_OUTPUT_STREAM, G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
  g_object_class_install_property(object_class,PROP_CLOSE_BASE,g_param_spec_boolean ("close-base-stream","Close Base Stream",
                                  "If the base stream should be closed when the filter stream is closed.", TRUE,G_PARAM_READWRITE |
                                  G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB));
}
static void g_filter_output_stream_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GFilterOutputStream *filter_stream;
  GObject *obj;
  filter_stream = G_FILTER_OUTPUT_STREAM(object);
  switch(prop_id) {
      case PROP_BASE_STREAM:
          obj = g_value_dup_object(value);
          filter_stream->base_stream = G_OUTPUT_STREAM(obj);
          break;
      case PROP_CLOSE_BASE: g_filter_output_stream_set_close_base_stream(filter_stream, g_value_get_boolean(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_filter_output_stream_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GFilterOutputStream *filter_stream;
  filter_stream = G_FILTER_OUTPUT_STREAM(object);
  switch(prop_id) {
    case PROP_BASE_STREAM: g_value_set_object(value, filter_stream->base_stream); break;
    case PROP_CLOSE_BASE: g_value_set_boolean(value, GET_PRIVATE(filter_stream)->close_base); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_filter_output_stream_dispose(GObject *object) {
  GFilterOutputStream *stream;
  stream = G_FILTER_OUTPUT_STREAM(object);
  G_OBJECT_CLASS(g_filter_output_stream_parent_class)->dispose(object);
  if (stream->base_stream) {
      g_object_unref(stream->base_stream);
      stream->base_stream = NULL;
  }
}
static void g_filter_output_stream_init(GFilterOutputStream *stream) {}
GOutputStream *g_filter_output_stream_get_base_stream(GFilterOutputStream *stream) {
  g_return_val_if_fail(G_IS_FILTER_OUTPUT_STREAM(stream), NULL);
  return stream->base_stream;
}
gboolean g_filter_output_stream_get_close_base_stream(GFilterOutputStream *stream) {
  g_return_val_if_fail(G_IS_FILTER_OUTPUT_STREAM(stream), FALSE);
  return GET_PRIVATE(stream)->close_base;
}
void g_filter_output_stream_set_close_base_stream(GFilterOutputStream *stream, gboolean close_base) {
  GFilterOutputStreamPrivate *priv;
  g_return_if_fail(G_IS_FILTER_OUTPUT_STREAM(stream));
  close_base = !!close_base;
  priv = GET_PRIVATE(stream);
  if (priv->close_base != close_base) {
      priv->close_base = close_base;
      g_object_notify(G_OBJECT(stream), "close-base-stream");
  }
}
static gssize g_filter_output_stream_write(GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  GFilterOutputStream *filter_stream;
  gssize nwritten;
  filter_stream = G_FILTER_OUTPUT_STREAM(stream);
  nwritten = g_output_stream_write(filter_stream->base_stream, buffer, count, cancellable, error);
  return nwritten;
}
static gboolean g_filter_output_stream_flush(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  GFilterOutputStream *filter_stream;
  gboolean res;
  filter_stream = G_FILTER_OUTPUT_STREAM(stream);
  res = g_output_stream_flush(filter_stream->base_stream, cancellable, error);
  return res;
}
static gboolean g_filter_output_stream_close(GOutputStream *stream, GCancellable *cancellable, GError **error) {
  gboolean res = TRUE;
  if (GET_PRIVATE(stream)->close_base) {
      GFilterOutputStream *filter_stream;
      filter_stream = G_FILTER_OUTPUT_STREAM(stream);
      res = g_output_stream_close(filter_stream->base_stream, cancellable, error);
  }
  return res;
}