#include <errno.h>
#include <zlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gzlibcompressor.h"
#include "gfileinfo.h"
#include "gioerror.h"
#include "gioenums.h"
#include "gioenumtypes.h"

enum {
  PROP_0,
  PROP_FORMAT,
  PROP_LEVEL,
  PROP_FILE_INFO
};
static void g_zlib_compressor_iface_init          (GConverterIface *iface);
struct _GZlibCompressor {
  GObject parent_instance;
  GZlibCompressorFormat format;
  int level;
  z_stream zstream;
  gz_header gzheader;
  GFileInfo *file_info;
};
static void g_zlib_compressor_set_gzheader(GZlibCompressor *compressor) {
#if !defined (G_OS_WIN32) || ZLIB_VERNUM >= 0x1240
  const gchar *filename;
  if (compressor->format != G_ZLIB_COMPRESSOR_FORMAT_GZIP || compressor->file_info == NULL) return;
  memset (&compressor->gzheader, 0, sizeof(gz_header));
  compressor->gzheader.os = 0x03;
  filename = g_file_info_get_name(compressor->file_info);
  compressor->gzheader.name = (Bytef*)filename;
  compressor->gzheader.name_max = filename ? strlen(filename) + 1 : 0;
  compressor->gzheader.time = (uLong)g_file_info_get_attribute_uint64(compressor->file_info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
  if (deflateSetHeader(&compressor->zstream, &compressor->gzheader) != Z_OK) g_warning("unexpected zlib error: %s\n", compressor->zstream.msg);
#endif
}
G_DEFINE_TYPE_WITH_CODE(GZlibCompressor, g_zlib_compressor, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_CONVERTER, g_zlib_compressor_iface_init));
static void g_zlib_compressor_finalize(GObject *object) {
  GZlibCompressor *compressor;
  compressor = G_ZLIB_COMPRESSOR(object);
  deflateEnd(&compressor->zstream);
  if (compressor->file_info) g_object_unref(compressor->file_info);
  G_OBJECT_CLASS(g_zlib_compressor_parent_class)->finalize (object);
}
static void g_zlib_compressor_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GZlibCompressor *compressor;
  compressor = G_ZLIB_COMPRESSOR(object);
  switch(prop_id) {
      case PROP_FORMAT: compressor->format = g_value_get_enum(value); break;
      case PROP_LEVEL: compressor->level = g_value_get_int(value); break;
      case PROP_FILE_INFO: g_zlib_compressor_set_file_info(compressor, g_value_get_object(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_zlib_compressor_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GZlibCompressor *compressor;
  compressor = G_ZLIB_COMPRESSOR(object);
  switch(prop_id) {
      case PROP_FORMAT: g_value_set_enum(value, compressor->format); break;
      case PROP_LEVEL: g_value_set_int(value, compressor->level); break;
      case PROP_FILE_INFO: g_value_set_object(value, compressor->file_info); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_zlib_compressor_init(GZlibCompressor *compressor) {}
static void g_zlib_compressor_constructed(GObject *object) {
  GZlibCompressor *compressor;
  int res;
  compressor = G_ZLIB_COMPRESSOR(object);
  if (compressor->format == G_ZLIB_COMPRESSOR_FORMAT_GZIP) {
      res = deflateInit2(&compressor->zstream,compressor->level, Z_DEFLATED,MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
  } else if (compressor->format == G_ZLIB_COMPRESSOR_FORMAT_RAW) {
      res = deflateInit2(&compressor->zstream,compressor->level, Z_DEFLATED,-MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
  } else res = deflateInit(&compressor->zstream, compressor->level);
  if (res == Z_MEM_ERROR) g_error("GZlibCompressor: Not enough memory for zlib use");
  if (res != Z_OK) g_warning("unexpected zlib error: %s\n", compressor->zstream.msg);
  g_zlib_compressor_set_gzheader(compressor);
}
static void g_zlib_compressor_class_init(GZlibCompressorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_zlib_compressor_finalize;
  gobject_class->constructed = g_zlib_compressor_constructed;
  gobject_class->get_property = g_zlib_compressor_get_property;
  gobject_class->set_property = g_zlib_compressor_set_property;
  g_object_class_install_property(gobject_class, PROP_FORMAT, g_param_spec_enum("format", P_("compression format"), P_("The format of the compressed "
                                  "data"), G_TYPE_ZLIB_COMPRESSOR_FORMAT, G_ZLIB_COMPRESSOR_FORMAT_ZLIB, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
						          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class,PROP_LEVEL,g_param_spec_int("level", P_("compression level"), P_("The level of "
                                  "compression from 0 (no compression) to 9 (most compression), -1 for the default level"),-1, 9,-1,
						    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_FILE_INFO, g_param_spec_object("file-info", P_("file info"), P_("File info"), G_TYPE_FILE_INFO,
                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
GZlibCompressor *g_zlib_compressor_new(GZlibCompressorFormat format, int level) {
  GZlibCompressor *compressor;
  compressor = g_object_new(G_TYPE_ZLIB_COMPRESSOR, "format", format, "level", level, NULL);
  return compressor;
}
GFileInfo *g_zlib_compressor_get_file_info(GZlibCompressor *compressor) {
  g_return_val_if_fail(G_IS_ZLIB_COMPRESSOR(compressor), NULL);
  return compressor->file_info;
}
void g_zlib_compressor_set_file_info(GZlibCompressor *compressor, GFileInfo *file_info) {
  g_return_if_fail(G_IS_ZLIB_COMPRESSOR (compressor));
  if (file_info == compressor->file_info) return;
  if (compressor->file_info) g_object_unref(compressor->file_info);
  if (file_info) g_object_ref(file_info);
  compressor->file_info = file_info;
  g_object_notify(G_OBJECT(compressor), "file-info");
  g_zlib_compressor_set_gzheader(compressor);
}
static void g_zlib_compressor_reset(GConverter *converter) {
  GZlibCompressor *compressor = G_ZLIB_COMPRESSOR(converter);
  int res;
  res = deflateReset(&compressor->zstream);
  if (res != Z_OK) g_warning("unexpected zlib error: %s\n", compressor->zstream.msg);
  g_zlib_compressor_set_gzheader(compressor);
}
static GConverterResult g_zlib_compressor_convert(GConverter *converter, const void *inbuf, gsize inbuf_size, void *outbuf, gsize outbuf_size, GConverterFlags flags,
			                                      gsize *bytes_read, gsize *bytes_written, GError **error) {
  GZlibCompressor *compressor;
  int res;
  int flush;
  compressor = G_ZLIB_COMPRESSOR(converter);
  compressor->zstream.next_in = (void*)inbuf;
  compressor->zstream.avail_in = inbuf_size;
  compressor->zstream.next_out = outbuf;
  compressor->zstream.avail_out = outbuf_size;
  flush = Z_NO_FLUSH;
  if (flags & G_CONVERTER_INPUT_AT_END) flush = Z_FINISH;
  else if (flags & G_CONVERTER_FLUSH) flush = Z_SYNC_FLUSH;
  res = deflate(&compressor->zstream, flush);
  if (res == Z_MEM_ERROR) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED,	               _("Not enough memory"));
      return G_CONVERTER_ERROR;
  }
  if (res == Z_STREAM_ERROR) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Internal error: %s"), compressor->zstream.msg);
      return G_CONVERTER_ERROR;
  }
  if (res == Z_BUF_ERROR) {
	  if (flags & G_CONVERTER_FLUSH) return G_CONVERTER_FLUSHED;
 	  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PARTIAL_INPUT, _("Need more input"));
	  return G_CONVERTER_ERROR;
  }
  if (res == Z_OK || res == Z_STREAM_END) {
      *bytes_read = inbuf_size - compressor->zstream.avail_in;
      *bytes_written = outbuf_size - compressor->zstream.avail_out;
      if (res == Z_STREAM_END) return G_CONVERTER_FINISHED;
      return G_CONVERTER_CONVERTED;
  }
  g_assert_not_reached();
}
static void g_zlib_compressor_iface_init(GConverterIface *iface) {
  iface->convert = g_zlib_compressor_convert;
  iface->reset = g_zlib_compressor_reset;
}