#include <errno.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../../glib/gstdio.h"
#include "../config.h"
#include "../dbus.h"
#include "test-utils-glib.h"

typedef struct {
  TestMainContext *ctx;
  DBusError e;
  DBusServer *server;
  DBusConnection *server_conn;
  GQueue server_messages;
  DBusConnection *client_conn;
  gchar *tmp_runtime_dir;
  gchar *saved_runtime_dir;
} Fixture;
static void assert_no_error(const DBusError *e) {
  if (G_UNLIKELY(dbus_error_is_set(e))) g_error ("expected success but got error: %s: %s", e->name, e->message);
}
static DBusHandlerResult server_message_cb(DBusConnection *server_conn, DBusMessage *message, void *data) {
  Fixture *f = data;
  g_assert(server_conn == f->server_conn);
  g_queue_push_tail(&f->server_messages, dbus_message_ref(message));
  return DBUS_HANDLER_RESULT_HANDLED;
}
static void new_conn_cb(DBusServer *server, DBusConnection *server_conn, void *data) {
  Fixture *f = data;
  dbus_bool_t have_mem;
  g_assert(f->server_conn == NULL);
  f->server_conn = dbus_connection_ref(server_conn);
  test_connection_setup(f->ctx, server_conn);
  have_mem = dbus_connection_add_filter(server_conn, server_message_cb, f, NULL);
  g_assert(have_mem);
}
static void setup(Fixture *f, gconstpointer addr) {
  f->ctx = test_main_context_get();
  dbus_error_init(&f->e);
  g_queue_init(&f->server_messages);
  f->server = dbus_server_listen(addr, &f->e);
  assert_no_error(&f->e);
  g_assert(f->server != NULL);
  dbus_server_set_new_connection_function(f->server, new_conn_cb, f, NULL);
  test_server_setup(f->ctx, f->server);
}
#ifdef DBUS_UNIX
static void setup_runtime(Fixture *f, gconstpointer addr) {
  char *listening_at;
  GError *error = NULL;
  //f->tmp_runtime_dir = g_dir_make_tmp("dbus=daemon=test.XXXXXX", &error);
  g_assert_no_error(error);
  f->saved_runtime_dir = g_strdup(g_getenv("XDG_RUNTIME_DIR"));
  g_setenv("XDG_RUNTIME_DIR", f->tmp_runtime_dir, TRUE);
  setup(f, addr);
  listening_at = dbus_server_get_address(f->server);
  g_test_message("listening at %s", listening_at);
  g_assert(g_str_has_prefix(listening_at, "unix:path="));
  g_assert(strstr(listening_at, "dbus%3ddaemon%3dtest.") != NULL);
  g_assert(strstr(listening_at, "/bus,") != NULL || g_str_has_suffix(listening_at, "/bus"));
  dbus_free(listening_at);
}
static void setup_no_runtime(Fixture *f, gconstpointer addr) {
  char *listening_at;
  f->saved_runtime_dir = g_strdup(g_getenv("XDG_RUNTIME_DIR"));
  g_unsetenv("XDG_RUNTIME_DIR");
  setup(f, addr);
  listening_at = dbus_server_get_address(f->server);
  g_test_message("listening at %s", listening_at);
  g_assert(g_str_has_prefix(listening_at, "unix:"));
  g_assert(strstr (listening_at, "=/tmp/") != NULL);
  dbus_free(listening_at);
}
#endif
static void test_connect(Fixture *f, gconstpointer addr) {
  const char *listening_address = addr;
  char *address;
  DBusAddressEntry **entries;
  int n_entries;
  dbus_bool_t ok;
  g_assert(f->server_conn == NULL);
  address = dbus_server_get_address(f->server);
  g_test_message("listening at %s", address);
  ok = dbus_parse_address(address, &entries, &n_entries, &f->e);
  assert_no_error(&f->e);
  //g_assert_true(ok);
  g_assert_cmpint(n_entries, ==, 1);
  g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "guid"), !=, NULL);
  if (g_strcmp0(listening_address, "tcp:host=127.0.0.1") == 0) {
      g_assert_cmpstr(dbus_address_entry_get_method(entries[0]), ==, "tcp");
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "host"), ==,"127.0.0.1");
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "port"), !=, NULL);
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "noncefile"), ==, NULL);
  } else if (g_strcmp0(listening_address, "nonce-tcp:host=127.0.0.1") == 0) {
      g_assert_cmpstr(dbus_address_entry_get_method(entries[0]), ==, "nonce-tcp");
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "host"), ==,"127.0.0.1");
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "port"), !=, NULL);
      g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "noncefile"), !=, NULL);
  }
