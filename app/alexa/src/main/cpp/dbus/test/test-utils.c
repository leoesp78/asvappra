#include "../config.h"
#include "test-utils.h"

typedef struct {
  DBusLoop *loop;
  DBusConnection *connection;
} CData;
static dbus_bool_t add_watch(DBusWatch *watch, void *data) {
  CData *cd = data;
  return _dbus_loop_add_watch(cd->loop, watch);
}
static void remove_watch(DBusWatch *watch, void *data) {
  CData *cd = data;
  _dbus_loop_remove_watch(cd->loop, watch);
}
static void toggle_watch(DBusWatch *watch, void *data) {
  CData *cd = data;
  _dbus_loop_toggle_watch(cd->loop, watch);
}
static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data) {
  CData *cd = data;
  return _dbus_loop_add_timeout(cd->loop, timeout);
}
static void remove_timeout(DBusTimeout *timeout, void *data) {
  CData *cd = data;
  _dbus_loop_remove_timeout(cd->loop, timeout);
}
static void dispatch_status_function(DBusConnection *connection, DBusDispatchStatus new_status, void *data) {
  DBusLoop *loop = data;
  if (new_status != DBUS_DISPATCH_COMPLETE) {
      while(!_dbus_loop_queue_dispatch(loop, connection)) _dbus_wait_for_memory();
  }
}
static void cdata_free(void *data) {
  CData *cd = data;
  dbus_connection_unref(cd->connection);
  _dbus_loop_unref(cd->loop);
  dbus_free(cd);
}
static CData* cdata_new(DBusLoop *loop, DBusConnection *connection) {
  CData *cd;
  cd = dbus_new0(CData, 1);
  if (cd == NULL) return NULL;
  cd->loop = loop;
  cd->connection = connection;
  dbus_connection_ref(cd->connection);
  _dbus_loop_ref(cd->loop);
  return cd;
}
dbus_bool_t test_connection_try_setup(TestMainContext *ctx, DBusConnection *connection) {
  DBusLoop *loop = ctx;
  CData *cd;
  cd = NULL;
  dbus_connection_set_dispatch_status_function(connection, dispatch_status_function, loop, NULL);
  cd = cdata_new(loop, connection);
  if (cd == NULL) goto nomem;
  if (!dbus_connection_set_watch_functions(connection, add_watch, remove_watch, toggle_watch, cd, cdata_free)) goto nomem;
  cd = cdata_new(loop, connection);
  if (cd == NULL) goto nomem;
  if (!dbus_connection_set_timeout_functions(connection, add_timeout, remove_timeout,NULL, cd, cdata_free)) goto nomem;
  cd = NULL;
  if (dbus_connection_get_dispatch_status(connection) != DBUS_DISPATCH_COMPLETE) {
      if (!_dbus_loop_queue_dispatch(loop, connection)) goto nomem;
  }
  return TRUE;
nomem:
  if (cd) cdata_free(cd);
  dbus_connection_set_dispatch_status_function(connection, NULL, NULL, NULL);
  dbus_connection_set_watch_functions(connection, NULL, NULL, NULL, NULL, NULL);
  dbus_connection_set_timeout_functions(connection, NULL, NULL, NULL, NULL, NULL);
  return FALSE;
}
static void die(const char *message) _DBUS_GNUC_NORETURN;
static void die(const char *message) {
  printf("Bail out! %s\n", message);
  fflush(stdout);
  exit(1);
}
void test_connection_setup(TestMainContext *ctx, DBusConnection *connection) {
  if (!test_connection_try_setup(ctx, connection)) die("Not enough memory to set up connection");
}
void test_connection_shutdown(TestMainContext *ctx, DBusConnection *connection) {
  if (!dbus_connection_set_watch_functions(connection,NULL,NULL,NULL,NULL, NULL))
      die("setting watch functions to NULL failed");
  if (!dbus_connection_set_timeout_functions(connection,NULL,NULL,NULL,NULL,NULL))
      die("setting timeout functions to NULL failed");
  dbus_connection_set_dispatch_status_function(connection, NULL, NULL, NULL);
}
typedef struct {
  DBusLoop *loop;
  DBusServer *server;
} ServerData;
static void serverdata_free(void *data) {
  ServerData *sd = data;
  dbus_server_unref(sd->server);
  _dbus_loop_unref(sd->loop);
  dbus_free(sd);
}
static ServerData* serverdata_new(DBusLoop *loop, DBusServer *server) {
  ServerData *sd;
  sd = dbus_new0(ServerData, 1);
  if (sd == NULL) return NULL;
  sd->loop = loop;
  sd->server = server;
  dbus_server_ref(sd->server);
  _dbus_loop_ref(sd->loop);
  return sd;
}
static dbus_bool_t add_server_watch(DBusWatch *watch, void *data) {
  ServerData *context = data;
  return _dbus_loop_add_watch(context->loop, watch);
}
static void toggle_server_watch(DBusWatch *watch, void *data) {
  ServerData *context = data;
  _dbus_loop_toggle_watch(context->loop, watch);
}
static void remove_server_watch(DBusWatch *watch, void *data) {
  ServerData *context = data;
  _dbus_loop_remove_watch(context->loop, watch);
}
static dbus_bool_t add_server_timeout(DBusTimeout *timeout, void *data) {
  ServerData *context = data;
  return _dbus_loop_add_timeout(context->loop, timeout);
}
static void remove_server_timeout(DBusTimeout *timeout, void *data) {
  ServerData *context = data;
  _dbus_loop_remove_timeout(context->loop, timeout);
}
dbus_bool_t test_server_try_setup(TestMainContext *ctx, DBusServer *server) {
  DBusLoop *loop = ctx;
  ServerData *sd;
  sd = serverdata_new(loop, server);
  if (sd == NULL) goto nomem;
  if (!dbus_server_set_watch_functions(server, add_server_watch, remove_server_watch, toggle_server_watch, sd, serverdata_free)) goto nomem;
  sd = serverdata_new(loop, server);
  if (sd == NULL) goto nomem;
  if (!dbus_server_set_timeout_functions(server, add_server_timeout, remove_server_timeout,NULL, sd, serverdata_free)) goto nomem;
  return TRUE;
nomem:
  if (sd) serverdata_free(sd);
  test_server_shutdown(loop, server);
  return FALSE;
}
void test_server_setup(TestMainContext *ctx, DBusServer *server) {
  if (!test_server_try_setup(ctx, server)) die("Not enough memory to set up server");
}
void test_server_shutdown(TestMainContext *ctx, DBusServer *server) {
  dbus_server_disconnect(server);
  if (!dbus_server_set_watch_functions(server,NULL,NULL,NULL,NULL,NULL))
      die("setting watch functions to NULL failed");
  if (!dbus_server_set_timeout_functions(server,NULL,NULL,NULL,NULL,NULL))
      die("setting timeout functions to NULL failed");
}
TestMainContext *test_main_context_get(void) {
  TestMainContext *ret = _dbus_loop_new();
  if (ret == NULL) die("Out of memory");
  return ret;
}
TestMainContext *test_main_context_try_get(void) {
  return _dbus_loop_new();
}
TestMainContext *test_main_context_ref(TestMainContext *ctx) {
  return _dbus_loop_ref(ctx);
}
void test_main_context_unref (TestMainContext *ctx) {
  _dbus_loop_unref(ctx);
}
void test_main_context_iterate (TestMainContext *ctx, dbus_bool_t may_block) {
  _dbus_loop_iterate(ctx, may_block);
}
void test_pending_call_store_reply(DBusPendingCall *pc, void *data) {
  DBusMessage **message_p = data;
  *message_p = dbus_pending_call_steal_reply(pc);
  _dbus_assert(*message_p != NULL);
}