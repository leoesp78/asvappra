#ifndef DBUS_MESSAGE_INTERNAL_H
#define DBUS_MESSAGE_INTERNAL_H

#include "dbus-marshal-validate.h"
#include "dbus-message.h"
#include "dbus-resources.h"
#include "dbus-list.h"

DBUS_BEGIN_DECLS
#ifdef DBUS_ENABLE_VERBOSE_MODE
void _dbus_message_trace_ref (DBusMessage *message, int old_refcount, int new_refcount, const char *why);
#else
#define _dbus_message_trace_ref(m, o, n, w) \
  do {\
      (void)(o); \
      (void)(n); \
  } while(0);
#endif
typedef struct DBusMessageLoader DBusMessageLoader;
void _dbus_message_get_network_data(DBusMessage *message, const DBusString **header, const DBusString **body);
DBUS_PRIVATE_EXPORT void _dbus_message_get_unix_fds(DBusMessage *message, const int **fds, unsigned *n_fds);
unsigned int _dbus_message_get_n_unix_fds(DBusMessage *message);
void _dbus_message_lock(DBusMessage *message);
void _dbus_message_unlock(DBusMessage *message);
dbus_bool_t _dbus_message_add_counter(DBusMessage *message, DBusCounter *counter);
void _dbus_message_add_counter_link(DBusMessage *message, DBusList *link);
void _dbus_message_remove_counter(DBusMessage *message, DBusCounter *counter);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_message_remove_unknown_fields(DBusMessage *message);
DBUS_PRIVATE_EXPORT DBusMessageLoader* _dbus_message_loader_new(void);
DBUS_PRIVATE_EXPORT DBusMessageLoader* _dbus_message_loader_ref(DBusMessageLoader *loader);
DBUS_PRIVATE_EXPORT void _dbus_message_loader_unref(DBusMessageLoader *loader);
DBUS_PRIVATE_EXPORT void _dbus_message_loader_get_buffer(DBusMessageLoader *loader, DBusString **buffer, int *max_to_read, dbus_bool_t *may_read_unix_fds);
DBUS_PRIVATE_EXPORT void _dbus_message_loader_return_buffer(DBusMessageLoader *loader, DBusString *buffer);
#ifdef HAVE_UNIX_FD_PASSING
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_message_loader_get_unix_fds(DBusMessageLoader *loader, int **fds, unsigned *max_n_fds);
DBUS_PRIVATE_EXPORT void _dbus_message_loader_return_unix_fds(DBusMessageLoader *loader, int *fds, unsigned n_fds);
#endif
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_message_loader_queue_messages(DBusMessageLoader *loader);
DBusMessage* _dbus_message_loader_peek_message(DBusMessageLoader *loader);
DBUS_PRIVATE_EXPORT DBusMessage* _dbus_message_loader_pop_message(DBusMessageLoader *loader);
DBusList* _dbus_message_loader_pop_message_link(DBusMessageLoader *loader);
void _dbus_message_loader_putback_message_link(DBusMessageLoader *loader, DBusList *link);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_message_loader_get_is_corrupted(DBusMessageLoader *loader);
DBusValidity _dbus_message_loader_get_corruption_reason(DBusMessageLoader *loader);
void  _dbus_message_loader_set_max_message_size(DBusMessageLoader *loader, long size);
DBUS_PRIVATE_EXPORT long _dbus_message_loader_get_max_message_size(DBusMessageLoader *loader);
void _dbus_message_loader_set_max_message_unix_fds(DBusMessageLoader *loader, long n);
long _dbus_message_loader_get_max_message_unix_fds(DBusMessageLoader *loader);
int _dbus_message_loader_get_pending_fds_count (DBusMessageLoader *loader);
void _dbus_message_loader_set_pending_fds_function(DBusMessageLoader *loader, void (*callback)(void*), void *data);
typedef struct DBusVariant DBusVariant;
DBUS_PRIVATE_EXPORT DBusVariant *_dbus_variant_read(DBusMessageIter *reader);
DBUS_PRIVATE_EXPORT dbus_bool_t _dbus_variant_write(DBusVariant *self, DBusMessageIter *writer);
DBUS_PRIVATE_EXPORT void _dbus_variant_free(DBusVariant *self);
DBUS_PRIVATE_EXPORT int _dbus_variant_get_length(DBusVariant *self);
DBUS_PRIVATE_EXPORT const DBusString *_dbus_variant_peek(DBusVariant *self);
DBUS_PRIVATE_EXPORT const char *_dbus_variant_get_signature(DBusVariant *self);
static inline void _dbus_clear_variant(DBusVariant **variant_p) {
  _dbus_clear_pointer_impl(DBusVariant, variant_p, _dbus_variant_free);
}
typedef struct DBusInitialFDs DBusInitialFDs;
DBusInitialFDs *_dbus_check_fdleaks_enter(void);
void _dbus_check_fdleaks_leave(DBusInitialFDs *fds);
DBUS_END_DECLS

#endif