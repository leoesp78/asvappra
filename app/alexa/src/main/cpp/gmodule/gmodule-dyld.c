#include <string.h>
#include "../gio/config.h"
#include "../glib/glib.h"

static gpointer self_module = GINT_TO_POINTER (1);
static gpointer _g_module_open(const gchar *file_name, gboolean bind_lazy, gboolean bind_local) {
    /*NSObjectFileImage image;
    NSObjectFileImageReturnCode ret;
    NSModule module;
    unsigned long options;
    char *msg;
    ret = NSCreateObjectFileImageFromFile(file_name, &image);
    if (ret != NSObjectFileImageSuccess){
        switch(ret) {
            case NSObjectFileImageInappropriateFile: case NSObjectFileImageFormat:
                msg = g_strdup_printf("%s is not a loadable module", file_name);
                break;
            case NSObjectFileImageArch:
                msg = g_strdup_printf("%s is not built for this architecture", file_name);
                break;
            case NSObjectFileImageAccess:
                if (access(file_name, F_OK) == 0) msg = g_strdup_printf("%s: permission denied", file_name);
                else msg = g_strdup_printf("%s: no such file or directory", file_name);
                break;
            default: msg = g_strdup_printf("unknown error for %s", file_name); break;
        }
        g_module_set_error(msg);
        g_free(msg);
        return NULL;
    }
    options = NSLINKMODULE_OPTION_RETURN_ON_ERROR;
    if (bind_local) options |= NSLINKMODULE_OPTION_PRIVATE;
    if (!bind_lazy) options |= NSLINKMODULE_OPTION_BINDNOW;
    module = NSLinkModule(image, file_name, options);
    NSDestroyObjectFileImage(image);
    if (!module) {
        NSLinkEditErrors c;
        int error_number;
        const char *file, *error;
        NSLinkEditError(&c, &error_number, &file, &error);
        msg = g_strdup_printf("could not link %s: %s", file_name, error);
        g_module_set_error (msg);
        g_free(msg);
        return NULL;
    }
    return module;*/
    return NULL;
}
static gpointer _g_module_self(void) {
    return &self_module;
}
static void _g_module_close(gpointer handle, gboolean is_unref) {
  if (handle == &self_module) return;
  //if (!NSUnLinkModule (handle, 0)) g_module_set_error ("could not unlink module");
}
static gpointer _g_module_symbol(gpointer handle, const gchar *symbol_name) {
    /*NSSymbol sym;
    char *msg;
    if (handle == &self_module) {
        if (NSIsSymbolNameDefined(symbol_name)) sym = NSLookupAndBindSymbol (symbol_name);
        else sym = NULL;
    } else sym = NSLookupSymbolInModule(handle, symbol_name);
    if (!sym) {
        msg = g_strdup_printf ("no such symbol %s", symbol_name);
        g_module_set_error(msg);
        g_free (msg);
        return NULL;
    }
    return NSAddressOfSymbol(sym);*/
    return NULL;
}
static gchar* _g_module_build_path(const gchar *directory, const gchar *module_name) {
    if (directory && *directory) {
        if (strncmp (module_name, "lib", 3) == 0) return g_strconcat (directory, "/", module_name, NULL);
        else return g_strconcat (directory, "/lib", module_name, "." G_MODULE_SUFFIX, NULL);
    } else if (strncmp (module_name, "lib", 3) == 0) return g_strdup (module_name);
    else return g_strconcat ("lib", module_name, "." G_MODULE_SUFFIX, NULL);
}