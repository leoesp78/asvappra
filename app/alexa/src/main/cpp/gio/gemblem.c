#include <stdlib.h>
#include <string.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "gicon.h"
#include "gemblem.h"
#include "gioenums.h"
#include "gioenumtypes.h"
#include "gioerror.h"

static void g_emblem_iface_init (GIconIface *iface);
struct _GEmblem {
  GObject parent_instance;
  GIcon *icon;
  GEmblemOrigin origin;
};
struct _GEmblemClass {
  GObjectClass parent_class;
};
enum {
  PROP_0_GEMBLEM,
  PROP_ICON,
  PROP_ORIGIN
};
G_DEFINE_TYPE_WITH_CODE(GEmblem, g_emblem, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_ICON, g_emblem_iface_init));
static void g_emblem_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GEmblem *emblem = G_EMBLEM(object);
  switch(prop_id) {
      case PROP_ICON: g_value_set_object(value, emblem->icon); break;
      case PROP_ORIGIN: g_value_set_enum(value, emblem->origin); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_emblem_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GEmblem *emblem = G_EMBLEM(object);
  switch(prop_id) {
      case PROP_ICON: emblem->icon = g_value_dup_object(value);break;
      case PROP_ORIGIN:emblem->origin = g_value_get_enum(value);break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void g_emblem_finalize(GObject *object) {
  GEmblem *emblem = G_EMBLEM(object);
  g_object_unref(emblem->icon);
  (*G_OBJECT_CLASS(g_emblem_parent_class)->finalize)(object);
}
static void g_emblem_class_init(GEmblemClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = g_emblem_finalize;
  gobject_class->set_property = g_emblem_set_property;
  gobject_class->get_property = g_emblem_get_property;
  g_object_class_install_property(gobject_class, PROP_ORIGIN, g_param_spec_enum("origin", "GEmblem's origin", "Tells which origin the emblem "
                                  "is derived from", G_TYPE_EMBLEM_ORIGIN, G_EMBLEM_ORIGIN_UNKNOWN, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_ICON, g_param_spec_object("icon","The icon of the emblem", "The actual icon of the "
                                  "emblem", G_TYPE_OBJECT, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}
static void g_emblem_init(GEmblem *emblem) {}
GEmblem *g_emblem_new(GIcon *icon) {
  GEmblem* emblem;
  g_return_val_if_fail(icon != NULL, NULL);
  g_return_val_if_fail(G_IS_ICON (icon), NULL);
  g_return_val_if_fail(!G_IS_EMBLEM (icon), NULL);
  emblem = g_object_new(G_TYPE_EMBLEM, NULL);
  emblem->icon = g_object_ref(icon);
  emblem->origin = G_EMBLEM_ORIGIN_UNKNOWN;
  return emblem;
}
GEmblem *g_emblem_new_with_origin(GIcon *icon, GEmblemOrigin origin) {
  GEmblem* emblem;
  g_return_val_if_fail(icon != NULL, NULL);
  g_return_val_if_fail(G_IS_ICON(icon), NULL);
  g_return_val_if_fail(!G_IS_EMBLEM(icon), NULL);
  emblem = g_object_new(G_TYPE_EMBLEM,NULL);
  emblem->icon = g_object_ref(icon);
  emblem->origin = origin;
  return emblem;
}
GIcon *g_emblem_get_icon(GEmblem *emblem) {
  g_return_val_if_fail(G_IS_EMBLEM(emblem), NULL);
  return emblem->icon;
}
GEmblemOrigin g_emblem_get_origin(GEmblem *emblem) {
  g_return_val_if_fail(G_IS_EMBLEM(emblem), G_EMBLEM_ORIGIN_UNKNOWN);
  return emblem->origin;
}
static guint g_emblem_hash(GIcon *icon) {
  GEmblem *emblem = G_EMBLEM(icon);
  guint hash;
  hash  = g_icon_hash(g_emblem_get_icon(emblem));
  hash ^= emblem->origin;
  return hash;
}
static gboolean g_emblem_equal(GIcon *icon1, GIcon *icon2) {
  GEmblem *emblem1 = G_EMBLEM(icon1);
  GEmblem *emblem2 = G_EMBLEM(icon2);
  return emblem1->origin == emblem2->origin && g_icon_equal(emblem1->icon, emblem2->icon);
}
static gboolean g_emblem_to_tokens(GIcon *icon, GPtrArray *tokens, gint *out_version) {
  GEmblem *emblem = G_EMBLEM(icon);
  char *s;
  g_return_val_if_fail(out_version != NULL, FALSE);
  *out_version = 0;
  s = g_icon_to_string(emblem->icon);
  if (s == NULL) return FALSE;
  g_ptr_array_add(tokens, s);
  s = g_strdup_printf("%d", emblem->origin);
  g_ptr_array_add(tokens, s);
  return TRUE;
}
static GIcon *g_emblem_from_tokens(gchar **tokens, gint num_tokens, gint version, GError **error) {
  GEmblem *emblem;
  GIcon *icon;
  GEmblemOrigin origin;
  emblem = NULL;
  if (version != 0) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Can't handle version %d of GEmblem encoding", version);
      return NULL;
  }
  if (num_tokens != 2) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_INVALID_ARGUMENT,"Malformed number of tokens (%d) in GEmblem encoding", num_tokens);
      return NULL;
  }
  icon = g_icon_new_for_string(tokens[0], error);
  if (icon == NULL) return NULL;
  origin = atoi(tokens[1]);
  emblem = g_emblem_new_with_origin(icon, origin);
  g_object_unref(icon);
  return G_ICON(emblem);
}
static void g_emblem_iface_init (GIconIface *iface) {
  iface->hash  = g_emblem_hash;
  iface->equal = g_emblem_equal;
  iface->to_tokens = g_emblem_to_tokens;
  iface->from_tokens = g_emblem_from_tokens;
}