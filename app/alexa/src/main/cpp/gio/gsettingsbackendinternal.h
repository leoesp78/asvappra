#ifndef __G_SETTINGS_BACKEND_INTERNAL_H__
#define __G_SETTINGS_BACKEND_INTERNAL_H__

#include "gsettingsbackend.h"

typedef struct {
  void (*changed)(GObject *target, GSettingsBackend *backend, const gchar *key, gpointer origin_tag);
  void (*path_changed)(GObject *target, GSettingsBackend *backend, const gchar *path, gpointer origin_tag);
  void (*keys_changed)(GObject *target, GSettingsBackend *backend, const gchar *prefix, const gchar * const *names, gpointer origin_tag);
  void (*writable_changed)(GObject *target, GSettingsBackend *backend, const gchar *key);
  void (*path_writable_changed)(GObject *target, GSettingsBackend *backend, const gchar *path);
} GSettingsListenerVTable;
G_GNUC_INTERNAL void g_settings_backend_watch(GSettingsBackend *backend, const GSettingsListenerVTable *vtable, GObject *target, GMainContext *context);
G_GNUC_INTERNAL void g_settings_backend_unwatch(GSettingsBackend *backend, GObject *target);
G_GNUC_INTERNAL GTree *g_settings_backend_create_tree(void);
G_GNUC_INTERNAL GVariant *g_settings_backend_read(GSettingsBackend *backend, const gchar *key, const GVariantType *expected_type, gboolean default_value);
G_GNUC_INTERNAL gboolean g_settings_backend_write(GSettingsBackend *backend, const gchar *key, GVariant *value, gpointer origin_tag);
G_GNUC_INTERNAL gboolean g_settings_backend_write_tree(GSettingsBackend *backend, GTree *tree, gpointer origin_tag);
G_GNUC_INTERNAL void g_settings_backend_reset(GSettingsBackend *backend, const gchar *key, gpointer origin_tag);
G_GNUC_INTERNAL gboolean g_settings_backend_get_writable(GSettingsBackend *backend, const char *key);
G_GNUC_INTERNAL void g_settings_backend_unsubscribe(GSettingsBackend *backend, const char *name);
G_GNUC_INTERNAL void g_settings_backend_subscribe(GSettingsBackend *backend, const char *name);
G_GNUC_INTERNAL GPermission *g_settings_backend_get_permission(GSettingsBackend *backend, const gchar *path);
G_GNUC_INTERNAL void g_settings_backend_sync_default(void);
G_GNUC_INTERNAL GType g_null_settings_backend_get_type(void);
G_GNUC_INTERNAL GType g_memory_settings_backend_get_type(void);

#endif