#include <sys/types.h>
#include "../config.h"
#include "../dbus-internals.h"
#include "../dbus-hash.h"
#include "../dbus-message-internal.h"
#include "../dbus-sysdeps-unix.h"
#include "containers.h"
#include "connection.h"
#include "dispatch.h"
#include "driver.h"
#include "utils.h"
#ifdef DBUS_ENABLE_CONTAINERS

#ifndef DBUS_UNIX
# error DBUS_ENABLE_CONTAINERS requires DBUS_UNIX
#endif

typedef struct {
  int refcount;
  char *path;
  char *type;
  char *name;
  DBusVariant *metadata;
  BusContext *context;
  BusContainers *containers;
  DBusServer *server;
  DBusConnection *creator;
  DBusList *connections;
  unsigned long uid;
  unsigned announced:1;
} BusContainerInstance;
typedef struct {
  DBusList *instances;
} BusContainerCreatorData;
static dbus_int32_t container_creator_data_slot = -1;
struct BusContainers {
  int refcount;
  DBusHashTable *instances_by_path;
  DBusHashTable *n_containers_by_user;
  DBusString address_template;
  dbus_uint64_t next_container_id;
};
static dbus_int32_t contained_data_slot = -1;
BusContainers *bus_containers_new(void) {
  BusContainers *self = NULL;
  DBusString invalid = _DBUS_STRING_INIT_INVALID;
  if (!dbus_connection_allocate_data_slot(&contained_data_slot)) goto oom;
  if (!dbus_connection_allocate_data_slot(&container_creator_data_slot)) goto oom;
  self = dbus_new0(BusContainers, 1);
  if (self == NULL) goto oom;
  self->refcount = 1;
  self->instances_by_path = NULL;
  //self->next_container_id = DBUS_UINT64_CONSTANT(0);
  self->address_template = invalid;
  if (!_dbus_string_init(&self->address_template)) goto oom;
  if (_dbus_getuid() == 0) {
      DBusString dir;
      _dbus_string_init_const(&dir, DBUS_RUNSTATEDIR "/dbus/containers");
      if (!_dbus_string_append(&self->address_template, "unix:dir=") || !_dbus_address_append_escaped(&self->address_template, &dir)) goto oom;
  }
  return self;
oom:
  if (self != NULL) bus_containers_unref(self);
  else {
      if (contained_data_slot != -1) dbus_connection_free_data_slot(&contained_data_slot);
      if (container_creator_data_slot != -1) dbus_connection_free_data_slot(&container_creator_data_slot);
  }
  return NULL;
}
BusContainers *bus_containers_ref(BusContainers *self) {
  _dbus_assert(self->refcount > 0);
  _dbus_assert(self->refcount < _DBUS_INT_MAX);
  self->refcount++;
  return self;
}
void bus_containers_unref(BusContainers *self) {
  _dbus_assert(self != NULL);
  _dbus_assert(self->refcount > 0);
  if (--self->refcount == 0) {
      _dbus_clear_hash_table(&self->instances_by_path);
      _dbus_clear_hash_table(&self->n_containers_by_user);
      _dbus_string_free(&self->address_template);
      dbus_free(self);
      if (contained_data_slot != -1) dbus_connection_free_data_slot(&contained_data_slot);
      if (container_creator_data_slot != -1) dbus_connection_free_data_slot(&container_creator_data_slot);
  }
}
static BusContainerInstance *bus_container_instance_ref(BusContainerInstance *self) {
  _dbus_assert(self->refcount > 0);
  _dbus_assert(self->refcount < _DBUS_INT_MAX);
  self->refcount++;
  return self;
}
static dbus_bool_t bus_container_instance_emit_removed(BusContainerInstance *self) {
  BusTransaction *transaction = NULL;
  DBusMessage *message = NULL;
  DBusError error = DBUS_ERROR_INIT;
  transaction = bus_transaction_new(self->context);
  if (transaction == NULL) goto oom;
  message = dbus_message_new_signal(DBUS_PATH_DBUS, DBUS_INTERFACE_CONTAINERS1,"InstanceRemoved");
  if (message == NULL || !dbus_message_set_sender(message, DBUS_SERVICE_DBUS) || !dbus_message_append_args(message, DBUS_TYPE_OBJECT_PATH, &self->path,
      DBUS_TYPE_INVALID) || !bus_transaction_capture(transaction, NULL, NULL, message))
      goto oom;
  if (!bus_dispatch_matches(transaction, NULL, NULL, message, &error)) {
      if (dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY)) goto oom;
      _dbus_warn("Failed to send InstanceRemoved for a reason other than OOM: %s: %s", error.name, error.message);
      dbus_error_free(&error);
  }
  bus_transaction_execute_and_free(transaction);
  dbus_message_unref(message);
  _DBUS_ASSERT_ERROR_IS_CLEAR(&error);
  return TRUE;
