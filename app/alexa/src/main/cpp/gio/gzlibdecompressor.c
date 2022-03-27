#include <errno.h>
#include <zlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gzlibdecompressor.h"
#include "gfileinfo.h"
#include "gioerror.h"
#include "gioenums.h"
#include "gioenumtypes.h"

enum {
  PROP_0,
  PROP_FORMAT,
  PROP_FILE_INFO
};
static void g_zlib_decompressor_iface_init(GConverterIface *iface);
typedef struct {
  gz_header gzheader;
  char filename[257];
  GFileInfo *file_info;
} HeaderData;
struct _GZlibDecompressor {
  GObject parent_instance;
  GZlibCompressorFormat format;
  z_stream zstream;
  HeaderData *header_data;
};
static void g_zlib_decompressor_set_gzheader(GZlibDecompressor *decompressor) {
#if !defined(G_OS_WIN32) || ZLIB_VERNUM >= 0x1240
  if (decompressor->format != G_ZLIB_COMPRESSOR_FORMAT_GZIP) return;
  if (decompressor->header_data != NULL) {
      if (decompressor->header_data->file_info) g_object_unref(decompressor->header_data->file_info);
      memset(decompressor->header_data, 0, sizeof(HeaderData));
  } else decompressor->header_data = g_new0(HeaderData, 1);
  decompressor->header_data->gzheader.name = (Bytef*)&decompressor->header_data->filename;
  decompressor->header_data->gzheader.name_max = 256;
  if (inflateGetHeader(&decompressor->zstream, &decompressor->header_data->gzheader) != Z_OK)
      g_warning("unexpected zlib error: %s\n", decompressor->zstream.msg);
#endif
}
G_DEFINE_TYPE_WITH_CODE(GZlibDecompressor, g_zlib_decompressor, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_CONVERTER, g_zlib_decompressor_iface_init));
static void g_zlib_decompressor_finalize(GObject *object) {
  GZlibDecompressor *decompressor;
  decompressor = G_ZLIB_DECOMPRESSOR(object);
  inflateEnd(&decompressor->zstream);
  if (decompressor->header_data != NULL) {
      if (decompressor->header_data->file_info) g_object_unref(decompressor->header_data->file_info);
      g_free(decompressor->header_data);
  }
  G_OBJECT_CLASS(g_zlib_decompressor_parent_class)->finalize(object);
}
static void g_zlib_decompressor_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GZlibDecompressor *decompressor;
  decompressor = G_ZLIB_DECOMPRESSOR(object);
  switch(prop_id) {
      case PROP_FORMAT: decompressor->format = g_value_get_enum(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_zlib_decompressor_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GZlibDecompressor *decompressor;
  decompressor = G_ZLIB_DECOMPRESSOR(object);
  switch(prop_id) {
      case PROP_FORMAT: g_value_set_enum(value, decompressor->format); break;
      case PROP_FILE_INFO:
          if (decompressor->header_data) g_value_set_object(value, decompressor->header_data->file_info);
          else g_value_set_object(value, NULL);
          break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_zlib_decompressor_init(GZlibDecompressor *decompressor) {}
static void g_zlib_decompressor_constructed(GObject *object) {
  GZlibDecompressor *decompressor;
  int res;
  decompressor = G_ZLIB_DECOMPRESSOR(object);
  if (decompressor->format == G_ZLIB_COMPRESSOR_FORMAT_GZIP) res = inflateInit2(&decompressor->zstream, MAX_WBITS + 16);
  else if (decompressor->format == G_ZLIB_COMPRESSOR_FORMAT_RAW) res = inflateInit2(&decompressor->zstream, -MAX_WBITS);
  else res = inflateInit(&decompressor->zstream);
  if (res == Z_MEM_ERROR) g_error("GZlibDecompressor: Not enough memory for zlib use");
  if (res != Z_OK) g_warning("unexpected zlib error: %s\n", decompressor->zstream.msg);
  g_zlib_decompressor_set_gzheader(decompressor);
}
static void g_zlib_decompressor_class_init(GZlibDecompressorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_zlib_decompressor_finalize;
  gobject_class->constructed = g_zlib_decompressor_constructed;
  gobject_class->get_property = g_zlib_decompressor_get_property;
  gobject_class->set_property = g_zlib_decompressor_set_property;
  g_object_class_install_property(gobject_class, PROP_FORMAT, g_param_spec_enum("format", P_("compression format"), P_("The format of the compressed data"),
						          G_TYPE_ZLIB_COMPRESSOR_FORMAT, G_ZLIB_COMPRESSOR_FORMAT_ZLIB, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_FILE_INFO, g_param_spec_object ("file-info", P_("file info"), P_("File info"), G_TYPE_FILE_INFO,
                                  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}
GZlibDecompressor *g_zlib_decompressor_new(GZlibCompressorFormat format) {
  GZlibDecompressor *decompressor;
  decompressor = g_object_new(G_TYPE_ZLIB_DECOMPRESSOR, "format", format, NULL);
  return decompressor;
}
GFileInfo *g_zlib_decompressor_get_file_info(GZlibDecompressor *decompressor) {
  g_return_val_if_fail(G_IS_ZLIB_DECOMPRESSOR(decompressor), NULL);
  if (decompressor->header_data) return decompressor->header_data->file_info;
  return NULL;
}
static void g_zlib_decompressor_reset(GConverter *converter) {
  GZlibDecompressor *decompressor = G_ZLIB_DECOMPRESSOR(converter);
  int res;
  res = inflateReset(&decompressor->zstream);
  if (res != Z_OK) g_warning("unexpected zlib error: %s\n", decompressor->zstream.msg);
  g_zlib_decompressor_set_gzheader(decompressor);
}
static GConverterResult g_zlib_decompressor_convert(GConverter *converter, const void *inbuf, gsize inbuf_size, void *outbuf, gsize outbuf_size, GConverterFlags flags,
			                                        gsize *bytes_read, gsize *bytes_written, GError **error) {
  GZlibDecompressor *decompressor;
  int res;
  decompressor = G_ZLIB_DECOMPRESSOR(converter);
  decompressor->zstream.next_in = (void*)inbuf;
  decompressor->zstream.avail_in = inbuf_size;
  decompressor->zstream.next_out = outbuf;
  decompressor->zstream.avail_out = outbuf_size;
  res = inflate(&decompressor->zstream, Z_NO_FLUSH);
  if (res == Z_DATA_ERROR || res == Z_NEED_DICT) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA, _("Invalid compressed data"));
      return G_CONVERTER_ERROR;
  }
  if (res == Z_MEM_ERROR) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Not enough memory"));
      return G_CONVERTER_ERROR;
  }
  if (res == Z_STREAM_ERROR) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Internal error: %s"), decompressor->zstream.msg);
      return G_CONVERTER_ERROR;
  }
  if (res == Z_BUF_ERROR) {
	  if (flags & G_CONVERTER_FLUSH) return G_CONVERTER_FLUSHED;
	  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PARTIAL_INPUT, _("Need more input"));
	  return G_CONVERTER_ERROR;
  }
  g_assert(res == Z_OK || res == Z_STREAM_END);
  *bytes_read = inbuf_size - decompressor->zstream.avail_in;
  *bytes_written = outbuf_size - decompressor->zstream.avail_out;
#if !defined(G_OS_WIN32) || ZLIB_VERNUM >= 0x1240
  if (decompressor->header_data != NULL && decompressor->header_data->gzheader.done == 1) {
      HeaderData *data = decompressor->header_data;
      data->gzheader.done = 2;
      data->file_info = g_file_info_new();
      g_file_info_set_attribute_uint64(data->file_info, G_FILE_ATTRIBUTE_TIME_MODIFIED, data->gzheader.time);
      g_file_info_set_attribute_uint32(data->file_info, G_FILE_ATTRIBUTE_TIME_MODIFIED_USEC,0);
      if (data->filename[0] != '\0') g_file_info_set_attribute_byte_string(data->file_info, G_FILE_ATTRIBUTE_STANDARD_NAME, data->filename);
      g_object_notify(G_OBJECT (decompressor), "file-info");
  }
#endif
  if (res == Z_STREAM_END) return G_CONVERTER_FINISHED;
  return G_CONVERTER_CONVERTED;
}
static void g_zlib_decompressor_iface_init(GConverterIface *iface) {
  iface->convert = g_zlib_decompressor_convert;
  iface->reset = g_zlib_decompressor_reset;
}