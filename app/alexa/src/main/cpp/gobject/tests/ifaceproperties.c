#include <stdlib.h>
#include <string.h>
#include "../../glib/glib-object.h"
#include "testcommon.h"

static GType base_object_get_type ();
static GType derived_object_get_type ();
enum {
    BASE_PROP_0,
    BASE_PROP1,
    BASE_PROP2,
    BASE_PROP3,
    BASE_PROP4
};
enum {
    DERIVED_PROP_0,
    DERIVED_PROP3,
    DERIVED_PROP4
};
#define BASE_TYPE_OBJECT  (base_object_get_type())
#define BASE_OBJECT(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), BASE_TYPE_OBJECT, BaseObject))
typedef struct _BaseObject  BaseObject;
typedef struct _BaseObjectClass  BaseObjectClass;
struct _BaseObject {
    GObject parent_instance;
    gint val1;
    gint val2;
    gint val3;
    gint val4;
};
struct _BaseObjectClass {
  GObjectClass parent_class;
};
GObjectClass *base_parent_class;
#define DERIVED_TYPE_OBJECT  (derived_object_get_type ())
typedef struct _DerivedObject   DerivedObject;
typedef struct _DerivedObjectClass   DerivedObjectClass;
struct _DerivedObject {
  BaseObject parent_instance;
};
struct _DerivedObjectClass {
  BaseObjectClass parent_class;
};
typedef struct _TestIfaceClass TestIfaceClass;
struct _TestIfaceClass {
  GTypeInterface base_iface;
};
#define TEST_TYPE_IFACE (test_iface_get_type ())
static GParamSpec *iface_spec1, *iface_spec2, *iface_spec3;
static GParamSpec *inherited_spec1, *inherited_spec2, *inherited_spec3, *inherited_spec4;
static void test_iface_default_init (TestIfaceClass *iface_vtable) {
  inherited_spec1 = iface_spec1 = g_param_spec_int ("prop1","Prop1","Property 1",G_MININT,0xFFFF,42,
                                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_interface_install_property (iface_vtable, iface_spec1);
  iface_spec2 = g_param_spec_int ("prop2","Prop2","Property 2",G_MININT, G_MAXINT,0,G_PARAM_WRITABLE);
  g_object_interface_install_property (iface_vtable, iface_spec2);
  inherited_spec3 = iface_spec3 = g_param_spec_int ("prop3","Prop3","Property 3",G_MININT, G_MAXINT,0,
                                                    G_PARAM_READWRITE);
  g_object_interface_install_property (iface_vtable, iface_spec3);
}
static DEFINE_IFACE(TestIface, test_iface, NULL, test_iface_default_init)
static GObject* base_object_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
    GValue value1 = { 0, };
    GValue value2 = { 0, };
    GParamSpec *pspec;
    g_assert(n_construct_properties == 1);
    pspec = construct_properties->pspec;
    g_assert(G_IS_PARAM_SPEC_OVERRIDE (pspec));
    g_assert(pspec->param_id == BASE_PROP1);
    g_assert(strcmp (g_param_spec_get_name(pspec), "prop1") == 0);
    g_assert(g_param_spec_get_redirect_target(pspec) == iface_spec1);
    g_assert(strcmp (g_param_spec_get_nick(pspec), "Prop1") == 0);
    g_assert(strcmp (g_param_spec_get_blurb(pspec), "Property 1") == 0);
    g_value_init(&value1, G_TYPE_INT);
    g_value_init(&value2, G_TYPE_INT);
    g_param_value_set_default(pspec, &value1);
    g_assert(g_value_get_int(&value1) == 42);
    g_value_reset(&value1);
    g_value_set_int(&value1, 0x10000);
    g_assert(g_param_value_validate(pspec, &value1));
    g_assert(g_value_get_int(&value1) == 0xFFFF);
    g_assert(!g_param_value_validate(pspec, &value1));
    g_value_reset(&value1);
    g_value_set_int(&value1, 1);
    g_value_set_int(&value2, 2);
    g_assert(g_param_values_cmp(pspec, &value1, &value2) < 0);
    g_assert(g_param_values_cmp(pspec, &value2, &value1) > 0);
    g_value_unset(&value1);
    g_value_unset(&value2);
    return base_parent_class->constructor(type, n_construct_properties, construct_properties);
}
static void base_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    BaseObject *base_object = BASE_OBJECT(object);
    switch(prop_id) {
        case BASE_PROP1:
            g_assert(pspec == inherited_spec1);
            base_object->val1 = g_value_get_int(value);
            break;
        case BASE_PROP2:
            g_assert(pspec == inherited_spec2);
            base_object->val2 = g_value_get_int(value);
            break;
        case BASE_PROP3: g_assert_not_reached(); break;
        case BASE_PROP4: g_assert_not_reached(); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void base_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    BaseObject *base_object = BASE_OBJECT(object);
    switch(prop_id) {
        case BASE_PROP1:
            g_assert(pspec == inherited_spec1);
            g_value_set_int(value, base_object->val1);
            break;
        case BASE_PROP2:
            g_assert(pspec == inherited_spec2);
            g_value_set_int(value, base_object->val2);
            break;
        case BASE_PROP3: g_assert_not_reached(); break;
        case BASE_PROP4: g_assert_not_reached (); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}
static void base_object_notify(GObject *object, GParamSpec *pspec) {
    g_assert(pspec == inherited_spec1 || pspec == inherited_spec2 || pspec == inherited_spec3 || pspec == inherited_spec4);
}
static void base_object_class_init(BaseObjectClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    base_parent_class= g_type_class_peek_parent(class);
    object_class->constructor = base_object_constructor;
    object_class->set_property = base_object_set_property;
    object_class->get_property = base_object_get_property;
    object_class->notify = base_object_notify;
    g_object_class_override_property(object_class, BASE_PROP1, "prop1");
    inherited_spec2 = g_param_spec_int("prop2","Prop2","Property 2",G_MININT, G_MAXINT,0,G_PARAM_READWRITE);
    g_object_class_install_property(object_class, BASE_PROP2, inherited_spec2);
    g_object_class_override_property(object_class, BASE_PROP3, "prop3");
    inherited_spec4 = g_param_spec_int("prop4","Prop4","Property 4",G_MININT, G_MAXINT,0,G_PARAM_READWRITE);
    g_object_class_install_property(object_class, BASE_PROP4, inherited_spec4);
}
static void base_object_init(BaseObject *base_object) {
    base_object->val1 = 42;
}
static DEFINE_TYPE_FULL(BaseObject, base_object, base_object_class_init, NULL, base_object_init, G_TYPE_OBJECT,INTERFACE(NULL, TEST_TYPE_IFACE))
static void derived_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    BaseObject *base_object = BASE_OBJECT(object);
    switch(prop_id) {
        case DERIVED_PROP3:
            g_assert(pspec == inherited_spec3);
            base_object->val3 = g_value_get_int(value);
            break;
        case DERIVED_PROP4:
            g_assert(pspec == inherited_spec4);
            base_object->val4 = g_value_get_int(value);
            break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); break;
    }
}
static void derived_object_get_property(GObject    *object, guint       prop_id, GValue     *value, GParamSpec *pspec) {
    BaseObject *base_object = BASE_OBJECT (object);
    switch (prop_id) {
        case DERIVED_PROP3:
            g_assert (pspec == inherited_spec3);
            g_value_set_int (value, base_object->val3);
            break;
        case DERIVED_PROP4:
            g_assert (pspec == inherited_spec4);
            g_value_set_int (value, base_object->val4);
            break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void derived_object_class_init(DerivedObjectClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->set_property = derived_object_set_property;
    object_class->get_property = derived_object_get_property;
    g_object_class_override_property(object_class, DERIVED_PROP3, "prop3");
    g_object_class_override_property(object_class, DERIVED_PROP4, "prop4");
}
static DEFINE_TYPE(DerivedObject, derived_object, derived_object_class_init, NULL, NULL, BASE_TYPE_OBJECT)
static void assert_in_properties(GParamSpec  *param_spec, GParamSpec **properties, gint n_properties) {
    gint i;
    gboolean found = FALSE;
    for (i = 0; i < n_properties; i++) {
        if (properties[i] == param_spec) found = TRUE;
    }
    g_assert(found);
}
static void test_set(void) {
    BaseObject *object;
    gint val1, val2, val3, val4;
    object = g_object_new(DERIVED_TYPE_OBJECT, NULL);
    g_object_set(object,"prop1", 0x0101, "prop2", 0x0202, "prop3", 0x0303, "prop4", 0x0404, NULL);
    g_object_get(object,"prop1", &val1, "prop2", &val2, "prop3", &val3, "prop4", &val4, NULL);
    g_assert(val1 == 0x0101);
    g_assert(val2 == 0x0202);
    g_assert(val3 == 0x0303);
    g_assert(val4 == 0x0404);
    g_object_unref(object);
}
static void test_notify(void) {
    BaseObject *object;
    object = g_object_new(DERIVED_TYPE_OBJECT, NULL);
    g_object_freeze_notify(G_OBJECT(object));
    g_object_notify(G_OBJECT(object), "prop1");
    g_object_notify(G_OBJECT(object), "prop2");
    g_object_notify(G_OBJECT(object), "prop3");
    g_object_notify(G_OBJECT(object), "prop4");
    g_object_thaw_notify(G_OBJECT(object));
    g_object_unref(object);
}
static void test_find_overridden(void) {
    GObjectClass *object_class;
    object_class = g_type_class_peek(DERIVED_TYPE_OBJECT);
    g_assert(g_object_class_find_property(object_class, "prop1") == inherited_spec1);
    g_assert(g_object_class_find_property(object_class, "prop2") == inherited_spec2);
    g_assert(g_object_class_find_property(object_class, "prop3") == inherited_spec3);
    g_assert(g_object_class_find_property(object_class, "prop4") == inherited_spec4);
}
static void test_list_overridden (void) {
    GObjectClass *object_class;
    GParamSpec **properties;
    guint n_properties;
    object_class = g_type_class_peek(DERIVED_TYPE_OBJECT);
    properties = g_object_class_list_properties(object_class, &n_properties);
    g_assert(n_properties == 4);
    assert_in_properties(inherited_spec1, properties, n_properties);
    assert_in_properties(inherited_spec2, properties, n_properties);
    assert_in_properties(inherited_spec3, properties, n_properties);
    assert_in_properties(inherited_spec4, properties, n_properties);
    g_free(properties);
}
static void test_find_interface (void) {
    TestIfaceClass *iface;
    iface = g_type_default_interface_peek(TEST_TYPE_IFACE);
    g_assert(g_object_interface_find_property(iface, "prop1") == iface_spec1);
    g_assert(g_object_interface_find_property(iface, "prop2") == iface_spec2);
    g_assert(g_object_interface_find_property(iface, "prop3") == iface_spec3);
}
static void test_list_interface (void) {
    TestIfaceClass *iface;
    GParamSpec **properties;
    guint n_properties;
    iface = g_type_default_interface_peek(TEST_TYPE_IFACE);
    properties = g_object_interface_list_properties(iface, &n_properties);
    g_assert(n_properties == 3);
    assert_in_properties(iface_spec1, properties, n_properties);
    assert_in_properties(iface_spec2, properties, n_properties);
    assert_in_properties(iface_spec3, properties, n_properties);
    g_free(properties);
}
#define BASE2_TYPE_OBJECT  (base2_object_get_type ())
#define BASE2_OBJECT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), BASE2_TYPE_OBJECT, Base2Object))
typedef struct _Base2Object  Base2Object;
typedef struct _Base2ObjectClass   Base2ObjectClass;
static void base2_object_test_iface_init(TestIfaceClass *iface) {}
enum {
    BASE2_PROP_0,
    BASE2_PROP1,
    BASE2_PROP2
};
struct _Base2Object {
    GObject parent_instance;
};
struct _Base2ObjectClass {
    GObjectClass parent_class;
};
G_DEFINE_TYPE_WITH_CODE(Base2Object, base2_object, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (TEST_TYPE_IFACE, base2_object_test_iface_init));
static void base2_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    switch(prop_id) {
        case BASE2_PROP1: g_value_set_int(value, 0); break;
        case BASE2_PROP2: g_value_set_int(value, 0); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void base2_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    switch(prop_id) {
        case BASE2_PROP1: case BASE2_PROP2: break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void base2_object_class_init(Base2ObjectClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  object_class->set_property = base2_object_set_property;
  object_class->get_property = base2_object_get_property;
  g_object_class_override_property(object_class, BASE2_PROP1, "prop1");
  g_object_class_override_property(object_class, BASE2_PROP2, "prop2");
}
static void base2_object_init(Base2Object *object) {}
static void test_not_overridden(void) {
    g_test_bug("637738");
    if (g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
        Base2Object *object;
        object = g_object_new(BASE2_TYPE_OBJECT, NULL);
        g_object_unref(object);
        exit(0);
    }
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*Base2Object doesn't implement property 'prop3' from interface 'TestIface'*");
}
int main(int argc, char *argv[]) {
    g_type_init();
    g_test_init(&argc, &argv, NULL);
    g_test_bug_base("http://bugzilla.gnome.org/");
    g_test_add_func("/interface/properties/set", test_set);
    g_test_add_func("/interface/properties/notify", test_notify);
    g_test_add_func("/interface/properties/find-overridden", test_find_overridden);
    g_test_add_func("/interface/properties/list-overridden", test_list_overridden);
    g_test_add_func("/interface/properties/find-interface", test_find_interface);
    g_test_add_func("/interface/properties/list-interface", test_list_interface);
    g_test_add_func("/interface/properties/not-overridden", test_not_overridden);
    return g_test_run();
}