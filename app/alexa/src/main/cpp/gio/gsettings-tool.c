#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include "../glib/gi18n.h"
#include "../pulse_audio/pulsecore/i18n.h"
#include "config.h"
#include "gio.h"

static gboolean contained(const gchar * const *items, const gchar *item) {
  while(*items)
      if (strcmp(*items++, item) == 0) return TRUE;
  return FALSE;
}
static gboolean is_schema(const gchar *schema) {
  return contained(g_settings_list_schemas(), schema);
}
static gboolean is_relocatable_schema(const gchar *schema) {
  return contained(g_settings_list_relocatable_schemas(), schema);
}
static gboolean check_relocatable_schema(const gchar *schema) {
  if (is_relocatable_schema(schema)) return TRUE;
  if (is_schema(schema)) g_printerr("Schema '%s' is not relocatable (path must not be specified)\n", schema);
  else g_printerr("No such schema '%s'\n", schema);
  return FALSE;
}
static gboolean check_schema(const gchar *schema) {
  if (is_schema(schema)) return TRUE;
  if (is_relocatable_schema(schema)) g_printerr ("Schema '%s' is relocatable (path must be specified)\n", schema);
  else g_printerr("No such schema '%s'\n", schema);
  return FALSE;
}
static gboolean check_path(const gchar *path) {
  if (path[0] == '\0') {
      g_printerr("Empty path given.\n");
      return FALSE;
  }
  if (path[0] != '/') {
      g_printerr("Path must begin with a slash (/)\n");
      return FALSE;
  }
  if (!g_str_has_suffix(path, "/")) {
      g_printerr("Path must end with a slash (/)\n");
      return FALSE;
  }
  if (strstr(path, "//")) {
      g_printerr("Path must not contain two adjacent slashes (//)\n");
      return FALSE;
  }
  return TRUE;
}
static gboolean check_key(GSettings *settings, const gchar *key) {
  gboolean good;
  gchar **keys;
  keys = g_settings_list_keys(settings);
  good = contained((const gchar**)keys, key);
  g_strfreev(keys);
  if (good) return TRUE;
  g_printerr("No such key '%s'\n", key);
  return FALSE;
}
static void output_list(const gchar * const *list) {
  gint i;
  for (i = 0; list[i]; i++) g_print("%s\n", list[i]);
}
static void gsettings_list_schemas(GSettings *settings, const gchar *key, const gchar *value) {
  output_list(g_settings_list_schemas());
}
static void gsettings_list_relocatable_schemas(GSettings *settings, const gchar *key, const gchar *value) {
  output_list(g_settings_list_relocatable_schemas());
}
static void gsettings_list_keys(GSettings *settings, const gchar *key, const gchar *value) {
  gchar **keys;
  keys = g_settings_list_keys(settings);
  output_list((const gchar**)keys);
  g_strfreev(keys);
}
static void gsettings_list_children(GSettings *settings, const gchar *key, const gchar *value) {
  gchar **children;
  gint max = 0;
  gint i;
  children = g_settings_list_children(settings);
  for (i = 0; children[i]; i++)
      if (strlen (children[i]) > max) max = strlen(children[i]);
  for (i = 0; children[i]; i++) {
      GSettings *child;
      gchar *schema;
      gchar *path;
      child = g_settings_get_child(settings, children[i]);
      g_object_get(child,"schema", &schema, "path", &path, NULL);
      if (is_schema(schema)) g_print("%-*s   %s\n", max, children[i], schema);
      else g_print("%-*s   %s:%s\n", max, children[i], schema, path);
      g_object_unref(child);
      g_free(schema);
      g_free(path);
  }
  g_strfreev(children);
}
static void enumerate(GSettings *settings) {
  gchar **keys;
  gchar *schema;
  gint i;
  g_object_get(settings, "schema", &schema, NULL);
  keys = g_settings_list_keys(settings);
  for (i = 0; keys[i]; i++) {
      GVariant *value;
      gchar *printed;
      value = g_settings_get_value(settings, keys[i]);
      printed = g_variant_print(value, TRUE);
      g_print("%s %s %s\n", schema, keys[i], printed);
      g_variant_unref(value);
      g_free(printed);
  }
  g_free(schema);
  g_strfreev(keys);
}
static void gsettings_list_recursively(GSettings *settings, const gchar *key, const gchar *value) {
  if (settings) {
      gchar **children;
      gint i;
      enumerate(settings);
      children = g_settings_list_children(settings);
      for (i = 0; children[i]; i++) {
          GSettings *child;
          gchar *schema;
          child = g_settings_get_child(settings, children[i]);
          g_object_get(child, "schema", &schema, NULL);
          if (is_schema(schema)) enumerate(child);
          g_object_unref(child);
          g_free(schema);
      }
      g_strfreev(children);
  } else {
      const gchar * const *schemas;
      gint i;
      schemas = g_settings_list_schemas();
      for (i = 0; schemas[i]; i++) {
          settings = g_settings_new(schemas[i]);
          enumerate(settings);
          g_object_unref(settings);
      }
  }
}
static void gsettings_range(GSettings *settings, const gchar *key, const gchar *value) {
  GVariant *range, *detail;
  const gchar *type;
  range = g_settings_get_range(settings, key);
  g_variant_get(range, "(&sv)", &type, &detail);
  if (strcmp(type, "type") == 0) g_print("type %s\n", g_variant_get_type_string(detail) + 1);
  else if (strcmp(type, "range") == 0) {
      GVariant *min, *max;
      gchar *smin, *smax;
      g_variant_get(detail, "(**)", &min, &max);
      smin = g_variant_print(min, FALSE);
      smax = g_variant_print(max, FALSE);
      g_print("range %s %s %s\n", g_variant_get_type_string(min), smin, smax);
      g_variant_unref(min);
      g_variant_unref(max);
      g_free(smin);
      g_free(smax);
  } else if (strcmp(type, "enum") == 0 || strcmp(type, "flags") == 0) {
      GVariantIter iter;
      GVariant *item;
      g_print("%s\n", type);
      g_variant_iter_init(&iter, detail);
      while (g_variant_iter_loop(&iter, "*", &item)) {
          gchar *printed;
          printed = g_variant_print(item, FALSE);
          g_print("%s\n", printed);
          g_free(printed);
      }
  }
  g_variant_unref(detail);
  g_variant_unref(range);
}
static void gsettings_get(GSettings *settings, const gchar *key, const gchar *value_) {
  GVariant *value;
  gchar *printed;
  value = g_settings_get_value(settings, key);
  printed = g_variant_print(value, TRUE);
  g_print("%s\n", printed);
  g_variant_unref(value);
  g_free(printed);
}
static void gsettings_reset(GSettings *settings, const gchar *key, const gchar *value) {
  g_settings_reset(settings, key);
  g_settings_sync();
}
static void gsettings_writable(GSettings *settings, const gchar *key, const gchar *value) {
  g_print("%s\n", g_settings_is_writable(settings, key) ? "true" : "false");
}
static void value_changed(GSettings *settings, const gchar *key, gpointer user_data) {
  GVariant *value;
  gchar *printed;
  value = g_settings_get_value(settings, key);
  printed = g_variant_print(value, TRUE);
  g_print("%s: %s\n", key, printed);
  g_variant_unref(value);
  g_free(printed);
}
static void gsettings_monitor(GSettings *settings, const gchar *key, const gchar *value) {
  if (key) {
      gchar *name;
      name = g_strdup_printf("changed::%s", key);
      g_signal_connect(settings, name, G_CALLBACK(value_changed), NULL);
  } else g_signal_connect(settings, "changed", G_CALLBACK(value_changed), NULL);
  g_main_loop_run(g_main_loop_new(NULL, FALSE));
}
static void gsettings_set(GSettings *settings, const gchar *key, const gchar *value) {
  const GVariantType *type;
  GError *error = NULL;
  GVariant *existing;
  GVariant *new;
  gchar *freeme = NULL;
  existing = g_settings_get_value(settings, key);
  type = g_variant_get_type(existing);
  new = g_variant_parse(type, value, NULL, NULL, &error);
  if (new == NULL && g_error_matches(error, G_VARIANT_PARSE_ERROR,G_VARIANT_PARSE_ERROR_UNKNOWN_KEYWORD)) {
      value = freeme = g_strdup_printf("\"%s\"", value);
      new = g_variant_parse(type, value, NULL, NULL, NULL);
      if (new != NULL) g_clear_error(&error);
  }
  if (new == NULL) {
      g_printerr("%s\n", error->message);
      exit(1);
  }
  if (!g_settings_range_check(settings, key, new)) {
      g_printerr("The provided value is outside of the valid range\n");
      g_variant_unref(new);
      exit(1);
  }
  g_settings_set_value(settings, key, new);
  g_variant_unref(existing);
  g_variant_unref(new);
  g_settings_sync();
  g_free(freeme);
}
static int gsettings_help(gboolean requested, const gchar *command) {
  const gchar *description;
  const gchar *synopsis;
  GString *string;
  string = g_string_new(NULL);
  if (command == NULL);
  else if (strcmp(command, "help") == 0) {
      description = "Print help";
      synopsis = "[COMMAND]";
  } else if (strcmp(command, "list-schemas") == 0) {
      description = "List the installed (non-relocatable) schemas";
      synopsis = "";
  } else if (strcmp(command, "list-relocatable-schemas") == 0) {
      description = "List the installed relocatable schemas";
      synopsis = "";
  } else if (strcmp(command, "list-keys") == 0) {
      description = "List the keys in SCHEMA";
      synopsis = N_("SCHEMA[:PATH]");
  } else if (strcmp(command, "list-children") == 0) {
      description = "List the children of SCHEMA";
      synopsis = N_("SCHEMA[:PATH]");
  } else if (strcmp(command, "list-recursively") == 0) {
      description = "List keys and values, recursively\nIf no SCHEMA is given, list all keys\n";
      synopsis = N_("[SCHEMA[:PATH]]");
  } else if (strcmp(command, "get") == 0) {
      description = "Get the value of KEY";
      synopsis = N_("SCHEMA[:PATH] KEY");
  } else if (strcmp(command, "range") == 0) {
      description = "Query the range of valid values for KEY";
      synopsis = N_("SCHEMA[:PATH] KEY");
  } else if (strcmp(command, "set") == 0) {
      description = "Set the value of KEY to VALUE";
      synopsis = N_("SCHEMA[:PATH] KEY VALUE");
  } else if (strcmp(command, "reset") == 0) {
      description = "Reset KEY to its default value";
      synopsis = N_("SCHEMA[:PATH] KEY");
  } else if (strcmp(command, "writable") == 0) {
      description = "Check if KEY is writable";
      synopsis = N_("SCHEMA[:PATH] KEY");
  } else if (strcmp (command, "monitor") == 0) {
      description = "Monitor KEY for changes.\nIf no KEY is specified, monitor all keys in SCHEMA.\nUse ^C to stop monitoring.\n";
      synopsis = N_("SCHEMA[:PATH] [KEY]");
  } else {
      g_string_printf(string, "Unknown command %s\n\n", command);
      requested = FALSE;
      command = NULL;
  }
  if (command == NULL) {
      g_string_append(string, "Usage:\n  gsettings COMMAND [ARGS...]\n\nCommands:\n  help                      Show this information\n             List "
                      "installed schemas\n  list-relocatable-schemas  List relocatable schemas\n  list-keys                 List keys in a schema\n  list-children"
                      "             List children of a schema\n  list-recursively          List keys and values, recursively\n  range                     "
                      "Queries the range of a key\n  get                       Get the value of a key\n  set                       Set the value of a key\n"
                      "  reset                     Reset the value of a key\n  writable                  Check if a key is writable\n  monitor                   "
                      "Watch for changes\n\nUse 'gsettings help COMMAND' to get detailed help.\n\n");
  } else {
      g_string_append_printf(string, "Usage:\n  gsettings %s %s\n\n%s\n\n"), command, synopsis[0] ? synopsis : "", description;
      if (synopsis[0]) {
          g_string_append(string, "Arguments:\n");
          if (strstr(synopsis, "[COMMAND]")) g_string_append(string, "  COMMAND   The (optional) command to explain\n");
          else if (strstr(synopsis, "SCHEMA")) {
              g_string_append(string, "  SCHEMA    The name of the schema\n  PATH      The path, for relocatable schemas\n");
          }
          if (strstr(synopsis, "[KEY]")) g_string_append(string, "  KEY       The (optional) key within the schema\n");
          else if (strstr(synopsis, "KEY")) g_string_append(string, "  KEY       The key within the schema\n");
          if (strstr(synopsis, "VALUE")) g_string_append(string, "  VALUE     The value to set\n");
          g_string_append(string, "\n");
      }
  }
  if (requested) g_print("%s", string->str);
  else g_printerr("%s\n", string->str);
  g_string_free(string, TRUE);
  return requested ? 0 : 1;
}
int main(int argc, char **argv) {
  void (* function)(GSettings *, const gchar *, const gchar *);
  GSettings *settings;
  const gchar *key;
  setlocale(LC_ALL, "");
  textdomain(GETTEXT_PACKAGE);
#ifndef G_OS_WIN32
  gchar *tmp = _glib_get_locale_dir();
  bindtextdomain(GETTEXT_PACKAGE, tmp);
  g_free(tmp);
#else
  bindtextdomain(GETTEXT_PACKAGE, GLIB_LOCALE_DIR);
#endif
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
  if (argc < 2) return gsettings_help(FALSE, NULL);
  else if (strcmp(argv[1], "help") == 0) return gsettings_help(TRUE, argv[2]);
  else if (argc == 2 && strcmp(argv[1], "list-schemas") == 0) function = gsettings_list_schemas;
  else if (argc == 2 && strcmp(argv[1], "list-relocatable-schemas") == 0) function = gsettings_list_relocatable_schemas;
  else if (argc == 3 && strcmp(argv[1], "list-keys") == 0) function = gsettings_list_keys;
  else if (argc == 3 && strcmp(argv[1], "list-children") == 0) function = gsettings_list_children;
  else if ((argc == 2 || argc == 3) && strcmp(argv[1], "list-recursively") == 0) function = gsettings_list_recursively;
  else if (argc == 4 && strcmp(argv[1], "range") == 0) function = gsettings_range;
  else if (argc == 4 && strcmp(argv[1], "get") == 0) function = gsettings_get;
  else if (argc == 5 && strcmp(argv[1], "set") == 0) function = gsettings_set;
  else if (argc == 4 && strcmp(argv[1], "reset") == 0) function = gsettings_reset;
  else if (argc == 4 && strcmp(argv[1], "writable") == 0) function = gsettings_writable;
  else if ((argc == 3 || argc == 4) && strcmp(argv[1], "monitor") == 0) function = gsettings_monitor;
  else return gsettings_help(FALSE, argv[1]);
  g_type_init ();
  if (argc > 2) {
      gchar **parts;
      if (argv[2][0] == '\0') {
          g_printerr(_("Empty schema name given\n"));
          return 1;
      }
      parts = g_strsplit(argv[2], ":", 2);
      if (parts[1]) {
          if (!check_relocatable_schema(parts[0]) || !check_path(parts[1])) return 1;
          settings = g_settings_new_with_path(parts[0], parts[1]);
      } else {
          if (!check_schema(parts[0])) return 1;
          settings = g_settings_new(parts[0]);
      }
      g_strfreev(parts);
  } else settings = NULL;
  if (argc > 3) {
      if (!check_key(settings, argv[3])) return 1;
      key = argv[3];
  } else key = NULL;
  (*function)(settings, key, argc > 4 ? argv[4] : NULL);
  if (settings != NULL) g_object_unref(settings);
  return 0;
}