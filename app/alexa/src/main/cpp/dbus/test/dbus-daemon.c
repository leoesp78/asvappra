#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "../../glib/glib.h"
#include "../../glib/gstdio.h"
#include "../../gio/gio.h"
#include "../../gio/gunixfdmessage.h"
#include "../config.h"
#include "../dbus.h"
#include "../dbus-sysdeps-win.h"
#include "../bus/stats.h"
#include "test-utils-glib.h"

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__linux__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define UNIX_USER_SHOULD_WORK
#define PID_SHOULD_WORK
#endif
#if 0
#define UNIX_USER_SHOULD_WORK
#endif
typedef struct {
  gboolean skip;
  TestMainContext *ctx;
  DBusError e;
  GError *ge;
  GPid daemon_pid;
  gchar *address;
  DBusConnection *left_conn;
  DBusConnection *right_conn;
  GQueue held_messages;
  gboolean right_conn_echo;
  gboolean right_conn_hold;
  gboolean wait_forever_called;
  gchar *tmp_runtime_dir;
  gchar *saved_runtime_dir;
} Fixture;
static DBusHandlerResult echo_filter(DBusConnection *connection, DBusMessage *message, void *user_data) {
  Fixture *f = user_data;
  DBusMessage *reply;
  if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  if (dbus_message_is_method_call(message, "com.example", "WaitForever")) {
      f->wait_forever_called = TRUE;
      return DBUS_HANDLER_RESULT_HANDLED;
  }
  reply = dbus_message_new_method_return(message);
  if (reply == NULL) g_error("OOM");
  if (!dbus_connection_send(connection, reply, NULL)) g_error("OOM");
  dbus_clear_message(&reply);
  return DBUS_HANDLER_RESULT_HANDLED;
}
static DBusHandlerResult hold_filter(DBusConnection *connection, DBusMessage *message, void *user_data) {
  Fixture *f = user_data;
  if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  g_queue_push_tail(&f->held_messages, dbus_message_ref(message));
  return DBUS_HANDLER_RESULT_HANDLED;
}
typedef struct {
  const char *bug_ref;
  guint min_messages;
  const char *config_file;
  enum { SPECIFY_ADDRESS = 0, RELY_ON_DEFAULT } connect_mode;
} Config;
static void setup(Fixture *f, gconstpointer context) {
  const Config *config = context;
  test_timeout_reset(1);
  f->ctx = test_main_context_get();
  f->ge = NULL;
  dbus_error_init(&f->e);
  if (config != NULL && config->connect_mode == RELY_ON_DEFAULT) {
      //f->tmp_runtime_dir = g_dir_make_tmp("dbus=daemon=test.XXXXXX", &f->ge);
      g_assert_no_error(f->ge);
      f->saved_runtime_dir = g_strdup(g_getenv("XDG_RUNTIME_DIR"));
      g_setenv("XDG_RUNTIME_DIR", f->tmp_runtime_dir, TRUE);
      g_unsetenv("DBUS_SESSION_BUS_ADDRESS");
  }
  f->address = test_get_dbus_daemon(config ? config->config_file : NULL,TEST_USER_ME, NULL, &f->daemon_pid);
  if (f->address == NULL) {
      f->skip = TRUE;
      return;
  }
  f->left_conn = test_connect_to_bus(f->ctx, f->address);
  if (config != NULL && config->connect_mode == RELY_ON_DEFAULT) {
      f->right_conn = dbus_bus_get_private(DBUS_BUS_SESSION, &f->e);
      test_assert_no_error(&f->e);
      test_connection_setup(f->ctx, f->right_conn);
  } else f->right_conn = test_connect_to_bus(f->ctx, f->address);
}
static void add_echo_filter(Fixture *f) {
  if (!dbus_connection_add_filter(f->right_conn, echo_filter, f, NULL)) g_error("OOM");
  f->right_conn_echo = TRUE;
}
static void add_hold_filter(Fixture *f) {
  if (!dbus_connection_add_filter(f->right_conn, hold_filter, f, NULL)) g_error("OOM");
  f->right_conn_hold = TRUE;
}
static void pc_count(DBusPendingCall *pc, void *data) {
  guint *received_p = data;
  (*received_p)++;
}
static void pc_enqueue(DBusPendingCall *pc, void *data) {
  GQueue *q = data;
  DBusMessage *m = dbus_pending_call_steal_reply(pc);
  g_test_message("message of type %d", dbus_message_get_type(m));
  g_queue_push_tail(q, m);
}
static void test_echo(Fixture *f, gconstpointer context) {
  const Config *config = context;
  guint count = 2000;
  guint sent;
  guint received = 0;
  double elapsed;
  if (f->skip) return;
  if (config != NULL && config->bug_ref != NULL) g_test_bug(config->bug_ref);
  if (g_test_perf()) count = 100000;
  if (config != NULL) count = MAX(config->min_messages, count);
  add_echo_filter(f);
  g_test_timer_start();
  for (sent = 0; sent < count; sent++) {
      DBusMessage *m = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn), "/","com.example", "Spam");
      DBusPendingCall *pc;
      if (m == NULL) g_error("OOM");
      if (!dbus_connection_send_with_reply(f->left_conn, m, &pc, DBUS_TIMEOUT_INFINITE) || pc == NULL) g_error("OOM");
      if (dbus_pending_call_get_completed(pc)) pc_count(pc, &received);
      else if (!dbus_pending_call_set_notify(pc, pc_count, &received,NULL)) g_error("OOM");
      dbus_clear_pending_call(&pc);
      dbus_clear_message(&m);
  }
  while(received < count) test_main_context_iterate(f->ctx, TRUE);
  elapsed = g_test_timer_elapsed();
  g_test_maximized_result(count / elapsed, "%u messages / %f seconds", count, elapsed);
}
static void test_no_reply(Fixture *f, gconstpointer context) {
  const Config *config = context;
  DBusMessage *m;
  DBusPendingCall *pc;
  DBusMessage *reply = NULL;
  enum { TIMEOUT, DISCONNECT } mode;
  gboolean ok;
  if (f->skip) return;
  g_test_bug("76112");
  if (config != NULL && config->config_file != NULL) mode = TIMEOUT;
  else mode = DISCONNECT;
  m = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn), "/","com.example", "WaitForever");
  add_echo_filter(f);
  if (m == NULL) g_error("OOM");
  if (!dbus_connection_send_with_reply(f->left_conn, m, &pc, DBUS_TIMEOUT_INFINITE) || pc == NULL) g_error("OOM");
  if (dbus_pending_call_get_completed(pc)) test_pending_call_store_reply(pc, &reply);
  else if (!dbus_pending_call_set_notify(pc, test_pending_call_store_reply, &reply, NULL)) g_error("OOM");
  dbus_clear_pending_call(&pc);
  dbus_clear_message(&m);
  if (mode == DISCONNECT) {
      while(!f->wait_forever_called) test_main_context_iterate(f->ctx, TRUE);
      dbus_connection_remove_filter(f->right_conn, echo_filter, f);
      dbus_connection_close(f->right_conn);
      dbus_clear_connection(&f->right_conn);
  }
  while(reply == NULL) test_main_context_iterate(f->ctx, TRUE);
  g_assert_cmpstr(dbus_message_type_to_string(dbus_message_get_type(reply)), ==,dbus_message_type_to_string(DBUS_MESSAGE_TYPE_ERROR));
  ok = dbus_set_error_from_message(&f->e, reply);
  g_assert(ok);
  g_assert_cmpstr(f->e.name, ==, DBUS_ERROR_NO_REPLY);
  if (mode == DISCONNECT) { g_assert_cmpstr(f->e.message, ==,"Message recipient disconnected from message bus without replying"); }
  else { g_assert_cmpstr(f->e.message, ==,"Message did not receive a reply (timeout by message bus)"); }
  dbus_clear_message(&reply);
}
static void test_creds(Fixture *f, gconstpointer context) {
  const char *unique = dbus_bus_get_unique_name(f->left_conn);
  DBusMessage *m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionCredentials");
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  DBusMessageIter pair_iter;
  DBusMessageIter var_iter;
  enum {
      SEEN_UNIX_USER = 1,
      SEEN_PID = 2,
      SEEN_WINDOWS_SID = 4,
      SEEN_LINUX_SECURITY_LABEL = 8
  } seen = 0;
  if (m == NULL) g_error("OOM");
  if (!dbus_message_append_args(m, DBUS_TYPE_STRING, &unique, DBUS_TYPE_INVALID)) g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  g_assert_cmpstr(dbus_message_get_signature(reply), ==, "a{sv}");
  dbus_message_iter_init(reply, &args_iter);
  g_assert_cmpuint(dbus_message_iter_get_arg_type(&args_iter), ==, DBUS_TYPE_ARRAY);
  g_assert_cmpuint(dbus_message_iter_get_element_type(&args_iter), ==, DBUS_TYPE_DICT_ENTRY);
  dbus_message_iter_recurse(&args_iter, &arr_iter);
  while (dbus_message_iter_get_arg_type(&arr_iter) != DBUS_TYPE_INVALID) {
      const char *name;
      dbus_message_iter_recurse(&arr_iter, &pair_iter);
      g_assert_cmpuint(dbus_message_iter_get_arg_type(&pair_iter), ==, DBUS_TYPE_STRING);
      dbus_message_iter_get_basic(&pair_iter, &name);
      dbus_message_iter_next(&pair_iter);
      g_assert_cmpuint(dbus_message_iter_get_arg_type(&pair_iter), ==, DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse(&pair_iter, &var_iter);
      if (g_strcmp0(name, "UnixUserID") == 0) {
      #ifndef G_OS_UNIX
          guint32 u32;
          g_assert(!(seen & SEEN_UNIX_USER));
          g_assert_cmpuint(dbus_message_iter_get_arg_type(&var_iter), ==, DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic(&var_iter, &u32);
          g_test_message("%s of this process is %u", name, u32);
          g_assert_cmpuint(u32, ==, geteuid());
          seen |= SEEN_UNIX_USER;
      #else
          g_assert_not_reached();
      #endif
      } else if (g_strcmp0(name, "WindowsSID") == 0) {
      #ifndef G_OS_WIN32
          gchar *sid;
          char *self_sid;
          g_assert(!(seen & SEEN_WINDOWS_SID));
          g_assert_cmpuint(dbus_message_iter_get_arg_type(&var_iter), ==, DBUS_TYPE_STRING);
          dbus_message_iter_get_basic(&var_iter, &sid);
          g_test_message("%s of this process is %s", name, sid);
          if (_dbus_getsid (&self_sid, 0)) {
              g_assert_cmpstr (self_sid, ==, sid);
              LocalFree(self_sid);
          }
          seen |= SEEN_WINDOWS_SID;
      #else
          g_assert_not_reached();
      #endif
      } else if (g_strcmp0(name, "ProcessID") == 0) {
          guint32 u32;
          g_assert(!(seen & SEEN_PID));
          g_assert_cmpuint(dbus_message_iter_get_arg_type(&var_iter), ==, DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic(&var_iter, &u32);
          g_test_message("%s of this process is %u", name, u32);
      #ifdef G_OS_UNIX
          g_assert_cmpuint(u32, ==, getpid());
      #elif !defined(G_OS_WIN32)
          g_assert_cmpuint(u32, ==, GetCurrentProcessId());
      #else
          g_assert_not_reached();
      #endif
          seen |= SEEN_PID;
      } else if (g_strcmp0(name, "LinuxSecurityLabel") == 0) {
      #ifdef __linux__
          gchar *label;
          int len;
          DBusMessageIter ay_iter;
          g_assert(!(seen & SEEN_LINUX_SECURITY_LABEL));
          g_assert_cmpuint(dbus_message_iter_get_arg_type(&var_iter), ==, DBUS_TYPE_ARRAY);
          dbus_message_iter_recurse(&var_iter, &ay_iter);
          g_assert_cmpuint(dbus_message_iter_get_arg_type(&ay_iter), ==, DBUS_TYPE_BYTE);
          dbus_message_iter_get_fixed_array(&ay_iter, &label, &len);
          g_test_message("%s of this process is %s", name, label);
          g_assert_cmpuint(strlen (label) + 1, ==, len);
          seen |= SEEN_LINUX_SECURITY_LABEL;
      #else
          g_assert_not_reached();
      #endif
      } else if (g_str_has_prefix(name, DBUS_INTERFACE_CONTAINERS1 ".")) { g_assert_not_reached(); }
      dbus_message_iter_next(&arr_iter);
  }
#ifdef UNIX_USER_SHOULD_WORK
  g_assert(seen & SEEN_UNIX_USER);
#endif
#ifdef PID_SHOULD_WORK
  g_assert(seen & SEEN_PID);
#endif
#ifdef G_OS_WIN32
  g_assert(seen & SEEN_WINDOWS_SID);
#endif
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_processid(Fixture *f, gconstpointer context) {
  const char *unique = dbus_bus_get_unique_name(f->left_conn);
  DBusMessage *m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionUnixProcessID");
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  guint32 pid;
  if (m == NULL) g_error("OOM");
  if (!dbus_message_append_args(m, DBUS_TYPE_STRING, &unique, DBUS_TYPE_INVALID)) g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (dbus_set_error_from_message(&error, reply)) {
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNIX_PROCESS_ID_UNKNOWN);
  #ifdef PID_SHOULD_WORK
      g_error("Expected pid to be passed, but got %s: %s", error.name, error.message);
  #endif
      dbus_error_free(&error);
  } else if (dbus_message_get_args(reply, &error, DBUS_TYPE_UINT32, &pid, DBUS_TYPE_INVALID)) {
      g_assert_cmpstr(dbus_message_get_signature(reply), ==, "u");
      test_assert_no_error(&error);
      g_test_message("GetConnectionUnixProcessID returned %u", pid);
  #ifndef G_OS_UNIX
      g_assert_cmpuint(pid, ==, getpid());
  #elif defined(G_OS_WIN32)
      g_assert_cmpuint(pid, ==, GetCurrentProcessId());
  #else
      g_assert_not_reached();
  #endif
  } else g_error("Unexpected error: %s: %s", error.name, error.message);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_canonical_path_uae(Fixture *f, gconstpointer context) {
  DBusMessage *m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  if (m == NULL) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY,"{ss}", &arr_iter) || !dbus_message_iter_close_container(&args_iter, &arr_iter))
      g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_METHOD_RETURN);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS,"/com/example/Wrong", DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");
  if (m == NULL) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_ARRAY,"{ss}", &arr_iter) || !dbus_message_iter_close_container(&args_iter, &arr_iter))
      g_error("OOM");
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  g_assert_cmpint(dbus_message_get_type(reply), ==, DBUS_MESSAGE_TYPE_ERROR);
  g_assert_cmpstr(dbus_message_get_error_name(reply), ==, DBUS_ERROR_ACCESS_DENIED);
  g_assert_cmpstr(dbus_message_get_signature(reply), ==, "s");
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_max_connections(Fixture *f, gconstpointer context) {
  DBusError error = DBUS_ERROR_INIT;
  DBusConnection *third_conn;
  DBusConnection *failing_conn;
  if (f->skip) return;
  g_assert(f->left_conn != NULL);
  g_assert(f->right_conn != NULL);
  third_conn = test_connect_to_bus(f->ctx, f->address);
  failing_conn = dbus_connection_open_private(f->address, &error);
  if (failing_conn != NULL) {
      gboolean ok = dbus_bus_register(failing_conn, &error);
      g_assert(!ok);
  }
  g_assert(dbus_error_is_set (&error));
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
  if (failing_conn != NULL) dbus_connection_close(failing_conn);
  dbus_clear_connection(&failing_conn);
  dbus_connection_close(third_conn);
  dbus_clear_connection(&third_conn);
}
static void test_max_replies_per_connection(Fixture *f, gconstpointer context) {
  GQueue received = G_QUEUE_INIT;
  GQueue errors = G_QUEUE_INIT;
  DBusMessage *m;
  DBusPendingCall *pc;
  guint i;
  DBusError error = DBUS_ERROR_INIT;
  if (f->skip) return;
  add_hold_filter(f);
  for (i = 0; i < 3; i++) {
      m = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn), "/","com.example", "Spam");
      if (m == NULL) g_error("OOM");
      if (!dbus_connection_send_with_reply(f->left_conn, m, &pc, DBUS_TIMEOUT_INFINITE) || pc == NULL) g_error("OOM");
      if (dbus_pending_call_get_completed(pc)) pc_enqueue(pc, &received);
      else if (!dbus_pending_call_set_notify(pc, pc_enqueue, &received,NULL)) g_error("OOM");
      dbus_pending_call_unref(pc);
      dbus_message_unref(m);
  }
  while(g_queue_get_length(&f->held_messages) < 3) test_main_context_iterate(f->ctx, TRUE);
  g_assert_cmpuint(g_queue_get_length(&received), ==, 0);
  g_assert_cmpuint(g_queue_get_length(&errors), ==, 0);
  for (i = 0; i < 2; i++) {
      m = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn), "/","com.example", "Spam");
      if (m == NULL) g_error("OOM");
      if (!dbus_connection_send_with_reply(f->left_conn, m, &pc, DBUS_TIMEOUT_INFINITE) || pc == NULL) g_error("OOM");
      if (dbus_pending_call_get_completed(pc)) pc_enqueue(pc, &errors);
      else if (!dbus_pending_call_set_notify(pc, pc_enqueue, &errors,NULL)) g_error("OOM");
      dbus_pending_call_unref(pc);
      dbus_message_unref(m);
  }
  for (m = g_queue_pop_head(&f->held_messages); m != NULL; m = g_queue_pop_head(&f->held_messages)) {
      DBusMessage *reply = dbus_message_new_method_return(m);
      if (reply == NULL) g_error("OOM");
      if (!dbus_connection_send(f->right_conn, reply, NULL)) g_error("OOM");
      dbus_clear_message(&m);
      dbus_clear_message(&reply);
  }
  while (g_queue_get_length(&received) + g_queue_get_length(&errors) < 5) test_main_context_iterate(f->ctx, TRUE);
  for (i = 0; i < 3; i++) {
      m = g_queue_pop_head(&received);
      g_assert(m != NULL);
      if (dbus_set_error_from_message(&error, m)) g_error("Unexpected error: %s: %s", error.name, error.message);
      dbus_clear_message(&m);
  }
  for (i = 0; i < 2; i++) {
      m = g_queue_pop_head(&errors);
      g_assert(m != NULL);
      if (!dbus_set_error_from_message(&error, m)) g_error("Unexpected success");
      g_assert_cmpstr(error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
      dbus_error_free(&error);
      dbus_clear_message(&m);
  }
  g_assert_cmpuint(g_queue_get_length(&received), ==, 0);
  g_assert_cmpuint(g_queue_get_length(&errors), ==, 0);
  g_queue_clear(&received);
  g_queue_clear(&errors);
}
static void test_max_match_rules_per_connection(Fixture *f, gconstpointer context) {
  DBusError error = DBUS_ERROR_INIT;
  if (f->skip) return;
  dbus_bus_add_match(f->left_conn, "sender='com.example.C1'", &error);
  test_assert_no_error(&error);
  dbus_bus_add_match(f->left_conn, "sender='com.example.C2'", &error);
  test_assert_no_error(&error);
  dbus_bus_add_match(f->left_conn, "sender='com.example.C3'", &error);
  test_assert_no_error(&error);
  dbus_bus_add_match(f->left_conn, "sender='com.example.C4'", &error);
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
  dbus_error_free(&error);
  dbus_bus_remove_match(f->left_conn, "sender='com.example.C3'", &error);
  test_assert_no_error(&error);
  dbus_bus_add_match(f->left_conn, "sender='com.example.C4'", &error);
  test_assert_no_error(&error);
}
static void test_max_names_per_connection(Fixture *f, gconstpointer context) {
  DBusError error = DBUS_ERROR_INIT;
  int ret;
  if (f->skip) return;
  ret = dbus_bus_request_name(f->left_conn, "com.example.C1", 0, &error);
  test_assert_no_error(&error);
  g_assert_cmpint(ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  ret = dbus_bus_request_name(f->left_conn, "com.example.C2", 0, &error);
  test_assert_no_error(&error);
  g_assert_cmpint(ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  ret = dbus_bus_request_name(f->left_conn, "com.example.C3", 0, &error);
  test_assert_no_error(&error);
  g_assert_cmpint(ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  ret = dbus_bus_request_name(f->left_conn, "com.example.C4", 0, &error);
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
  dbus_error_free(&error);
  g_assert_cmpint(ret, ==, -1);
  ret = dbus_bus_release_name(f->left_conn, "com.example.C3", &error);
  test_assert_no_error(&error);
  g_assert_cmpint(ret, ==, DBUS_RELEASE_NAME_REPLY_RELEASED);
  ret = dbus_bus_request_name(f->left_conn, "com.example.C4", 0, &error);
  test_assert_no_error(&error);
  g_assert_cmpint(ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
}
#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && !defined(HAVE_GIO_UNIX)
static DBusHandlerResult wait_for_disconnected_cb(DBusConnection *client_conn, DBusMessage *message, void *data) {
  gboolean *disconnected = data;
  if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
      *disconnected = TRUE;
      return DBUS_HANDLER_RESULT_HANDLED;
  }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
static const guchar partial_message[] = {
  DBUS_LITTLE_ENDIAN,
  DBUS_MESSAGE_TYPE_METHOD_CALL,
  0, /* flags */
  1, /* version */
  0xff, 0xff, 0, 0, /* length of body = 65535 bytes */
  1, 2, 3, 4, /* cookie */
  0xff, 0xff, 0, 0, /* length of header fields array = 65535 bytes */
  42 /* pretending to be the beginning of the header fields array */
};
static void send_all_with_fd(GSocket *socket, const guchar *local_partial_message, gsize len, int fd) {
  GSocketControlMessage *fdm = g_unix_fd_message_new();
  GError *error = NULL;
  gssize sent;
  GOutputVector vector = { local_partial_message, len };
  g_unix_fd_message_append_fd(G_UNIX_FD_MESSAGE(fdm), fd, &error);
  g_assert_no_error(error);
  sent = g_socket_send_message(socket, NULL, &vector, 1, &fdm, 1,G_SOCKET_MSG_NONE, NULL, &error);
  g_assert_no_error(error);
  g_assert_cmpint(sent, >=, 1);
  g_assert_cmpint(sent, <=, vector.size);
  while(((gsize)sent) < vector.size) {
      vector.size -= sent;
      vector.buffer = ((const guchar*)vector.buffer) + sent;
      sent = g_socket_send_message(socket, NULL, &vector, 1, NULL, 0,G_SOCKET_MSG_NONE, NULL, &error);
      g_assert_no_error(error);
      g_assert_cmpint(sent, >=, 1);
      g_assert_cmpint(sent, <=, vector.size);
  }
  g_object_unref(fdm);
}
static void test_pending_fd_timeout(Fixture *f, gconstpointer context) {
  GError *error = NULL;
  gint64 start;
  int fd;
  GSocket *socket;
  gboolean have_mem;
  gboolean disconnected = FALSE;
  if (f->skip) return;
  if (getuid() == 0) {
      //g_test_skip("Cannot test, uid 0 is immune to this limit");
      return;
  }
  have_mem = dbus_connection_add_filter(f->left_conn, wait_for_disconnected_cb, &disconnected, NULL);
  g_assert(have_mem);
  if (!dbus_connection_get_socket(f->left_conn, &fd)) g_error("failed to steal fd from left connection");
  socket = g_socket_new_from_fd(fd, &error);
  g_assert_no_error(error);
  g_assert(socket != NULL);
  start = g_get_monotonic_time();
  send_all_with_fd(socket, partial_message, G_N_ELEMENTS(partial_message), fd);
  while(!disconnected) {
      test_progress('.');
      test_main_context_iterate(f->ctx, TRUE);
      g_assert_cmpint(g_get_monotonic_time(), <=, start + G_USEC_PER_SEC);
  }
  g_object_unref (socket);
}
typedef struct {
  const gchar *path;
  guint n_fds;
  gboolean should_work;
} CountFdsVector;
static const CountFdsVector count_fds_vectors[] = {
  /* Deny sending if number of fds <= 2 */
  { "/test/DenySendMax2", 1, FALSE },
  { "/test/DenySendMax2", 2, FALSE },
  { "/test/DenySendMax2", 3, TRUE },
  { "/test/DenySendMax2", 4, TRUE },
  /* Deny receiving if number of fds <= 3 */
  { "/test/DenyReceiveMax3", 2, FALSE },
  { "/test/DenyReceiveMax3", 3, FALSE },
  { "/test/DenyReceiveMax3", 4, TRUE },
  { "/test/DenyReceiveMax3", 5, TRUE },
  /* Deny sending if number of fds >= 4 */
  { "/test/DenySendMin4", 2, TRUE },
  { "/test/DenySendMin4", 3, TRUE },
  { "/test/DenySendMin4", 4, FALSE },
  { "/test/DenySendMin4", 5, FALSE },
  /* Deny receiving if number of fds >= 5 */
  { "/test/DenyReceiveMin5", 3, TRUE },
  { "/test/DenyReceiveMin5", 4, TRUE },
  { "/test/DenyReceiveMin5", 5, FALSE },
  { "/test/DenyReceiveMin5", 6, FALSE },
};
static void test_count_fds(Fixture *f, gconstpointer context) {
  GQueue received = G_QUEUE_INIT;
  DBusMessage *m;
  DBusPendingCall *pc;
  guint i;
  DBusError error = DBUS_ERROR_INIT;
  const int stdin_fd = 0;
  if (f->skip) return;
  add_hold_filter(f);
  for (i = 0; i < G_N_ELEMENTS(count_fds_vectors); i++) {
      const CountFdsVector *vector = &count_fds_vectors[i];
      guint j;
      m = dbus_message_new_method_call(dbus_bus_get_unique_name(f->right_conn), vector->path,"com.example", "Spam");
      if (m == NULL) g_error("OOM");
      for (j = 0; j < vector->n_fds; j++) {
          if (!dbus_message_append_args(m, DBUS_TYPE_UNIX_FD, &stdin_fd, DBUS_TYPE_INVALID))
            g_error("OOM");
      }
      if (!dbus_connection_send_with_reply(f->left_conn, m, &pc, DBUS_TIMEOUT_INFINITE) || pc == NULL) g_error("OOM");
      if (dbus_pending_call_get_completed(pc)) pc_enqueue(pc, &received);
      else if (!dbus_pending_call_set_notify(pc, pc_enqueue, &received,NULL)) g_error("OOM");
      dbus_pending_call_unref(pc);
      dbus_message_unref(m);
      if (vector->should_work) {
          DBusMessage *reply;
          while (g_queue_get_length(&f->held_messages) < 1) test_main_context_iterate(f->ctx, TRUE);
          g_assert_cmpint(g_queue_get_length(&f->held_messages), ==, 1);
          m = g_queue_pop_head(&f->held_messages);
          g_assert_cmpint(g_queue_get_length(&f->held_messages), ==, 0);
          reply = dbus_message_new_method_return(m);
          if (reply == NULL) g_error("OOM");
          if (!dbus_connection_send(f->right_conn, reply, NULL)) g_error("OOM");
          dbus_message_unref(reply);
          dbus_message_unref(m);
      }
      while (g_queue_get_length(&received) < 1) test_main_context_iterate(f->ctx, TRUE);
      g_assert_cmpint(g_queue_get_length(&received), ==, 1);
      m = g_queue_pop_head(&received);
      g_assert(m != NULL);
      g_assert_cmpint(g_queue_get_length(&received), ==, 0);
      if (vector->should_work) {
          if (dbus_set_error_from_message(&error, m)) g_error("Unexpected error: %s: %s", error.name, error.message);
          g_test_message("Sending %u fds to %s was not denied, as expected", vector->n_fds, vector->path);
      } else if (!dbus_set_error_from_message(&error, m)) g_error("Unexpected success");
      else {
          g_assert_cmpstr(error.name, ==, DBUS_ERROR_ACCESS_DENIED);
          dbus_error_free(&error);
          g_test_message("Sending %u fds to %s was denied, as expected", vector->n_fds, vector->path);
      }
      dbus_message_unref(m);
  }
}
#endif
static void test_peer_get_machine_id(Fixture *f, gconstpointer context) {
  char *what_i_think;
  const char *what_daemon_thinks;
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  if (f->skip) return;
  what_i_think = dbus_try_get_local_machine_id(&error);
  if (what_i_think == NULL) {
      if (g_getenv("DBUS_TEST_UNINSTALLED") != NULL) {
          //g_test_skip("Machine UUID not available");
          return;
      } else g_error("%s", error.message);
  }
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PEER,"GetMachineId");
  if (m == NULL) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_message_get_args(reply, &error, DBUS_TYPE_STRING, &what_daemon_thinks, DBUS_TYPE_INVALID)) g_error("%s: %s", error.name, error.message);
  g_assert_cmpstr(what_i_think, ==, what_daemon_thinks);
  //g_assert_nonnull(what_daemon_thinks);
  g_assert_cmpuint(strlen (what_daemon_thinks), ==, 32);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
  dbus_free(what_i_think);
}
static void test_peer_ping(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PEER, "Ping");
  if (m == NULL) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_message_get_args(reply, &error, DBUS_TYPE_INVALID)) g_error("%s: %s", error.name, error.message);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_invalid_path(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Interfaces";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, "/", DBUS_INTERFACE_PROPERTIES, "Get");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) test_oom ();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message (&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_invalid_iface(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";
  const char *property = "Whatever";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_invalid(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Whatever";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_PROPERTY);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_all_invalid_iface(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "GetAll");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_all_invalid_path(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS,"/", DBUS_INTERFACE_PROPERTIES, "GetAll");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_set_invalid_iface(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";
  const char *property = "Whatever";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) g_error("OOM");
  dbus_message_iter_init_append (m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_VARIANT, "b", &var_iter) || !dbus_message_iter_append_basic(&var_iter,
      DBUS_TYPE_BOOLEAN, &b) || !dbus_message_iter_close_container(&args_iter, &var_iter))
      test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_set_invalid_path(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Interfaces";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS,"/", DBUS_INTERFACE_PROPERTIES, "Set");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_VARIANT, "b", &var_iter) || !dbus_message_iter_append_basic(&var_iter,
      DBUS_TYPE_BOOLEAN, &b) || !dbus_message_iter_close_container(&args_iter, &var_iter))
      test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_set_invalid(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Whatever";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_VARIANT, "b", &var_iter) || !dbus_message_iter_append_basic(&var_iter,
      DBUS_TYPE_BOOLEAN, &b) || !dbus_message_iter_close_container(&args_iter, &var_iter))
      test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_UNKNOWN_PROPERTY);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_set(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Features";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) g_error("OOM");
  dbus_message_iter_init_append(m, &args_iter);
  if (!dbus_message_iter_open_container(&args_iter, DBUS_TYPE_VARIANT, "b", &var_iter) || !dbus_message_iter_append_basic(&var_iter,
      DBUS_TYPE_BOOLEAN, &b) || !dbus_message_iter_close_container(&args_iter, &var_iter)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_set_error_from_message(&error, reply)) g_error("Unexpected success");
  g_assert_cmpstr(error.name, ==, DBUS_ERROR_PROPERTY_READ_ONLY);
  dbus_error_free(&error);
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void check_features(DBusMessageIter *var_iter) {
  DBusMessageIter arr_iter;
  gboolean have_systemd_activation = FALSE;
  gboolean have_header_filtering = FALSE;
  g_assert_cmpint(dbus_message_iter_get_arg_type(var_iter), ==, DBUS_TYPE_ARRAY);
  g_assert_cmpint(dbus_message_iter_get_element_type(var_iter), ==, DBUS_TYPE_STRING);
  dbus_message_iter_recurse(var_iter, &arr_iter);
  while (dbus_message_iter_get_arg_type(&arr_iter) != DBUS_TYPE_INVALID) {
      const char *feature;
      g_assert_cmpint(dbus_message_iter_get_arg_type(&arr_iter), ==, DBUS_TYPE_STRING);
      dbus_message_iter_get_basic(&arr_iter, &feature);
      g_test_message("Feature: %s", feature);
      if (g_strcmp0(feature, "HeaderFiltering") == 0) have_header_filtering = TRUE;
      else if (g_strcmp0(feature, "SystemdActivation") == 0) have_systemd_activation = TRUE;
      dbus_message_iter_next(&arr_iter);
  }
  //g_assert_true(have_header_filtering);
#ifdef DBUS_UNIX
 // g_assert_true(have_systemd_activation);
#else
  g_assert_false(have_systemd_activation);
#endif
}
static void test_features(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *features = "Features";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &features, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_message_iter_init(reply, &args_iter)) g_error("Reply has no arguments");
  g_assert_cmpint(dbus_message_iter_get_arg_type(&args_iter), ==, DBUS_TYPE_VARIANT);
  dbus_message_iter_recurse(&args_iter, &var_iter);
  check_features(&var_iter);
  if (dbus_message_iter_next(&args_iter)) g_error("Reply has too many arguments");
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void check_interfaces(DBusMessageIter *var_iter) {
  DBusMessageIter arr_iter;
  gboolean have_monitoring = FALSE;
  gboolean have_stats = FALSE;
  gboolean have_verbose = FALSE;
  g_assert_cmpint(dbus_message_iter_get_arg_type(var_iter), ==, DBUS_TYPE_ARRAY);
  g_assert_cmpint(dbus_message_iter_get_element_type(var_iter), ==, DBUS_TYPE_STRING);
  dbus_message_iter_recurse(var_iter, &arr_iter);
  while (dbus_message_iter_get_arg_type(&arr_iter) != DBUS_TYPE_INVALID) {
      const char *iface;
      g_assert_cmpint(dbus_message_iter_get_arg_type(&arr_iter), ==, DBUS_TYPE_STRING);
      dbus_message_iter_get_basic(&arr_iter, &iface);
      g_test_message("Interface: %s", iface);
      g_assert_cmpstr(iface, !=, DBUS_INTERFACE_DBUS);
      g_assert_cmpstr(iface, !=, DBUS_INTERFACE_PROPERTIES);
      g_assert_cmpstr(iface, !=, DBUS_INTERFACE_INTROSPECTABLE);
      g_assert_cmpstr(iface, !=, DBUS_INTERFACE_PEER);
      if (g_strcmp0(iface, DBUS_INTERFACE_MONITORING) == 0) have_monitoring = TRUE;
      else if (g_strcmp0(iface, BUS_INTERFACE_STATS) == 0) have_stats = TRUE;
      else if (g_strcmp0(iface, DBUS_INTERFACE_VERBOSE) == 0) have_verbose = TRUE;
      dbus_message_iter_next(&arr_iter);
  }
  //g_assert_true(have_monitoring);
#ifdef DBUS_ENABLE_STATS
  //g_assert_true(have_stats);
#else
  g_assert_false(have_stats);
#endif
#ifdef DBUS_ENABLE_VERBOSE_MODE
  //g_assert_true(have_verbose);
#else
  g_assert_false(have_verbose);
#endif
}
static void test_interfaces(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *ifaces = "Interfaces";
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &ifaces, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  if (!dbus_message_iter_init(reply, &args_iter)) g_error("Reply has no arguments");
  if (dbus_message_iter_get_arg_type(&args_iter) != DBUS_TYPE_VARIANT) g_error ("Reply does not have a variant argument");
  dbus_message_iter_recurse(&args_iter, &var_iter);
  check_interfaces(&var_iter);
  if (dbus_message_iter_next(&args_iter)) g_error("Reply has too many arguments");
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void test_get_all(Fixture *f, gconstpointer context) {
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  DBusMessageIter pair_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  gboolean have_features = FALSE;
  gboolean have_interfaces = FALSE;
  if (f->skip) return;
  m = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "GetAll");
  if (m == NULL || !dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_INVALID)) test_oom();
  reply = test_main_context_call_and_wait(f->ctx, f->left_conn, m, DBUS_TIMEOUT_USE_DEFAULT);
  dbus_message_iter_init(reply, &args_iter);
  g_assert_cmpuint(dbus_message_iter_get_arg_type (&args_iter), ==, DBUS_TYPE_ARRAY);
  g_assert_cmpuint(dbus_message_iter_get_element_type (&args_iter), ==, DBUS_TYPE_DICT_ENTRY);
  dbus_message_iter_recurse(&args_iter, &arr_iter);
  while(dbus_message_iter_get_arg_type(&arr_iter) != DBUS_TYPE_INVALID) {
      const char *name;
      dbus_message_iter_recurse(&arr_iter, &pair_iter);
      g_assert_cmpuint(dbus_message_iter_get_arg_type(&pair_iter), ==, DBUS_TYPE_STRING);
      dbus_message_iter_get_basic(&pair_iter, &name);
      dbus_message_iter_next(&pair_iter);
      g_assert_cmpuint(dbus_message_iter_get_arg_type(&pair_iter), ==, DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse(&pair_iter, &var_iter);
      if (g_strcmp0(name, "Features") == 0) {
          check_features(&var_iter);
          have_features = TRUE;
      } else if (g_strcmp0(name, "Interfaces") == 0) {
          check_interfaces(&var_iter);
          have_interfaces = TRUE;
      }
      dbus_message_iter_next(&arr_iter);
  }
  //g_assert_true(have_features);
  //g_assert_true(have_interfaces);
  if (dbus_message_iter_next(&args_iter)) g_error("Reply has too many arguments");
  dbus_clear_message(&reply);
  dbus_clear_message(&m);
}
static void teardown(Fixture *f, gconstpointer context G_GNUC_UNUSED) {
  dbus_error_free(&f->e);
  g_clear_error(&f->ge);
  if (f->left_conn != NULL) dbus_connection_close(f->left_conn);
  if (f->right_conn != NULL) {
      if (f->right_conn_echo) {
          dbus_connection_remove_filter(f->right_conn, echo_filter, f);
          f->right_conn_echo = FALSE;
      }
      if (f->right_conn_hold) {
          dbus_connection_remove_filter(f->right_conn, hold_filter, f);
          f->right_conn_hold = FALSE;
      }
      g_queue_foreach(&f->held_messages, (GFunc) dbus_message_unref, NULL);
      g_queue_clear(&f->held_messages);
      dbus_connection_close(f->right_conn);
  }
  dbus_clear_connection(&f->left_conn);
  dbus_clear_connection(&f->right_conn);
  if (f->daemon_pid != 0) {
      test_kill_pid(f->daemon_pid);
      g_spawn_close_pid(f->daemon_pid);
      f->daemon_pid = 0;
  }
  if (f->tmp_runtime_dir != NULL){
      gchar *path;
      path = g_strdup_printf("%s/bus", f->tmp_runtime_dir);
      test_remove_if_exists(path);
      g_free(path);
      test_rmdir_must_exist(f->tmp_runtime_dir);
      if (f->saved_runtime_dir != NULL) g_setenv("XDG_RUNTIME_DIR", f->saved_runtime_dir, TRUE);
      else g_unsetenv("XDG_RUNTIME_DIR");
      g_free(f->saved_runtime_dir);
      g_free(f->tmp_runtime_dir);
  }
  test_main_context_unref(f->ctx);
  g_free(f->address);
}
static Config limited_config = {
    "34393", 10000, "valid-config-files/incoming-limit.conf",
    SPECIFY_ADDRESS
};
static Config finite_timeout_config = {
    NULL, 1, "valid-config-files/finite-timeout.conf",
    SPECIFY_ADDRESS
};
#ifdef DBUS_UNIX
static Config listen_unix_runtime_config = {
    "61303", 1, "valid-config-files/listen-unix-runtime.conf",
    RELY_ON_DEFAULT
};
#endif
static Config max_completed_connections_config = {
    NULL, 1, "valid-config-files/max-completed-connections.conf",
    SPECIFY_ADDRESS
};
static Config max_connections_per_user_config = {
    NULL, 1, "valid-config-files/max-connections-per-user.conf",
    SPECIFY_ADDRESS
};
static Config max_replies_per_connection_config = {
    NULL, 1, "valid-config-files/max-replies-per-connection.conf",
    SPECIFY_ADDRESS
};
static Config max_match_rules_per_connection_config = {
    NULL, 1, "valid-config-files/max-match-rules-per-connection.conf",
    SPECIFY_ADDRESS
};
static Config max_names_per_connection_config = {
    NULL, 1, "valid-config-files/max-names-per-connection.conf",
    SPECIFY_ADDRESS
};
#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && !defined(HAVE_GIO_UNIX)
static Config pending_fd_timeout_config = {
    NULL, 1, "valid-config-files/pending-fd-timeout.conf",
    SPECIFY_ADDRESS
};
static Config count_fds_config = {
    NULL, 1, "valid-config-files/count-fds.conf",
    SPECIFY_ADDRESS
};
#endif
int main(int argc, char **argv) {
  test_init (&argc, &argv);
  g_test_add("/echo/session", Fixture, NULL, setup, test_echo, teardown);
  g_test_add("/echo/limited", Fixture, &limited_config, setup, test_echo, teardown);
  g_test_add("/no-reply/disconnect", Fixture, NULL, setup, test_no_reply, teardown);
  g_test_add("/no-reply/timeout", Fixture, &finite_timeout_config, setup, test_no_reply, teardown);
  g_test_add("/creds", Fixture, NULL, setup, test_creds, teardown);
  g_test_add("/processid", Fixture, NULL, setup, test_processid, teardown);
  g_test_add("/canonical-path/uae", Fixture, NULL, setup, test_canonical_path_uae, teardown);
  g_test_add("/limits/max-completed-connections", Fixture,&max_completed_connections_config, setup, test_max_connections, teardown);
  g_test_add("/limits/max-connections-per-user", Fixture,&max_connections_per_user_config, setup, test_max_connections, teardown);
  g_test_add("/limits/max-replies-per-connection", Fixture,&max_replies_per_connection_config, setup, test_max_replies_per_connection, teardown);
  g_test_add("/limits/max-match-rules-per-connection", Fixture,&max_match_rules_per_connection_config, setup, test_max_match_rules_per_connection, teardown);
  g_test_add("/limits/max-names-per-connection", Fixture, &max_names_per_connection_config, setup, test_max_names_per_connection, teardown);
  g_test_add("/peer/ping", Fixture, NULL, setup, test_peer_ping, teardown);
  g_test_add("/peer/get-machine-id", Fixture, NULL, setup, test_peer_get_machine_id, teardown);
  g_test_add("/properties/get-invalid-iface", Fixture, NULL, setup, test_get_invalid_iface, teardown);
  g_test_add("/properties/get-invalid-path", Fixture, NULL, setup, test_get_invalid_path, teardown);
  g_test_add("/properties/get-invalid", Fixture, NULL, setup, test_get_invalid, teardown);
  g_test_add("/properties/get-all-invalid-iface", Fixture, NULL, setup, test_get_all_invalid_iface, teardown);
  g_test_add("/properties/get-all-invalid-path", Fixture, NULL, setup, test_get_all_invalid_path, teardown);
  g_test_add("/properties/set-invalid-iface", Fixture, NULL, setup, test_set_invalid_iface, teardown);
  g_test_add("/properties/set-invalid-path", Fixture, NULL, setup, test_set_invalid_path, teardown);
  g_test_add("/properties/set-invalid", Fixture, NULL, setup, test_set_invalid, teardown);
  g_test_add("/properties/set", Fixture, NULL, setup, test_set, teardown);
  g_test_add("/properties/features", Fixture, NULL, setup, test_features, teardown);
  g_test_add("/properties/interfaces", Fixture, NULL, setup, test_interfaces, teardown);
  g_test_add("/properties/get-all", Fixture, NULL, setup, test_get_all, teardown);
#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && !defined(HAVE_GIO_UNIX)
  g_test_add("/limits/pending-fd-timeout", Fixture, &pending_fd_timeout_config, setup, test_pending_fd_timeout, teardown);
  g_test_add("/policy/count-fds", Fixture, &count_fds_config, setup, test_count_fds, teardown);
#endif
#ifdef DBUS_UNIX
  g_test_add("/unix-runtime-is-default", Fixture, &listen_unix_runtime_config, setup, test_echo, teardown);
#endif
  return g_test_run();
}