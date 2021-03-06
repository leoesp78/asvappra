#include <stdlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "../gobject/gtype.h"
#include "../gobject/gboxed.h"
#include "config.h"
#include "gdbusintrospection.h"

#define _MY_DEFINE_BOXED_TYPE(TypeName, type_name)  G_DEFINE_BOXED_TYPE(TypeName, type_name, type_name##_ref, type_name##_unref)
_MY_DEFINE_BOXED_TYPE(GDBusNodeInfo, g_dbus_node_info);
_MY_DEFINE_BOXED_TYPE(GDBusInterfaceInfo, g_dbus_interface_info);
_MY_DEFINE_BOXED_TYPE(GDBusMethodInfo, g_dbus_method_info);
_MY_DEFINE_BOXED_TYPE(GDBusSignalInfo, g_dbus_signal_info);
_MY_DEFINE_BOXED_TYPE(GDBusPropertyInfo, g_dbus_property_info);
_MY_DEFINE_BOXED_TYPE(GDBusArgInfo, g_dbus_arg_info);
_MY_DEFINE_BOXED_TYPE(GDBusAnnotationInfo, g_dbus_annotation_info);
typedef struct {
  GPtrArray *args;
  GPtrArray *out_args;
  GPtrArray *methods;
  GPtrArray *signals;
  GPtrArray *properties;
  GPtrArray *interfaces;
  GPtrArray *nodes;
  GPtrArray *annotations;
  GSList *annotations_stack;
  GSList *interfaces_stack;
  GSList *nodes_stack;
  gboolean last_arg_was_in;
  guint num_args;
} ParseData;
GDBusNodeInfo *g_dbus_node_info_ref(GDBusNodeInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusInterfaceInfo *g_dbus_interface_info_ref(GDBusInterfaceInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusMethodInfo *g_dbus_method_info_ref(GDBusMethodInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusSignalInfo *g_dbus_signal_info_ref(GDBusSignalInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusPropertyInfo *g_dbus_property_info_ref(GDBusPropertyInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusArgInfo *g_dbus_arg_info_ref(GDBusArgInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
GDBusAnnotationInfo *g_dbus_annotation_info_ref(GDBusAnnotationInfo *info) {
  if (info->ref_count == -1) return info;
  g_atomic_int_inc(&info->ref_count);
  return info;
}
static void free_null_terminated_array(gpointer array, GDestroyNotify unref_func) {
  guint n;
  gpointer *p = array;
  if (p == NULL) return;
  for (n = 0; p[n] != NULL; n++) unref_func(p[n]);
  g_free(p);
}
void g_dbus_annotation_info_unref(GDBusAnnotationInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->key);
      g_free(info->value);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_arg_info_unref(GDBusArgInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->name);
      g_free(info->signature);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_method_info_unref(GDBusMethodInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->name);
      free_null_terminated_array(info->in_args, (GDestroyNotify)g_dbus_arg_info_unref);
      free_null_terminated_array(info->out_args, (GDestroyNotify)g_dbus_arg_info_unref);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_signal_info_unref(GDBusSignalInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->name);
      free_null_terminated_array(info->args, (GDestroyNotify)g_dbus_arg_info_unref);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_property_info_unref(GDBusPropertyInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->name);
      g_free(info->signature);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_interface_info_unref(GDBusInterfaceInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->name);
      free_null_terminated_array(info->methods, (GDestroyNotify)g_dbus_method_info_unref);
      free_null_terminated_array(info->signals, (GDestroyNotify)g_dbus_signal_info_unref);
      free_null_terminated_array(info->properties, (GDestroyNotify)g_dbus_property_info_unref);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
void g_dbus_node_info_unref(GDBusNodeInfo *info) {
  if (info->ref_count == -1) return;
  if (g_atomic_int_dec_and_test(&info->ref_count)) {
      g_free(info->path);
      free_null_terminated_array(info->interfaces, (GDestroyNotify)g_dbus_interface_info_unref);
      free_null_terminated_array(info->nodes, (GDestroyNotify)g_dbus_node_info_unref);
      free_null_terminated_array(info->annotations, (GDestroyNotify)g_dbus_annotation_info_unref);
      g_free(info);
  }
}
static void g_dbus_annotation_info_set(ParseData *data, GDBusAnnotationInfo *info, const gchar *key, const gchar *value, GDBusAnnotationInfo **embedded_annotations) {
  info->ref_count = 1;
  if (key != NULL) info->key = g_strdup(key);
  if (value != NULL) info->value = g_strdup(value);
  if (embedded_annotations != NULL) info->annotations = embedded_annotations;
}
static void g_dbus_arg_info_set(ParseData *data, GDBusArgInfo *info, const gchar *name, const gchar *signature, GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (name != NULL) info->name = g_strdup(name);
  if (signature != NULL) info->signature = g_strdup(signature);
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_method_info_set(ParseData *data, GDBusMethodInfo *info, const gchar *name, GDBusArgInfo **in_args, GDBusArgInfo **out_args,
                                   GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (name != NULL) info->name = g_strdup(name);
  if (in_args != NULL) info->in_args = in_args;
  if (out_args != NULL) info->out_args = out_args;
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_signal_info_set(ParseData *data, GDBusSignalInfo *info, const gchar *name, GDBusArgInfo **args, GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (name != NULL) info->name = g_strdup(name);
  if (args != NULL) info->args = args;
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_property_info_set(ParseData *data, GDBusPropertyInfo *info, const gchar *name, const gchar *signature, GDBusPropertyInfoFlags flags,
                                     GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (name != NULL) info->name = g_strdup(name);
  if (flags != G_DBUS_PROPERTY_INFO_FLAGS_NONE) info->flags = flags;
  if (signature != NULL) info->signature = g_strdup(signature);
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_interface_info_set(ParseData *data, GDBusInterfaceInfo *info, const gchar *name, GDBusMethodInfo **methods, GDBusSignalInfo **signals,
                                      GDBusPropertyInfo **properties, GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (name != NULL) info->name = g_strdup(name);
  if (methods != NULL) info->methods = methods;
  if (signals != NULL) info->signals = signals;
  if (properties != NULL) info->properties = properties;
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_node_info_set(ParseData *data, GDBusNodeInfo *info, const gchar *path, GDBusInterfaceInfo **interfaces, GDBusNodeInfo **nodes,
                                 GDBusAnnotationInfo **annotations) {
  info->ref_count = 1;
  if (path != NULL) info->path = g_strdup(path);
  if (interfaces != NULL) info->interfaces = interfaces;
  if (nodes != NULL) info->nodes = nodes;
  if (annotations != NULL) info->annotations = annotations;
}
static void g_dbus_annotation_info_generate_xml(GDBusAnnotationInfo *info, guint indent, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<annotation name=\"%s\" value=\"%s\"", indent, "", info->key, info->value);
  if (info->annotations == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      g_string_append_printf(string_builder, "%*s</annotation>\n", indent, "");
  }
}
static void g_dbus_arg_info_generate_xml(GDBusArgInfo *info, guint indent, const gchar *extra_attributes, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<arg type=\"%s\"", indent, "", info->signature);
  if (info->name != NULL) g_string_append_printf(string_builder, " name=\"%s\"", info->name);
  if (extra_attributes != NULL) g_string_append_printf(string_builder, " %s", extra_attributes);
  if (info->annotations == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      g_string_append_printf(string_builder, "%*s</arg>\n", indent, "");
  }
}
static void g_dbus_method_info_generate_xml(GDBusMethodInfo *info, guint indent, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<method name=\"%s\"", indent, "", info->name);
  if (info->annotations == NULL && info->in_args == NULL && info->out_args == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      for (n = 0; info->in_args != NULL && info->in_args[n] != NULL; n++)
          g_dbus_arg_info_generate_xml(info->in_args[n], indent + 2, "direction=\"in\"", string_builder);
      for (n = 0; info->out_args != NULL && info->out_args[n] != NULL; n++)
          g_dbus_arg_info_generate_xml(info->out_args[n],indent + 2,"direction=\"out\"", string_builder);
      g_string_append_printf(string_builder, "%*s</method>\n", indent, "");
  }
}
static void g_dbus_signal_info_generate_xml(GDBusSignalInfo *info, guint indent, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<signal name=\"%s\"", indent, "", info->name);
  if (info->annotations == NULL && info->args == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      for (n = 0; info->args != NULL && info->args[n] != NULL; n++)
          g_dbus_arg_info_generate_xml(info->args[n],indent + 2,NULL, string_builder);
      g_string_append_printf(string_builder, "%*s</signal>\n", indent, "");
  }
}
static void g_dbus_property_info_generate_xml(GDBusPropertyInfo *info, guint indent, GString *string_builder) {
  guint n;
  const gchar *access_string;
  if ((info->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE) && (info->flags & G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE)) access_string = "readwrite";
  else if (info->flags & G_DBUS_PROPERTY_INFO_FLAGS_READABLE) access_string = "read";
  else if (info->flags & G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE) access_string = "write";
  else { g_assert_not_reached(); }
  g_string_append_printf(string_builder, "%*s<property type=\"%s\" name=\"%s\" access=\"%s\"", indent, "", info->signature, info->name, access_string);
  if (info->annotations == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      g_string_append_printf(string_builder, "%*s</property>\n", indent, "");
  }
}
void g_dbus_interface_info_generate_xml(GDBusInterfaceInfo *info, guint indent, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<interface name=\"%s\">\n", indent, "", info->name);
  for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
      g_dbus_annotation_info_generate_xml(info->annotations[n], indent + 2, string_builder);
  for (n = 0; info->methods != NULL && info->methods[n] != NULL; n++)
      g_dbus_method_info_generate_xml(info->methods[n], indent + 2, string_builder);
  for (n = 0; info->signals != NULL && info->signals[n] != NULL; n++)
      g_dbus_signal_info_generate_xml(info->signals[n],indent + 2, string_builder);
  for (n = 0; info->properties != NULL && info->properties[n] != NULL; n++)
      g_dbus_property_info_generate_xml(info->properties[n],indent + 2, string_builder);
  g_string_append_printf(string_builder, "%*s</interface>\n", indent, "");
}
void g_dbus_node_info_generate_xml(GDBusNodeInfo *info, guint indent, GString *string_builder) {
  guint n;
  g_string_append_printf(string_builder, "%*s<node", indent, "");
  if (info->path != NULL) g_string_append_printf(string_builder, " name=\"%s\"", info->path);
  if (info->interfaces == NULL && info->nodes == NULL && info->annotations == NULL) g_string_append(string_builder, "/>\n");
  else {
      g_string_append(string_builder, ">\n");
      for (n = 0; info->annotations != NULL && info->annotations[n] != NULL; n++)
          g_dbus_annotation_info_generate_xml(info->annotations[n],indent + 2, string_builder);
      for (n = 0; info->interfaces != NULL && info->interfaces[n] != NULL; n++)
          g_dbus_interface_info_generate_xml(info->interfaces[n],indent + 2, string_builder);
      for (n = 0; info->nodes != NULL && info->nodes[n] != NULL; n++)
          g_dbus_node_info_generate_xml(info->nodes[n],indent + 2, string_builder);
      g_string_append_printf(string_builder, "%*s</node>\n", indent, "");
  }
}
static GDBusAnnotationInfo **parse_data_steal_annotations(ParseData *data, guint *out_num_elements) {
  GDBusAnnotationInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->annotations->len;
  if (data->annotations == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->annotations, NULL);
      ret = (GDBusAnnotationInfo**) g_ptr_array_free(data->annotations, FALSE);
  }
  data->annotations = g_ptr_array_new();
  return ret;
}
static GDBusArgInfo **parse_data_steal_args(ParseData *data, guint *out_num_elements) {
  GDBusArgInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->args->len;
  if (data->args == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->args, NULL);
      ret = (GDBusArgInfo**) g_ptr_array_free(data->args, FALSE);
  }
  data->args = g_ptr_array_new();
  return ret;
}
static GDBusArgInfo **parse_data_steal_out_args(ParseData *data, guint *out_num_elements) {
  GDBusArgInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->out_args->len;
  if (data->out_args == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->out_args, NULL);
      ret = (GDBusArgInfo**) g_ptr_array_free(data->out_args, FALSE);
  }
  data->out_args = g_ptr_array_new();
  return ret;
}
static GDBusMethodInfo **parse_data_steal_methods(ParseData *data, guint *out_num_elements) {
  GDBusMethodInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->methods->len;
  if (data->methods == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->methods, NULL);
      ret = (GDBusMethodInfo**)g_ptr_array_free(data->methods, FALSE);
  }
  data->methods = g_ptr_array_new();
  return ret;
}
static GDBusSignalInfo **parse_data_steal_signals(ParseData *data, guint *out_num_elements) {
  GDBusSignalInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->signals->len;
  if (data->signals == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->signals, NULL);
      ret = (GDBusSignalInfo**)g_ptr_array_free(data->signals, FALSE);
  }
  data->signals = g_ptr_array_new();
  return ret;
}
static GDBusPropertyInfo **parse_data_steal_properties(ParseData *data, guint *out_num_elements) {
  GDBusPropertyInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->properties->len;
  if (data->properties == NULL) ret = NULL;
  else {
      g_ptr_array_add (data->properties, NULL);
      ret = (GDBusPropertyInfo**)g_ptr_array_free(data->properties, FALSE);
  }
  data->properties = g_ptr_array_new();
  return ret;
}
static GDBusInterfaceInfo **parse_data_steal_interfaces(ParseData *data, guint *out_num_elements) {
  GDBusInterfaceInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->interfaces->len;
  if (data->interfaces == NULL) ret = NULL;
  else {
      g_ptr_array_add(data->interfaces, NULL);
      ret = (GDBusInterfaceInfo**)g_ptr_array_free(data->interfaces, FALSE);
  }
  data->interfaces = g_ptr_array_new();
  return ret;
}
static GDBusNodeInfo **parse_data_steal_nodes(ParseData *data, guint *out_num_elements) {
  GDBusNodeInfo **ret;
  if (out_num_elements != NULL) *out_num_elements = data->nodes->len;
  if (data->nodes == NULL) ret = NULL;
  else {
      g_ptr_array_add (data->nodes, NULL);
      ret = (GDBusNodeInfo**)g_ptr_array_free(data->nodes, FALSE);
  }
  data->nodes = g_ptr_array_new();
  return ret;
}
static void parse_data_free_annotations(ParseData *data) {
  if (data->annotations == NULL) return;
  g_ptr_array_foreach(data->annotations, (GFunc)g_dbus_annotation_info_unref, NULL);
  g_ptr_array_free(data->annotations, TRUE);
  data->annotations = NULL;
}
static void parse_data_free_args(ParseData *data) {
  if (data->args == NULL) return;
  g_ptr_array_foreach(data->args, (GFunc)g_dbus_arg_info_unref, NULL);
  g_ptr_array_free(data->args, TRUE);
  data->args = NULL;
}
static void parse_data_free_out_args(ParseData *data) {
  if (data->out_args == NULL) return;
  g_ptr_array_foreach(data->out_args, (GFunc)g_dbus_arg_info_unref, NULL);
  g_ptr_array_free(data->out_args, TRUE);
  data->out_args = NULL;
}
static void parse_data_free_methods(ParseData *data) {
  if (data->methods == NULL) return;
  g_ptr_array_foreach(data->methods, (GFunc)g_dbus_method_info_unref, NULL);
  g_ptr_array_free(data->methods, TRUE);
  data->methods = NULL;
}
static void parse_data_free_signals(ParseData *data) {
  if (data->signals == NULL) return;
  g_ptr_array_foreach(data->signals, (GFunc)g_dbus_signal_info_unref, NULL);
  g_ptr_array_free(data->signals, TRUE);
  data->signals = NULL;
}
static void parse_data_free_properties(ParseData *data) {
  if (data->properties == NULL) return;
  g_ptr_array_foreach(data->properties, (GFunc)g_dbus_property_info_unref, NULL);
  g_ptr_array_free(data->properties, TRUE);
  data->properties = NULL;
}
static void parse_data_free_interfaces(ParseData *data) {
  if (data->interfaces == NULL) return;
  g_ptr_array_foreach(data->interfaces, (GFunc)g_dbus_interface_info_unref, NULL);
  g_ptr_array_free(data->interfaces, TRUE);
  data->interfaces = NULL;
}
static void parse_data_free_nodes(ParseData *data) {
  if (data->nodes == NULL) return;
  g_ptr_array_foreach(data->nodes, (GFunc)g_dbus_node_info_unref, NULL);
  g_ptr_array_free(data->nodes, TRUE);
  data->nodes = NULL;
}
static GDBusAnnotationInfo *parse_data_get_annotation(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->annotations, g_new0(GDBusAnnotationInfo, 1));
  return data->annotations->pdata[data->annotations->len - 1];
}
static GDBusArgInfo *parse_data_get_arg(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->args, g_new0(GDBusArgInfo, 1));
  return data->args->pdata[data->args->len - 1];
}
static GDBusArgInfo *parse_data_get_out_arg(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->out_args, g_new0(GDBusArgInfo, 1));
  return data->out_args->pdata[data->out_args->len - 1];
}
static GDBusMethodInfo *parse_data_get_method(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->methods, g_new0(GDBusMethodInfo, 1));
  return data->methods->pdata[data->methods->len - 1];
}
static GDBusSignalInfo *parse_data_get_signal(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->signals, g_new0(GDBusSignalInfo, 1));
  return data->signals->pdata[data->signals->len - 1];
}
static GDBusPropertyInfo *parse_data_get_property(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->properties, g_new0(GDBusPropertyInfo, 1));
  return data->properties->pdata[data->properties->len - 1];
}
static GDBusInterfaceInfo *parse_data_get_interface(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->interfaces, g_new0(GDBusInterfaceInfo, 1));
  return data->interfaces->pdata[data->interfaces->len - 1];
}
static GDBusNodeInfo *parse_data_get_node(ParseData *data, gboolean create_new) {
  if (create_new) g_ptr_array_add(data->nodes, g_new0(GDBusNodeInfo, 1));
  return data->nodes->pdata[data->nodes->len - 1];
}
static ParseData *parse_data_new(void) {
  ParseData *data;
  data = g_new0(ParseData, 1);
  parse_data_steal_annotations(data,NULL);
  parse_data_steal_args(data,NULL);
  parse_data_steal_out_args(data,NULL);
  parse_data_steal_methods(data,NULL);
  parse_data_steal_signals(data,NULL);
  parse_data_steal_properties(data,NULL);
  parse_data_steal_interfaces(data,NULL);
  parse_data_steal_nodes(data,NULL);
  return data;
}
static void parse_data_free(ParseData *data) {
  GSList *l;
  for (l = data->annotations_stack; l != NULL; l = l->next) {
      GPtrArray *annotations = l->data;
      g_ptr_array_foreach(annotations, (GFunc)g_dbus_annotation_info_unref,NULL);
      g_ptr_array_free(annotations, TRUE);
  }
  g_slist_free(data->annotations_stack);
  for (l = data->interfaces_stack; l != NULL; l = l->next) {
      GPtrArray *interfaces = l->data;
      g_ptr_array_foreach(interfaces, (GFunc)g_dbus_interface_info_unref,NULL);
      g_ptr_array_free(interfaces, TRUE);
  }
  g_slist_free(data->interfaces_stack);
  for (l = data->nodes_stack; l != NULL; l = l->next) {
      GPtrArray *nodes = l->data;
      g_ptr_array_foreach(nodes, (GFunc)g_dbus_node_info_unref, NULL);
      g_ptr_array_free(nodes, TRUE);
  }
  g_slist_free(data->nodes_stack);
  parse_data_free_args(data);
  parse_data_free_out_args(data);
  parse_data_free_methods(data);
  parse_data_free_signals(data);
  parse_data_free_properties(data);
  parse_data_free_interfaces(data);
  parse_data_free_annotations(data);
  parse_data_free_nodes(data);
  g_free(data);
}
static void parser_start_element(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values,
                                 gpointer user_data, GError **error) {
  ParseData *data = user_data;
  GSList *stack;
  const gchar *name;
  const gchar *type;
  const gchar *access;
  const gchar *direction;
  const gchar *value;
  name = NULL;
  type = NULL;
  access = NULL;
  direction = NULL;
  value = NULL;
  stack = (GSList*)g_markup_parse_context_get_element_stack(context);
  if (strcmp(element_name, "node") == 0) {
      if (!(g_slist_length(stack) >= 1 || strcmp(stack->next->data, "node") != 0)) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<node> elements can only be top-level or embedded in other "
                              "<node> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL,
                              "name", &name, G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "xmlns:doc", NULL, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      g_dbus_node_info_set(data,parse_data_get_node(data, TRUE), name,NULL,NULL,NULL);
      data->interfaces_stack = g_slist_prepend(data->interfaces_stack, data->interfaces);
      data->interfaces = NULL;
      parse_data_steal_interfaces(data, NULL);
      data->nodes_stack = g_slist_prepend(data->nodes_stack, data->nodes);
      data->nodes = NULL;
      parse_data_steal_nodes(data, NULL);

  } else if (strcmp(element_name, "interface") == 0) {
      if (g_slist_length(stack) < 2 || strcmp(stack->next->data, "node") != 0) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<interface> elements can only be embedded in <node> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING,"name", &name,
                                       G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "version", NULL, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      g_dbus_interface_info_set(data,parse_data_get_interface(data, TRUE), name,NULL,NULL,NULL,NULL);

  } else if (strcmp(element_name, "method") == 0) {
      if (g_slist_length(stack) < 2 || strcmp(stack->next->data, "interface") != 0) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<method> elements can only be embedded in <interface> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING, "name", &name,
                                       G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "version", NULL, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      g_dbus_method_info_set(data,parse_data_get_method(data, TRUE), name,NULL,NULL,NULL);
      data->num_args = 0;

  } else if (strcmp (element_name, "signal") == 0) {
      if (g_slist_length(stack) < 2 || strcmp(stack->next->data, "interface") != 0) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<signal> elements can only be embedded in <interface> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING,"name", &name,
                                       G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      g_dbus_signal_info_set(data,parse_data_get_signal(data, TRUE), name,NULL,NULL);
      data->num_args = 0;
  } else if (strcmp (element_name, "property") == 0) {
      GDBusPropertyInfoFlags flags;
      if (g_slist_length(stack) < 2 || strcmp (stack->next->data, "interface") != 0) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<property> elements can only be embedded in <interface> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING, "name", &name,
                                       G_MARKUP_COLLECT_STRING, "type", &type, G_MARKUP_COLLECT_STRING, "access", &access, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      if (strcmp (access, "read") == 0) flags = G_DBUS_PROPERTY_INFO_FLAGS_READABLE;
      else if (strcmp (access, "write") == 0) flags = G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE;
      else if (strcmp (access, "readwrite") == 0) flags = G_DBUS_PROPERTY_INFO_FLAGS_READABLE | G_DBUS_PROPERTY_INFO_FLAGS_WRITABLE;
      else {
          g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,"Unknown value '%s' of access attribute for element <property>", access);
          return;
      }
      g_dbus_property_info_set(data,parse_data_get_property(data, TRUE), name, type, flags,NULL);
  } else if (strcmp (element_name, "arg") == 0) {
      gboolean is_in;
      gchar *name_to_use;
      if (g_slist_length (stack) < 2 || (strcmp (stack->next->data, "method") != 0 && strcmp (stack->next->data, "signal") != 0)) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<arg> elements can only be embedded in <method> or <signal>"
                              " elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL,
                              "name", &name, G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "direction", &direction, G_MARKUP_COLLECT_STRING, "type",
                                       &type, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      if (strcmp(stack->next->data, "method") == 0) is_in = TRUE;
      else is_in = FALSE;
      if (direction != NULL) {
          if (strcmp(direction, "in") == 0) is_in = TRUE;
          else if (strcmp(direction, "out") == 0) is_in = FALSE;
          else {
              g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, "Unknown value '%s' of direction attribute", direction);
              return;
          }
      }
      if (is_in && strcmp(stack->next->data, "signal") == 0) {
          g_set_error_literal(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, "Only direction 'out' is allowed for <arg> elements embedded in <signal>");
          return;
      }
      if (name == NULL) name_to_use = g_strdup_printf("arg_%d", data->num_args);
      else name_to_use = g_strdup(name);
      data->num_args++;
      if (is_in) {
          g_dbus_arg_info_set(data,parse_data_get_arg(data, TRUE), name_to_use, type,NULL);
          data->last_arg_was_in = TRUE;
      } else {
          g_dbus_arg_info_set(data,parse_data_get_out_arg(data, TRUE), name_to_use, type,NULL);
          data->last_arg_was_in = FALSE;
      }
      g_free(name_to_use);
  } else if (strcmp(element_name, "annotation") == 0) {
      if (g_slist_length(stack) < 2 || (strcmp(stack->next->data, "node") != 0 && strcmp(stack->next->data, "interface") != 0 &&
          strcmp(stack->next->data, "signal") != 0 && strcmp(stack->next->data, "method") != 0 && strcmp(stack->next->data, "property") != 0 &&
          strcmp(stack->next->data, "arg") != 0 && strcmp(stack->next->data, "annotation") != 0)) {
          g_set_error_literal(error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"<annotation> elements can only be embedded in <node>, "
                              "<interface>, <signal>, <method>, <property>, <arg> or <annotation> elements");
          return;
      }
      if (!g_markup_collect_attributes(element_name, attribute_names, attribute_values, error,G_MARKUP_COLLECT_STRING, "name", &name,
                                       G_MARKUP_COLLECT_STRING, "value", &value, G_MARKUP_COLLECT_INVALID)) {
          return;
      }
      g_dbus_annotation_info_set(data,parse_data_get_annotation(data, TRUE), name, value,NULL);
  }
  data->annotations_stack = g_slist_prepend(data->annotations_stack, data->annotations);
  data->annotations = NULL;
  parse_data_steal_annotations(data, NULL);
}
static GDBusAnnotationInfo **steal_annotations(ParseData *data) {
  return parse_data_steal_annotations(data, NULL);
}
static void parser_end_element(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error) {
  ParseData *data = user_data;
  gboolean have_popped_annotations;
  have_popped_annotations = FALSE;
  if (strcmp(element_name, "node") == 0) {
      guint num_nodes;
      guint num_interfaces;
      GDBusNodeInfo **nodes;
      GDBusInterfaceInfo **interfaces;
      nodes = parse_data_steal_nodes(data, &num_nodes);
      interfaces = parse_data_steal_interfaces(data, &num_interfaces);
      parse_data_free_interfaces(data);
      data->interfaces = (GPtrArray*)data->interfaces_stack->data;
      data->interfaces_stack = g_slist_remove(data->interfaces_stack, data->interfaces_stack->data);
      parse_data_free_nodes(data);
      data->nodes = (GPtrArray*)data->nodes_stack->data;
      data->nodes_stack = g_slist_remove(data->nodes_stack, data->nodes_stack->data);
      g_dbus_node_info_set(data,parse_data_get_node(data,FALSE),NULL, interfaces, nodes, steal_annotations(data));
  } else if (strcmp(element_name, "interface") == 0) {
      guint num_methods;
      guint num_signals;
      guint num_properties;
      GDBusMethodInfo **methods;
      GDBusSignalInfo **signals;
      GDBusPropertyInfo **properties;
      methods = parse_data_steal_methods(data, &num_methods);
      signals = parse_data_steal_signals(data, &num_signals);
      properties = parse_data_steal_properties(data, &num_properties);
      g_dbus_interface_info_set(data, parse_data_get_interface(data, FALSE), NULL, methods, signals, properties, steal_annotations(data));
  } else if (strcmp(element_name, "method") == 0) {
      guint in_num_args;
      guint out_num_args;
      GDBusArgInfo **in_args;
      GDBusArgInfo **out_args;
      in_args  = parse_data_steal_args(data, &in_num_args);
      out_args = parse_data_steal_out_args(data, &out_num_args);
      g_dbus_method_info_set(data, parse_data_get_method(data,FALSE),NULL, in_args, out_args, steal_annotations(data));
  } else if (strcmp(element_name, "signal") == 0) {
      guint num_args;
      GDBusArgInfo **args;
      args = parse_data_steal_out_args(data, &num_args);
      g_dbus_signal_info_set(data,parse_data_get_signal(data,FALSE),NULL, args, steal_annotations(data));
  } else if (strcmp(element_name, "property") == 0) {
      g_dbus_property_info_set(data,parse_data_get_property(data, FALSE),NULL,NULL,G_DBUS_PROPERTY_INFO_FLAGS_NONE,
                               steal_annotations(data));
  } else if (strcmp(element_name, "arg") == 0) {
      g_dbus_arg_info_set(data,data->last_arg_was_in ? parse_data_get_arg(data, FALSE) : parse_data_get_out_arg(data, FALSE),
                           NULL,NULL, steal_annotations(data));
  } else if (strcmp(element_name, "annotation") == 0) {
      GDBusAnnotationInfo **embedded_annotations;
      embedded_annotations = steal_annotations(data);
      parse_data_free_annotations(data);
      data->annotations = (GPtrArray*)data->annotations_stack->data;
      data->annotations_stack = g_slist_remove(data->annotations_stack, data->annotations_stack->data);
      have_popped_annotations = TRUE;
      g_dbus_annotation_info_set(data,parse_data_get_annotation(data, FALSE),NULL,NULL, embedded_annotations);
  }
  if (!have_popped_annotations) {
      parse_data_free_annotations(data);
      data->annotations = (GPtrArray*)data->annotations_stack->data;
      data->annotations_stack = g_slist_remove(data->annotations_stack, data->annotations_stack->data);
  }
}
static void parser_error(GMarkupParseContext *context, GError *error, gpointer user_data) {
  gint line_number;
  gint char_number;
  g_markup_parse_context_get_position(context, &line_number, &char_number);
  g_prefix_error(&error, "%d:%d: ", line_number, char_number);
}
GDBusNodeInfo *g_dbus_node_info_new_for_xml(const gchar *xml_data, GError **error) {
  GDBusNodeInfo *ret;
  GMarkupParseContext *context;
  GMarkupParser *parser;
  guint num_nodes;
  ParseData *data;
  GDBusNodeInfo **ughret;
  ret = NULL;
  parser = NULL;
  context = NULL;
  parser = g_new0(GMarkupParser, 1);
  parser->start_element = parser_start_element;
  parser->end_element = parser_end_element;
  parser->error = parser_error;
  data = parse_data_new();
  context = g_markup_parse_context_new(parser,0, data, (GDestroyNotify)parse_data_free);
  if (!g_markup_parse_context_parse(context, xml_data, strlen(xml_data), error)) goto out;
  if (!g_markup_parse_context_end_parse(context, error)) goto out;
  ughret = parse_data_steal_nodes(data, &num_nodes);
  if (num_nodes != 1) {
      guint n;
      g_set_error (error, G_MARKUP_ERROR,G_MARKUP_ERROR_INVALID_CONTENT,"Expected a single node in introspection XML, found %d", num_nodes);
      for (n = 0; n < num_nodes; n++) {
          for (n = 0; n < num_nodes; n++) {
              g_dbus_node_info_unref(ughret[n]);
              ughret[n] = NULL;
          }
      }
  }
  ret = ughret[0];
  g_free (ughret);
out:
  if (parser != NULL) g_free(parser);
  if (context != NULL) g_markup_parse_context_free(context);
  return ret;
}
const gchar *g_dbus_annotation_info_lookup(GDBusAnnotationInfo **annotations, const gchar *name) {
  guint n;
  const gchar *ret;
  ret = NULL;
  for (n = 0; annotations != NULL && annotations[n] != NULL; n++) {
      if (g_strcmp0(annotations[n]->key, name) == 0) {
          ret = annotations[n]->value;
          goto out;
      }
  }
out:
  return ret;
}
GDBusMethodInfo *g_dbus_interface_info_lookup_method(GDBusInterfaceInfo *info, const gchar *name) {
  guint n;
  GDBusMethodInfo *result;
  for (n = 0; info->methods != NULL && info->methods[n] != NULL; n++) {
      GDBusMethodInfo *i = info->methods[n];
      if (g_strcmp0(i->name, name) == 0) {
          result = i;
          goto out;
      }
  }
  result = NULL;
out:
  return result;
}
GDBusSignalInfo *g_dbus_interface_info_lookup_signal(GDBusInterfaceInfo *info, const gchar *name) {
  guint n;
  GDBusSignalInfo *result;
  for (n = 0; info->signals != NULL && info->signals[n] != NULL; n++) {
      GDBusSignalInfo *i = info->signals[n];
      if (g_strcmp0(i->name, name) == 0) {
          result = i;
          goto out;
      }
  }
  result = NULL;
out:
  return result;
}
GDBusPropertyInfo *g_dbus_interface_info_lookup_property(GDBusInterfaceInfo *info, const gchar *name) {
  guint n;
  GDBusPropertyInfo *result;
  for (n = 0; info->properties != NULL && info->properties[n] != NULL; n++) {
      GDBusPropertyInfo *i = info->properties[n];
      if (g_strcmp0(i->name, name) == 0) {
          result = i;
          goto out;
      }
  }
  result = NULL;
out:
  return result;
}
GDBusInterfaceInfo *g_dbus_node_info_lookup_interface(GDBusNodeInfo *info, const gchar *name) {
  guint n;
  GDBusInterfaceInfo *result;
  for (n = 0; info->interfaces != NULL && info->interfaces[n] != NULL; n++) {
      GDBusInterfaceInfo *i = info->interfaces[n];
      if (g_strcmp0(i->name, name) == 0) {
          result = i;
          goto out;
      }
  }
  result = NULL;
out:
  return result;
}