oom:
  dbus_error_free(&error);
  dbus_clear_message(&message);
  if (transaction != NULL) bus_transaction_cancel_and_free(transaction);
  return FALSE;
}
static void bus_container_instance_unref(BusContainerInstance *self) {
  _dbus_assert(self->refcount > 0);
  if (--self->refcount == 0) {
      BusContainerCreatorData *creator_data;
      for (; self->announced; _dbus_wait_for_memory ()) {
          if (bus_container_instance_emit_removed(self)) self->announced = FALSE;
      }
      _dbus_assert(self->server == NULL);
      _dbus_assert(self->connections == NULL);
      creator_data = dbus_connection_get_data(self->creator, container_creator_data_slot);
      _dbus_assert(creator_data != NULL);
      _dbus_list_remove(&creator_data->instances, self);
      if (self->path != NULL && self->containers->instances_by_path != NULL && _dbus_hash_table_remove_string(self->containers->instances_by_path, self->path)) {
          DBusHashIter entry;
          uintptr_t n = 0;
          if (!_dbus_hash_iter_lookup(self->containers->n_containers_by_user, (void*)(uintptr_t)self->uid, FALSE, &entry))
              _dbus_assert_not_reached("Container should not be placed in instances_by_path until its n_containers_by_user entry has been allocated");
          n = (uintptr_t)_dbus_hash_iter_get_value(&entry);
          _dbus_assert(n > 0);
          n -= 1;
          _dbus_hash_iter_set_value(&entry, (void*)n);
      }
      _dbus_clear_variant(&self->metadata);
      bus_context_unref(self->context);
      bus_containers_unref(self->containers);
      dbus_connection_unref(self->creator);
      dbus_free(self->path);
      dbus_free(self->type);
      dbus_free(self->name);
      dbus_free(self);
  }
}
static inline void bus_clear_container_instance (BusContainerInstance **instance_p) {
  _dbus_clear_pointer_impl(BusContainerInstance, instance_p, bus_container_instance_unref);
}
static void bus_container_instance_stop_listening(BusContainerInstance *self) {
  bus_container_instance_ref(self);
  if (self->server != NULL) {
      dbus_server_set_new_connection_function(self->server, NULL, NULL, NULL);
      dbus_server_disconnect(self->server);
      dbus_clear_server(&self->server);
  }
  bus_container_instance_unref(self);
}
static BusContainerInstance *bus_container_instance_new(BusContext *context, BusContainers *containers, DBusConnection *creator, DBusError *error) {
  BusContainerInstance *self = NULL;
  DBusString path = _DBUS_STRING_INIT_INVALID;
  _dbus_assert(context != NULL);
  _dbus_assert(containers != NULL);
  _dbus_assert(creator != NULL);
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (!_dbus_string_init(&path)) {
      BUS_SET_OOM(error);
      goto fail;
  }
  self = dbus_new0(BusContainerInstance, 1);
  if (self == NULL) {
      BUS_SET_OOM(error);
      goto fail;
  }
  self->refcount = 1;
  self->type = NULL;
  self->name = NULL;
  self->metadata = NULL;
  self->context = bus_context_ref(context);
  self->containers = bus_containers_ref(containers);
  self->server = NULL;
  self->creator = dbus_connection_ref(creator);
  /*if (containers->next_container_id >= DBUS_UINT64_CONSTANT(0xFFFFFFFFFFFFFFFF)) {
      dbus_set_error(error, DBUS_ERROR_LIMITS_EXCEEDED,"Too many containers created during the lifetime of this bus");
      goto fail;
  }*/
  if (!_dbus_string_append_printf(&path,"/org/freedesktop/DBus/Containers1/c%" PRIu64, containers->next_container_id++)) {
      BUS_SET_OOM(error);
      goto fail;
  }
  if (!_dbus_string_steal_data(&path, &self->path)) goto fail;
  return self;
fail:
  _dbus_string_free(&path);
  if (self != NULL) bus_container_instance_unref(self);
  return NULL;
}
static void bus_container_creator_data_free(BusContainerCreatorData *self) {
  _dbus_assert(self->instances == NULL);
  dbus_free(self);
}
static const char * const auth_mechanisms[] = { "EXTERNAL", NULL };
_DBUS_STATIC_ASSERT(sizeof(uid_t) <= sizeof(uintptr_t));
_DBUS_STATIC_ASSERT(sizeof(void*) == sizeof(uintptr_t));
static dbus_bool_t allow_same_uid_only(DBusConnection *connection, unsigned long uid, void *data) {
  return (uid == (uintptr_t)data);
}
static void bus_container_instance_lost_connection(BusContainerInstance *instance, DBusConnection *connection) {
  bus_container_instance_ref(instance);
  dbus_connection_ref(connection);
  if (_dbus_list_remove(&instance->connections, connection)) dbus_connection_unref(connection);
  dbus_connection_set_data(connection, contained_data_slot, NULL, NULL);
  dbus_connection_unref(connection);
  bus_container_instance_unref(instance);
}
static void new_connection_cb(DBusServer *server, DBusConnection *new_connection, void *data) {
  BusContainerInstance *instance = data;
  int limit = bus_context_get_max_connections_per_container(instance->context);
  if (_dbus_list_get_length(&instance->connections) >= limit) {
      bus_context_log(instance->context, DBUS_SYSTEM_LOG_WARNING,"Closing connection to container server %s (%s \"%s\") because it would exceed "
                      "resource limit (max_connections_per_container=%d)", instance->path, instance->type, instance->name, limit);
      return;
  }
  if (!dbus_connection_set_data(new_connection, contained_data_slot, bus_container_instance_ref(instance), (DBusFreeFunction)bus_container_instance_unref)) {
      bus_container_instance_unref(instance);
      bus_container_instance_lost_connection(instance, new_connection);
      return;
  }
  if (_dbus_list_append(&instance->connections, new_connection)) dbus_connection_ref(new_connection);
  else {
      bus_container_instance_lost_connection(instance, new_connection);
      return;
  }
  if (!bus_context_add_incoming_connection(instance->context, new_connection)) {
      bus_container_instance_lost_connection(instance, new_connection);
      return;
  }
  dbus_connection_set_unix_user_function(new_connection, allow_same_uid_only, (void*)(uintptr_t)instance->uid,NULL);
}
static const char *bus_containers_ensure_address_template(BusContainers *self, DBusError *error) {
  DBusString dir = _DBUS_STRING_INIT_INVALID;
  const char *ret = NULL;
  const char *runtime_dir;
  if (_dbus_string_get_length(&self->address_template) > 0) return _dbus_string_get_const_data(&self->address_template);
  runtime_dir = _dbus_getenv("XDG_RUNTIME_DIR");
  if (runtime_dir != NULL) {
      if (!_dbus_string_init(&dir)) {
          BUS_SET_OOM(error);
          goto out;
      }
      if (!_dbus_string_append(&dir, runtime_dir) || !_dbus_string_append(&dir, "/dbus-1")) {
          BUS_SET_OOM(error);
          goto out;
      }
      if (!_dbus_ensure_directory(&dir, error)) goto out;
      if (!_dbus_string_append(&dir, "/containers")) {
          BUS_SET_OOM(error);
          goto out;
      }
      if (!_dbus_ensure_directory(&dir, error)) goto out;
  } else {
      const char *tmpdir;
      tmpdir = _dbus_get_tmpdir();
      _dbus_string_init_const(&dir, tmpdir);
  }
  if (!_dbus_string_append(&self->address_template, "unix:dir=") || !_dbus_address_append_escaped(&self->address_template, &dir)) {
      _dbus_string_set_length(&self->address_template, 0);
      BUS_SET_OOM(error);
      goto out;
  }
  ret = _dbus_string_get_const_data(&self->address_template);
out:
  _dbus_string_free(&dir);
  return ret;
}
static dbus_bool_t bus_container_instance_listen(BusContainerInstance *self, DBusError *error) {
  BusContainers *containers = bus_context_get_containers(self->context);
  const char *address;
  address = bus_containers_ensure_address_template(containers, error);
  if (address == NULL) return FALSE;
  self->server = dbus_server_listen(address, error);
  if (self->server == NULL) return FALSE;
  if (!bus_context_setup_server(self->context, self->server, error)) return FALSE;
  if (!dbus_server_set_auth_mechanisms(self->server, (const char**)auth_mechanisms)) {
      BUS_SET_OOM(error);
      return FALSE;
  }
  dbus_server_set_new_connection_function(self->server, new_connection_cb, bus_container_instance_ref(self), (DBusFreeFunction)bus_container_instance_unref);
  return TRUE;
}
dbus_bool_t bus_containers_handle_add_server(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error) {
  BusContainerCreatorData *creator_data;
  DBusMessageIter iter;
  DBusMessageIter dict_iter;
  DBusMessageIter writer;
  DBusMessageIter array_writer;
  const char *type;
  const char *name;
  const char *path;
  BusContainerInstance *instance = NULL;
  BusContext *context;
  BusContainers *containers;
  char *address = NULL;
  DBusAddressEntry **entries = NULL;
  int n_entries;
  DBusMessage *reply = NULL;
  int metadata_size;
  int limit;
  DBusHashIter n_containers_by_user_entry;
  uintptr_t this_user_containers = 0;
  context = bus_transaction_get_context(transaction);
  containers = bus_context_get_containers(context);
  creator_data = dbus_connection_get_data(connection, container_creator_data_slot);
  if (creator_data == NULL) {
      creator_data = dbus_new0(BusContainerCreatorData, 1);
      if (creator_data == NULL) goto oom;
      creator_data->instances = NULL;
      if (!dbus_connection_set_data(connection, container_creator_data_slot, creator_data, (DBusFreeFunction)bus_container_creator_data_free)) {
          bus_container_creator_data_free(creator_data);
          goto oom;
      }
  }
  instance = bus_container_instance_new(context, containers, connection, error);
  if (instance == NULL) goto fail;
  if (!dbus_connection_get_unix_user(connection, &instance->uid)) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Unable to determine user ID of caller");
      goto fail;
  }
  _dbus_assert(dbus_message_has_signature (message, "ssa{sv}a{sv}"));
  if (!dbus_message_iter_init(message, &iter)) _dbus_assert_not_reached("Message type was already checked");
  _dbus_assert(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic(&iter, &type);
  instance->type = _dbus_strdup(type);
  if (instance->type == NULL) goto oom;
  if (!dbus_validate_interface(type, NULL)) {
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"The container type identifier must have the syntax of an interface name");
      goto fail;
  }
  if (!dbus_message_iter_next(&iter)) _dbus_assert_not_reached("Message type was already checked");
  _dbus_assert(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic(&iter, &name);
  instance->name = _dbus_strdup(name);
  if (instance->name == NULL) goto oom;
  if (!dbus_message_iter_next(&iter)) _dbus_assert_not_reached("Message type was already checked");
  _dbus_assert(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY);
  instance->metadata = _dbus_variant_read(&iter);
  _dbus_assert(strcmp(_dbus_variant_get_signature(instance->metadata), "a{sv}") == 0);
  metadata_size = _dbus_variant_get_length(instance->metadata) + (int)strlen(type) + (int)strlen(name);
  limit = bus_context_get_max_container_metadata_bytes(context);
  if (metadata_size > limit) {
      DBusError local_error = DBUS_ERROR_INIT;
      dbus_set_error(&local_error, DBUS_ERROR_LIMITS_EXCEEDED,"Connection \"%s\" (%s) is not allowed to set %d bytes of container metadata (max_container_"
                     "metadata_bytes=%d)", bus_connection_get_name(connection), bus_connection_get_loginfo(connection), metadata_size, limit);
      bus_context_log_literal(context, DBUS_SYSTEM_LOG_WARNING, local_error.message);
      dbus_move_error(&local_error, error);
      goto fail;
  }
  if (!dbus_message_iter_next(&iter)) _dbus_assert_not_reached("Message type was already checked");
  _dbus_assert(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse(&iter, &dict_iter);
  while (dbus_message_iter_get_arg_type(&dict_iter) != DBUS_TYPE_INVALID) {
      DBusMessageIter pair_iter;
      const char *param_name;
      _dbus_assert(dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY);
      dbus_message_iter_recurse(&dict_iter, &pair_iter);
      _dbus_assert(dbus_message_iter_get_arg_type(&pair_iter) == DBUS_TYPE_STRING);
      dbus_message_iter_get_basic(&pair_iter, &param_name);
      dbus_set_error(error, DBUS_ERROR_INVALID_ARGS,"Named parameter %s is not understood", param_name);
      goto fail;
  }
  _dbus_assert(!dbus_message_iter_has_next (&iter));
  if (containers->instances_by_path == NULL){
      containers->instances_by_path = _dbus_hash_table_new(DBUS_HASH_STRING,NULL, NULL);
      if (containers->instances_by_path == NULL) goto oom;
  }
  if (containers->n_containers_by_user == NULL) {
      containers->n_containers_by_user = _dbus_hash_table_new(DBUS_HASH_UINTPTR,NULL, NULL);
      if (containers->n_containers_by_user == NULL) goto oom;
  }
  limit = bus_context_get_max_containers(context);
  if (_dbus_hash_table_get_n_entries(containers->instances_by_path) >= limit) {
      DBusError local_error = DBUS_ERROR_INIT;
      dbus_set_error(&local_error, DBUS_ERROR_LIMITS_EXCEEDED,"Connection \"%s\" (%s) is not allowed to create more container servers (max_containers=%d)",
                     bus_connection_get_name(connection), bus_connection_get_loginfo(connection), limit);
      bus_context_log_literal(context, DBUS_SYSTEM_LOG_WARNING, local_error.message);
      dbus_move_error(&local_error, error);
      goto fail;
  }
  if (!_dbus_hash_iter_lookup(containers->n_containers_by_user, (void*)(uintptr_t)instance->uid, TRUE, &n_containers_by_user_entry)) goto oom;
  this_user_containers = (uintptr_t)_dbus_hash_iter_get_value(&n_containers_by_user_entry);
  limit = bus_context_get_max_containers_per_user(context);
  if (limit <= 0 || this_user_containers >= (unsigned)limit) {
      DBusError local_error = DBUS_ERROR_INIT;
      dbus_set_error(&local_error, DBUS_ERROR_LIMITS_EXCEEDED,"Connection \"%s\" (%s) is not allowed to create more container servers for uid %lu "
                     "(max_containers_per_user=%d)", bus_connection_get_name(connection), bus_connection_get_loginfo(connection), instance->uid, limit);
      bus_context_log_literal(context, DBUS_SYSTEM_LOG_WARNING, local_error.message);
      dbus_move_error(&local_error, error);
      goto fail;
  }
  if (!_dbus_hash_table_insert_string(containers->instances_by_path, instance->path, instance)) goto oom;
  this_user_containers += 1;
  _dbus_hash_iter_set_value(&n_containers_by_user_entry, (void*)this_user_containers);
  if (!_dbus_list_append(&creator_data->instances, instance)) goto oom;
  if (!bus_container_instance_listen(instance, error)) goto fail;
  address = dbus_server_get_address(instance->server);
  if (!dbus_parse_address(address, &entries, &n_entries, error)) _dbus_assert_not_reached("listening on unix:dir= should yield a valid address");
  _dbus_assert(n_entries == 1);
  _dbus_assert(strcmp(dbus_address_entry_get_method(entries[0]), "unix") == 0);
  path = dbus_address_entry_get_value(entries[0], "path");
  _dbus_assert(path != NULL);
  reply = dbus_message_new_method_return(message);
  if (!dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &instance->path, DBUS_TYPE_INVALID)) goto oom;
  dbus_message_iter_init_append(reply, &writer);
  if (!dbus_message_iter_open_container(&writer, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array_writer)) goto oom;
  if (!dbus_message_iter_append_fixed_array(&array_writer, DBUS_TYPE_BYTE, &path, strlen(path) + 1)) {
      dbus_message_iter_abandon_container(&writer, &array_writer);
      goto oom;
  }
  if (!dbus_message_iter_close_container(&writer, &array_writer)) goto oom;
  if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &address, DBUS_TYPE_INVALID)) goto oom;
  _dbus_assert(dbus_message_has_signature(reply, "oays"));
  if (! bus_transaction_send_from_driver(transaction, connection, reply)) goto oom;
  instance->announced = TRUE;
  dbus_message_unref(reply);
  bus_container_instance_unref(instance);
  dbus_address_entries_free(entries);
  dbus_free(address);
  return TRUE;
