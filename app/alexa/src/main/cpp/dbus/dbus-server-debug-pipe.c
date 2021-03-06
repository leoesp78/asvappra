#include "config.h"
#include "dbus-internals.h"
#include "dbus-server-debug-pipe.h"
#include "dbus-transport-socket.h"
#include "dbus-connection-internal.h"
#include "dbus-hash.h"
#include "dbus-string.h"
#include "dbus-protocol.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
typedef struct DBusServerDebugPipe DBusServerDebugPipe;
struct DBusServerDebugPipe {
  DBusServer base;
  char *name;
  dbus_bool_t disconnected;
};
static DBusHashTable *server_pipe_hash;
static int server_pipe_hash_refcount = 0;
static dbus_bool_t pipe_hash_ref(void) {
  if (!server_pipe_hash) {
      _dbus_assert(server_pipe_hash_refcount == 0);
      server_pipe_hash = _dbus_hash_table_new(DBUS_HASH_STRING, NULL, NULL);
      if (!server_pipe_hash) return FALSE;
  }
  server_pipe_hash_refcount = 1;
  return TRUE;
}
static void pipe_hash_unref(void) {
  _dbus_assert(server_pipe_hash != NULL);
  _dbus_assert(server_pipe_hash_refcount > 0);
  server_pipe_hash_refcount -= 1;
  if (server_pipe_hash_refcount == 0) {
      _dbus_hash_table_unref(server_pipe_hash);
      server_pipe_hash = NULL;
  }
}
static void debug_finalize(DBusServer *server) {
  DBusServerDebugPipe *debug_server = (DBusServerDebugPipe*)server;
  pipe_hash_unref();
  _dbus_server_finalize_base(server);
  dbus_free(debug_server->name);
  dbus_free(server);
}
static void debug_disconnect(DBusServer *server) {
  ((DBusServerDebugPipe*)server)->disconnected = TRUE;
}
static DBusServerVTable debug_vtable = {
  debug_finalize,
  debug_disconnect
};
DBusServer* _dbus_server_debug_pipe_new(const char *server_name, DBusError *error) {
  DBusServerDebugPipe *debug_server;
  DBusString address;
  DBusString name_str;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (!pipe_hash_ref()) return NULL;
  if (_dbus_hash_table_lookup_string(server_pipe_hash, server_name) != NULL) {
      dbus_set_error(error, DBUS_ERROR_ADDRESS_IN_USE, NULL);
      pipe_hash_unref();
      return NULL;
  }
  debug_server = dbus_new0(DBusServerDebugPipe, 1);
  if (debug_server == NULL) goto nomem_0;
  if (!_dbus_string_init(&address)) goto nomem_1;
  _dbus_string_init_const(&name_str, server_name);
  if (!_dbus_string_append(&address, "debug-pipe:name=") || !_dbus_address_append_escaped(&address, &name_str)) goto nomem_2;
  debug_server->name = _dbus_strdup(server_name);
  if (debug_server->name == NULL) goto nomem_2;
  if (!_dbus_server_init_base(&debug_server->base, &debug_vtable, &address, error)) goto fail_3;
  if (!_dbus_hash_table_insert_string(server_pipe_hash, debug_server->name, debug_server)) goto nomem_4;
  _dbus_string_free(&address);
  _dbus_server_trace_ref(&debug_server->base, 0, 1, "debug_pipe_new");
  return (DBusServer*)debug_server;
nomem_4:
  _dbus_server_finalize_base(&debug_server->base);
fail_3:
  dbus_free(debug_server->name);
nomem_2:
  _dbus_string_free(&address);
nomem_1:
  dbus_free(debug_server);
nomem_0:
  pipe_hash_unref();
  if (error != NULL && !dbus_error_is_set(error)) _DBUS_SET_OOM(error);
  return NULL;
}
DBusTransport* _dbus_transport_debug_pipe_new(const char *server_name, DBusError *error) {
  DBusTransport *client_transport;
  DBusTransport *server_transport;
  DBusConnection *connection;
  DBusSocket client_fd, server_fd;
  DBusServer *server;
  DBusString address;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (server_pipe_hash == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_SERVER, NULL);
      return NULL;
  }
  server = _dbus_hash_table_lookup_string(server_pipe_hash, server_name);
  if (server == NULL || ((DBusServerDebugPipe*)server)->disconnected) {
      dbus_set_error(error, DBUS_ERROR_NO_SERVER, NULL);
      return NULL;
  }
  if (!_dbus_string_init(&address)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
  }
  if (!_dbus_string_append(&address, "debug-pipe:name=") || !_dbus_string_append(&address, server_name)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&address);
      return NULL;
  }
  if (!_dbus_socketpair(&client_fd, &server_fd, FALSE, NULL)) {
      _dbus_verbose("failed to create full duplex pipe\n");
      dbus_set_error(error, DBUS_ERROR_FAILED, "Could not create full-duplex pipe");
      _dbus_string_free(&address);
      return NULL;
  }
  client_transport = _dbus_transport_new_for_socket(client_fd, NULL, &address);
  if (client_transport == NULL) {
      _dbus_close_socket(client_fd, NULL);
      _dbus_close_socket(server_fd, NULL);
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&address);
      return NULL;
  }
  _dbus_string_free(&address);
  _dbus_socket_invalidate(&client_fd);
  server_transport = _dbus_transport_new_for_socket(server_fd, &server->guid_hex, NULL);
  if (server_transport == NULL) {
      _dbus_transport_unref(client_transport);
      _dbus_close_socket(server_fd, NULL);
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
  }
  _dbus_socket_invalidate(&server_fd);
  if (!_dbus_transport_set_auth_mechanisms(server_transport, (const char**)server->auth_mechanisms)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_transport_unref(server_transport);
      _dbus_transport_unref(client_transport);
      return NULL;
  }
  connection = _dbus_connection_new_for_transport(server_transport);
  _dbus_transport_unref(server_transport);
  server_transport = NULL;
  if (connection == NULL) {
      _dbus_transport_unref(client_transport);
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
  }
  if (server->new_connection_function) {
      dbus_server_ref(server);
      (*server->new_connection_function)(server, connection, server->new_connection_data);
      dbus_server_unref(server);
  }
  _dbus_connection_close_if_only_one_ref(connection);
  dbus_connection_unref(connection);
  return client_transport;
}
DBusServerListenResult _dbus_server_listen_debug_pipe(DBusAddressEntry *entry, DBusServer **server_p, DBusError *error) {
  const char *method;
  *server_p = NULL;
  method = dbus_address_entry_get_method(entry);
  if (strcmp(method, "debug-pipe") == 0) {
      const char *name = dbus_address_entry_get_value(entry, "name");
      if (name == NULL) {
          _dbus_set_bad_address(error, "debug-pipe", "name",NULL);
          return DBUS_SERVER_LISTEN_BAD_ADDRESS;
      }
      *server_p = _dbus_server_debug_pipe_new(name, error);
      if (*server_p) {
          _DBUS_ASSERT_ERROR_IS_CLEAR(error);
          return DBUS_SERVER_LISTEN_OK;
      } else {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
      }
  } else {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return DBUS_SERVER_LISTEN_NOT_HANDLED;
  }
}
DBusTransportOpenResult _dbus_transport_open_debug_pipe(DBusAddressEntry *entry, DBusTransport **transport_p, DBusError *error) {
  const char *method;
  method = dbus_address_entry_get_method(entry);
  _dbus_assert(method != NULL);
  if (strcmp(method, "debug-pipe") == 0) {
      const char *name = dbus_address_entry_get_value(entry, "name");
      if (name == NULL) {
          _dbus_set_bad_address(error, "debug-pipe", "name",NULL);
          return DBUS_TRANSPORT_OPEN_BAD_ADDRESS;
      }
      *transport_p = _dbus_transport_debug_pipe_new(name, error);
      if (*transport_p == NULL) {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return DBUS_TRANSPORT_OPEN_DID_NOT_CONNECT;
      } else {
          _DBUS_ASSERT_ERROR_IS_CLEAR(error);
          return DBUS_TRANSPORT_OPEN_OK;
      }
  } else {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return DBUS_TRANSPORT_OPEN_NOT_HANDLED;
  }
}
#endif