#ifdef DBUS_UNIX
  else if (g_strcmp0(listening_address, "unix:tmpdir=/tmp") == 0) {
      g_assert_cmpstr(dbus_address_entry_get_method(entries[0]), ==, "unix");
      if (dbus_address_entry_get_value(entries[0], "abstract") != NULL) {
          const char *abstract = dbus_address_entry_get_value(entries[0], "abstract");
          //g_assert_true(g_str_has_prefix(abstract, "/tmp/dbus-"));
          g_assert_cmpstr(dbus_address_entry_get_value(entries[0], "path"), ==, NULL);
      } else {
          const char *path = dbus_address_entry_get_value(entries[0],"path");
          //g_assert_nonnull(path);
          //g_assert_true(g_str_has_prefix(path, "/tmp/dbus-"));
      }
  } else if (g_strcmp0(listening_address, "unix:dir=/tmp") == 0) {
      const char *path = dbus_address_entry_get_value(entries[0],"path");
      g_assert_cmpstr(dbus_address_entry_get_method(entries[0]), ==, "unix");
      //g_assert_nonnull(path);
      //g_assert_true(g_str_has_prefix(path, "/tmp/dbus-"));
  } else if (g_strcmp0(listening_address,"unix:runtime=yes;unix:tmpdir=/tmp") == 0) {
      g_assert_cmpstr(dbus_address_entry_get_method(entries[0]), ==, "unix");
  }
