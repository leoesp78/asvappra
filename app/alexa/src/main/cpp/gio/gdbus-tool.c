#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "../gio/gio.h"
#include "../glib/gi18n.h"
#include "config.h"

G_GNUC_UNUSED static void completion_debug(const gchar *format, ...);
#if 0
G_GNUC_UNUSED static void completion_debug(const gchar *format, ...) {
  va_list var_args;
  gchar *s;
  static FILE *f = NULL;
  va_start(var_args, format);
  s = g_strdup_vprintf(format, var_args);
  if (f == NULL) f = fopen("/tmp/gdbus-completion-debug.txt", "a+");
  fprintf(f, "%s\n", s);
  g_free(s);
}
#else
static void completion_debug(const gchar *format, ...) {}
#endif
static void remove_arg(gint num, gint *argc, gchar **argv[]) {
  gint n;
  g_assert(num <= (*argc));
  for (n = num; (*argv)[n] != NULL; n++) (*argv)[n] = (*argv)[n+1];
  (*argv)[n] = NULL;
  (*argc) = (*argc)-1;
}
static void usage(gint *argc, gchar **argv[], gboolean use_stdout) {
  GOptionContext *o;
  gchar *s;
  gchar *program_name;
  o = g_option_context_new("COMMAND");
  g_option_context_set_help_enabled(o, FALSE);
  g_option_context_parse(o, argc, argv, NULL);
  program_name = g_path_get_basename((*argv)[0]);
  s = g_strdup_printf("Commands:\n  help         Shows this information\n  introspect   Introspect a remote object\n  monitor      Monitor a remote "
                      "object\n  call         Invoke a method on a remote object\n\nUse \"%s COMMAND --help\" to get help on each command.\n", program_name);
  g_free (program_name);
  g_option_context_set_description(o, s);
  g_free(s);
  s = g_option_context_get_help(o, FALSE, NULL);
  if (use_stdout) g_print("%s", s);
  else g_printerr("%s", s);
  g_free(s);
  g_option_context_free(o);
}
static void modify_argv0_for_command(gint *argc, gchar **argv[], const gchar *command) {
  gchar *s;
  gchar *program_name;
  g_assert(g_strcmp0((*argv)[1], command) == 0);
  remove_arg(1, argc, argv);
  program_name = g_path_get_basename((*argv)[0]);
  s = g_strdup_printf("%s %s", (*argv)[0], command);
  (*argv)[0] = s;
  g_free(program_name);
}
static void print_methods(GDBusConnection *c, const gchar *name, const gchar *path) {
  GVariant *result;
  GError *error;
  const gchar *xml_data;
  GDBusNodeInfo *node;
  guint n;
  guint m;
  error = NULL;
  result = g_dbus_connection_call_sync(c, name, path,"org.freedesktop.DBus.Introspectable","Introspect",NULL,
                                       G_VARIANT_TYPE("(s)"),G_DBUS_CALL_FLAGS_NONE,3000,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      return;
  }
  g_variant_get(result,"(&s)", &xml_data);
  error = NULL;
  node = g_dbus_node_info_new_for_xml(xml_data, &error);
  g_variant_unref(result);
  if (node == NULL) {
      g_printerr("Error parsing introspection XML: %s\n", error->message);
      g_error_free(error);
      return;
  }
  for (n = 0; node->interfaces != NULL && node->interfaces[n] != NULL; n++) {
      const GDBusInterfaceInfo *iface = node->interfaces[n];
      for (m = 0; iface->methods != NULL && iface->methods[m] != NULL; m++) {
          const GDBusMethodInfo *method = iface->methods[m];
          g_print("%s.%s \n", iface->name, method->name);
      }
  }
  g_dbus_node_info_unref(node);
}
static void print_paths(GDBusConnection *c, const gchar *name, const gchar *path) {
  GVariant *result;
  GError *error;
  const gchar *xml_data;
  GDBusNodeInfo *node;
  guint n;
  error = NULL;
  result = g_dbus_connection_call_sync(c, name, path,"org.freedesktop.DBus.Introspectable","Introspect",NULL,
                                       G_VARIANT_TYPE("(s)"),G_DBUS_CALL_FLAGS_NONE,3000,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      return;
  }
  g_variant_get(result, "(&s)", &xml_data);
  error = NULL;
  node = g_dbus_node_info_new_for_xml(xml_data, &error);
  g_variant_unref(result);
  if (node == NULL) {
      g_printerr("Error parsing introspection XML: %s\n", error->message);
      g_error_free(error);
      return;
  }
  if (node->interfaces != NULL) g_print("%s \n", path);
  for (n = 0; node->nodes != NULL && node->nodes[n] != NULL; n++) {
      gchar *s;
      if (g_strcmp0(path, "/") == 0) s = g_strdup_printf("/%s", node->nodes[n]->path);
      else s = g_strdup_printf("%s/%s", path, node->nodes[n]->path);
      print_paths(c, name, s);
      g_free(s);
  }
  g_dbus_node_info_unref(node);
}
static void print_names(GDBusConnection *c, gboolean include_unique_names) {
  GVariant *result;
  GError *error;
  GVariantIter *iter;
  gchar *str;
  GHashTable *name_set;
  GList *keys;
  GList *l;
  name_set = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  error = NULL;
  result = g_dbus_connection_call_sync(c,"org.freedesktop.DBus","/org/freedesktop/DBus","org.freedesktop.DBus",
                                       "ListNames",NULL, G_VARIANT_TYPE("(as)"),G_DBUS_CALL_FLAGS_NONE,
                                       3000,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      goto out;
  }
  g_variant_get(result, "(as)", &iter);
  while(g_variant_iter_loop(iter, "s", &str)) g_hash_table_insert(name_set, g_strdup(str), NULL);
  g_variant_iter_free(iter);
  g_variant_unref(result);
  error = NULL;
  result = g_dbus_connection_call_sync(c,"org.freedesktop.DBus","/org/freedesktop/DBus", "org.freedesktop.DBus",
                                       "ListActivatableNames",NULL, G_VARIANT_TYPE ("(as)"),
                                       G_DBUS_CALL_FLAGS_NONE,3000,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      goto out;
  }
  g_variant_get(result, "(as)", &iter);
  while(g_variant_iter_loop(iter, "s", &str)) g_hash_table_insert(name_set, g_strdup(str), NULL);
  g_variant_iter_free(iter);
  g_variant_unref(result);
  keys = g_hash_table_get_keys(name_set);
  keys = g_list_sort(keys, (GCompareFunc)g_strcmp0);
  for (l = keys; l != NULL; l = l->next) {
      const gchar *name = l->data;
      if (!include_unique_names && g_str_has_prefix(name, ":")) continue;
      g_print("%s \n", name);
  }
  g_list_free(keys);
out:
  g_hash_table_unref(name_set);
}
static gboolean  opt_connection_system  = FALSE;
static gboolean  opt_connection_session = FALSE;
static gchar    *opt_connection_address = NULL;
static const GOptionEntry connection_entries[] = {
  {"system",'y',0,G_OPTION_ARG_NONE, &opt_connection_system, N_("Connect to the system bus"),NULL },
  {"session",'e',0,G_OPTION_ARG_NONE, &opt_connection_session, N_("Connect to the session bus"),NULL },
  {"address",'a',0,G_OPTION_ARG_STRING, &opt_connection_address, N_("Connect to given D-Bus address"),NULL },
  {NULL }
};
static GOptionGroup *connection_get_group(void) {
  static GOptionGroup *g;
  g = g_option_group_new("connection", N_("Connection Endpoint Options:"), N_("Options specifying the connection endpoint"),NULL,
                         NULL);
  g_option_group_set_translation_domain(g, GETTEXT_PACKAGE);
  g_option_group_add_entries(g, connection_entries);
  return g;
}
static GDBusConnection *connection_get_dbus_connection(GError **error) {
  GDBusConnection *c;
  c = NULL;
  if (!opt_connection_system && !opt_connection_session && opt_connection_address == NULL) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"No connection endpoint specified");
      return c;
  } else if ((opt_connection_system && (opt_connection_session || opt_connection_address != NULL)) || (opt_connection_session && (opt_connection_system ||
             opt_connection_address != NULL)) || (opt_connection_address != NULL && (opt_connection_system || opt_connection_session))) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Multiple connection endpoints specified");
      return c;
  }
  if (opt_connection_system) c = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, error);
  else if (opt_connection_session) c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);
  else if (opt_connection_address != NULL) {
      c = g_dbus_connection_new_for_address_sync(opt_connection_address,G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,NULL,NULL, error);
  }
  return c;
}
static GPtrArray *call_helper_get_method_in_signature(GDBusConnection  *c, const gchar *dest, const gchar *path, const gchar *interface_name,
                                                      const gchar *method_name, GError **error) {
  GPtrArray *ret;
  GVariant *result;
  GDBusNodeInfo *node_info;
  const gchar *xml_data;
  GDBusInterfaceInfo *interface_info;
  GDBusMethodInfo *method_info;
  guint n;
  ret = NULL;
  result = NULL;
  node_info = NULL;
  result = g_dbus_connection_call_sync(c, dest, path,"org.freedesktop.DBus.Introspectable","Introspect",NULL,
                                       G_VARIANT_TYPE("(s)"),G_DBUS_CALL_FLAGS_NONE,3000,NULL, error);
  if (result == NULL) goto out;
  g_variant_get(result, "(&s)", &xml_data);
  node_info = g_dbus_node_info_new_for_xml(xml_data, error);
  if (node_info == NULL) goto out;
  interface_info = g_dbus_node_info_lookup_interface(node_info, interface_name);
  if (interface_info == NULL) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Warning: According to introspection data, interface `%s' does not exist\n",
                  interface_name);
      goto out;
  }
  method_info = g_dbus_interface_info_lookup_method(interface_info, method_name);
  if (method_info == NULL) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,"Warning: According to introspection data, method `%s' does not exist on interface `%s'\n",
                  method_name, interface_name);
      goto out;
  }
  ret = g_ptr_array_new_with_free_func((GDestroyNotify) g_variant_type_free);
  for (n = 0; method_info->in_args != NULL && method_info->in_args[n] != NULL; n++) {
      g_ptr_array_add(ret, g_variant_type_new(method_info->in_args[n]->signature));
  }