oom:
  BUS_SET_OOM(error);
fail:
  if (instance != NULL) bus_container_instance_stop_listening(instance);
  dbus_clear_message(&reply);
  dbus_clear_address_entries(&entries);
  bus_clear_container_instance(&instance);
  dbus_free(address);
  return FALSE;
}
dbus_bool_t bus_containers_supported_arguments_getter(BusContext *context, DBusMessageIter *var_iter) {
  DBusMessageIter arr_iter;
  return dbus_message_iter_open_container(var_iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &arr_iter) && dbus_message_iter_close_container(var_iter, &arr_iter);
}
dbus_bool_t bus_containers_handle_stop_instance(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error) {
  BusContext *context;
  BusContainers *containers;
  BusContainerInstance *instance = NULL;
  DBusList *iter;
  const char *path;
  unsigned long uid;
  if (!dbus_message_get_args(message, error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) goto failed;
  context = bus_transaction_get_context(transaction);
  containers = bus_context_get_containers(context);
  if (containers->instances_by_path != NULL) instance = _dbus_hash_table_lookup_string(containers->instances_by_path, path);
  if (instance == NULL) {
      dbus_set_error(error, DBUS_ERROR_NOT_CONTAINER,"There is no container with path '%s'", path);
      goto failed;
  }
  if (!dbus_connection_get_unix_user(connection, &uid)) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Unable to determine user ID of caller");
      goto failed;
  }
  if (uid != instance->uid) {
      dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED,"User %lu cannot stop a container server started by user %lu", uid, instance->uid);
      goto failed;
  }
  bus_container_instance_ref(instance);
  bus_container_instance_stop_listening(instance);
  for (iter = _dbus_list_get_first_link(&instance->connections); iter != NULL; iter = _dbus_list_get_next_link(&instance->connections, iter))
      dbus_connection_close(iter->data);
  bus_container_instance_unref(instance);
  if (!bus_driver_send_ack_reply(connection, transaction, message, error)) goto failed;
  return TRUE;
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  return FALSE;
}
dbus_bool_t bus_containers_handle_stop_listening(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error) {
  BusContext *context;
  BusContainers *containers;
  BusContainerInstance *instance = NULL;
  const char *path;
  unsigned long uid;
  if (!dbus_message_get_args(message, error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) goto failed;
  context = bus_transaction_get_context(transaction);
  containers = bus_context_get_containers(context);
  if (containers->instances_by_path != NULL) instance = _dbus_hash_table_lookup_string(containers->instances_by_path, path);
  if (instance == NULL) {
      dbus_set_error(error, DBUS_ERROR_NOT_CONTAINER,"There is no container with path '%s'", path);
      goto failed;
  }
  if (!dbus_connection_get_unix_user(connection, &uid)) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Unable to determine user ID of caller");
      goto failed;
  }
  if (uid != instance->uid) {
      dbus_set_error(error, DBUS_ERROR_ACCESS_DENIED,"User %lu cannot stop a container server started by user %lu", uid, instance->uid);
      goto failed;
  }
  bus_container_instance_ref(instance);
  bus_container_instance_stop_listening(instance);
  bus_container_instance_unref(instance);
  if (!bus_driver_send_ack_reply(connection, transaction, message, error)) goto failed;
  return TRUE;
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  return FALSE;
}
dbus_bool_t bus_containers_handle_get_connection_instance(DBusConnection *caller, BusTransaction *transaction, DBusMessage *message, DBusError *error) {
  BusContainerInstance *instance;
  BusDriverFound found;
  DBusConnection *subject;
  DBusMessage *reply = NULL;
  DBusMessageIter writer;
  DBusMessageIter arr_writer;
  const char *bus_name;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  found = bus_driver_get_conn_helper(caller, message, "container instance", &bus_name, &subject, error);
  switch(found) {
      case BUS_DRIVER_FOUND_SELF:
          dbus_set_error(error, DBUS_ERROR_NOT_CONTAINER,"The message bus is not in a container");
          goto failed;
      case BUS_DRIVER_FOUND_PEER: break;
      default: goto failed;
  }
  instance = dbus_connection_get_data(subject, contained_data_slot);
  if (instance == NULL) {
      dbus_set_error(error, DBUS_ERROR_NOT_CONTAINER,"Connection '%s' is not in a container", bus_name);
      goto failed;
  }
  reply = dbus_message_new_method_return(message);
  if (reply == NULL) goto oom;
  if (!dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &instance->path, DBUS_TYPE_INVALID)) goto oom;
  dbus_message_iter_init_append(reply, &writer);
  if (!dbus_message_iter_open_container(&writer, DBUS_TYPE_ARRAY, "{sv}", &arr_writer)) goto oom;
  if (!bus_driver_fill_connection_credentials(instance->creator, &arr_writer)) {
      dbus_message_iter_abandon_container(&writer, &arr_writer);
      goto oom;
  }
  if (!dbus_message_iter_close_container(&writer, &arr_writer)) goto oom;
  if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &instance->type, DBUS_TYPE_STRING, &instance->name, DBUS_TYPE_INVALID)) goto oom;
  dbus_message_iter_init_append(reply, &writer);
  if (!_dbus_variant_write(instance->metadata, &writer)) goto oom;
  if (!bus_transaction_send_from_driver(transaction, caller, reply)) goto oom;
  dbus_message_unref(reply);
  return TRUE;