#endif
  else { g_assert_not_reached(); }
  dbus_address_entries_free(entries);
  f->client_conn = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->client_conn != NULL);
  test_connection_setup(f->ctx, f->client_conn);
  while(f->server_conn == NULL) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  dbus_free(address);
}
static void test_bad_guid(Fixture *f, gconstpointer addr G_GNUC_UNUSED) {
  DBusMessage *incoming;
  char *address = dbus_server_get_address(f->server);
  gchar *guid;
  g_test_bug("39720");
  g_assert(f->server_conn == NULL);
  g_assert(strstr(address, "guid=") != NULL);
  guid = strstr(address, "guid=");
  g_assert_cmpuint(strlen(guid), >=, 5 + 32);
  if (guid[5] == '0') guid[5] = 'f';
  else guid[5] = '0';
  f->client_conn = dbus_connection_open_private(address, &f->e);
  assert_no_error(&f->e);
  g_assert(f->client_conn != NULL);
  test_connection_setup(f->ctx, f->client_conn);
  while(f->server_conn == NULL) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  while(g_queue_is_empty(&f->server_messages)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->server_messages), ==, 1);
  incoming = g_queue_pop_head(&f->server_messages);
  g_assert(!dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==, DBUS_INTERFACE_LOCAL);
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Disconnected");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(incoming), ==, "");
  g_assert_cmpstr(dbus_message_get_path(incoming), ==, DBUS_PATH_LOCAL);
  dbus_clear_message(&incoming);
  dbus_free(address);
}
static void test_message(Fixture *f, gconstpointer addr) {
  dbus_bool_t have_mem;
  dbus_uint32_t serial;
  DBusMessage *outgoing, *incoming;
  test_connect(f, addr);
  outgoing = dbus_message_new_signal("/com/example/Hello","com.example.Hello", "Greeting");
  g_assert(outgoing != NULL);
  have_mem = dbus_connection_send(f->client_conn, outgoing, &serial);
  g_assert(have_mem);
  g_assert(serial != 0);
  while(g_queue_is_empty(&f->server_messages)) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
  }
  g_assert_cmpuint(g_queue_get_length(&f->server_messages), ==, 1);
  incoming = g_queue_pop_head(&f->server_messages);
  g_assert(!dbus_message_contains_unix_fds(incoming));
  g_assert_cmpstr(dbus_message_get_destination(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_error_name(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_interface(incoming), ==,"com.example.Hello");
  g_assert_cmpstr(dbus_message_get_member(incoming), ==, "Greeting");
  g_assert_cmpstr(dbus_message_get_sender(incoming), ==, NULL);
  g_assert_cmpstr(dbus_message_get_signature(incoming), ==, "");
  g_assert_cmpstr(dbus_message_get_path(incoming), ==, "/com/example/Hello");
  g_assert_cmpuint(dbus_message_get_serial(incoming), ==, serial);
  dbus_clear_message(&incoming);
  dbus_clear_message(&outgoing);
}
static void teardown(Fixture *f, gconstpointer addr G_GNUC_UNUSED) {
  if (f->client_conn != NULL) dbus_connection_close(f->client_conn);
  if (f->server_conn != NULL) dbus_connection_close(f->server_conn);
  dbus_clear_connection(&f->client_conn);
  dbus_clear_connection(&f->server_conn);
  if (f->server != NULL) dbus_server_disconnect(f->server);
  dbus_clear_server(&f->server);
  test_main_context_unref(f->ctx);
}
#ifdef DBUS_UNIX
static void teardown_no_runtime(Fixture *f, gconstpointer addr) {
  teardown(f, addr);
  if (f->saved_runtime_dir != NULL) g_setenv("XDG_RUNTIME_DIR", f->saved_runtime_dir, TRUE);
  else g_unsetenv("XDG_RUNTIME_DIR");
  g_free(f->saved_runtime_dir);
}
static void teardown_runtime(Fixture *f, gconstpointer addr) {
  gchar *path;
  teardown(f, addr);
  path = g_strdup_printf ("%s/bus", f->tmp_runtime_dir);
  test_remove_if_exists (path);
  g_free (path);
  test_rmdir_must_exist (f->tmp_runtime_dir);
  if (f->saved_runtime_dir != NULL) g_setenv ("XDG_RUNTIME_DIR", f->saved_runtime_dir, TRUE);
  else g_unsetenv ("XDG_RUNTIME_DIR");
  g_free (f->saved_runtime_dir);
  g_free (f->tmp_runtime_dir);
}
#endif
int main(int argc, char **argv) {
  test_init(&argc, &argv);
  g_test_add("/connect/tcp", Fixture, "tcp:host=127.0.0.1", setup, test_connect, teardown);
  g_test_add("/message/tcp", Fixture, "tcp:host=127.0.0.1", setup, test_message, teardown);
  g_test_add("/connect/nonce-tcp", Fixture, "nonce-tcp:host=127.0.0.1", setup, test_connect, teardown);
  g_test_add("/message/nonce-tcp", Fixture, "nonce-tcp:host=127.0.0.1", setup, test_message, teardown);
#ifdef DBUS_UNIX
  g_test_add("/connect/unix/tmpdir", Fixture, "unix:tmpdir=/tmp", setup, test_connect, teardown);
  g_test_add("/message/unix/tmpdir", Fixture, "unix:tmpdir=/tmp", setup, test_message, teardown);
  g_test_add("/connect/unix/dir", Fixture, "unix:dir=/tmp", setup, test_connect, teardown);
  g_test_add("/message/unix/dir", Fixture, "unix:dir=/tmp", setup, test_message, teardown);
  g_test_add("/connect/unix/runtime", Fixture,"unix:runtime=yes;unix:tmpdir=/tmp", setup_runtime, test_connect, teardown_runtime);
  g_test_add("/connect/unix/no-runtime", Fixture,"unix:runtime=yes;unix:tmpdir=/tmp", setup_no_runtime, test_connect, teardown_no_runtime);
#endif
  g_test_add("/message/bad-guid", Fixture, "tcp:host=127.0.0.1", setup, test_bad_guid, teardown);
  return g_test_run();
}