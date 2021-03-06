#include "../../glib/glib.h"
#include "../../glib/glib-object.h"

static GMutex *sync_mutex = NULL;
static gboolean loaded = FALSE;
typedef struct _TestModule      TestModule;
typedef struct _TestModuleClass TestModuleClass;
void g_thread_init(GThreadFunctions *vtable);
#define TEST_TYPE_MODULE  (test_module_get_type ())
#define TEST_MODULE(module)  (G_TYPE_CHECK_INSTANCE_CAST ((module), TEST_TYPE_MODULE, TestModule))
#define TEST_MODULE_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST ((class), TEST_TYPE_MODULE, TestModuleClass))
#define TEST_IS_MODULE(module)  (G_TYPE_CHECK_INSTANCE_TYPE ((module), TEST_TYPE_MODULE))
#define TEST_IS_MODULE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), TEST_TYPE_MODULE))
#define TEST_MODULE_GET_CLASS(module)  (G_TYPE_INSTANCE_GET_CLASS ((module), TEST_TYPE_MODULE, TestModuleClass))
typedef void (*TestModuleRegisterFunc)  (GTypeModule *module);
struct _TestModule {
    GTypeModule parent_instance;
    TestModuleRegisterFunc register_func;
};
struct _TestModuleClass {
    GTypeModuleClass parent_class;
};
static GType test_module_get_type(void);
static gboolean
test_module_load(GTypeModule *module) {
    TestModule *test_module = TEST_MODULE(module);
    test_module->register_func(module);
    return TRUE;
}
static void test_module_unload(GTypeModule *module){}
static void test_module_class_init(TestModuleClass *class) {
    GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS(class);
    module_class->load = test_module_load;
    module_class->unload = test_module_unload;
}
static GType test_module_get_type(void) {
    static GType object_type = 0;
    if (!object_type) {
        static const GTypeInfo object_info = {
            sizeof(TestModuleClass),
            (GBaseInitFunc)NULL,
            (GBaseFinalizeFunc)NULL,
            (GClassInitFunc)test_module_class_init,
            (GClassFinalizeFunc)NULL,
            NULL,
            sizeof(TestModule),
            0,
            (GInstanceInitFunc)NULL
        };
        object_type = g_type_register_static(G_TYPE_TYPE_MODULE, "TestModule", &object_info, 0);
    }
    return object_type;
}
GTypeModule *test_module_new(TestModuleRegisterFunc register_func) {
    TestModule *test_module = g_object_new(TEST_TYPE_MODULE, NULL);
    GTypeModule *module = G_TYPE_MODULE(test_module);
    test_module->register_func = register_func;
    g_type_module_use(module);
    g_type_module_unuse(module);
    return G_TYPE_MODULE(module);
}
#define DYNAMIC_OBJECT_TYPE (dynamic_object_get_type ())
typedef GObject DynamicObject;
typedef struct _DynamicObjectClass DynamicObjectClass;
struct _DynamicObjectClass {
    GObjectClass parent_class;
    guint val;
};
G_DEFINE_DYNAMIC_TYPE(DynamicObject, dynamic_object, G_TYPE_OBJECT);
static void dynamic_object_class_init(DynamicObjectClass *class) {
    class->val = 42;
    g_assert(loaded == FALSE);
    loaded = TRUE;
}
static void dynamic_object_class_finalize(DynamicObjectClass *class) {
    g_assert(loaded == TRUE);
    loaded = FALSE;
}
static void dynamic_object_init(DynamicObject *dynamic_object) {}
static void module_register(GTypeModule *module) {
    dynamic_object_register_type(module);
}
#define N_THREADS 100
#define N_REFS 10000
static gpointer ref_unref_thread(gpointer data) {
    gint i;
    if (g_test_verbose()) g_print("WAITING!\n");
    //g_mutex_lock(sync_mutex);
    //g_mutex_unlock(sync_mutex);
    if (g_test_verbose()) g_print("STARTING\n");
    for (i = N_REFS; i; i--) {
        if (g_test_verbose())
            if (i % 10) g_print("%d\n", i);
        g_type_class_unref(g_type_class_ref((GType)data));
    }
    if (g_test_verbose()) g_print("DONE !\n");
    return NULL;
}
static void test_multithreaded_dynamic_type_init(void) {
    GTypeModule *module;
    DynamicObjectClass *class;
    GThread *threads[N_THREADS];
    guint i;
    module = test_module_new(module_register);
    class = g_type_class_peek(DYNAMIC_OBJECT_TYPE);
    g_assert(class == NULL);
    g_assert(!loaded);
    //g_mutex_lock(sync_mutex);
    for (i = 0; i < N_THREADS; i++) {
      threads[i] = g_thread_create(ref_unref_thread, (gpointer)DYNAMIC_OBJECT_TYPE, TRUE, NULL);
    }
    //g_mutex_unlock (sync_mutex);
    for (i = 0; i < N_THREADS; i++) g_thread_join(threads[i]);
}
enum {
    PROP_0,
    PROP_FOO
};
typedef struct _DynObj DynObj;
typedef struct _DynObjClass DynObjClass;
typedef struct _DynIfaceInterface DynIfaceInterface;
struct _DynObj {
    GObject obj;
    gint foo;
};
struct _DynObjClass {
    GObjectClass class;
};
struct _DynIfaceInterface {
    GTypeInterface iface;
};
static void dyn_obj_iface_init(DynIfaceInterface *iface);
//G_DEFINE_INTERFACE(DynIface, dyn_iface, G_TYPE_OBJECT);
//G_DEFINE_DYNAMIC_TYPE_EXTENDED(DynObj, dyn_obj, G_TYPE_OBJECT, 0,G_IMPLEMENT_INTERFACE_DYNAMIC(dyn_iface_get_type(), dyn_obj_iface_init));
static void dyn_iface_default_init(DynIfaceInterface *iface) {
    g_object_interface_install_property(iface,g_param_spec_int("foo", NULL, NULL, 0, 100, 0,
                                  G_PARAM_READWRITE));
}
static void dyn_obj_iface_init(DynIfaceInterface *iface) {}
static void dyn_obj_init(DynObj *obj) {
    obj->foo = 0;
}
static void set_prop(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    DynObj *obj = (DynObj*)object;
    switch(prop_id) {
        case PROP_FOO: obj->foo = g_value_get_int(value); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void get_prop(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    DynObj *obj = (DynObj*)object;
    switch(prop_id) {
        case PROP_FOO: g_value_set_int(value, obj->foo); break;
        default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
    }
}
static void dyn_obj_class_init(DynObjClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS (class);
    object_class->set_property = set_prop;
    object_class->get_property = get_prop;
    g_object_class_override_property(object_class, PROP_FOO, "foo");
}
static void dyn_obj_class_finalize(DynObjClass *class) {}
static void mod_register(GTypeModule *module) {
    //dyn_obj_register_type(module);
}
static void test_dynamic_interface_properties(void) {
    /*GTypeModule *module;
    DynObj *obj;
    module = test_module_new(mod_register);
    obj = g_object_new(dyn_obj_get_type(), "foo", 1, NULL);
    g_object_unref(obj);*/
}
int main(int argc, char *argv[]) {
    g_thread_init(NULL);
    g_test_init(&argc, &argv, NULL);
    g_type_init();
    sync_mutex = g_mutex_new();
    g_test_add_func("/GObject/threaded-dynamic-ref-unref-init", test_multithreaded_dynamic_type_init);
    g_test_add_func("/GObject/dynamic-interface-properties", test_dynamic_interface_properties);
    return g_test_run();
}
