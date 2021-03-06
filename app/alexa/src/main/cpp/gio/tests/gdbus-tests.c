#include <unistd.h>
#include "../gio.h"
#include "gdbus-tests.h"

typedef struct {
  GMainLoop *loop;
  gboolean timed_out;
} PropertyNotifyData;
static void on_property_notify(GObject *object, GParamSpec *pspec, gpointer user_data) {
  PropertyNotifyData *data = user_data;
  g_main_loop_quit(data->loop);
}
static gboolean on_property_notify_timeout(gpointer user_data) {
  PropertyNotifyData *data = user_data;
  data->timed_out = TRUE;
  g_main_loop_quit(data->loop);
  return TRUE;
}
gboolean _g_assert_property_notify_run(gpointer object, const gchar *property_name) {
  gchar *s;
  gulong handler_id;
  guint timeout_id;
  PropertyNotifyData data;
  data.loop = g_main_loop_new(NULL, FALSE);
  data.timed_out = FALSE;
  s = g_strdup_printf("notify::%s", property_name);
  handler_id = g_signal_connect(object, s,G_CALLBACK(on_property_notify),&data);
  g_free(s);
  timeout_id = g_timeout_add(30 * 1000, on_property_notify_timeout, &data);
  g_main_loop_run(data.loop);
  g_signal_handler_disconnect(object, handler_id);
  g_source_remove(timeout_id);
  g_main_loop_unref(data.loop);
  return data.timed_out;
}
typedef struct {
  GMainLoop *loop;
  gboolean timed_out;
} SignalReceivedData;
static void on_signal_received(gpointer user_data) {
  SignalReceivedData *data = user_data;
  g_main_loop_quit(data->loop);
}
static gboolean on_signal_received_timeout(gpointer user_data) {
  SignalReceivedData *data = user_data;
  data->timed_out = TRUE;
  g_main_loop_quit(data->loop);
  return TRUE;
}
gboolean _g_assert_signal_received_run(gpointer object, const gchar *signal_name) {
  gulong handler_id;
  guint timeout_id;
  SignalReceivedData data;
  data.loop = g_main_loop_new(NULL, FALSE);
  data.timed_out = FALSE;
  handler_id = g_signal_connect_swapped(object, signal_name,G_CALLBACK(on_signal_received),&data);
  timeout_id = g_timeout_add(30 * 1000, on_signal_received_timeout, &data);
  g_main_loop_run(data.loop);
  g_signal_handler_disconnect(object, handler_id);
  g_source_remove(timeout_id);
  g_main_loop_unref(data.loop);
  return data.timed_out;
}
GDBusConnection *_g_bus_get_priv(GBusType bus_type, GCancellable *cancellable, GError **error) {
  gchar *address;
  GDBusConnection *ret;
  ret = NULL;
  address = g_dbus_address_get_for_bus_sync (bus_type, cancellable, error);
  if (address == NULL) goto out;
  ret = g_dbus_connection_new_for_address_sync(address, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION, NULL,
                                               cancellable, error);
  g_free(address);
out:
  return ret;
}
#if 1
gboolean _g_object_wait_for_single_ref_do(gpointer object) {
  guint num_ms_elapsed;
  gboolean timed_out;
  timed_out = FALSE;
  num_ms_elapsed = 0;
  while(TRUE) {
      if (G_OBJECT(object)->ref_count == 1) goto out;
      if (num_ms_elapsed > 30000) {
          timed_out = TRUE;
          goto out;
      }
      usleep (10 * 1000);
      num_ms_elapsed += 10;
  }
out:
  return timed_out;
}
#else
typedef struct {
  GMainLoop *loop;
  gboolean timed_out;
} WaitSingleRefData;
static gboolean on_wait_single_ref_timeout(gpointer user_data) {
  WaitSingleRefData *data = user_data;
  data->timed_out = TRUE;
  g_main_loop_quit(data->loop);
  return TRUE;
}
static void on_wait_for_single_ref_toggled(gpointer user_data, GObject *object, gboolean is_last_ref) {
  WaitSingleRefData *data = user_data;
  g_main_loop_quit(data->loop);
}
gboolean _g_object_wait_for_single_ref_do(gpointer object) {
  WaitSingleRefData data;
  guint timeout_id;
  data.timed_out = FALSE;
  if (G_OBJECT (object)->ref_count == 1) goto out;
  data.loop = g_main_loop_new(NULL, FALSE);
  timeout_id = g_timeout_add(30 * 1000, on_wait_single_ref_timeout, &data);
  g_object_add_toggle_ref(G_OBJECT(object), on_wait_for_single_ref_toggled, &data);
  if (G_OBJECT(object)->ref_count == 2) goto single_ref_already;
  g_object_unref(object);
  g_main_loop_run(data.loop);
  g_object_ref(object);
  single_ref_already:
  g_object_remove_toggle_ref(object, on_wait_for_single_ref_toggled, &data);
  g_source_remove(timeout_id);
  g_main_loop_unref(data.loop);
out:
  if (data.timed_out) g_printerr("b ref_count is %d\n", G_OBJECT(object)->ref_count);
  return data.timed_out;
}
#endif