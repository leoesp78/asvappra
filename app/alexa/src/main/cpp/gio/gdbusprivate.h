#if defined (GIO_COMPILATION)
#error "gdbusprivate.h is a private header file."
#endif

#ifndef __G_DBUS_PRIVATE_H__
#define __G_DBUS_PRIVATE_H__

#include "../glib/glib.h"
#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gioenums.h"

G_BEGIN_DECLS
typedef struct GDBusWorker GDBusWorker;
typedef void (*GDBusWorkerMessageReceivedCallback)(GDBusWorker *worker, GDBusMessage *message, gpointer user_data);
typedef GDBusMessage *(*GDBusWorkerMessageAboutToBeSentCallback)(GDBusWorker *worker, GDBusMessage *message, gpointer user_data);
typedef void (*GDBusWorkerDisconnectedCallback)(GDBusWorker *worker, gboolean remote_peer_vanished, GError *error, gpointer user_data);
GDBusWorker *_g_dbus_worker_new(GIOStream *stream, GDBusCapabilityFlags capabilities, gboolean initially_frozen, GDBusWorkerMessageReceivedCallback message_received_callback,
                                GDBusWorkerMessageAboutToBeSentCallback message_about_to_be_sent_callback, GDBusWorkerDisconnectedCallback disconnected_callback,
                                gpointer user_data);
void _g_dbus_worker_send_message(GDBusWorker *worker, GDBusMessage *message, gchar *blob, gsize blob_len);
void _g_dbus_worker_stop(GDBusWorker *worker);
void _g_dbus_worker_unfreeze(GDBusWorker *worker);
gboolean _g_dbus_worker_flush_sync(GDBusWorker *worker, GCancellable *cancellable, GError **error);
void _g_dbus_initialize(void);
gboolean _g_dbus_debug_authentication(void);
gboolean _g_dbus_debug_transport(void);
gboolean _g_dbus_debug_message(void);
gboolean _g_dbus_debug_payload(void);
gboolean _g_dbus_debug_call(void);
gboolean _g_dbus_debug_signal(void);
gboolean _g_dbus_debug_incoming(void);
gboolean _g_dbus_debug_return(void);
gboolean _g_dbus_debug_emission(void);
gboolean _g_dbus_debug_address(void);
void _g_dbus_debug_print_lock(void);
void _g_dbus_debug_print_unlock(void);
gboolean _g_dbus_address_parse_entry(const gchar *address_entry, gchar **out_transport_name, GHashTable **out_key_value_pairs, GError **error);
GVariantType *_g_dbus_compute_complete_signature(GDBusArgInfo **args);
gchar *_g_dbus_hexdump(const gchar *data, gsize len, guint indent);
#ifdef G_OS_WIN32
gchar *_g_dbus_win32_get_user_sid(void);
#endif
gchar *_g_dbus_get_machine_id(GError **error);
gchar *_g_dbus_enum_to_string(GType enum_type, gint value);
G_END_DECLS
GDBusMethodInvocation *_g_dbus_method_invocation_new(const gchar *sender, const gchar *object_path, const gchar *interface_name, const gchar *method_name,
                                                     const GDBusMethodInfo *method_info, GDBusConnection *connection, GDBusMessage *message, GVariant *parameters,
                                                     gpointer user_data);
#endif