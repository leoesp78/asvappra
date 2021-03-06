#include "../config.h"
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "../dbus-internals.h"
#include "../dbus-list.h"
#include "../dbus-sysdeps.h"
#include "test.h"

static DBusList *clients = NULL;
static DBusLoop *client_loop = NULL;
static dbus_bool_t add_client_watch(DBusWatch *watch, void *data) {
  return _dbus_loop_add_watch(client_loop, watch);
}
static void remove_client_watch(DBusWatch *watch, void *data) {
  _dbus_loop_remove_watch(client_loop, watch);
}
static void toggle_client_watch(DBusWatch *watch, void *data) {
  _dbus_loop_toggle_watch(client_loop, watch);
}
static dbus_bool_t add_client_timeout(DBusTimeout *timeout, void *data) {
  return _dbus_loop_add_timeout(client_loop, timeout);
}
static void remove_client_timeout(DBusTimeout *timeout, void *data) {
  _dbus_loop_remove_timeout(client_loop, timeout);
}
static DBusHandlerResult client_disconnect_filter(DBusConnection *connection, DBusMessage *message, void *user_data) {
  if (!dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL,"Disconnected")) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  _dbus_verbose("Removing client %p in disconnect handler\n", connection);
  _dbus_list_remove(&clients, connection);
  dbus_connection_unref(connection);
  if (clients == NULL) {
      _dbus_loop_unref(client_loop);
      client_loop = NULL;
  }
  return DBUS_HANDLER_RESULT_HANDLED;
}
dbus_bool_t bus_setup_debug_client(DBusConnection *connection) {
  dbus_bool_t retval;
  if (!dbus_connection_add_filter(connection, client_disconnect_filter,NULL, NULL)) return FALSE;
  retval = FALSE;
  if (client_loop == NULL) {
      client_loop = _dbus_loop_new();
      if (client_loop == NULL) goto out;
  }
  if (!dbus_connection_set_watch_functions(connection, add_client_watch, remove_client_watch, toggle_client_watch, connection,NULL)) goto out;
  if (!dbus_connection_set_timeout_functions(connection, add_client_timeout, remove_client_timeout,NULL, connection,NULL)) goto out;
  if (!_dbus_list_append(&clients, connection)) goto out;
  retval = TRUE;
out:
  if (!retval) {
      dbus_connection_remove_filter(connection, client_disconnect_filter,NULL);
      dbus_connection_set_watch_functions(connection,NULL, NULL, NULL, NULL, NULL);
      dbus_connection_set_timeout_functions(connection,NULL, NULL, NULL, NULL, NULL);
      _dbus_list_remove_last(&clients, connection);
      if (clients == NULL) {
          _dbus_loop_unref(client_loop);
          client_loop = NULL;
      }
  }
  return retval;
}
void bus_test_clients_foreach(BusConnectionForeachFunction function, void *data) {
  DBusList *link;
  link = _dbus_list_get_first_link(&clients);
  while(link != NULL) {
      DBusConnection *connection = link->data;
      DBusList *next = _dbus_list_get_next_link(&clients, link);
      if (!(*function)(connection, data)) break;
      link = next;
  }
}
dbus_bool_t bus_test_client_listed(DBusConnection *connection) {
  DBusList *link;
  link = _dbus_list_get_first_link(&clients);
  while(link != NULL) {
      DBusConnection *c = link->data;
      DBusList *next = _dbus_list_get_next_link(&clients, link);
      if (c == connection) return TRUE;
      link = next;
  }
  return FALSE;
}
void bus_test_run_clients_loop(dbus_bool_t block_once) {
  if (client_loop == NULL) return;
  _dbus_verbose("---> Dispatching on \"client side\"\n");
  _dbus_loop_dispatch(client_loop);
  if (block_once) {
      _dbus_verbose("---> blocking on \"client side\"\n");
      _dbus_loop_iterate(client_loop, TRUE);
  }
  while(_dbus_loop_iterate(client_loop, FALSE));
  _dbus_verbose("---> Done dispatching on \"client side\"\n");
}
void bus_test_run_bus_loop(BusContext *context, dbus_bool_t block_once) {
  _dbus_verbose("---> Dispatching on \"server side\"\n");
  _dbus_loop_dispatch(bus_context_get_loop (context));
  if (block_once) {
      _dbus_verbose("---> blocking on \"server side\"\n");
      _dbus_loop_iterate(bus_context_get_loop (context), TRUE);
  }
  while(_dbus_loop_iterate(bus_context_get_loop(context), FALSE));
  _dbus_verbose("---> Done dispatching on \"server side\"\n");
}
void bus_test_run_everything(BusContext *context) {
  while(_dbus_loop_iterate(bus_context_get_loop(context), FALSE) || (client_loop == NULL || _dbus_loop_iterate(client_loop, FALSE)));
}
BusContext* bus_context_new_test(const DBusString *test_data_dir, const char *filename) {
  DBusError error;
  DBusString config_file;
  DBusString relative;
  BusContext *context;
  if (!_dbus_string_init(&config_file)) {
      _dbus_warn("No memory");
      return NULL;
  }
  if (!_dbus_string_copy(test_data_dir, 0, &config_file, 0)) {
      _dbus_warn("No memory");
      _dbus_string_free(&config_file);
      return NULL;
  }
  _dbus_string_init_const(&relative, filename);
  if (!_dbus_concat_dir_and_file(&config_file, &relative)) {
      _dbus_warn("No memory");
      _dbus_string_free(&config_file);
      return NULL;
  }
  dbus_error_init(&error);
  context = bus_context_new(&config_file, BUS_CONTEXT_FLAG_NONE, NULL, NULL, NULL, &error);
  if (context == NULL) {
      _DBUS_ASSERT_ERROR_IS_SET(&error);
      _dbus_warn("Failed to create debug bus context from configuration file %s: %s", filename, error.message);
      dbus_error_free(&error);
      _dbus_string_free(&config_file);
      return NULL;
  }
  _dbus_string_free(&config_file);
  return context;
}

#endif