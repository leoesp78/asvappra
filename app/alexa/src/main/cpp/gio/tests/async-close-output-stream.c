#include <stdlib.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../gio.h"

#define DATA_TO_WRITE "Hello world\n"
typedef struct {
  GOutputStream *conv_stream;
  GOutputStream *data_stream;
  gchar *expected_output;
  gsize expected_size;
  GMainLoop *main_loop;
} SetupData;
static void create_streams(SetupData *data) {
  GConverter *converter;
  converter = G_CONVERTER(g_zlib_compressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP, -1));
  data->data_stream = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
  data->conv_stream = g_converter_output_stream_new(data->data_stream, converter);
  g_object_unref(converter);
}
static void destroy_streams(SetupData *data) {
  g_object_unref(data->data_stream);
  g_object_unref(data->conv_stream);
}
static void write_data_to_stream(SetupData *data) {
  gsize bytes_written;
  GError *error = NULL;
  g_output_stream_write_all(data->conv_stream, DATA_TO_WRITE,sizeof(DATA_TO_WRITE), &bytes_written,NULL, &error);
  g_assert_no_error(error);
  g_assert_cmpint(sizeof(DATA_TO_WRITE), ==, bytes_written);
}
static void setup_data(SetupData *data, gconstpointer  user_data) {
  data->main_loop = g_main_loop_new(NULL, FALSE);
  create_streams(data);
}
static void teardown_data(SetupData *data, gconstpointer user_data) {
  g_main_loop_unref(data->main_loop);
  destroy_streams(data);
  g_free(data->expected_output);
}
static void compare_output(SetupData *data) {
  gsize size;
  gpointer written;
  size = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM(data->data_stream));
  g_assert_cmpint(size, ==, data->expected_size);
  written = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM(data->data_stream));
  g_assert(memcmp(written, data->expected_output, size) == 0);
}
static void async_close_ready(GOutputStream *stream, GAsyncResult *result, SetupData *data) {
  GError *error = NULL;
  g_output_stream_close_finish(stream, result, &error);
  g_assert_no_error(error);
  compare_output(data);
  g_main_loop_quit(data->main_loop);
}
static void prepare_data(SetupData *data, gboolean manual_flush) {
  GError *error = NULL;
  gpointer written;
  write_data_to_stream(data);
  if (manual_flush) {
      g_output_stream_flush(data->conv_stream, NULL, &error);
      g_assert_no_error(error);
  }
  g_output_stream_close(data->conv_stream, NULL, &error);
  g_assert_no_error(error);
  written = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM(data->data_stream));
  data->expected_size = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM(data->data_stream));
  g_assert_cmpint(data->expected_size, >, 0);
  data->expected_output = g_memdup(written, (guint)data->expected_size);
  destroy_streams(data);
  create_streams(data);
  write_data_to_stream(data);
}
static void test_without_flush(SetupData *data, gconstpointer user_data) {
  prepare_data(data, FALSE);
  g_test_bug("617937");
  g_output_stream_close_async(data->conv_stream, G_PRIORITY_DEFAULT,NULL, (GAsyncReadyCallback)async_close_ready, data);
  g_main_loop_run(data->main_loop);
}
static void test_with_flush(SetupData *data, gconstpointer user_data) {
  GError *error = NULL;
  g_test_bug("617937");
  prepare_data(data, TRUE);
  g_output_stream_flush(data->conv_stream, NULL, &error);
  g_assert_no_error(error);
  g_output_stream_close_async(data->conv_stream, G_PRIORITY_DEFAULT,NULL, (GAsyncReadyCallback)async_close_ready, data);
  g_main_loop_run(data->main_loop);
}
static void async_flush_ready(GOutputStream *stream, GAsyncResult  *result, SetupData *data) {
  GError *error = NULL;
  g_output_stream_flush_finish(stream, result, &error);
  g_assert_no_error(error);
  g_output_stream_close_async(data->conv_stream, G_PRIORITY_DEFAULT,NULL, (GAsyncReadyCallback)async_close_ready, data);
}
static void test_with_async_flush(SetupData *data, gconstpointer user_data) {
  g_test_bug("617937");
  prepare_data(data, TRUE);
  g_output_stream_flush_async(data->conv_stream, G_PRIORITY_DEFAULT,NULL, (GAsyncReadyCallback)async_flush_ready, data);
  g_main_loop_run(data->main_loop);
}
int main(int argc, char *argv[]) {
  SetupData *data;
  g_type_init();
  //g_thread_init(NULL);
  g_test_init(&argc, &argv, NULL);
  g_test_bug_base("http://bugzilla.gnome.org/");
  data = g_slice_new(SetupData);
  g_test_add("/close-async/without-flush", SetupData, data, setup_data, test_without_flush, teardown_data);
  g_test_add("/close-async/with-flush", SetupData, data, setup_data, test_with_flush, teardown_data);
  g_test_add("/close-async/with-async-flush", SetupData, data, setup_data, test_with_async_flush, teardown_data);
  g_slice_free(SetupData, data);
  return g_test_run();
}