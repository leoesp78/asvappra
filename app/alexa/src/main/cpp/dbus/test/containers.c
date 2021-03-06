#include <errno.h>
#include "../../gio/gio.h"
#include "../../glib/glib.h"
#include "../../glib/gstdio.h"
#include "../config.h"
#include "../dbus.h"
#if defined(DBUS_ENABLE_CONTAINERS) && !defined(HAVE_GIO_UNIX)
#define HAVE_CONTAINERS_TEST
#include "../../gio/gunixfdlist.h"
#include "../../gio/gunixsocketaddress.h"
#include "../dbus-sysdeps-unix.h"
#endif
#include "test-utils-glib.h"

typedef struct {
    gboolean skip;
    gchar *bus_address;
    GPid daemon_pid;
    GError *error;
    GDBusProxy *proxy;
    gchar *instance_path;
    gchar *socket_path;
    gchar *socket_dbus_address;
    GDBusConnection *unconfined_conn;
    GDBusConnection *confined_conn;
    GDBusConnection *observer_conn;
    GDBusProxy *observer_proxy;
    GHashTable *containers_removed;
    guint removed_sub;
} Fixture;
typedef struct {
  const gchar *config_file;
  enum {
      STOP_SERVER_EXPLICITLY,
      STOP_SERVER_DISCONNECT_FIRST,
      STOP_SERVER_NEVER_CONNECTED,
      STOP_SERVER_FORCE,
      STOP_SERVER_WITH_MANAGER
  }
  stop_server;
} Config;
static const Config default_config = {
  NULL,
  0
};
#ifdef DBUS_ENABLE_CONTAINERS
static void name_gone_set_boolean_cb(GDBusConnection *conn, const gchar *name, gpointer user_data) {
  gboolean *gone_p = user_data;
  //g_assert_nonnull(gone_p);
  //g_assert_false(*gone_p);
  *gone_p = TRUE;
}
#endif
static void instance_removed_cb(GDBusConnection *observer, const gchar *sender, const gchar *path, const gchar *iface, const gchar *member, GVariant *parameters,
                                gpointer user_data) {
  Fixture *f = user_data;
  const gchar *container;
  g_assert_cmpstr(sender, ==, DBUS_SERVICE_DBUS);
  g_assert_cmpstr(path, ==, DBUS_PATH_DBUS);
  g_assert_cmpstr(iface, ==, DBUS_INTERFACE_CONTAINERS1);
  g_assert_cmpstr(member, ==, "InstanceRemoved");
  g_assert_cmpstr(g_variant_get_type_string (parameters), ==, "(o)");
  g_variant_get(parameters, "(&o)", &container);
  //g_assert(!g_hash_table_contains(f->containers_removed, container));
  //g_hash_table_add(f->containers_removed, g_strdup(container));
}
static void setup(Fixture *f, gconstpointer context) {
  const Config *config = context;
  if (config == NULL) config = &default_config;
  f->bus_address = test_get_dbus_daemon(config->config_file, TEST_USER_ME,NULL, &f->daemon_pid);
  if (f->bus_address == NULL) {
      f->skip = TRUE;
      return;
  }
  f->unconfined_conn = g_dbus_connection_new_for_address_sync(f->bus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                              G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
  g_assert_no_error(f->error);
  f->observer_conn = g_dbus_connection_new_for_address_sync(f->bus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
  g_assert_no_error(f->error);
  f->containers_removed = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  f->removed_sub = g_dbus_connection_signal_subscribe(f->observer_conn, DBUS_SERVICE_DBUS, DBUS_INTERFACE_CONTAINERS1,"InstanceRemoved",
                                                      DBUS_PATH_DBUS, NULL, G_DBUS_SIGNAL_FLAGS_NONE, instance_removed_cb, f, NULL);
}
static void test_get_supported_arguments(Fixture *f, gconstpointer context) {
  GVariant *v;
#ifdef DBUS_ENABLE_CONTAINERS
  const gchar **args;
  gsize len;
#endif
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                   NULL, &f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_no_error(f->error);
  v = g_dbus_proxy_get_cached_property(f->proxy, "SupportedArguments");
  g_assert_cmpstr(g_variant_get_type_string(v), ==, "as");
  args = g_variant_get_strv(v, &len);
  g_assert_cmpuint(len, ==, 0);
  g_free(args);
  g_variant_unref(v);
#else
  g_assert_no_error(f->error);
  v = g_dbus_proxy_get_cached_property(f->proxy, "SupportedArguments");
  g_assert_null(v);
#endif
}
#ifdef HAVE_CONTAINERS_TEST
static gboolean add_container_server(Fixture *f, GVariant *parameters) {
  GVariant *tuple;
  GStatBuf stat_buf;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  g_test_message("Calling AddServer...");
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters,G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  if (f->error != NULL && _dbus_getuid() == 0 && _dbus_getenv("DBUS_TEST_UNINSTALLED") != NULL) {
      g_test_message("AddServer: %s", f->error->message);
      g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_FILE_NOT_FOUND);
      //g_test_skip("AddServer failed, probably because this dbus version is not fully installed");
      return FALSE;
  }
  g_assert_no_error(f->error);
  //g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string(tuple), ==, "(oays)");
  g_variant_get(tuple, "(o^ays)", &f->instance_path, &f->socket_path, &f->socket_dbus_address);
  //g_assert_true(g_str_has_prefix(f->socket_dbus_address, "unix:"));
  //g_assert_null(strchr(f->socket_dbus_address, ';'));
  /*g_assert_null(strchr(f->socket_dbus_address + strlen ("unix:"), ':'));
  g_clear_pointer(&tuple, g_variant_unref);
  g_assert_nonnull(f->instance_path);
  g_assert_true(g_variant_is_object_path(f->instance_path));
  g_assert_nonnull(f->socket_path);
  g_assert_true(g_path_is_absolute(f->socket_path));
  g_assert_nonnull(f->socket_dbus_address);*/
  g_assert_cmpstr(g_stat(f->socket_path, &stat_buf) == 0 ? NULL : g_strerror(errno), ==, NULL);
  g_assert_cmpuint((stat_buf.st_mode & S_IFMT), ==, S_IFSOCK);
  return TRUE;
}
#endif
static void test_basic(Fixture *f, gconstpointer context) {
/*#ifdef HAVE_CONTAINERS_TEST
  GVariant *asv;
  GVariant *creator;
  GVariant *parameters;
  GVariantDict dict;
  const gchar *confined_unique_name;
  const gchar *path_from_query;
  const gchar *manager_unique_name;
  const gchar *name;
  const gchar *name_owner;
  const gchar *type;
  guint32 uid;
  GStatBuf stat_buf;
  GVariant *tuple;
  if (f->skip) return;
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  if (!add_container_server(f, g_steal_pointer(&parameters))) return;
  g_test_message("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync(f->socket_dbus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
  g_assert_no_error(f->error);
  g_test_message("Making a method call from confined app...");
  tuple = g_dbus_connection_call_sync(f->confined_conn, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,"GetNameOwner",
                                      g_variant_new("(s)", DBUS_SERVICE_DBUS), G_VARIANT_TYPE("(s)"),
                                       G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string (tuple), ==, "(s)");
  g_variant_get(tuple, "(&s)", &name_owner);
  g_assert_cmpstr(name_owner, ==, DBUS_SERVICE_DBUS);
  g_clear_pointer(&tuple, g_variant_unref);
  g_test_message("Making a method call from confined app to unconfined...");
  manager_unique_name = g_dbus_connection_get_unique_name(f->unconfined_conn);
  tuple = g_dbus_connection_call_sync(f->confined_conn, manager_unique_name,"/", DBUS_INTERFACE_PEER,"Ping",NULL,
                                      G_VARIANT_TYPE_UNIT,G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string (tuple), ==, "()");
  g_clear_pointer(&tuple, g_variant_unref);
  g_test_message("Checking that confined app is not considered privileged...");
  tuple = g_dbus_connection_call_sync(f->confined_conn, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,"UpdateActivationEnvironment",
                                      g_variant_new("(a{ss})", NULL), G_VARIANT_TYPE_UNIT,G_DBUS_CALL_FLAGS_NONE,-1,
                                      NULL, &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
  g_test_message("Access denied as expected: %s", f->error->message);
  g_clear_error(&f->error);
  g_assert_null(tuple);
  g_test_message("Inspecting connection container info");
  confined_unique_name = g_dbus_connection_get_unique_name(f->confined_conn);
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetConnectionInstance",g_variant_new("(s)", confined_unique_name), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string(tuple), ==, "(oa{sv}ssa{sv})");
  g_variant_get(tuple, "(&o@a{sv}&s&s@a{sv})", &path_from_query, &creator, &type, &name, &asv);
  g_assert_cmpstr(path_from_query, ==, f->instance_path);
  g_variant_dict_init(&dict, creator);
  g_assert_true(g_variant_dict_lookup(&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint(uid, ==, _dbus_getuid());
  g_variant_dict_clear(&dict);
  g_assert_cmpstr(type, ==, "com.example.NotFlatpak");
  g_assert_cmpstr(name, ==, "sample-app");
  g_assert_cmpuint(g_variant_n_children (asv), ==, 0);
  g_clear_pointer(&asv, g_variant_unref);
  g_clear_pointer(&creator, g_variant_unref);
  g_clear_pointer(&tuple, g_variant_unref);
  g_test_message("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetInstanceInfo",g_variant_new("(o)", f->instance_path), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string(tuple), ==, "(a{sv}ssa{sv})");
  g_variant_get(tuple, "(@a{sv}&s&s@a{sv})", &creator, &type, &name, &asv);
  g_variant_dict_init(&dict, creator);
  g_assert_true(g_variant_dict_lookup(&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint(uid, ==, _dbus_getuid());
  g_variant_dict_clear(&dict);
  g_assert_cmpstr(type, ==, "com.example.NotFlatpak");
  g_assert_cmpstr(name, ==, "sample-app");
  g_assert_cmpuint(g_variant_n_children(asv), ==, 0);
  g_clear_pointer(&asv, g_variant_unref);
  g_clear_pointer(&creator, g_variant_unref);
  g_clear_pointer(&tuple, g_variant_unref);
  test_kill_pid(f->daemon_pid);
  g_spawn_close_pid(f->daemon_pid);
  f->daemon_pid = 0;
  while (g_stat(f->socket_path, &stat_buf) == 0) g_usleep(G_USEC_PER_SEC / 20);
  g_assert_cmpint(errno, ==, ENOENT);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
  */
}
static void test_wrong_uid(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  GVariant *parameters;
  if (f->skip) return;
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  if (!add_container_server(f, g_steal_pointer(&parameters))) return;
  g_test_message("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = test_try_connect_gdbus_as_user(f->socket_dbus_address, TEST_USER_OTHER, &f->error);
  if (f->error != NULL && g_error_matches(f->error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
      //g_test_skip(f->error->message);
      return;
  }
  g_assert_error(f->error, G_IO_ERROR, G_IO_ERROR_CLOSED);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
static void test_metadata(Fixture *f, gconstpointer context) {
/*#ifdef HAVE_CONTAINERS_TEST
  GVariant *asv;
  GVariant *creator;
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict dict;
  const gchar *confined_unique_name;
  const gchar *path_from_query;
  const gchar *name;
  const gchar *type;
  guint32 uid;
  guint u;
  gboolean b;
  const gchar *s;
  if (f->skip) return;
  g_variant_dict_init(&dict, NULL);
  g_variant_dict_insert(&dict, "Species", "s", "Martes martes");
  g_variant_dict_insert(&dict, "IsCrepuscular", "b", TRUE);
  g_variant_dict_insert(&dict, "NChildren", "u", 2);
  parameters = g_variant_new("(ss@a{sv}a{sv})", "org.example.Springwatch", "", g_variant_dict_end(&dict), NULL);
  if (!add_container_server(f, g_steal_pointer (&parameters))) return;
  g_test_message("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync(f->socket_dbus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
  g_assert_no_error(f->error);
  g_test_message("Inspecting connection credentials...");
  confined_unique_name = g_dbus_connection_get_unique_name(f->confined_conn);
  tuple = g_dbus_connection_call_sync(f->confined_conn, DBUS_SERVICE_DBUS,DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,"GetConnectionCredentials",
                                      g_variant_new("(s)",confined_unique_name), G_VARIANT_TYPE("(a{sv})"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string(tuple), ==, "(a{sv})");
  asv = g_variant_get_child_value(tuple, 0);
  g_variant_dict_init(&dict, asv);
  g_assert_true(g_variant_dict_lookup(&dict, DBUS_INTERFACE_CONTAINERS1 ".Instance", "&o", &path_from_query));
  g_assert_cmpstr(path_from_query, ==, f->instance_path);
  g_variant_dict_clear(&dict);
  g_clear_pointer(&asv, g_variant_unref);
  g_clear_pointer(&tuple, g_variant_unref);
  g_test_message("Inspecting connection container info");
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetConnectionInstance",g_variant_new ("(s)", confined_unique_name), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string (tuple), ==, "(oa{sv}ssa{sv})");
  g_variant_get(tuple, "(&o@a{sv}&s&s@a{sv})", &path_from_query, &creator, &type, &name, &asv);
  g_assert_cmpstr(path_from_query, ==, f->instance_path);
  g_variant_dict_init(&dict, creator);
  g_assert_true(g_variant_dict_lookup(&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint(uid, ==, _dbus_getuid ());
  g_variant_dict_clear(&dict);
  g_assert_cmpstr(type, ==, "org.example.Springwatch");
  g_assert_cmpstr(name, ==, "");
  g_variant_dict_init(&dict, asv);
  g_assert_true(g_variant_dict_lookup(&dict, "NChildren", "u", &u));
  g_assert_cmpuint(u, ==, 2);
  g_assert_true(g_variant_dict_lookup(&dict, "IsCrepuscular", "b", &b));
  g_assert_cmpint(b, ==, TRUE);
  g_assert_true(g_variant_dict_lookup(&dict, "Species", "&s", &s));
  g_assert_cmpstr(s, ==, "Martes martes");
  g_variant_dict_clear(&dict);
  g_assert_cmpuint(g_variant_n_children (asv), ==, 3);
  g_clear_pointer(&asv, g_variant_unref);
  g_clear_pointer(&creator, g_variant_unref);
  g_clear_pointer(&tuple, g_variant_unref);
  g_test_message("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetInstanceInfo",g_variant_new("(o)", f->instance_path),G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_assert_nonnull(tuple);
  g_assert_cmpstr(g_variant_get_type_string (tuple), ==, "(a{sv}ssa{sv})");
  g_variant_get(tuple, "(@a{sv}&s&s@a{sv})", &creator, &type, &name, &asv);
  g_variant_dict_init(&dict, creator);
  g_assert_true(g_variant_dict_lookup (&dict, "UnixUserID", "u", &uid));
  g_assert_cmpuint(uid, ==, _dbus_getuid ());
  g_variant_dict_clear(&dict);
  g_assert_cmpstr(type, ==, "org.example.Springwatch");
  g_assert_cmpstr(name, ==, "");
  g_variant_dict_init(&dict, asv);
  g_assert_true(g_variant_dict_lookup(&dict, "NChildren", "u", &u));
  g_assert_cmpuint(u, ==, 2);
  g_assert_true(g_variant_dict_lookup(&dict, "IsCrepuscular", "b", &b));
  g_assert_cmpint(b, ==, TRUE);
  g_assert_true(g_variant_dict_lookup(&dict, "Species", "&s", &s));
  g_assert_cmpstr(s, ==, "Martes martes");
  g_variant_dict_clear(&dict);
  g_assert_cmpuint(g_variant_n_children(asv), ==, 3);
  g_clear_pointer(&asv, g_variant_unref);
  g_clear_pointer(&creator, g_variant_unref);
  g_clear_pointer(&tuple, g_variant_unref);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif*/
}
static void test_stop_server(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  const Config *config = context;
  GDBusConnection *attacker;
  GDBusConnection *second_confined_conn;
  GDBusProxy *attacker_proxy;
  GSocket *client_socket;
  GSocketAddress *socket_address;
  GVariant *tuple;
  GVariant *parameters;
  gchar *error_name;
  const gchar *confined_unique_name;
  const gchar *manager_unique_name;
  const gchar *name_owner;
  gboolean gone = FALSE;
  guint name_watch;
  guint i;
  //g_assert_nonnull(config);
  if (f->skip) return;
  f->observer_proxy = g_dbus_proxy_new_sync(f->observer_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                            DBUS_INTERFACE_CONTAINERS1, NULL, &f->error);
  g_assert_no_error(f->error);
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  if (!add_container_server(f, g_steal_pointer(&parameters))) return;
  socket_address = g_unix_socket_address_new(f->socket_path);
  if (config->stop_server != STOP_SERVER_NEVER_CONNECTED) {
      g_test_message("Connecting to %s...", f->socket_dbus_address);
      f->confined_conn = g_dbus_connection_new_for_address_sync(f->socket_dbus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                                G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
      g_assert_no_error(f->error);
      if (config->stop_server == STOP_SERVER_DISCONNECT_FIRST) {
          g_test_message("Disconnecting confined connection...");
          gone = FALSE;
          confined_unique_name = g_dbus_connection_get_unique_name(f->confined_conn);
          name_watch = g_bus_watch_name_on_connection(f->observer_conn, confined_unique_name,G_BUS_NAME_WATCHER_FLAGS_NONE,NULL,
                                                      name_gone_set_boolean_cb, &gone, NULL);
          g_dbus_connection_close_sync(f->confined_conn, NULL, &f->error);
          g_assert_no_error(f->error);
          g_test_message("Waiting for confined app bus name to disappear...");
          while(!gone) g_main_context_iteration(NULL, TRUE);
          g_bus_unwatch_name(name_watch);
      }
  }
  attacker = test_try_connect_gdbus_as_user(f->bus_address, TEST_USER_OTHER, &f->error);
  if (attacker != NULL) {
      attacker_proxy = g_dbus_proxy_new_sync(attacker,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                             DBUS_INTERFACE_CONTAINERS1, NULL, &f->error);
      g_assert_no_error(f->error);
      tuple = g_dbus_proxy_call_sync(attacker_proxy, "StopListening",g_variant_new("(o)", f->instance_path),
                                     G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
      g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
      //g_assert_null(tuple);
      g_clear_error(&f->error);
      tuple = g_dbus_proxy_call_sync(attacker_proxy, "StopInstance",g_variant_new("(o)", f->instance_path),
                                    G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
      g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
      //g_assert_null(tuple);
      g_clear_error(&f->error);
      g_clear_object(&attacker_proxy);
      g_dbus_connection_close_sync(attacker, NULL, &f->error);
      g_assert_no_error(f->error);
      g_clear_object(&attacker);
  } else {
      g_assert_error(f->error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
      g_clear_error(&f->error);
  }
  //g_assert_false(g_hash_table_contains(f->containers_removed, f->instance_path));
  switch(config->stop_server) {
      case STOP_SERVER_WITH_MANAGER:
          g_test_message("Closing container manager...");
          manager_unique_name = g_dbus_connection_get_unique_name(f->unconfined_conn);
          name_watch = g_bus_watch_name_on_connection(f->confined_conn, manager_unique_name,G_BUS_NAME_WATCHER_FLAGS_NONE,NULL,
                                                      name_gone_set_boolean_cb, &gone, NULL);
          g_dbus_connection_close_sync(f->unconfined_conn, NULL, &f->error);
          g_assert_no_error(f->error);
          g_test_message("Waiting for container manager bus name to disappear...");
          while(!gone) g_main_context_iteration(NULL, TRUE);
          g_bus_unwatch_name(name_watch);
          break;
      case STOP_SERVER_EXPLICITLY:
          g_test_message("Stopping server (but not confined connection)...");
          tuple = g_dbus_proxy_call_sync(f->proxy, "StopListening",g_variant_new("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
          g_assert_no_error(f->error);
          g_variant_unref(tuple);
          g_test_message("Checking we do not get InstanceRemoved...");
          tuple = g_dbus_proxy_call_sync(f->proxy, "NoSuchMethod",NULL,G_DBUS_CALL_FLAGS_NONE,-1,NULL, &f->error);
          g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
          //g_assert_null(tuple);
          g_clear_error(&f->error);
          break;
      case STOP_SERVER_DISCONNECT_FIRST: case STOP_SERVER_NEVER_CONNECTED:
          g_test_message("Stopping server (with no confined connections)...");
          tuple = g_dbus_proxy_call_sync(f->proxy,"StopListening",g_variant_new("(o)", f->instance_path),G_DBUS_CALL_FLAGS_NONE,
                                        -1, NULL, &f->error);
          g_assert_no_error(f->error);
          g_variant_unref(tuple);
          g_test_message("Waiting for InstanceRemoved...");
          //while(!g_hash_table_contains(f->containers_removed, f->instance_path)) g_main_context_iteration(NULL, TRUE);
          break;
      case STOP_SERVER_FORCE:
          g_test_message("Stopping server and all confined connections...");
          tuple = g_dbus_proxy_call_sync(f->proxy,"StopInstance",g_variant_new("(o)", f->instance_path),G_DBUS_CALL_FLAGS_NONE,
                                        -1, NULL, &f->error);
          g_assert_no_error(f->error);
          g_variant_unref(tuple);
          g_test_message("Waiting for InstanceRemoved...");
          //while (!g_hash_table_contains(f->containers_removed, f->instance_path)) g_main_context_iteration(NULL, TRUE);
          break;
      default: g_assert_not_reached();
  }
  for (i = 0; i < 50; i++) {
      g_test_message("Trying to connect to %s again...", f->socket_path);
      client_socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT, &f->error);
      g_assert_no_error(f->error);
      if (!g_socket_connect(client_socket, socket_address, NULL, &f->error)) {
          g_assert_cmpstr(g_quark_to_string(f->error->domain), ==,g_quark_to_string(G_IO_ERROR));
          if (f->error->code != G_IO_ERROR_CONNECTION_REFUSED && f->error->code != G_IO_ERROR_NOT_FOUND) g_error("Unexpected error code %d", f->error->code);
          g_clear_error(&f->error);
          g_clear_object(&client_socket);
          break;
      }
      g_clear_object(&client_socket);
      g_usleep(G_USEC_PER_SEC / 10);
  }
  g_test_message("Trying to connect to %s again...", f->socket_dbus_address);
  second_confined_conn = g_dbus_connection_new_for_address_sync(f->socket_dbus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                                G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
  g_assert_cmpstr(g_quark_to_string (f->error->domain), ==,g_quark_to_string(G_IO_ERROR));
  if (f->error->code != G_IO_ERROR_CONNECTION_REFUSED && f->error->code != G_IO_ERROR_NOT_FOUND) g_error("Unexpected error code %d", f->error->code);
  g_clear_error(&f->error);
  //g_assert_null(second_confined_conn);
  //g_assert_false(g_file_test(f->socket_path, G_FILE_TEST_EXISTS));
  switch(config->stop_server) {
      case STOP_SERVER_FORCE:
          g_test_message("Checking that the confined app gets disconnected...");
          while(!g_dbus_connection_is_closed(f->confined_conn)) g_main_context_iteration(NULL, TRUE);
          break;
      case STOP_SERVER_DISCONNECT_FIRST: case STOP_SERVER_NEVER_CONNECTED: break;
      case STOP_SERVER_EXPLICITLY: case STOP_SERVER_WITH_MANAGER:
          g_test_message("Checking that the confined app still works...");
          tuple = g_dbus_connection_call_sync(f->confined_conn, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS,"GetNameOwner",
                                             g_variant_new("(s)", DBUS_SERVICE_DBUS), G_VARIANT_TYPE ("(s)"),G_DBUS_CALL_FLAGS_NONE,
                                             -1,NULL, &f->error);
          g_assert_no_error(f->error);
          //g_assert_nonnull(tuple);
          g_assert_cmpstr(g_variant_get_type_string(tuple), ==, "(s)");
          g_variant_get(tuple, "(&s)", &name_owner);
          g_assert_cmpstr(name_owner, ==, DBUS_SERVICE_DBUS);
          //g_clear_pointer(&tuple, g_variant_unref);
          tuple = g_dbus_proxy_call_sync(f->observer_proxy, "GetInstanceInfo",g_variant_new("(o)", f->instance_path),
                                        G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
          g_assert_no_error(f->error);
          //g_assert_nonnull(tuple);
          //g_clear_pointer(&tuple, g_variant_unref);
          g_test_message("Closing confined connection...");
          g_dbus_connection_close_sync(f->confined_conn, NULL, &f->error);
          g_assert_no_error(f->error);
          break;
      default: g_assert_not_reached();
  }
  g_test_message("Waiting for InstanceRemoved...");
  //while(!g_hash_table_contains(f->containers_removed, f->instance_path)) g_main_context_iteration(NULL, TRUE);
  tuple = g_dbus_proxy_call_sync(f->observer_proxy,"GetInstanceInfo",g_variant_new("(o)", f->instance_path),G_DBUS_CALL_FLAGS_NONE,
                                -1, NULL, &f->error);
  //g_assert_nonnull(f->error);
  error_name = g_dbus_error_get_remote_error(f->error);
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_NOT_CONTAINER);
  g_free(error_name);
  //g_assert_null(tuple);
  g_clear_error(&f->error);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
static void test_invalid_metadata_getters(Fixture *f, gconstpointer context) {
  const gchar *unique_name;
  GVariant *tuple;
  gchar *error_name;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  g_test_message("Inspecting unconfined connection");
  unique_name = g_dbus_connection_get_unique_name(f->unconfined_conn);
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetConnectionInstance",g_variant_new("(s)", unique_name),G_DBUS_CALL_FLAGS_NONE,
                                 -1, NULL, &f->error);
  //g_assert_nonnull(f->error);
  //g_assert_null(tuple);
  error_name = g_dbus_error_get_remote_error(f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_NOT_CONTAINER);
#else
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
#endif
  g_free(error_name);
  g_clear_error(&f->error);
  g_test_message("Inspecting a non-connection");
  unique_name = g_dbus_connection_get_unique_name(f->unconfined_conn);
  tuple = g_dbus_proxy_call_sync(f->proxy, "GetConnectionInstance",g_variant_new("(s)", "com.example.Nope"),
                                G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  //g_assert_nonnull(f->error);
  //g_assert_null(tuple);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_NAME_HAS_NO_OWNER);
#else
  error_name = g_dbus_error_get_remote_error(f->error);
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  g_free(error_name);
#endif
  g_clear_error(&f->error);
  g_test_message("Inspecting container instance info");
  tuple = g_dbus_proxy_call_sync(f->proxy,"GetInstanceInfo",g_variant_new("(o)", "/nope"),G_DBUS_CALL_FLAGS_NONE,
                                -1, NULL, &f->error);
  //g_assert_nonnull(f->error);
  //g_assert_null(tuple);
  error_name = g_dbus_error_get_remote_error(f->error);
#ifdef DBUS_ENABLE_CONTAINERS
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_NOT_CONTAINER);
#else
  g_assert_cmpstr(error_name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
#endif
  g_free(error_name);
  g_clear_error(&f->error);
}
static void test_unsupported_parameter(Fixture *f, gconstpointer context) {
/*#ifdef HAVE_CONTAINERS_TEST
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict named_argument_builder;
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1, NULL, &f->error);
  g_assert_no_error(f->error);
  g_variant_dict_init(&named_argument_builder, NULL);
  g_variant_dict_insert(&named_argument_builder, "ThisArgumentIsntImplemented", "b", FALSE);
  parameters = g_variant_new("(ssa{sv}@a{sv})", "com.example.NotFlatpak", "sample-app", NULL, g_variant_dict_end(&named_argument_builder));
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", g_steal_pointer(&parameters),G_DBUS_CALL_FLAGS_NONE,-1,NULL, &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS);
  g_assert_null(tuple);
  g_clear_error(&f->error);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif*/
}
static void test_invalid_type_name(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  GVariant *tuple;
  GVariant *parameters;
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  parameters = g_variant_new("(ssa{sv}a{sv})", "this is not a valid container type name", "sample-app", NULL, NULL);
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", g_steal_pointer(&parameters),G_DBUS_CALL_FLAGS_NONE, -1,NULL,
                                 &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS);
  //g_assert_null(tuple);
  g_clear_error(&f->error);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
static void test_invalid_nesting(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  GDBusProxy *nested_proxy;
  GVariant *tuple;
  GVariant *parameters;
  if (f->skip) return;
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  if (!add_container_server(f, g_steal_pointer(&parameters))) return;
  g_test_message("Connecting to %s...", f->socket_dbus_address);
  f->confined_conn = g_dbus_connection_new_for_address_sync(f->socket_dbus_address,(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT), NULL, NULL, &f->error);
  g_assert_no_error(f->error);
  g_test_message("Checking that confined app cannot nest containers...");
  nested_proxy = g_dbus_proxy_new_sync(f->confined_conn,G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,
                                      NULL, &f->error);
  g_assert_no_error(f->error);
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "inner-app", NULL, NULL);
  tuple = g_dbus_proxy_call_sync(nested_proxy, "AddServer", g_steal_pointer(&parameters),G_DBUS_CALL_FLAGS_NONE,-1,NULL,
                                 &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED);
  //g_assert_null(tuple);
  g_clear_error(&f->error);
  g_clear_object(&nested_proxy);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
static void test_max_containers(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  GVariant *parameters;
  GVariant *tuple;
  gchar *placeholders[3] = { NULL };
  guint i;
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  g_variant_ref_sink(parameters);
  for (i = 0; i < G_N_ELEMENTS(placeholders); i++) {
      tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters, G_DBUS_CALL_FLAGS_NONE, -1,NULL, &f->error);
      g_assert_no_error(f->error);
      //g_assert_nonnull(tuple);
      g_variant_get(tuple, "(o^ays)", &placeholders[i], NULL, NULL);
      g_variant_unref(tuple);
      g_test_message("Placeholder server at %s", placeholders[i]);
  }
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters, G_DBUS_CALL_FLAGS_NONE, -1,NULL, &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_LIMITS_EXCEEDED);
  g_clear_error(&f->error);
  //g_assert_null(tuple);
  tuple = g_dbus_proxy_call_sync(f->proxy, "StopListening",g_variant_new("(o)", placeholders[1]),G_DBUS_CALL_FLAGS_NONE,
                                -1, NULL, &f->error);
  g_assert_no_error(f->error);
  //g_assert_nonnull(tuple);
  g_variant_unref(tuple);
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters, G_DBUS_CALL_FLAGS_NONE, -1,NULL, &f->error);
  g_assert_no_error(f->error);
  //g_assert_nonnull(tuple);
  g_variant_unref(tuple);
  g_variant_unref(parameters);
  for (i = 0; i < G_N_ELEMENTS(placeholders); i++) g_free(placeholders[i]);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
#ifdef HAVE_CONTAINERS_TEST
static void assert_connection_closed(GError *error) {
  //if (error->code == G_IO_ERROR_BROKEN_PIPE) { g_assert_error(error, G_IO_ERROR, G_IO_ERROR_BROKEN_PIPE); }
  //else {
      g_assert_error(error, G_IO_ERROR, G_IO_ERROR_FAILED);
      g_test_message("Old GLib: %s", error->message);
      //g_assert_true(strstr(error->message, g_strerror(ECONNRESET)) != NULL);
  //}
}
#endif
static void
test_max_connections_per_container(Fixture *f, gconstpointer context) {
#ifdef HAVE_CONTAINERS_TEST
  gchar *socket_paths[2] = { NULL };
  gchar *dbus_addresses[G_N_ELEMENTS(socket_paths)] = { NULL };
  GSocketAddress *socket_addresses[G_N_ELEMENTS(socket_paths)] = { NULL };
  GSocket *placeholders[G_N_ELEMENTS(socket_paths) * 3] = { NULL };
  GVariant *parameters;
  GVariant *tuple;
  guint i;
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                    DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  parameters = g_variant_new("(ssa{sv}a{sv})", "com.example.NotFlatpak", "sample-app", NULL, NULL);
  g_variant_ref_sink(parameters);
  for (i = 0; i < G_N_ELEMENTS(socket_paths); i++) {
      tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters, G_DBUS_CALL_FLAGS_NONE, -1,NULL, &f->error);
      g_assert_no_error(f->error);
      //g_assert_nonnull(tuple);
      g_variant_get(tuple, "(o^ays)", NULL, &socket_paths[i], &dbus_addresses[i]);
      g_variant_unref(tuple);
      socket_addresses[i] = g_unix_socket_address_new(socket_paths[i]);
      g_test_message("Server #%u at %s", i, socket_paths[i]);
  }
  for (i = 0; i < G_N_ELEMENTS(placeholders); i++) {
      placeholders[i] = g_socket_new(G_SOCKET_FAMILY_UNIX,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_DEFAULT, &f->error);
      g_assert_no_error(f->error);
      g_socket_connect(placeholders[i],socket_addresses[i % G_N_ELEMENTS(socket_paths)],NULL, &f->error);
      g_assert_no_error(f->error);
      g_test_message("Placeholder connection #%u to %s", i, socket_paths[i % G_N_ELEMENTS(socket_paths)]);
  }
  for (i = 0; i < G_N_ELEMENTS(socket_paths); i++) {
      f->confined_conn = g_dbus_connection_new_for_address_sync(dbus_addresses[i],(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                                G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
      assert_connection_closed(f->error);
      g_clear_error(&f->error);
      //g_assert_null(f->confined_conn);
  }
  g_socket_close(placeholders[2], &f->error);
  g_assert_no_error(f->error);
  while(f->confined_conn == NULL) {
      g_test_message("Trying to use the slot we just freed up...");
      f->confined_conn = g_dbus_connection_new_for_address_sync(dbus_addresses[0],(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                                G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
      if (f->confined_conn == NULL) {
          assert_connection_closed(f->error);
          g_clear_error(&f->error);
          //g_assert_nonnull(f->confined_conn);
      } else { g_assert_no_error(f->error); }
  }
  for (i = 0; i < G_N_ELEMENTS(socket_paths); i++) {
      GDBusConnection *another = g_dbus_connection_new_for_address_sync(dbus_addresses[i],(G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
                                                                        G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),NULL, NULL, &f->error);
      assert_connection_closed(f->error);
      g_clear_error(&f->error);
      //g_assert_null(another);
  }
  g_variant_unref(parameters);
  for (i = 0; i < G_N_ELEMENTS(socket_paths); i++) {
      g_free(socket_paths[i]);
      g_free(dbus_addresses[i]);
      g_clear_object(&socket_addresses[i]);
  }
#undef LIMIT
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif
}
static void teardown(Fixture *f, gconstpointer context G_GNUC_UNUSED) {
  g_clear_object(&f->proxy);
  if (f->observer_conn != NULL) {
      GError *error = NULL;
      g_dbus_connection_signal_unsubscribe(f->observer_conn, f->removed_sub);
      g_dbus_connection_close_sync(f->observer_conn, NULL, &error);
      if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CLOSED)) g_clear_error(&error);
      else g_assert_no_error(error);
  }
  //g_clear_pointer(&f->containers_removed, g_hash_table_unref);
  g_clear_object(&f->observer_conn);
  if (f->unconfined_conn != NULL) {
      GError *error = NULL;
      g_dbus_connection_close_sync(f->unconfined_conn, NULL, &error);
      if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CLOSED)) g_clear_error(&error);
      else g_assert_no_error(error);
  }
  g_clear_object(&f->unconfined_conn);
  if (f->confined_conn != NULL) {
      GError *error = NULL;
      g_dbus_connection_close_sync(f->confined_conn, NULL, &error);
      if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CLOSED)) g_clear_error(&error);
      else g_assert_no_error(error);
  }
  g_clear_object(&f->confined_conn);
  if (f->daemon_pid != 0) {
      test_kill_pid(f->daemon_pid);
      g_spawn_close_pid(f->daemon_pid);
      f->daemon_pid = 0;
  }
  g_free(f->instance_path);
  g_free(f->socket_path);
  g_free(f->socket_dbus_address);
  g_free(f->bus_address);
  g_clear_error(&f->error);
}
static void test_max_container_metadata_bytes(Fixture *f, gconstpointer context) {
/*#ifdef HAVE_CONTAINERS_TEST
  guchar waste_of_space[4096] = { 0 };
  GVariant *tuple;
  GVariant *parameters;
  GVariantDict dict;
  if (f->skip) return;
  f->proxy = g_dbus_proxy_new_sync(f->unconfined_conn,G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
                                   DBUS_INTERFACE_CONTAINERS1,NULL, &f->error);
  g_assert_no_error(f->error);
  g_variant_dict_init(&dict, NULL);
  g_variant_dict_insert(&dict, "waste of space", "@ay", g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, waste_of_space, sizeof(waste_of_space), 1));
  parameters = g_variant_new("(ss@a{sv}a{sv})", "com.wasteheadquarters", "Packt Like Sardines in a Crushd Tin Box", g_variant_dict_end(&dict), NULL);
  tuple = g_dbus_proxy_call_sync(f->proxy, "AddServer", parameters,G_DBUS_CALL_FLAGS_NONE, -1, NULL, &f->error);
  g_assert_error(f->error, G_DBUS_ERROR, G_DBUS_ERROR_LIMITS_EXCEEDED);
  g_assert_null(tuple);
  g_clear_error(&f->error);
#else
  g_test_skip("Containers or gio-unix-2.0 not supported");
#endif*/
}
static const Config stop_server_explicitly = {
  "valid-config-files/multi-user.conf",
  STOP_SERVER_EXPLICITLY
};
static const Config stop_server_disconnect_first = {
  "valid-config-files/multi-user.conf",
  STOP_SERVER_DISCONNECT_FIRST
};
static const Config stop_server_never_connected = {
  "valid-config-files/multi-user.conf",
  STOP_SERVER_NEVER_CONNECTED
};
static const Config stop_server_force = {
  "valid-config-files/multi-user.conf",
  STOP_SERVER_FORCE
};
static const Config stop_server_with_manager = {
  "valid-config-files/multi-user.conf",
  STOP_SERVER_WITH_MANAGER
};
static const Config limit_containers =
{
  "valid-config-files/limit-containers.conf",
  0
};
static const Config max_containers = {
  "valid-config-files/max-containers.conf",
  0
};
int main(int argc, char **argv) {
  GError *error = NULL;
  gchar *runtime_dir;
  gchar *runtime_dbus_dir;
  gchar *runtime_containers_dir;
  gchar *runtime_services_dir;
  int ret;
  runtime_dir = NULL;//g_dir_make_tmp("dbus-test-containers.XXXXXX", &error);
  if (runtime_dir == NULL) {
      g_print("Bail out! %s\n", error->message);
      g_clear_error(&error);
      return 1;
  }
  g_setenv("XDG_RUNTIME_DIR", runtime_dir, TRUE);
  runtime_dbus_dir = g_build_filename(runtime_dir, "dbus-1", NULL);
  runtime_containers_dir = g_build_filename(runtime_dir, "dbus-1", "containers", NULL);
  runtime_services_dir = g_build_filename(runtime_dir, "dbus-1", "services", NULL);
  test_init(&argc, &argv);
  g_test_add("/containers/get-supported-arguments", Fixture, NULL, setup, test_get_supported_arguments, teardown);
  g_test_add("/containers/basic", Fixture, NULL, setup, test_basic, teardown);
  g_test_add("/containers/wrong-uid", Fixture, NULL, setup, test_wrong_uid, teardown);
  g_test_add("/containers/stop-server/explicitly", Fixture,&stop_server_explicitly, setup, test_stop_server, teardown);
  g_test_add("/containers/stop-server/disconnect-first", Fixture,&stop_server_disconnect_first, setup, test_stop_server, teardown);
  g_test_add("/containers/stop-server/never-connected", Fixture,&stop_server_never_connected, setup, test_stop_server, teardown);
  g_test_add("/containers/stop-server/force", Fixture,&stop_server_force, setup, test_stop_server, teardown);
  g_test_add("/containers/stop-server/with-manager", Fixture,&stop_server_with_manager, setup, test_stop_server, teardown);
  g_test_add("/containers/metadata", Fixture, &limit_containers,setup, test_metadata, teardown);
  g_test_add("/containers/invalid-metadata-getters", Fixture, NULL,setup, test_invalid_metadata_getters, teardown);
  g_test_add("/containers/unsupported-parameter", Fixture, NULL,setup, test_unsupported_parameter, teardown);
  g_test_add("/containers/invalid-type-name", Fixture, NULL, setup, test_invalid_type_name, teardown);
  g_test_add("/containers/invalid-nesting", Fixture, NULL, setup, test_invalid_nesting, teardown);
  g_test_add("/containers/max-containers", Fixture, &max_containers, setup, test_max_containers, teardown);
  g_test_add("/containers/max-containers-per-user", Fixture, &limit_containers, setup, test_max_containers, teardown);
  g_test_add("/containers/max-connections-per-container", Fixture,&limit_containers, setup, test_max_connections_per_container, teardown);
  g_test_add("/containers/max-container-metadata-bytes", Fixture,&limit_containers, setup, test_max_container_metadata_bytes, teardown);
  ret = g_test_run();
  test_rmdir_if_exists(runtime_containers_dir);
  test_rmdir_if_exists(runtime_services_dir);
  test_rmdir_if_exists(runtime_dbus_dir);
  test_rmdir_must_exist(runtime_dir);
  g_free(runtime_containers_dir);
  g_free(runtime_services_dir);
  g_free(runtime_dbus_dir);
  g_free(runtime_dir);
  return ret;
}