oom:
  BUS_SET_OOM(error);
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  dbus_clear_message(&reply);
  return FALSE;
}
dbus_bool_t bus_containers_handle_get_instance_info(DBusConnection *connection, BusTransaction *transaction, DBusMessage *message, DBusError *error) {
  BusContext *context;
  BusContainers *containers;
  BusContainerInstance *instance = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter writer;
  DBusMessageIter arr_writer;
  const char *path;
  if (!dbus_message_get_args(message, error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) goto failed;
  context = bus_transaction_get_context(transaction);
  containers = bus_context_get_containers(context);
  if (containers->instances_by_path != NULL) instance = _dbus_hash_table_lookup_string(containers->instances_by_path, path);
  if (instance == NULL) {
      dbus_set_error(error, DBUS_ERROR_NOT_CONTAINER,"There is no container with path '%s'", path);
      goto failed;
  }
  reply = dbus_message_new_method_return(message);
  if (reply == NULL) goto oom;
  dbus_message_iter_init_append(reply, &writer);
  if (!dbus_message_iter_open_container(&writer, DBUS_TYPE_ARRAY, "{sv}", &arr_writer)) goto oom;
  if (!bus_driver_fill_connection_credentials(instance->creator, &arr_writer)) {
      dbus_message_iter_abandon_container(&writer, &arr_writer);
      goto oom;
  }
  if (!dbus_message_iter_close_container(&writer, &arr_writer)) goto oom;
  if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &instance->type, DBUS_TYPE_STRING, &instance->name, DBUS_TYPE_INVALID)) goto oom;
  dbus_message_iter_init_append(reply, &writer);
  if (!_dbus_variant_write(instance->metadata, &writer)) goto oom;
  if (!bus_transaction_send_from_driver(transaction, connection, reply)) goto oom;
  dbus_message_unref(reply);
  return TRUE;
