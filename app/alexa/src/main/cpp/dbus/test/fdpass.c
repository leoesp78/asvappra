#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "../dbus.h"
#include "../dbus-internals.h"
#include "../dbus-sysdeps.h"
#include "../dbus-sysdeps-unix.h"
#include "test-utils-glib.h"

#define MAX_MESSAGE_UNIX_FDS 20
_DBUS_STATIC_ASSERT(MAX_MESSAGE_UNIX_FDS <= 253);
#define MAX_INCOMING_UNIX_FDS  (MAX_MESSAGE_UNIX_FDS * 4)
#define SOME_MESSAGES  20
_DBUS_STATIC_ASSERT(MAX_MESSAGE_UNIX_FDS * SOME_MESSAGES > 256);
#define TOO_MANY_FDS  255
_DBUS_STATIC_ASSERT(MAX_MESSAGE_UNIX_FDS < TOO_MANY_FDS);
typedef struct {
  TestMainContext *ctx;
  DBusError e;
  DBusServer *server;
  DBusConnection *left_client_conn;
  DBusConnection *left_server_conn;
  DBusConnection *right_server_conn;
  DBusConnection *right_client_conn;
  GQueue messages;
  int fd_before;
} Fixture;
static void oom(const gchar *doing) G_GNUC_NORETURN;
static void oom(const gchar *doing) {
  g_error("out of memory (%s)", doing);
  abort();
}
static void assert_no_error(const DBusError *e) {
  if (G_UNLIKELY(dbus_error_is_set(e))) g_error("expected success but got error: %s: %s", e->name, e->message);
}
static DBusHandlerResult left_server_message_cb(DBusConnection *server_conn, DBusMessage *message, void *data) {
  Fixture *f = data;
  g_assert(server_conn == f->left_server_conn);
  g_assert(f->right_server_conn != NULL);
  dbus_connection_send(f->right_server_conn, message, NULL);
  return DBUS_HANDLER_RESULT_HANDLED;
}
static DBusHandlerResult right_client_message_cb(DBusConnection *client_conn, DBusMessage *message, void *data) {
  Fixture *f = data;
  g_assert(client_conn == f->right_client_conn);
  g_queue_push_tail(&f->messages, dbus_message_ref(message));
  return DBUS_HANDLER_RESULT_HANDLED;
}
static void new_conn_cb(DBusServer *server, DBusConnection *server_conn, void *data) {
  Fixture *f = data;
  dbus_connection_set_max_message_unix_fds(server_conn, MAX_MESSAGE_UNIX_FDS);
  dbus_connection_set_max_received_unix_fds(server_conn,MAX_INCOMING_UNIX_FDS);
  if (f->left_server_conn == NULL) {
      f->left_server_conn = dbus_connection_ref(server_conn);
      if (!dbus_connection_add_filter(server_conn, left_server_message_cb, f, NULL)) oom("adding filter");
  } else {
      g_assert(f->right_server_conn == NULL);
      f->right_server_conn = dbus_connection_ref(server_conn);
  }
  test_connection_setup(f->ctx, server_conn);
}
static void test_connect(Fixture *f, gboolean should_support_fds) {
  char *address;
  g_assert(f->left_server_conn == NULL);
  g_assert(f->right_server_conn == NULL);
  address = dbus_server_get_address(f->server);
  g_assert(address != NULL);
  f->left_client_conn = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->left_client_conn != NULL);
  test_connection_setup(f->ctx, f->left_client_conn);
  dbus_connection_set_max_message_unix_fds(f->left_client_conn, 1000);
  dbus_connection_set_max_received_unix_fds(f->left_client_conn, 1000000);
  while(f->left_server_conn == NULL) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  f->right_client_conn = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->right_client_conn != NULL);
  test_connection_setup(f->ctx, f->right_client_conn);
  dbus_free(address);
  while(f->right_server_conn == NULL) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  if (!dbus_connection_add_filter(f->right_client_conn, right_client_message_cb, f, NULL)) oom("adding filter");
  dbus_connection_set_max_message_unix_fds(f->right_client_conn, 1000);
  dbus_connection_set_max_received_unix_fds(f->right_client_conn, 1000000);
  while(!dbus_connection_get_is_authenticated(f->left_client_conn) || !dbus_connection_get_is_authenticated(f->right_client_conn) ||
        !dbus_connection_get_is_authenticated(f->left_server_conn) || !dbus_connection_get_is_authenticated(f->right_server_conn)) {
      test_progress('*');
      test_main_context_iterate(f->ctx, TRUE);
  }
  if (!should_support_fds) return;
  if (!dbus_connection_can_send_type(f->left_client_conn, DBUS_TYPE_UNIX_FD)) g_error("left client connection cannot send Unix fds");
  if (!dbus_connection_can_send_type(f->left_server_conn, DBUS_TYPE_UNIX_FD)) g_error("left server connection cannot send Unix fds");
  if (!dbus_connection_can_send_type(f->right_client_conn, DBUS_TYPE_UNIX_FD)) g_error("right client connection cannot send Unix fds");
  if (!dbus_connection_can_send_type(f->right_server_conn, DBUS_TYPE_UNIX_FD)) g_error("right server connection cannot send Unix fds");
}
static void setup_common(Fixture *f, const char *address) {
  f->ctx = test_main_context_get();
  dbus_error_init(&f->e);
  g_queue_init(&f->messages);
  f->server = dbus_server_listen(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->server != NULL);
  dbus_server_set_new_connection_function(f->server, new_conn_cb, f, NULL);
  test_server_setup(f->ctx, f->server);
}
static void setup_unsupported(Fixture *f, gconstpointer data G_GNUC_UNUSED) {
  setup_common(f, "tcp:host=127.0.0.1");
}
static void setup(Fixture *f, gconstpointer data G_GNUC_UNUSED) {
#ifdef HAVE_UNIX_FD_PASSING
  setup_common(f, "unix:tmpdir=/tmp");
  f->fd_before = open("/dev/null", O_RDONLY);
  if (f->fd_before < 0) g_error("cannot open /dev/null for reading: %s", g_strerror (errno));
  _dbus_fd_set_close_on_exec(f->fd_before);
#endif
}
static void test_unsupported(Fixture *f, gconstpointer data) {
  test_connect(f, FALSE);
  if (dbus_connection_can_send_type(f->left_client_conn, DBUS_TYPE_UNIX_FD)) g_error("left client connection claims it can send Unix fds");
  if (dbus_connection_can_send_type(f->left_server_conn, DBUS_TYPE_UNIX_FD)) g_error("left server connection claims it can send Unix fds");
  if (dbus_connection_can_send_type(f->right_client_conn, DBUS_TYPE_UNIX_FD)) g_error("right client connection claims it can send Unix fds");
  if (dbus_connection_can_send_type(f->right_server_conn, DBUS_TYPE_UNIX_FD)) g_error("right server connection claims it can send Unix fds");
}
static void test_relay(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  dbus_uint32_t serial;
  DBusMessage *outgoing, *incoming;
  int fd_after;
  struct stat stat_before;
  struct stat stat_after;
  test_connect(f, TRUE);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  if (!dbus_message_append_args(outgoing, DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
  if (!dbus_connection_send(f->left_client_conn, outgoing, &serial)) oom("sending message");
  dbus_message_unref(outgoing);
  while(g_queue_get_length(&f->messages) < 1) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 1);
  incoming = g_queue_pop_head(&f->messages);
  g_assert(dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(incoming), ==, DBUS_TYPE_UNIX_FD_AS_STRING);
  g_assert_cmpstr(dbus_message_get_path(incoming), ==, "/com/example/Hello");
  g_assert_cmpuint(dbus_message_get_serial(incoming), ==, serial);
  if (dbus_set_error_from_message(&f->e, incoming)) g_error("%s: %s", f->e.name, f->e.message);
  else if (!dbus_message_get_args(incoming, &f->e, DBUS_TYPE_UNIX_FD, &fd_after, DBUS_TYPE_INVALID)) g_error("%s: %s", f->e.name, f->e.message);
  assert_no_error(&f->e);
  if (fstat(f->fd_before, &stat_before) < 0) g_error("%s", g_strerror(errno));
  if (fstat(fd_after, &stat_after) < 0) g_error("%s", g_strerror(errno));
  g_assert_cmpint(stat_before.st_dev, ==, stat_after.st_dev);
  g_assert_cmpint(stat_before.st_ino, ==, stat_after.st_ino);
  g_assert_cmpint(stat_before.st_rdev, ==, stat_after.st_rdev);
  dbus_message_unref(incoming);
  if (close(fd_after) < 0) g_error("%s", g_strerror(errno));
  g_assert(dbus_connection_get_is_connected(f->right_client_conn));
  g_assert(dbus_connection_get_is_connected(f->right_server_conn));
  g_assert(dbus_connection_get_is_connected(f->left_client_conn));
  g_assert(dbus_connection_get_is_connected(f->left_server_conn));
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void test_limit(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  dbus_uint32_t serial;
  DBusMessage *outgoing, *incoming;
  int i;
  test_connect(f, TRUE);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  for (i = 0; i < MAX_MESSAGE_UNIX_FDS; i++) {
      if (!dbus_message_append_args(outgoing, DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
  }
  if (!dbus_connection_send(f->left_client_conn, outgoing, &serial)) oom("sending message");
  dbus_message_unref(outgoing);
  while(g_queue_get_length(&f->messages) < 1) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 1);
  incoming = g_queue_pop_head(&f->messages);
  g_assert(dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_path(incoming), ==, "/com/example/Hello");
  g_assert_cmpuint(dbus_message_get_serial(incoming), ==, serial);
  dbus_message_unref(incoming);
  g_assert(dbus_connection_get_is_connected(f->right_client_conn));
  g_assert(dbus_connection_get_is_connected(f->right_server_conn));
  g_assert(dbus_connection_get_is_connected(f->left_client_conn));
  g_assert(dbus_connection_get_is_connected(f->left_server_conn));
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void test_too_many(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  DBusMessage *outgoing;
  unsigned int i;
  test_connect(f, TRUE);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  for (i = 0; i < MAX_MESSAGE_UNIX_FDS + GPOINTER_TO_UINT(data); i++) {
      if (!dbus_message_append_args(outgoing, DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
  }
  if (!dbus_connection_send(f->left_client_conn, outgoing, NULL)) oom("sending message");
  dbus_message_unref(outgoing);
  while(dbus_connection_get_is_connected(f->left_client_conn) || dbus_connection_get_is_connected(f->left_server_conn)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 0);
  g_assert(dbus_connection_get_is_connected(f->right_client_conn));
  g_assert(dbus_connection_get_is_connected(f->right_server_conn));
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void test_too_many_split(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  DBusMessage *outgoing;
  int i;
  DBusSocket left_client_socket;
  char *payload;
  int payload_len;
  DBusString buffer;
  int fds[TOO_MANY_FDS];
  int done;
#ifdef HAVE_GETRLIMIT
  struct rlimit lim;
  if (getrlimit (RLIMIT_NOFILE, &lim) == 0) {
      if (lim.rlim_cur != RLIM_INFINITY && lim.rlim_cur < 4 * TOO_MANY_FDS) {
          g_test_skip("not enough RLIMIT_NOFILE");
          return;
      }
  }
#endif
  test_connect(f, TRUE);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  for (i = 0; i < TOO_MANY_FDS; i++) {
      if (!dbus_message_append_args(outgoing, DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
  }
  if (!dbus_message_marshal(outgoing, &payload, &payload_len)) oom("marshalling message");
  _dbus_string_init_const_len(&buffer, payload, payload_len);
  for (i = 0; i < TOO_MANY_FDS; i++) {
      fds[i] = dup(f->fd_before);
      if (fds[i] < 0) g_error("could not dup fd: %s", g_strerror (errno));
  }
  if (!dbus_connection_get_socket(f->left_client_conn, &left_client_socket.fd)) g_error("'unix:' DBusConnection should have had a socket");
  dbus_connection_flush(f->left_client_conn);
  done = _dbus_write_socket_with_unix_fds(left_client_socket, &buffer, 0, 1,&fds[0], TOO_MANY_FDS / 2);
  if (done < 0) g_error("could not send first byte and first batch of fds: %s", g_strerror(errno));
  done = _dbus_write_socket_with_unix_fds(left_client_socket, &buffer, 1,payload_len - 1,&fds[TOO_MANY_FDS / 2],TOO_MANY_FDS - (TOO_MANY_FDS / 2));
  if (done < 0) g_error("could not send rest of message and rest of fds: %s", g_strerror(errno));
  else if (done < payload_len - 1) g_error("short write in sendmsg(), fix this test: %d/%d", done, payload_len - 1);
  dbus_free(payload);
  for (i = 0; i < TOO_MANY_FDS; i++) close(fds[i]);
  dbus_message_unref(outgoing);
  while(dbus_connection_get_is_connected(f->left_client_conn) || dbus_connection_get_is_connected(f->left_server_conn)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 0);
  g_assert(dbus_connection_get_is_connected(f->right_client_conn));
  g_assert(dbus_connection_get_is_connected(f->right_server_conn));
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void test_flood(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  unsigned int i, j;
  DBusMessage *outgoing[SOME_MESSAGES];
  dbus_uint32_t serial;
  test_connect(f, TRUE);
  for (j = 0; j < SOME_MESSAGES; j++) {
      outgoing[j] = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
      g_assert(outgoing[j] != NULL);
      for (i = 0; i < GPOINTER_TO_UINT(data); i++) {
          if (!dbus_message_append_args(outgoing[j], DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
      }
  }
  for (j = 0; j < SOME_MESSAGES; j++) {
      if (!dbus_connection_send(f->left_client_conn, outgoing[j], &serial)) oom("sending message");
  }
  for (j = 0; j < SOME_MESSAGES; j++) dbus_message_unref(outgoing[j]);
  while(g_queue_get_length(&f->messages) < SOME_MESSAGES) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->messages), ==, SOME_MESSAGES);
  for (j = 0; j < SOME_MESSAGES; j++) {
      DBusMessage *incoming;
      incoming = g_queue_pop_head(&f->messages);
      g_assert(dbus_message_contains_unix_fds(incoming));
      g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
      g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
      g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_path(incoming), ==, "/com/example/Hello");
      dbus_message_unref(incoming);
  }
  g_assert(dbus_connection_get_is_connected(f->right_client_conn));
  g_assert(dbus_connection_get_is_connected(f->right_server_conn));
  g_assert(dbus_connection_get_is_connected(f->left_client_conn));
  g_assert(dbus_connection_get_is_connected(f->left_server_conn));
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void test_odd_limit(Fixture *f, gconstpointer data) {
#ifdef HAVE_UNIX_FD_PASSING
  DBusMessage *outgoing;
  int i;
  test_connect(f, TRUE);
  dbus_connection_set_max_message_unix_fds(f->left_server_conn, 7);
  dbus_connection_set_max_message_unix_fds(f->right_server_conn, 7);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  for (i = 0; i < 7 + GPOINTER_TO_INT(data); i++) {
      if (!dbus_message_append_args(outgoing, DBUS_TYPE_UNIX_FD, &f->fd_before, DBUS_TYPE_INVALID)) oom("appending fd");
  }
  if (!dbus_connection_send(f->left_client_conn, outgoing, NULL)) oom("sending message");
  dbus_message_unref(outgoing);
  if (GPOINTER_TO_INT(data) > 0) {
      while(dbus_connection_get_is_connected(f->left_client_conn) || dbus_connection_get_is_connected(f->left_server_conn)) {
          test_progress('.');
          test_main_context_iterate(f->ctx, TRUE);
      }
      g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 0);
      g_assert(dbus_connection_get_is_connected(f->right_client_conn));
      g_assert(dbus_connection_get_is_connected(f->right_server_conn));
  } else {
      DBusMessage *incoming;
      while(g_queue_get_length(&f->messages) < 1) {
          test_progress('.');
          test_main_context_iterate(f->ctx, TRUE);
      }
      g_assert_cmpuint(g_queue_get_length(&f->messages), ==, 1);
      incoming = g_queue_pop_head(&f->messages);
      g_assert(dbus_message_contains_unix_fds(incoming));
      g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
      g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
      g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
      g_assert_cmpstr(dbus_message_get_path(incoming), ==,"/com/example/Hello");
      dbus_message_unref(incoming);
      g_assert(dbus_connection_get_is_connected(f->right_client_conn));
      g_assert(dbus_connection_get_is_connected(f->right_server_conn));
      g_assert(dbus_connection_get_is_connected(f->left_client_conn));
      g_assert(dbus_connection_get_is_connected(f->left_server_conn));
  }
#else
  g_test_skip("fd-passing not supported on this platform");
#endif
}
static void teardown(Fixture *f, gconstpointer data G_GNUC_UNUSED) {
  if (f->left_client_conn != NULL) {
      dbus_connection_close(f->left_client_conn);
      dbus_connection_unref(f->left_client_conn);
      f->left_client_conn = NULL;
  }
  if (f->right_client_conn != NULL) {
      dbus_connection_close(f->right_client_conn);
      dbus_connection_unref(f->right_client_conn);
      f->right_client_conn = NULL;
  }
  if (f->left_server_conn != NULL) {
      dbus_connection_close(f->left_server_conn);
      dbus_connection_unref(f->left_server_conn);
      f->left_server_conn = NULL;
  }
  if (f->right_server_conn != NULL) {
      dbus_connection_close(f->right_server_conn);
      dbus_connection_unref(f->right_server_conn);
      f->right_server_conn = NULL;
  }
  if (f->server != NULL) {
      dbus_server_disconnect(f->server);
      dbus_server_unref(f->server);
      f->server = NULL;
  }
  if (f->ctx != NULL) test_main_context_unref(f->ctx);
#ifdef HAVE_UNIX_FD_PASSING
  if (f->fd_before >= 0 && close(f->fd_before) < 0) g_error("%s", g_strerror(errno));
#endif
}
int main(int argc, char **argv) {
  test_init(&argc, &argv);
#ifdef HAVE_GETRLIMIT
  {
      struct rlimit lim;
      if (getrlimit(RLIMIT_NOFILE, &lim) < 0) g_error("Failed to get RLIMIT_NOFILE limit: %s", g_strerror(errno));
      if (lim.rlim_cur != RLIM_INFINITY && lim.rlim_cur < 2 * MAX_MESSAGE_UNIX_FDS * SOME_MESSAGES) {
          g_message("not enough RLIMIT_NOFILE to run this test");
          return 77;
      }
  }
#endif
  g_test_add("/unsupported", Fixture, NULL, setup_unsupported, test_unsupported, teardown);
  g_test_add("/relay", Fixture, NULL, setup, test_relay, teardown);
  g_test_add("/limit", Fixture, NULL, setup, test_limit, teardown);
  g_test_add("/too-many/plus1", Fixture, GUINT_TO_POINTER(1), setup, test_too_many, teardown);
  g_test_add("/too-many/plus2", Fixture, GUINT_TO_POINTER(2), setup, test_too_many, teardown);
  g_test_add("/too-many/plus17", Fixture, GUINT_TO_POINTER(17), setup, test_too_many, teardown);
  g_test_add("/too-many/split", Fixture, NULL, setup, test_too_many_split, teardown);
  g_test_add("/flood/1", Fixture, GUINT_TO_POINTER(1), setup, test_flood, teardown);
#if MAX_MESSAGE_UNIX_FDS >= 2
  g_test_add("/flood/half-limit", Fixture,GUINT_TO_POINTER(MAX_MESSAGE_UNIX_FDS / 2), setup, test_flood, teardown);
#endif
#if MAX_MESSAGE_UNIX_FDS >= 4
  g_test_add("/flood/over-half-limit", Fixture,GUINT_TO_POINTER(3 * MAX_MESSAGE_UNIX_FDS / 4), setup, test_flood, teardown);
#endif
  g_test_add("/flood/limit", Fixture,GUINT_TO_POINTER(MAX_MESSAGE_UNIX_FDS), setup, test_flood, teardown);
  g_test_add("/odd-limit/minus1", Fixture, GINT_TO_POINTER(-1), setup, test_odd_limit, teardown);
  g_test_add("/odd-limit/at", Fixture, GINT_TO_POINTER(0), setup, test_odd_limit, teardown);
  g_test_add("/odd-limit/plus1", Fixture, GINT_TO_POINTER(+1), setup, test_odd_limit, teardown);
  g_test_add("/odd-limit/plus2", Fixture, GINT_TO_POINTER(+2), setup, test_odd_limit, teardown);
  return g_test_run();
}