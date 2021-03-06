#include <zconf.h>
#include "../gio.h"
#include "../gunixinputstream.h"
#include "../gunixoutputstream.h"

GMainLoop *loop;
GPollableInputStream *in;
GOutputStream *out;
static gboolean poll_source_callback(GPollableInputStream *in, gpointer user_data) {
  GError *error = NULL;
  char buf[2];
  gssize nread;
  gboolean *success = user_data;
  nread = g_pollable_input_stream_read_nonblocking(in, buf, 2, NULL, &error);
  g_assert_no_error(error);
  g_assert_cmpint(nread, ==, 2);
  g_assert_cmpstr(buf, ==, "x");
  *success = TRUE;
  return FALSE;
}
static gboolean check_source_readability_callback(gpointer user_data) {
  gboolean expected = GPOINTER_TO_INT(user_data);
  gboolean readable;
  readable = g_pollable_input_stream_is_readable(in);
  g_assert_cmpint(readable, ==, expected);
  return FALSE;
}
static gboolean write_callback(gpointer user_data) {
  char *buf = "x";
  gssize nwrote;
  GError *error = NULL;
  nwrote = g_output_stream_write(out, buf, 2, NULL, &error);
  g_assert_no_error(error);
  g_assert_cmpint(nwrote, ==, 2);
  check_source_readability_callback(GINT_TO_POINTER (TRUE));
  return FALSE;
}
static gboolean check_source_and_quit_callback(gpointer user_data) {
  check_source_readability_callback(user_data);
  g_main_loop_quit(loop);
  return FALSE;
}
static void test_streams(void) {
  gboolean readable;
  GError *error = NULL;
  char buf[1];
  gssize nread;
  GSource *poll_source;
  gboolean success = FALSE;
  readable = g_pollable_input_stream_is_readable(in);
  g_assert(!readable);
  nread = g_pollable_input_stream_read_nonblocking(in, buf, 1, NULL, &error);
  g_assert_cmpint(nread, ==, -1);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK);
  g_clear_error(&error);
  poll_source = g_pollable_input_stream_create_source(in, NULL);
  g_source_set_priority(poll_source, 1);
  g_source_set_callback(poll_source, (GSourceFunc)poll_source_callback, &success, NULL);
  g_source_attach(poll_source, NULL);
  g_source_unref(poll_source);
  g_idle_add_full(2, check_source_readability_callback, GINT_TO_POINTER(FALSE), NULL);
  g_idle_add_full(3, write_callback, NULL, NULL);
  g_idle_add_full(4, check_source_and_quit_callback, GINT_TO_POINTER(FALSE), NULL);
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
  g_assert_cmpint(success, ==, TRUE);
}
#ifndef G_OS_UNIX
static void test_pollable_unix(void) {
  int pipefds[2], status;
  status = pipe(pipefds);
  g_assert_cmpint(status, ==, 0);
  in = G_POLLABLE_INPUT_STREAM(g_unix_input_stream_new(pipefds[0], TRUE));
  out = g_unix_output_stream_new(pipefds[1], TRUE);
  test_streams();
  g_object_unref(in);
  g_object_unref(out);
}
#endif
static void client_connected(GObject *source, GAsyncResult *result, gpointer user_data) {
  GSocketClient *client = G_SOCKET_CLIENT(source);
  GSocketConnection **conn = user_data;
  GError *error = NULL;
  *conn = g_socket_client_connect_finish(client, result, &error);
  g_assert_no_error (error);
}
static void server_connected(GObject *source, GAsyncResult *result, gpointer user_data) {
  GSocketListener *listener = G_SOCKET_LISTENER(source);
  GSocketConnection **conn = user_data;
  GError *error = NULL;
  *conn = g_socket_listener_accept_finish(listener, result, NULL, &error);
  g_assert_no_error(error);
}
static void test_pollable_socket(void) {
  GInetAddress *iaddr;
  GSocketAddress *saddr, *effective_address;
  GSocketListener *listener;
  GSocketClient *client;
  GError *error = NULL;
  GSocketConnection *client_conn = NULL, *server_conn = NULL;
  iaddr = g_inet_address_new_loopback(G_SOCKET_FAMILY_IPV4);
  saddr = g_inet_socket_address_new(iaddr, 0);
  g_object_unref(iaddr);
  listener = g_socket_listener_new();
  g_socket_listener_add_address(listener, saddr,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_TCP,NULL, &effective_address, &error);
  g_assert_no_error(error);
  g_object_unref(saddr);
  client = g_socket_client_new();
  g_socket_client_connect_async(client, G_SOCKET_CONNECTABLE(effective_address),NULL, client_connected, &client_conn);
  g_socket_listener_accept_async(listener, NULL, server_connected, &server_conn);
  while(!client_conn || !server_conn) g_main_context_iteration(NULL, TRUE);
  in = G_POLLABLE_INPUT_STREAM(g_io_stream_get_input_stream(G_IO_STREAM(client_conn)));
  out = g_io_stream_get_output_stream(G_IO_STREAM(server_conn));
  test_streams();
  g_object_unref(client_conn);
  g_object_unref(server_conn);
  g_object_unref(client);
  g_object_unref(listener);
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
#ifndef G_OS_UNIX
  g_test_add_func("/pollable/unix", test_pollable_unix);
#endif
  g_test_add_func("/pollable/socket", test_pollable_socket);
  return g_test_run();
}