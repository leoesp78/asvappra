#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include <string.h>
#include "config.h"
#include "gapplication.h"
#include "gapplicationcommandline.h"
#include "gapplicationimpl.h"
#include "gactiongroup.h"
#include "gioenumtypes.h"
#include "gioenums.h"
#include "gfile.h"

struct _GApplicationPrivate {
  GApplicationFlags flags;
  gchar *id;
  GActionGroup *actions;
  GMainLoop *mainloop;
  guint inactivity_timeout_id;
  guint inactivity_timeout;
  guint use_count;
  guint is_registered : 1;
  guint is_remote : 1;
  GHashTable *remote_actions;
  GApplicationImpl *impl;
};
enum {
  PROP_NONE,
  PROP_APPLICATION_ID,
  PROP_FLAGS,
  PROP_IS_REGISTERED,
  PROP_IS_REMOTE,
  PROP_INACTIVITY_TIMEOUT,
  PROP_ACTION_GROUP
};
enum {
  SIGNAL_STARTUP,
  SIGNAL_ACTIVATE,
  SIGNAL_OPEN,
  SIGNAL_ACTION,
  SIGNAL_COMMAND_LINE,
  NR_SIGNALS
};
static guint g_application_signals[NR_SIGNALS];
static void g_application_action_group_iface_init(GActionGroupInterface *);
G_DEFINE_TYPE_WITH_CODE(GApplication, g_application, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP, g_application_action_group_iface_init));
static void g_application_real_before_emit(GApplication *application, GVariant *platform_data) {}
static void g_application_real_after_emit(GApplication *application, GVariant *platform_data) {}
static void g_application_real_startup(GApplication *application) {}
static void g_application_real_activate(GApplication *application) {
  if (!g_signal_has_handler_pending(application, g_application_signals[SIGNAL_ACTIVATE],0, TRUE) &&
      G_APPLICATION_GET_CLASS(application)->activate == g_application_real_activate) {
      static gboolean warned;
      if (warned) return;
      g_warning("Your application does not implement g_application_activate() and has no handlers connected to the 'activate' signal.  It should do "
                "one of these.");
      warned = TRUE;
  }
}
static void g_application_real_open(GApplication *application, GFile **files, gint n_files, const gchar *hint) {
  if (!g_signal_has_handler_pending(application, g_application_signals[SIGNAL_OPEN],0, TRUE) &&
      G_APPLICATION_GET_CLASS(application)->open == g_application_real_open) {
      static gboolean warned;
      if (warned) return;
      g_warning("Your application claims to support opening files but does not implement g_application_open() and has no handlers connected to the "
                "'open' signal.");
      warned = TRUE;
    }
}
static int g_application_real_command_line(GApplication *application, GApplicationCommandLine *cmdline) {
  static gboolean warned;
  if (warned) return 1;
  g_warning("Your application claims to support custom command line handling but does not implement g_application_command_line() and has no handlers "
            "connected to the 'command-line' signal.");
  warned = TRUE;
  return 1;
}
static gboolean g_application_real_local_command_line(GApplication *application, gchar ***arguments, int *exit_status) {
  if (application->priv->flags & G_APPLICATION_HANDLES_COMMAND_LINE) return FALSE;
  else {
      GError *error = NULL;
      gint n_args;
      if (!g_application_register(application, NULL, &error)) {
          g_critical("%s", error->message);
          g_error_free(error);
          *exit_status = 1;
          return TRUE;
      }
      n_args = g_strv_length (*arguments);
      if (application->priv->flags & G_APPLICATION_IS_SERVICE) {
          if ((*exit_status = n_args > 1)) {
              g_printerr("GApplication service mode takes no arguments.\n");
              application->priv->flags &= ~G_APPLICATION_IS_SERVICE;
          }
          return TRUE;
      }
      if (n_args <= 1) {
          g_application_activate(application);
          *exit_status = 0;
      } else {
          if (~application->priv->flags & G_APPLICATION_HANDLES_OPEN) {
              g_critical("This application can not open files.");
              *exit_status = 1;
          } else {
              GFile **files;
              gint n_files;
              gint i;
              n_files = n_args - 1;
              files = g_new (GFile *, n_files);
              for (i = 0; i < n_files; i++) files[i] = g_file_new_for_commandline_arg((*arguments)[i + 1]);
              g_application_open(application, files, n_files, "");
              for (i = 0; i < n_files; i++) g_object_unref(files[i]);
              g_free(files);
              *exit_status = 0;
          }
      }
      return TRUE;
  }
}
static void g_application_real_add_platform_data(GApplication *application, GVariantBuilder *builder) {}
static void g_application_real_quit_mainloop(GApplication *application) {
  if (application->priv->mainloop != NULL) g_main_loop_quit(application->priv->mainloop);
}
static void g_application_real_run_mainloop(GApplication *application) {
  if (application->priv->mainloop == NULL) application->priv->mainloop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(application->priv->mainloop);
}
static void g_application_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GApplication *application = G_APPLICATION (object);
  switch (prop_id) {
      case PROP_APPLICATION_ID: g_application_set_application_id(application, g_value_get_string(value)); break;
      case PROP_FLAGS: g_application_set_flags(application, g_value_get_flags(value)); break;
      case PROP_INACTIVITY_TIMEOUT: g_application_set_inactivity_timeout(application, g_value_get_uint(value)); break;
      case PROP_ACTION_GROUP: g_application_set_action_group(application, g_value_get_object(value)); break;
      default: g_assert_not_reached();
  }
}
void g_application_set_action_group(GApplication *application, GActionGroup *action_group) {
  g_return_if_fail(G_IS_APPLICATION (application));
  g_return_if_fail(!application->priv->is_registered);
  if (application->priv->actions != NULL) g_object_unref(application->priv->actions);
  application->priv->actions = action_group;
  if (application->priv->actions != NULL) g_object_ref(application->priv->actions);
}
static void g_application_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GApplication *application = G_APPLICATION (object);
  switch (prop_id) {
      case PROP_APPLICATION_ID: g_value_set_string(value, g_application_get_application_id(application)); break;
      case PROP_FLAGS: g_value_set_flags(value, g_application_get_flags(application)); break;
      case PROP_IS_REGISTERED: g_value_set_boolean(value, g_application_get_is_registered(application)); break;
      case PROP_IS_REMOTE: g_value_set_boolean(value, g_application_get_is_remote(application)); break;
      case PROP_INACTIVITY_TIMEOUT: g_value_set_uint(value, g_application_get_inactivity_timeout(application)); break;
      default: g_assert_not_reached();
  }
}
static void g_application_constructed(GObject *object) {
  GApplication *application = G_APPLICATION (object);
  g_assert(application->priv->id != NULL);
}
static void g_application_finalize(GObject *object) {
  GApplication *application = G_APPLICATION (object);
  if (application->priv->impl) g_application_impl_destroy(application->priv->impl);
  g_free (application->priv->id);
  if (application->priv->mainloop) g_main_loop_unref(application->priv->mainloop);
  G_OBJECT_CLASS(g_application_parent_class)->finalize(object);
}
static void g_application_init(GApplication *application) {
  application->priv = G_TYPE_INSTANCE_GET_PRIVATE(application, G_TYPE_APPLICATION, GApplicationPrivate);
}
static void g_application_class_init(GApplicationClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->constructed = g_application_constructed;
  object_class->finalize = g_application_finalize;
  object_class->get_property = g_application_get_property;
  object_class->set_property = g_application_set_property;
  class->before_emit = g_application_real_before_emit;
  class->after_emit = g_application_real_after_emit;
  class->startup = g_application_real_startup;
  class->activate = g_application_real_activate;
  class->open = g_application_real_open;
  class->command_line = g_application_real_command_line;
  class->local_command_line = g_application_real_local_command_line;
  class->add_platform_data = g_application_real_add_platform_data;
  class->quit_mainloop = g_application_real_quit_mainloop;
  class->run_mainloop = g_application_real_run_mainloop;
  g_object_class_install_property(object_class, PROP_APPLICATION_ID,g_param_spec_string("application-id", "Application identifier",
                                  "The unique identifier for the application" ,NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_FLAGS, g_param_spec_flags("flags","Application flags",
                                  "Flags specifying the behaviour of the application", G_TYPE_APPLICATION_FLAGS, G_APPLICATION_FLAGS_NONE,
                                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_IS_REGISTERED, g_param_spec_boolean("is-registered", "Is registered",
                                  "If g_application_register() has been called", FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_IS_REMOTE,g_param_spec_boolean("is-remote", "Is remote",
                                  "If this application instance is remote", FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_INACTIVITY_TIMEOUT,g_param_spec_uint("inactivity-timeout",
                                  "Inactivity timeout", "Iime (ms) to stay alive after becoming idle", 0, G_MAXUINT,
                                  0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_ACTION_GROUP,g_param_spec_object("action-group", "Action group",
                                  "The group of actions that the application exports", G_TYPE_ACTION_GROUP, G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
  g_application_signals[SIGNAL_STARTUP] = g_signal_new("startup", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GApplicationClass, startup),
                                                       NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g_application_signals[SIGNAL_ACTIVATE] = g_signal_new("activate", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GApplicationClass, activate),
                                                        NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g_application_signals[SIGNAL_OPEN] = g_signal_new("open", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GApplicationClass, open), NULL, NULL,
                                                    NULL, G_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_INT, G_TYPE_STRING);
  g_application_signals[SIGNAL_COMMAND_LINE] = g_signal_new("command-line", G_TYPE_APPLICATION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GApplicationClass, command_line),
                                                            g_signal_accumulator_first_wins, NULL, NULL, G_TYPE_INT, 1, G_TYPE_APPLICATION_COMMAND_LINE);
  g_type_class_add_private(class, sizeof(GApplicationPrivate));
}
static GVariant *get_platform_data(GApplication *application) {
  GVariantBuilder *builder;
  GVariant *result;
  builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
  {
      gchar *cwd = g_get_current_dir();
      g_variant_builder_add(builder, "{sv}", "cwd", g_variant_new_bytestring(cwd));
      g_free(cwd);
  }
  if (application->priv->flags & G_APPLICATION_SEND_ENVIRONMENT) {
      GVariant *array;
      gchar **envp;
      envp = g_get_environ();
      array = g_variant_new_bytestring_array((const gchar**)envp, -1);
      g_strfreev(envp);
      g_variant_builder_add(builder, "{sv}", "environ", array);
  }
  G_APPLICATION_GET_CLASS(application)->add_platform_data(application, builder);
  result = g_variant_builder_end(builder);
  g_variant_builder_unref(builder);
  return result;
}
gboolean g_application_id_is_valid(const gchar *application_id) {
  gboolean allow_dot;
  if (strlen(application_id) > 255) return FALSE;
  if (!g_ascii_isalpha(*application_id)) return FALSE;
  application_id++;
  allow_dot = FALSE;
  for (; *application_id; application_id++) {
      if (g_ascii_isalnum(*application_id) || (*application_id == '-') || (*application_id == '_')) allow_dot = TRUE;
      else if (allow_dot && *application_id == '.') allow_dot = FALSE;
      else return FALSE;
  }
  return TRUE;
}
GApplication *g_application_new(const gchar *application_id, GApplicationFlags flags) {
  g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
  g_type_init();
  return g_object_new(G_TYPE_APPLICATION, "application-id", application_id, "flags", flags, NULL);
}
const gchar *g_application_get_application_id(GApplication *application) {
  g_return_val_if_fail(G_IS_APPLICATION(application), NULL);
  return application->priv->id;
}
void g_application_set_application_id(GApplication *application, const gchar *application_id) {
  g_return_if_fail(G_IS_APPLICATION(application));
  if (g_strcmp0(application->priv->id, application_id) != 0) {
      g_return_if_fail(g_application_id_is_valid(application_id));
      g_return_if_fail(!application->priv->is_registered);
      g_free(application->priv->id);
      application->priv->id = g_strdup(application_id);
      g_object_notify(G_OBJECT(application), "application-id");
  }
}
GApplicationFlags g_application_get_flags(GApplication *application) {
  g_return_val_if_fail(G_IS_APPLICATION(application), 0);
  return application->priv->flags;
}
void g_application_set_flags(GApplication *application, GApplicationFlags flags) {
  g_return_if_fail(G_IS_APPLICATION(application));
  if (application->priv->flags != flags) {
      g_return_if_fail(!application->priv->is_registered);
      application->priv->flags = flags;
      g_object_notify(G_OBJECT(application), "flags");
  }
}
guint g_application_get_inactivity_timeout(GApplication *application) {
  g_return_val_if_fail(G_IS_APPLICATION(application), 0);
  return application->priv->inactivity_timeout;
}
void g_application_set_inactivity_timeout(GApplication *application, guint inactivity_timeout) {
  g_return_if_fail(G_IS_APPLICATION(application));
  if (application->priv->inactivity_timeout != inactivity_timeout) {
      application->priv->inactivity_timeout = inactivity_timeout;
      g_object_notify(G_OBJECT(application), "inactivity-timeout");
  }
}
gboolean g_application_get_is_registered(GApplication *application) {
  g_return_val_if_fail(G_IS_APPLICATION(application), FALSE);
  return application->priv->is_registered;
}
gboolean g_application_get_is_remote(GApplication *application) {
  g_return_val_if_fail(G_IS_APPLICATION(application), FALSE);
  g_return_val_if_fail(application->priv->is_registered, FALSE);
  return application->priv->is_remote;
}
gboolean g_application_register(GApplication *application, GCancellable *cancellable, GError  **error) {
  g_return_val_if_fail(G_IS_APPLICATION(application), FALSE);
  if (!application->priv->is_registered) {
      application->priv->impl = g_application_impl_register(application, application->priv->id, application->priv->flags, &application->priv->remote_actions,
                                                            cancellable, error);
      if (application->priv->impl == NULL) return FALSE;
      application->priv->is_remote = application->priv->remote_actions != NULL;
      application->priv->is_registered = TRUE;
      g_object_notify(G_OBJECT(application), "is-registered");
      if (!application->priv->is_remote) g_signal_emit(application, g_application_signals[SIGNAL_STARTUP], 0);
  }
  return TRUE;
}
void g_application_hold(GApplication *application) {
  if (application->priv->inactivity_timeout_id) {
      g_source_remove(application->priv->inactivity_timeout_id);
      application->priv->inactivity_timeout_id = 0;
  }
  application->priv->use_count++;
}
static gboolean inactivity_timeout_expired(gpointer data) {
  GApplication *application = G_APPLICATION(data);
  G_APPLICATION_GET_CLASS(application)->quit_mainloop(application);
  return FALSE;
}
void g_application_release(GApplication *application) {
  application->priv->use_count--;
  if (application->priv->use_count == 0) {
      if (application->priv->inactivity_timeout) {
          application->priv->inactivity_timeout_id = g_timeout_add(application->priv->inactivity_timeout, inactivity_timeout_expired, application);
      } else G_APPLICATION_GET_CLASS(application)->quit_mainloop(application);
  }
}
void g_application_activate(GApplication *application) {
  g_return_if_fail(G_IS_APPLICATION (application));
  g_return_if_fail(application->priv->is_registered);
  if (application->priv->is_remote) g_application_impl_activate(application->priv->impl, get_platform_data(application));
  else g_signal_emit(application, g_application_signals[SIGNAL_ACTIVATE], 0);
}
void g_application_open(GApplication *application, GFile **files, gint n_files, const gchar *hint) {
  g_return_if_fail(G_IS_APPLICATION (application));
  g_return_if_fail(application->priv->flags & G_APPLICATION_HANDLES_OPEN);
  g_return_if_fail(application->priv->is_registered);
  if (application->priv->is_remote) g_application_impl_open(application->priv->impl, files, n_files, hint, get_platform_data(application));
  else g_signal_emit(application, g_application_signals[SIGNAL_OPEN],0, files, n_files, hint);
}
int g_application_run(GApplication *application, int argc, char **argv) {
  gchar **arguments;
  int status;
  gint i;
  g_return_val_if_fail(G_IS_APPLICATION (application), 1);
  g_return_val_if_fail(argc == 0 || argv != NULL, 1);
  arguments = g_new(gchar *, argc + 1);
  for (i = 0; i < argc; i++) arguments[i] = g_strdup(argv[i]);
  arguments[i] = NULL;
  if (g_get_prgname () == NULL && argc > 0) {
      gchar *prgname;
      prgname = g_path_get_basename(argv[0]);
      g_set_prgname(prgname);
      g_free(prgname);
  }
  if (!G_APPLICATION_GET_CLASS(application)->local_command_line (application, &arguments, &status)) {
      GError *error = NULL;
      if (!g_application_register(application, NULL, &error)) {
          g_printerr("%s", error->message);
          g_error_free(error);
          return 1;
      }
      if (application->priv->is_remote) {
          GVariant *platform_data;
          platform_data = get_platform_data (application);
          status = g_application_impl_command_line(application->priv->impl, arguments, platform_data);
      } else {
          GApplicationCommandLine *cmdline;
          GVariant *v;
          v = g_variant_new_bytestring_array((const gchar**)arguments, -1);
          cmdline = g_object_new(G_TYPE_APPLICATION_COMMAND_LINE, "arguments", v, NULL);
          g_signal_emit(application, g_application_signals[SIGNAL_COMMAND_LINE],0, cmdline, &status);
          g_object_unref(cmdline);
      }
  }
  g_strfreev (arguments);
  if (application->priv->flags & G_APPLICATION_IS_SERVICE && application->priv->is_registered && !application->priv->use_count &&
      !application->priv->inactivity_timeout_id) {
      application->priv->inactivity_timeout_id = g_timeout_add(10000, inactivity_timeout_expired, application);
  }
  if (application->priv->use_count || application->priv->inactivity_timeout_id) {
      G_APPLICATION_GET_CLASS(application)->run_mainloop(application);
      status = 0;
  }
  if (application->priv->impl) g_application_impl_flush(application->priv->impl);
  return status;
}
static gboolean g_application_has_action(GActionGroup *action_group, const gchar *action_name) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail (application->priv->is_registered, FALSE);
  if (application->priv->remote_actions != NULL) return g_hash_table_lookup (application->priv->remote_actions, action_name) != NULL;
  return application->priv->actions && g_action_group_has_action (application->priv->actions, action_name);
}
static gchar **g_application_list_actions(GActionGroup *action_group) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail(application->priv->is_registered, NULL);
  if (application->priv->remote_actions != NULL) {
      GHashTableIter iter;
      gint n, i = 0;
      gchar **keys;
      gpointer key;
      n = g_hash_table_size(application->priv->remote_actions);
      keys = g_new(gchar *, n + 1);
      g_hash_table_iter_init(&iter, application->priv->remote_actions);
      while(g_hash_table_iter_next(&iter, &key, NULL)) keys[i++] = g_strdup (key);
      g_assert_cmpint(i, ==, n);
      keys[n] = NULL;
      return keys;
  } else if (application->priv->actions != NULL) return g_action_group_list_actions(application->priv->actions);
  else return g_new0(gchar *, 1);
}
static gboolean g_application_get_action_enabled(GActionGroup *action_group, const gchar *action_name) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail(application->priv->actions != NULL, FALSE);
  g_return_val_if_fail(application->priv->is_registered, FALSE);
  if (application->priv->remote_actions) {
      RemoteActionInfo *info;
      info = g_hash_table_lookup(application->priv->remote_actions, action_name);
      return info && info->enabled;
  }
  return g_action_group_get_action_enabled(application->priv->actions, action_name);
}
static const GVariantType *g_application_get_action_parameter_type(GActionGroup *action_group, const gchar *action_name) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail(application->priv->actions != NULL, NULL);
  g_return_val_if_fail(application->priv->is_registered, NULL);
  if (application->priv->remote_actions) {
      RemoteActionInfo *info;
      info = g_hash_table_lookup(application->priv->remote_actions, action_name);
      if (info) return info->parameter_type;
      else return NULL;
  }
  return g_action_group_get_action_parameter_type(application->priv->actions, action_name);
}
static const GVariantType *g_application_get_action_state_type(GActionGroup *action_group, const gchar *action_name) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail(application->priv->actions != NULL, NULL);
  g_return_val_if_fail(application->priv->is_registered, NULL);
  if (application->priv->remote_actions) {
      RemoteActionInfo *info;
      info = g_hash_table_lookup(application->priv->remote_actions, action_name);
      if (info && info->state) return g_variant_get_type(info->state);
      else return NULL;
  }
  return g_action_group_get_action_state_type(application->priv->actions, action_name);
}
static GVariant *g_application_get_action_state(GActionGroup *action_group, const gchar *action_name) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_val_if_fail(application->priv->actions != NULL, NULL);
  g_return_val_if_fail(application->priv->is_registered, NULL);
  if (application->priv->remote_actions) {
      RemoteActionInfo *info;
      info = g_hash_table_lookup(application->priv->remote_actions, action_name);
      if (info && info->state) return g_variant_ref(info->state);
      else return NULL;
  }
  return g_action_group_get_action_state(application->priv->actions, action_name);
}
static void g_application_change_action_state(GActionGroup *action_group, const gchar *action_name, GVariant *value) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_if_fail(application->priv->is_registered);
  if (application->priv->is_remote) g_application_impl_change_action_state(application->priv->impl, action_name, value, get_platform_data(application));
  else g_action_group_change_action_state(application->priv->actions, action_name, value);
}
static void g_application_activate_action(GActionGroup *action_group, const gchar *action_name, GVariant *parameter) {
  GApplication *application = G_APPLICATION(action_group);
  g_return_if_fail(application->priv->is_registered);
  if (application->priv->is_remote) g_application_impl_activate_action(application->priv->impl, action_name, parameter, get_platform_data(application));
  else g_action_group_activate_action(application->priv->actions, action_name, parameter);
}
static void g_application_action_group_iface_init(GActionGroupInterface *iface) {
  iface->has_action = g_application_has_action;
  iface->list_actions = g_application_list_actions;
  iface->get_action_enabled = g_application_get_action_enabled;
  iface->get_action_parameter_type = g_application_get_action_parameter_type;
  iface->get_action_state_type = g_application_get_action_state_type;
  iface->get_action_state = g_application_get_action_state;
  iface->change_action_state = g_application_change_action_state;
  iface->activate_action = g_application_activate_action;
}