out:
  if (node_info != NULL) g_dbus_node_info_unref(node_info);
  if (result != NULL) g_variant_unref(result);
  return ret;
}
static GVariant *_g_variant_parse_me_harder(GVariantType *type, const gchar *given_str, GError **error) {
  GVariant *value;
  gchar *s;
  guint n;
  GString *str;
  str = g_string_new("\"");
  for (n = 0; given_str[n] != '\0'; n++) {
      if (G_UNLIKELY(given_str[n] == '\"')) g_string_append(str, "\\\"");
      else g_string_append_c(str, given_str[n]);
  }
  g_string_append_c(str, '"');
  s = g_string_free(str, FALSE);
  value = g_variant_parse(type, s,NULL,NULL, error);
  g_free(s);
  return value;
}
static gchar *opt_call_dest = NULL;
static gchar *opt_call_object_path = NULL;
static gchar *opt_call_method = NULL;
static const GOptionEntry call_entries[] = {
  {"dest",'d',0,G_OPTION_ARG_STRING, &opt_call_dest, N_("Destination name to invoke method on"),NULL },
  {"object-path",'o',0,G_OPTION_ARG_STRING, &opt_call_object_path, N_("Object path to invoke method on"),NULL },
  {"method",'m',0,G_OPTION_ARG_STRING, &opt_call_method, N_("Method and interface name"),NULL },
  {NULL }
};
static gboolean handle_call(gint *argc, gchar **argv[], gboolean request_completion, const gchar *completion_cur, const gchar *completion_prev) {
  gint ret;
  GOptionContext *o;
  gchar *s;
  GError *error;
  GDBusConnection *c;
  GVariant *parameters;
  gchar *interface_name;
  gchar *method_name;
  GVariant *result;
  GPtrArray *in_signature_types;
  gboolean complete_names;
  gboolean complete_paths;
  gboolean complete_methods;
  GVariantBuilder builder;
  guint n;
  ret = FALSE;
  c = NULL;
  parameters = NULL;
  interface_name = NULL;
  method_name = NULL;
  result = NULL;
  in_signature_types = NULL;
  modify_argv0_for_command(argc, argv, "call");
  o = g_option_context_new(NULL);
  g_option_context_set_help_enabled(o, FALSE);
  g_option_context_set_summary(o,"Invoke a method on a remote object.");
  g_option_context_add_main_entries(o, call_entries, GETTEXT_PACKAGE);
  g_option_context_add_group(o, connection_get_group ());
  complete_names = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--dest") == 0) {
      complete_names = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  complete_paths = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--object-path") == 0) {
      complete_paths = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  complete_methods = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--method") == 0) {
      complete_methods = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  if (!g_option_context_parse(o, argc, argv, NULL)) {
      if (!request_completion) {
          s = g_option_context_get_help(o, FALSE, NULL);
          g_printerr("%s", s);
          g_free(s);
          goto out;
      }
  }
  error = NULL;
  c = connection_get_dbus_connection(&error);
  if (c == NULL) {
      if (request_completion) {
          if (g_strcmp0(completion_prev, "--address") == 0) g_print("unix:\ntcp:\nnonce-tcp:\n");
          else g_print("--system \n--session \n--address \n");
      } else {
          g_printerr("Error connecting: %s\n", error->message);
          g_error_free(error);
      }
      goto out;
  }
  if (g_dbus_connection_get_unique_name (c) != NULL) {
      if (complete_names) {
          print_names(c, FALSE);
          goto out;
      }
      if (opt_call_dest == NULL) {
          if (request_completion) g_print("--dest \n");
          else g_printerr("Error: Destination is not specified\n");
          goto out;
      }
      if (request_completion && g_strcmp0("--dest", completion_prev) == 0) {
          print_names(c, g_str_has_prefix(opt_call_dest, ":"));
          goto out;
      }
  }
  if (complete_paths) {
      print_paths(c, opt_call_dest, "/");
      goto out;
  }
  if (opt_call_object_path == NULL) {
      if (request_completion) g_print("--object-path \n");
      else g_printerr("Error: Object path is not specified\n");
      goto out;
    }
  if (request_completion && g_strcmp0("--object-path", completion_prev) == 0) {
      gchar *p;
      s = g_strdup(opt_call_object_path);
      p = strrchr(s, '/');
      if (p != NULL) {
          if (p == s) p++;
          *p = '\0';
      }
      print_paths(c, opt_call_dest, s);
      g_free (s);
      goto out;
  }
  if (!request_completion && !g_variant_is_object_path (opt_call_object_path)) {
      g_printerr("Error: %s is not a valid object path\n", opt_call_object_path);
      goto out;
  }
  if (complete_methods) {
      print_methods(c, opt_call_dest, opt_call_object_path);
      goto out;
  }
  if (opt_call_method == NULL) {
      if (request_completion) g_print("--method \n");
      else g_printerr("Error: Method name is not specified\n");
      goto out;
  }
  if (request_completion && g_strcmp0("--method", completion_prev) == 0) {
      print_methods(c, opt_call_dest, opt_call_object_path);
      goto out;
  }
  s = strrchr(opt_call_method, '.');
  if (!request_completion && s == NULL) {
      g_printerr("Error: Method name `%s' is invalid\n", opt_call_method);
      goto out;
  }
  method_name = g_strdup(s + 1);
  interface_name = g_strndup(opt_call_method, s - opt_call_method);
  if (request_completion) goto out;
  in_signature_types = call_helper_get_method_in_signature(c, opt_call_dest, opt_call_object_path, interface_name, method_name, &error);
  if (in_signature_types == NULL) {
      g_error_free(error);
      error = NULL;
  }
  g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
  for (n = 1; n < (guint) *argc; n++) {
      GVariant *value;
      GVariantType *type;
      type = NULL;
      if (in_signature_types != NULL) {
          if (n - 1 >= in_signature_types->len) {
              if (n - 1 == in_signature_types->len) {
                  g_printerr("Warning: Introspection data indicates %d parameters but more was passed\n", in_signature_types->len);
              }
          } else type = in_signature_types->pdata[n - 1];
      }
      error = NULL;
      value = g_variant_parse(type, (*argv)[n],NULL,NULL, &error);
      if (value == NULL) {
          g_error_free(error);
          error = NULL;
          value = _g_variant_parse_me_harder(type, (*argv)[n], &error);
          if (value == NULL) {
              if (type != NULL) {
                  s = g_variant_type_dup_string(type);
                  g_printerr("Error parsing parameter %d of type `%s': %s\n", n, s, error->message);
                  g_free(s);
              } else g_printerr("Error parsing parameter %d: %s\n", n, error->message);
              g_error_free(error);
              g_variant_builder_clear(&builder);
              goto out;
          }
      }
      g_variant_builder_add_value(&builder, value);
  }
  parameters = g_variant_builder_end(&builder);
  if (parameters != NULL) parameters = g_variant_ref_sink(parameters);
  result = g_dbus_connection_call_sync(c, opt_call_dest, opt_call_object_path, interface_name, method_name, parameters,NULL,G_DBUS_CALL_FLAGS_NONE,
                                       -1,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      if (in_signature_types != NULL) {
          GString *s;
          s = g_string_new(NULL);
          for (n = 0; n < in_signature_types->len; n++) {
              GVariantType *type = in_signature_types->pdata[n];
              g_string_append_len(s, g_variant_type_peek_string(type), g_variant_type_get_string_length(type));
          }
          g_printerr("(According to introspection data, you need to pass `%s')\n", s->str);
          g_string_free(s, TRUE);
      }
      goto out;
  }
  s = g_variant_print(result, TRUE);
  g_print("%s\n", s);
  g_free(s);
  ret = TRUE;
out:
  if (in_signature_types != NULL) g_ptr_array_unref(in_signature_types);
  if (result != NULL) g_variant_unref(result);
  if (c != NULL) g_object_unref(c);
  if (parameters != NULL) g_variant_unref(parameters);
  g_free(interface_name);
  g_free(method_name);
  g_option_context_free(o);
  return ret;
}
static void dump_annotation(const GDBusAnnotationInfo *o, guint indent, gboolean ignore_indent) {
  guint n;
  g_print("%*s@%s(\"%s\")\n", ignore_indent ? 0 : indent, "", o->key, o->value);
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++)
      dump_annotation(o->annotations[n], indent + 2, FALSE);
}
static void dump_arg(const GDBusArgInfo *o, guint indent, const gchar *direction, gboolean ignore_indent, gboolean include_newline) {
  guint n;
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) {
      dump_annotation(o->annotations[n], indent, ignore_indent);
      ignore_indent = FALSE;
  }
  g_print("%*s%s%s %s%s", ignore_indent ? 0 : indent, "", direction, o->signature, o->name, include_newline ? ",\n" : "");
}
static guint count_args(GDBusArgInfo **args) {
  guint n;
  n = 0;
  if (args == NULL) goto out;
  while(args[n] != NULL) n++;
out:
  return n;
}
static void dump_method(const GDBusMethodInfo *o, guint indent) {
  guint n;
  guint m;
  guint name_len;
  guint total_num_args;
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) dump_annotation(o->annotations[n], indent, FALSE);
  g_print("%*s%s(", indent, "", o->name);
  name_len = strlen(o->name);
  total_num_args = count_args(o->in_args) + count_args(o->out_args);
  for (n = 0, m = 0; o->in_args != NULL && o->in_args[n] != NULL; n++, m++) {
      gboolean ignore_indent = (m == 0);
      gboolean include_newline = (m != total_num_args - 1);
      dump_arg(o->in_args[n],indent + name_len + 1,"in  ", ignore_indent, include_newline);
  }
  for (n = 0; o->out_args != NULL && o->out_args[n] != NULL; n++, m++) {
      gboolean ignore_indent = (m == 0);
      gboolean include_newline = (m != total_num_args - 1);
      dump_arg(o->out_args[n],indent + name_len + 1,"out ", ignore_indent, include_newline);
  }
  g_print(");\n");
}
static void dump_signal(const GDBusSignalInfo *o, guint indent) {
  guint n;
  guint name_len;
  guint total_num_args;
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) dump_annotation(o->annotations[n], indent, FALSE);
  g_print("%*s%s(", indent, "", o->name);
  name_len = strlen(o->name);
  total_num_args = count_args(o->args);
  for (n = 0; o->args != NULL && o->args[n] != NULL; n++) {
      gboolean ignore_indent = (n == 0);
      gboolean include_newline = (n != total_num_args - 1);
      dump_arg(o->args[n],indent + name_len + 1,"", ignore_indent, include_newline);
  }
  g_print(");\n");
}
static void dump_property(const GDBusPropertyInfo *o, guint indent, GVariant *value) {
  const gchar *access;
  guint n;
  if (o->flags == G_DBUS_PROPERTY_INFO_FLAGS_READABLE) access = "readonly";
  else if (o->flags == G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE) access = "writeonly";
  else if (o->flags == (G_DBUS_PROPERTY_INFO_FLAGS_READABLE | G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE)) access = "readwrite";
  else g_assert_not_reached();
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) dump_annotation(o->annotations[n], indent, FALSE);
  if (value != NULL) {
      gchar *s = g_variant_print(value, FALSE);
      g_print("%*s%s %s %s = %s;\n", indent, "", access, o->signature, o->name, s);
      g_free(s);
  } else g_print("%*s%s %s %s;\n", indent, "", access, o->signature, o->name);
}
static void dump_interface(GDBusConnection *c, const gchar *name, const GDBusInterfaceInfo *o, guint indent, const gchar *object_path) {
  guint n;
  GHashTable *properties;
  properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);
  if (c != NULL && name != NULL && object_path != NULL && o->properties != NULL) {
      GVariant *result;
      result = g_dbus_connection_call_sync(c, name, object_path,"org.freedesktop.DBus.Properties","GetAll",
                                           g_variant_new("(s)", o->name),NULL,G_DBUS_CALL_FLAGS_NONE,
                                           3000,NULL,NULL);
      if (result != NULL) {
          if (g_variant_is_of_type(result, G_VARIANT_TYPE ("(a{sv})"))) {
              GVariantIter *iter;
              GVariant *item;
              g_variant_get(result,"(a{sv})", &iter);
              while((item = g_variant_iter_next_value(iter))) {
                  gchar *key;
                  GVariant *value;
                  g_variant_get(item,"{sv}", &key, &value);
                  g_hash_table_insert(properties, key, g_variant_ref (value));
              }
          }
          g_variant_unref(result);
      } else {
          guint n;
          for (n = 0; o->properties != NULL && o->properties[n] != NULL; n++) {
              result = g_dbus_connection_call_sync(c, name, object_path,"org.freedesktop.DBus.Properties","Get",
                                                   g_variant_new("(ss)", o->name, o->properties[n]->name),
                                                   G_VARIANT_TYPE("(v)"),G_DBUS_CALL_FLAGS_NONE,3000,NULL,
                                                   NULL);
              if (result != NULL) {
                  GVariant *property_value;
                  g_variant_get(result,"(v)", &property_value);
                  g_hash_table_insert(properties, g_strdup (o->properties[n]->name), g_variant_ref (property_value));
                  g_variant_unref(result);
              }
          }
      }
  }
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) dump_annotation(o->annotations[n], indent, FALSE);
  g_print("%*sinterface %s {\n", indent, "", o->name);
  if (o->methods != NULL) {
      g_print("%*s  methods:\n", indent, "");
      for (n = 0; o->methods[n] != NULL; n++)
        dump_method(o->methods[n], indent + 4);
  }
  if (o->signals != NULL) {
      g_print("%*s  signals:\n", indent, "");
      for (n = 0; o->signals[n] != NULL; n++) dump_signal(o->signals[n], indent + 4);
  }
  if (o->properties != NULL) {
      g_print("%*s  properties:\n", indent, "");
      for (n = 0; o->properties[n] != NULL; n++) {
          dump_property(o->properties[n],indent + 4, g_hash_table_lookup(properties, (o->properties[n])->name));
      }
  }
  g_print("%*s};\n", indent, "");
  g_hash_table_unref(properties);
}
static void dump_node(GDBusConnection *c, const gchar *name, const GDBusNodeInfo *o, guint indent, const gchar *object_path) {
  guint n;
  const gchar *object_path_to_print;
  object_path_to_print = object_path;
  if (o->path != NULL) object_path_to_print = o->path;
  for (n = 0; o->annotations != NULL && o->annotations[n] != NULL; n++) dump_annotation(o->annotations[n], indent, FALSE);
  g_print("%*snode %s", indent, "", object_path_to_print != NULL ? object_path_to_print : "(not set)");
  if (o->interfaces != NULL || o->nodes != NULL) {
      g_print(" {\n");
      for (n = 0; o->interfaces != NULL && o->interfaces[n] != NULL; n++) dump_interface(c, name, o->interfaces[n], indent + 2, object_path);
      for (n = 0; o->nodes != NULL && o->nodes[n] != NULL; n++) dump_node(NULL, NULL, o->nodes[n], indent + 2, NULL);
      g_print("%*s};\n", indent, "");
  } else g_print("\n");
}
static gchar *opt_introspect_dest = NULL;
static gchar *opt_introspect_object_path = NULL;
static gboolean opt_introspect_xml = FALSE;
static const GOptionEntry introspect_entries[] = {
  {"dest",'d',0,G_OPTION_ARG_STRING, &opt_introspect_dest, N_("Destination name to introspect"),NULL },
  {"object-path",'o',0,G_OPTION_ARG_STRING, &opt_introspect_object_path, N_("Object path to introspect"),NULL },
  {"xml",'x',0,G_OPTION_ARG_NONE, &opt_introspect_xml, N_("Print XML"),NULL },
  {NULL }
};
static gboolean handle_introspect(gint *argc, gchar **argv[], gboolean request_completion, const gchar *completion_cur, const gchar *completion_prev) {
  gint ret;
  GOptionContext *o;
  gchar *s;
  GError *error;
  GDBusConnection *c;
  GVariant *result;
  const gchar *xml_data;
  GDBusNodeInfo *node;
  gboolean complete_names;
  gboolean complete_paths;
  ret = FALSE;
  c = NULL;
  node = NULL;
  result = NULL;
  modify_argv0_for_command(argc, argv, "introspect");
  o = g_option_context_new(NULL);
  if (request_completion) g_option_context_set_ignore_unknown_options(o, TRUE);
  g_option_context_set_help_enabled(o, FALSE);
  g_option_context_set_summary(o,"Introspect a remote object.");
  g_option_context_add_main_entries(o, introspect_entries, GETTEXT_PACKAGE);
  g_option_context_add_group(o, connection_get_group());
  complete_names = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--dest") == 0) {
      complete_names = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  complete_paths = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--object-path") == 0) {
      complete_paths = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  if (!g_option_context_parse(o, argc, argv,NULL)){
      if (!request_completion) {
          s = g_option_context_get_help(o, FALSE, NULL);
          g_printerr("%s", s);
          g_free(s);
          goto out;
      }
  }
  error = NULL;
  c = connection_get_dbus_connection(&error);
  if (c == NULL) {
      if (request_completion) {
          if (g_strcmp0(completion_prev, "--address") == 0) g_print("unix:\ntcp:\nnonce-tcp:\n");
          else g_print("--system \n--session \n--address \n");
      } else {
          g_printerr("Error connecting: %s\n", error->message);
          g_error_free(error);
      }
      goto out;
  }
  if (g_dbus_connection_get_unique_name(c) != NULL) {
      if (complete_names) {
          print_names(c, FALSE);
          goto out;
      }
      if (opt_introspect_dest == NULL) {
          if (request_completion) g_print("--dest \n");
          else g_printerr("Error: Destination is not specified\n");
          goto out;
      }
      if (request_completion && g_strcmp0("--dest", completion_prev) == 0) {
          print_names(c, g_str_has_prefix(opt_introspect_dest, ":"));
          goto out;
      }
  }
  if (complete_paths) {
      print_paths(c, opt_introspect_dest, "/");
      goto out;
  }
  if (opt_introspect_object_path == NULL) {
      if (request_completion) g_print("--object-path \n");
      else g_printerr("Error: Object path is not specified\n");
      goto out;
  }
  if (request_completion && g_strcmp0("--object-path", completion_prev) == 0) {
      gchar *p;
      s = g_strdup(opt_introspect_object_path);
      p = strrchr(s, '/');
      if (p != NULL) {
          if (p == s) p++;
          *p = '\0';
      }
      print_paths(c, opt_introspect_dest, s);
      g_free(s);
      goto out;
  }
  if (!request_completion && !g_variant_is_object_path(opt_introspect_object_path)) {
      g_printerr("Error: %s is not a valid object path\n", opt_introspect_object_path);
      goto out;
  }
  if (request_completion) goto out;
  result = g_dbus_connection_call_sync(c, opt_introspect_dest, opt_introspect_object_path,"org.freedesktop.DBus.Introspectable","Introspect",
                                       NULL, G_VARIANT_TYPE("(s)"), G_DBUS_CALL_FLAGS_NONE,3000,NULL, &error);
  if (result == NULL) {
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      goto out;
  }
  g_variant_get(result, "(&s)", &xml_data);
  if (opt_introspect_xml) g_print("%s", xml_data);
  else {
      error = NULL;
      node = g_dbus_node_info_new_for_xml(xml_data, &error);
      if (node == NULL) {
          g_printerr("Error parsing introspection XML: %s\n", error->message);
          g_error_free(error);
          goto out;
      }
      dump_node(c, opt_introspect_dest, node, 0, opt_introspect_object_path);
  }
  ret = TRUE;
out:
  if (node != NULL) g_dbus_node_info_unref(node);
  if (result != NULL) g_variant_unref(result);
  if (c != NULL) g_object_unref(c);
  g_option_context_free(o);
  return ret;
}
static gchar *opt_monitor_dest = NULL;
static gchar *opt_monitor_object_path = NULL;
static guint monitor_filter_id = 0;
static void monitor_signal_cb(GDBusConnection *connection, const gchar *sender_name, const gchar *object_path, const gchar *interface_name,
                              const gchar *signal_name, GVariant *parameters, gpointer user_data) {
  gchar *s;
  s = g_variant_print(parameters, TRUE);
  g_print("%s: %s.%s %s\n", object_path, interface_name, signal_name, s);
  g_free(s);
}
static void monitor_on_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data) {
  g_print("The name %s is owned by %s\n", name, name_owner);
  g_assert(monitor_filter_id == 0);
  monitor_filter_id = g_dbus_connection_signal_subscribe(connection, name_owner,NULL,NULL, opt_monitor_object_path,NULL,
                                                         G_DBUS_SIGNAL_FLAGS_NONE, monitor_signal_cb,NULL,NULL);
}
static void monitor_on_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data) {
  g_print ("The name %s does not have an owner\n", name);
  if (monitor_filter_id != 0) {
      g_dbus_connection_signal_unsubscribe(connection, monitor_filter_id);
      monitor_filter_id = 0;
  }
}
static const GOptionEntry monitor_entries[] = {
  { "dest", 'd', 0, G_OPTION_ARG_STRING, &opt_monitor_dest, N_("Destination name to monitor"), NULL},
  { "object-path", 'o', 0, G_OPTION_ARG_STRING, &opt_monitor_object_path, N_("Object path to monitor"), NULL},
  { NULL }
};
static gboolean handle_monitor(gint *argc, gchar **argv[], gboolean request_completion, const gchar *completion_cur, const gchar *completion_prev) {
  gint ret;
  GOptionContext *o;
  gchar *s;
  GError *error;
  GDBusConnection *c;
  GVariant *result;
  GDBusNodeInfo *node;
  gboolean complete_names;
  gboolean complete_paths;
  GMainLoop *loop;
  ret = FALSE;
  c = NULL;
  node = NULL;
  result = NULL;
  modify_argv0_for_command(argc, argv, "monitor");
  o = g_option_context_new(NULL);
  if (request_completion) g_option_context_set_ignore_unknown_options(o, TRUE);
  g_option_context_set_help_enabled(o, FALSE);
  g_option_context_set_summary(o,"Monitor a remote object.");
  g_option_context_add_main_entries(o, monitor_entries, GETTEXT_PACKAGE);
  g_option_context_add_group(o, connection_get_group ());
  complete_names = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--dest") == 0) {
      complete_names = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  complete_paths = FALSE;
  if (request_completion && *argc > 1 && g_strcmp0((*argv)[(*argc)-1], "--object-path") == 0) {
      complete_paths = TRUE;
      remove_arg((*argc) - 1, argc, argv);
  }
  if (!g_option_context_parse(o, argc, argv, NULL)) {
      if (!request_completion) {
          s = g_option_context_get_help(o, FALSE, NULL);
          g_printerr("%s", s);
          g_free(s);
          goto out;
      }
  }
  error = NULL;
  c = connection_get_dbus_connection(&error);
  if (c == NULL) {
      if (request_completion) {
          if (g_strcmp0(completion_prev, "--address") == 0) g_print("unix:\ntcp:\nnonce-tcp:\n");
          else  g_print("--system \n--session \n--address \n");
      } else {
          g_printerr("Error connecting: %s\n", error->message);
          g_error_free(error);
      }
      goto out;
  }
  if (g_dbus_connection_get_unique_name(c) != NULL) {
      if (complete_names) {
          print_names(c, FALSE);
          goto out;
      }
      if (opt_monitor_dest == NULL) {
          if (request_completion) g_print("--dest \n");
          else g_printerr("Error: Destination is not specified\n");
          goto out;
      }
      if (request_completion && g_strcmp0("--dest", completion_prev) == 0) {
          print_names(c, g_str_has_prefix(opt_monitor_dest, ":"));
          goto out;
      }
  }
  if (complete_paths) {
      print_paths(c, opt_monitor_dest, "/");
      goto out;
  }
  if (opt_monitor_object_path == NULL) {
      if (request_completion) {
          g_print("--object-path \n");
          goto out;
      }
  }
  if (request_completion && g_strcmp0("--object-path", completion_prev) == 0) {
      gchar *p;
      s = g_strdup(opt_monitor_object_path);
      p = strrchr(s, '/');
      if (p != NULL) {
          if (p == s) p++;
          *p = '\0';
      }
      print_paths(c, opt_monitor_dest, s);
      g_free(s);
      goto out;
  }
  if (!request_completion && (opt_monitor_object_path != NULL && !g_variant_is_object_path(opt_monitor_object_path))) {
      g_printerr("Error: %s is not a valid object path\n", opt_monitor_object_path);
      goto out;
  }
  if (request_completion) goto out;
  if (opt_monitor_object_path != NULL) g_print("Monitoring signals on object %s owned by %s\n", opt_monitor_object_path, opt_monitor_dest);
  else g_print("Monitoring signals from all objects owned by %s\n", opt_monitor_dest);
  loop = g_main_loop_new(NULL, FALSE);
  g_bus_watch_name_on_connection(c, opt_monitor_dest,G_BUS_NAME_WATCHER_FLAGS_AUTO_START, monitor_on_name_appeared, monitor_on_name_vanished,NULL,
                                 NULL);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
  ret = TRUE;
out:
  if (node != NULL) g_dbus_node_info_unref(node);
  if (result != NULL) g_variant_unref(result);
  if (c != NULL) g_object_unref(c);
  g_option_context_free(o);
  return ret;
}
static gchar *pick_word_at(const gchar *s, gint cursor, gint *out_word_begins_at) {
  gint begin;
  gint end;
  if (s[0] == '\0') {
      if (out_word_begins_at != NULL) *out_word_begins_at = -1;
      return NULL;
  }
  if (g_ascii_isspace(s[cursor]) && ((cursor > 0 && g_ascii_isspace(s[cursor-1])) || cursor == 0)) {
      if (out_word_begins_at != NULL) *out_word_begins_at = cursor;
      return g_strdup("");
  }
  while(!g_ascii_isspace(s[cursor - 1]) && cursor > 0) cursor--;
  begin = cursor;
  end = begin;
  while(!g_ascii_isspace(s[end]) && s[end] != '\0') end++;
  if (out_word_begins_at != NULL) *out_word_begins_at = begin;
  return g_strndup(s + begin, end - begin);
}
gint main(gint argc, gchar *argv[]) {
  gint ret;
  const gchar *command;
  gboolean request_completion;
  gchar *completion_cur;
  gchar *completion_prev;
  setlocale(LC_ALL, "");
  textdomain(GETTEXT_PACKAGE);
#ifndef G_OS_WIN32
  extern gchar *_glib_get_locale_dir(void);
  gchar *tmp = _glib_get_locale_dir();
  bindtextdomain(GETTEXT_PACKAGE, tmp);
  g_free(tmp);
#else
  bindtextdomain(GETTEXT_PACKAGE, GLIB_LOCALE_DIR);
#endif
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
  ret = 1;
  completion_cur = NULL;
  completion_prev = NULL;
  g_type_init();
  if (argc < 2) {
      usage(&argc, &argv, FALSE);
      goto out;
  }
  request_completion = FALSE;
again:
  command = argv[1];
  if (g_strcmp0(command, "help") == 0) {
      if (request_completion);
      else {
          usage(&argc, &argv, TRUE);
          ret = 0;
      }
      goto out;
  } else if (g_strcmp0(command, "call") == 0) {
      if (handle_call(&argc, &argv, request_completion, completion_cur, completion_prev)) ret = 0;
      goto out;
  } else if (g_strcmp0(command, "introspect") == 0) {
      if (handle_introspect(&argc, &argv, request_completion, completion_cur, completion_prev)) ret = 0;
      goto out;
  } else if (g_strcmp0(command, "monitor") == 0) {
      if (handle_monitor(&argc, &argv, request_completion, completion_cur, completion_prev)) ret = 0;
      goto out;
  } else if (g_strcmp0(command, "complete") == 0 && argc == 4 && !request_completion) {
      const gchar *completion_line;
      gchar **completion_argv;
      gint completion_argc;
      gint completion_point;
      gchar *endp;
      gint cur_begin;
      request_completion = TRUE;
      completion_line = argv[2];
      completion_point = strtol(argv[3], &endp, 10);
      if (endp == argv[3] || *endp != '\0') goto out;
  #if 0
      completion_debug("completion_point=%d", completion_point);
      completion_debug("----");
      completion_debug(" 0123456789012345678901234567890123456789012345678901234567890123456789");
      completion_debug("`%s'", completion_line);
      completion_debug(" %*s^", completion_point, "");
      completion_debug("----");
  #endif
      if (!g_shell_parse_argv(completion_line, &completion_argc, &completion_argv,NULL)) goto out;
      completion_prev = NULL;
      completion_cur = pick_word_at(completion_line, completion_point, &cur_begin);
      if (cur_begin > 0) {
          gint prev_end;
          for (prev_end = cur_begin - 1; prev_end >= 0; prev_end--) {
              if (!g_ascii_isspace(completion_line[prev_end])) {
                  completion_prev = pick_word_at (completion_line, prev_end, NULL);
                  break;
              }
          }
      }
  #if 0
      completion_debug(" cur=`%s'", completion_cur);
      completion_debug("prev=`%s'", completion_prev);
  #endif
      argc = completion_argc;
      argv = completion_argv;
      ret = 0;
      goto again;
  } else {
      if (request_completion) {
          g_print("help \ncall \nintrospect \nmonitor \n");
          ret = 0;
          goto out;
      } else {
          g_printerr("Unknown command `%s'\n", command);
          usage(&argc, &argv, FALSE);
          goto out;
      }
  }
out:
  g_free(completion_cur);
  g_free(completion_prev);
  return ret;
}