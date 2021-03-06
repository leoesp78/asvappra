#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "config.h"
#include "gsimpleaction.h"
#include "gaction.h"

static void g_simple_action_iface_init (GActionInterface *iface);
G_DEFINE_TYPE_WITH_CODE (GSimpleAction, g_simple_action, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (G_TYPE_ACTION, g_simple_action_iface_init));
struct _GSimpleActionPrivate {
  gchar *name;
  GVariantType *parameter_type;
  guint enabled : 1;
  guint state_set : 1;
  GVariant *state;
};
enum {
  PROP_NONE,
  PROP_NAME,
  PROP_PARAMETER_TYPE,
  PROP_ENABLED,
  PROP_STATE_TYPE,
  PROP_STATE
};
enum {
  SIGNAL_ACTIVATE,
  NR_SIGNALS
};
static guint g_simple_action_signals[NR_SIGNALS];
static const gchar *g_simple_action_get_name(GAction *action) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  return simple->priv->name;
}
const GVariantType *g_simple_action_get_parameter_type(GAction *action) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  return simple->priv->parameter_type;
}
static const GVariantType *g_simple_action_get_state_type(GAction *action) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  if (simple->priv->state != NULL) return g_variant_get_type(simple->priv->state);
  else return NULL;
}
static GVariant *g_simple_action_get_state_hint(GAction *action) {
  return NULL;
}
static gboolean g_simple_action_get_enabled(GAction *action) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  return simple->priv->enabled;
}
static void g_simple_action_set_state(GAction *action, GVariant *value) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  g_return_if_fail(value != NULL);
  {
      const GVariantType *state_type;
      state_type = simple->priv->state ? g_variant_get_type(simple->priv->state) : NULL;
      g_return_if_fail(state_type != NULL);
      g_return_if_fail(g_variant_is_of_type(value, state_type));
  }
  g_variant_ref_sink(value);
  if (!g_variant_equal(simple->priv->state, value)) {
      if (simple->priv->state) g_variant_unref(simple->priv->state);
      simple->priv->state = g_variant_ref(value);
      g_object_notify(G_OBJECT(simple), "state");
  }
  g_variant_unref(value);
}
static GVariant *g_simple_action_get_state(GAction *action) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  return simple->priv->state ? g_variant_ref(simple->priv->state) : NULL;
}
static void g_simple_action_activate(GAction *action, GVariant *parameter) {
  GSimpleAction *simple = G_SIMPLE_ACTION(action);
  g_return_if_fail(simple->priv->parameter_type == NULL ? parameter == NULL : (parameter != NULL && g_variant_is_of_type(parameter, simple->priv->parameter_type)));
  if (parameter != NULL) g_variant_ref_sink(parameter);
  if (simple->priv->enabled) g_signal_emit(simple, g_simple_action_signals[SIGNAL_ACTIVATE], 0, parameter);
  if (parameter != NULL) g_variant_unref(parameter);
}
static void g_simple_action_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSimpleAction *simple = G_SIMPLE_ACTION(object);
  switch(prop_id) {
      case PROP_NAME:
          g_assert(simple->priv->name == NULL);
          simple->priv->name = g_value_dup_string(value);
          break;
      case PROP_PARAMETER_TYPE:
          g_assert(simple->priv->parameter_type == NULL);
          simple->priv->parameter_type = g_value_dup_boxed(value);
          break;
      case PROP_ENABLED: g_simple_action_set_enabled(simple, g_value_get_boolean(value)); break;
      case PROP_STATE:
          if (simple->priv->state_set) g_simple_action_set_state(G_ACTION(simple), g_value_get_variant(value));
          else {
              simple->priv->state_set = TRUE;
              simple->priv->state = g_value_dup_variant(value);
          }
          break;
      default: g_assert_not_reached();
  }
}
static void g_simple_action_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GAction *action = G_ACTION(object);
  switch(prop_id) {
      case PROP_NAME: g_value_set_string(value, g_simple_action_get_name(action)); break;
      case PROP_PARAMETER_TYPE: g_value_set_boxed(value, g_simple_action_get_parameter_type(action)); break;
      case PROP_ENABLED: g_value_set_boolean(value, g_simple_action_get_enabled(action)); break;
      case PROP_STATE_TYPE: g_value_set_boxed(value, g_simple_action_get_state_type(action)); break;
      case PROP_STATE: g_value_take_variant(value, g_simple_action_get_state(action)); break;
      default: g_assert_not_reached();
  }
}
static void g_simple_action_finalize(GObject *object) {
  GSimpleAction *simple = G_SIMPLE_ACTION(object);
  g_free (simple->priv->name);
  if (simple->priv->parameter_type) g_variant_type_free(simple->priv->parameter_type);
  if (simple->priv->state) g_variant_unref(simple->priv->state);
  //G_OBJECT_CLASS(g_simple_action_parent_class)->finalize(object);
}
void g_simple_action_init(GSimpleAction *simple) {
  simple->priv = G_TYPE_INSTANCE_GET_PRIVATE(simple, G_TYPE_SIMPLE_ACTION, GSimpleActionPrivate);
}
void g_simple_action_iface_init(GActionInterface *iface) {
  iface->get_name = g_simple_action_get_name;
  iface->get_parameter_type = g_simple_action_get_parameter_type;
  iface->get_state_type = g_simple_action_get_state_type;
  iface->get_state_hint = g_simple_action_get_state_hint;
  iface->get_enabled = g_simple_action_get_enabled;
  iface->get_state = g_simple_action_get_state;
  iface->set_state = g_simple_action_set_state;
  iface->activate = g_simple_action_activate;
}
void g_simple_action_class_init(GSimpleActionClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  object_class->get_property = g_simple_action_get_property;
  object_class->set_property = g_simple_action_set_property;
  object_class->finalize = g_simple_action_finalize;
  g_simple_action_signals[SIGNAL_ACTIVATE] = g_signal_new(I_("activate"), G_TYPE_SIMPLE_ACTION, G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET(GSimpleActionClass, activate),
                                                          NULL, NULL, g_cclosure_marshal_VOID__VARIANT, G_TYPE_NONE, 1, G_TYPE_VARIANT);
  g_object_class_install_property(object_class, PROP_NAME,g_param_spec_string("name", P_("Action Name"), P_("The name used to "
                                  "invoke the action"),NULL,G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_PARAMETER_TYPE, g_param_spec_boxed("parameter-type", P_("Parameter Type"), P_("The type of "
                                  "GVariant passed to activate()"), G_TYPE_VARIANT_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_ENABLED,g_param_spec_boolean("enabled", P_("Enabled"), P_("If the action "
                                  "can be activated"), TRUE,G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class, PROP_STATE_TYPE, g_param_spec_boxed ("state-type", P_("State Type"), P_("The type of the state kept "
                                  "by the action"), G_TYPE_VARIANT_TYPE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(object_class,PROP_STATE,g_param_spec_variant("state", P_("State"), P_("The state the action is in"),
                                  G_VARIANT_TYPE_ANY,NULL,G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_type_class_add_private(class, sizeof(GSimpleActionPrivate));
}
void g_simple_action_set_enabled(GSimpleAction *simple, gboolean enabled) {
  g_return_if_fail(G_IS_SIMPLE_ACTION(simple));
  enabled = !!enabled;
  if (simple->priv->enabled != enabled) {
      simple->priv->enabled = enabled;
      g_object_notify(G_OBJECT(simple), "enabled");
  }
}
GSimpleAction *g_simple_action_new(const gchar *name, const GVariantType *parameter_type) {
  return g_object_new(G_TYPE_SIMPLE_ACTION,"name", name, "parameter-type", parameter_type, NULL);
}
GSimpleAction *g_simple_action_new_stateful(const gchar *name, const GVariantType *parameter_type, GVariant *state) {
  return g_object_new(G_TYPE_SIMPLE_ACTION, "name", name, "parameter-type", parameter_type, "state", state, NULL);
}