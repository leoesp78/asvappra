#include <stdlib.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../gio.h"

static void test_truncate(void) {
  GOutputStream *mo;
  GDataOutputStream *o;
  int i;
  GError *error = NULL;
  g_test_bug("540423");
  mo = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
  g_assert(g_seekable_can_truncate(G_SEEKABLE(mo)));
  o = g_data_output_stream_new(mo);
  for (i = 0; i < 1000; i++) {
      g_data_output_stream_put_byte(o, 1, NULL, &error);
      g_assert_no_error(error);
  }
  g_seekable_truncate(G_SEEKABLE(mo), 0, NULL, &error);
  g_assert_no_error(error);
  for (i = 0; i < 2000; i++) {
      g_data_output_stream_put_byte(o, 1, NULL, &error);
      g_assert_no_error(error);
  }
  g_object_unref(o);
  g_object_unref(mo);
}
static void test_seek(void) {
  GOutputStream *mo;
  GError *error;
  mo = g_memory_output_stream_new(g_new(gchar, 100), 100, g_realloc, g_free);
  g_assert(G_IS_SEEKABLE (mo));
  g_assert(g_seekable_can_seek(G_SEEKABLE (mo)));
  error = NULL;
  g_assert(g_seekable_seek(G_SEEKABLE(mo), 26, G_SEEK_SET, NULL, &error));
  g_assert_no_error(error);
  g_assert_cmpint(g_seekable_tell(G_SEEKABLE(mo)), ==, 26);
  g_assert(!g_seekable_seek(G_SEEKABLE(mo), 200, G_SEEK_CUR, NULL, &error));
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_error_free(error);
  g_object_unref(mo);
}
static void test_data_size(void) {
  GOutputStream *mo;
  GDataOutputStream *o;
  int pos;
  g_test_bug("540459");
  mo = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
  o = g_data_output_stream_new(mo);
  g_data_output_stream_put_byte(o, 1, NULL, NULL);
  pos = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM(mo));
  g_assert_cmpint(pos, ==, 1);
  g_seekable_seek(G_SEEKABLE(mo), 0, G_SEEK_CUR, NULL, NULL);
  pos = g_seekable_tell(G_SEEKABLE (mo));
  g_assert_cmpint(pos, ==, 1);
  g_test_bug("540461");
  g_seekable_seek(G_SEEKABLE(mo), 0, G_SEEK_SET, NULL, NULL);
  pos = g_seekable_tell(G_SEEKABLE(mo));
  g_assert_cmpint(pos, ==, 0);
  pos = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM(mo));
  g_assert_cmpint(pos, ==, 1);
  g_assert_cmpint(g_memory_output_stream_get_size(G_MEMORY_OUTPUT_STREAM(mo)), ==, 16);
  g_object_unref(o);
  g_object_unref(mo);
}
static void test_properties(void) {
  GOutputStream *mo;
  GDataOutputStream *o;
  int i;
  GError *error = NULL;
  g_test_bug("605733");
  mo = (GOutputStream*) g_object_new(G_TYPE_MEMORY_OUTPUT_STREAM,"realloc-function", g_realloc, "destroy-function", g_free, NULL);
  o = g_data_output_stream_new(mo);
  for (i = 0; i < 1000; i++) {
      g_data_output_stream_put_byte(o, 1, NULL, &error);
      g_assert_no_error(error);
  }
  gsize data_size_fun = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM (mo));
  gsize data_size_prop;
  g_object_get(mo, "data-size", &data_size_prop, NULL);
  g_assert_cmpint(data_size_fun, ==, data_size_prop);
  gpointer data_fun = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM(mo));
  gpointer data_prop;
  g_object_get(mo, "data", &data_prop, NULL);
  g_assert_cmphex(GPOINTER_TO_SIZE(data_fun), ==, GPOINTER_TO_SIZE(data_prop));
  g_object_unref(o);
  g_object_unref(mo);
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  g_test_bug_base("http://bugzilla.gnome.org/");
  g_test_add_func("/memory-output-stream/truncate", test_truncate);
  g_test_add_func("/memory-output-stream/seek", test_seek);
  g_test_add_func("/memory-output-stream/get-data-size", test_data_size);
  g_test_add_func("/memory-output-stream/properties", test_properties);
  return g_test_run();
}