oom:
  BUS_SET_OOM(error);
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  dbus_clear_message(&reply);
  return FALSE;
}
void bus_containers_stop_listening(BusContainers *self) {
  if (self->instances_by_path != NULL) {
      DBusHashIter iter;
      _dbus_hash_iter_init(self->instances_by_path, &iter);
      while(_dbus_hash_iter_next(&iter)) {
          BusContainerInstance *instance = _dbus_hash_iter_get_value(&iter);
          bus_container_instance_stop_listening(instance);
      }
  }
}
#else
BusContainers *bus_containers_new(void) {
  return (BusContainers*)1;
}
BusContainers *bus_containers_ref(BusContainers *self) {
  _dbus_assert(self == (BusContainers*)1);
  return self;
}
void bus_containers_unref(BusContainers *self) {
  _dbus_assert(self == (BusContainers*)1);
}
void bus_containers_stop_listening(BusContainers *self) {
  _dbus_assert(self == (BusContainers*)1);
}
#endif
void bus_containers_remove_connection(BusContainers *self, DBusConnection *connection) {
#ifdef DBUS_ENABLE_CONTAINERS
  BusContainerCreatorData *creator_data;
  BusContainerInstance *instance;
  dbus_connection_ref(connection);
  creator_data = dbus_connection_get_data(connection, container_creator_data_slot);
  if (creator_data != NULL) {
      DBusList *iter;
      DBusList *next;
      for (iter = _dbus_list_get_first_link(&creator_data->instances); iter != NULL; iter = next) {
          instance = iter->data;
          next = _dbus_list_get_next_link(&creator_data->instances, iter);
          _dbus_assert(instance->creator == connection);
          bus_container_instance_stop_listening(instance);
      }
  }
  instance = dbus_connection_get_data(connection, contained_data_slot);
  if (instance != NULL) {
      bus_container_instance_ref(instance);
      if (_dbus_list_remove(&instance->connections, connection)) dbus_connection_unref(connection);
      bus_container_instance_unref(instance);
  }
  dbus_connection_unref(connection);
#endif
}
dbus_bool_t bus_containers_connection_is_contained(DBusConnection *connection, const char **path, const char **type, const char **name) {
#ifdef DBUS_ENABLE_CONTAINERS
  BusContainerInstance *instance;
  instance = dbus_connection_get_data(connection, contained_data_slot);
  if (instance != NULL) {
      if (path != NULL) *path = instance->path;
      if (type != NULL) *type = instance->type;
      if (name != NULL) *name = instance->name;
      return TRUE;
  }
#endif
  return FALSE;
}