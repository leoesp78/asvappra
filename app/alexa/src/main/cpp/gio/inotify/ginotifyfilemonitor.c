#include "../../glib/gtestutils.h"
#include "../config.h"
#include "../giomodule.h"
#include "ginotifyfilemonitor.h"
#define USE_INOTIFY 1
#include "inotify-helper.h"

struct _GInotifyFileMonitor {
  GLocalFileMonitor parent_instance;
  gchar *filename;
  gchar *dirname;
  inotify_sub *sub;
  gboolean pair_moves;
};
static gboolean g_inotify_file_monitor_cancel(GFileMonitor* monitor);
#define g_inotify_file_monitor_get_type _g_inotify_file_monitor_get_type
G_DEFINE_TYPE_WITH_CODE(GInotifyFileMonitor, g_inotify_file_monitor, G_TYPE_LOCAL_FILE_MONITOR,
			       g_io_extension_point_implement(G_LOCAL_FILE_MONITOR_EXTENSION_POINT_NAME, g_define_type_id, "inotify", 20))
static void g_inotify_file_monitor_finalize(GObject *object) {
  GInotifyFileMonitor *inotify_monitor = G_INOTIFY_FILE_MONITOR (object);
  inotify_sub *sub = inotify_monitor->sub;
  if (sub) {
      _ih_sub_cancel(sub);
      _ih_sub_free(sub);
      inotify_monitor->sub = NULL;
  }
  if (inotify_monitor->filename) {
      g_free(inotify_monitor->filename);
      inotify_monitor->filename = NULL;
  }
  if (inotify_monitor->dirname) {
      g_free(inotify_monitor->dirname);
      inotify_monitor->dirname = NULL;
  }
  if (G_OBJECT_CLASS(g_inotify_file_monitor_parent_class)->finalize) (*G_OBJECT_CLASS(g_inotify_file_monitor_parent_class)->finalize)(object);
}

static GObject *g_inotify_file_monitor_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
  GObject *obj;
  GInotifyFileMonitorClass *klass;
  GObjectClass *parent_class;
  GInotifyFileMonitor *inotify_monitor;
  const gchar *filename = NULL;
  inotify_sub *sub = NULL;
  gboolean pair_moves;
  gboolean ret_ih_startup;
  klass = G_INOTIFY_FILE_MONITOR_CLASS(g_type_class_peek(G_TYPE_INOTIFY_FILE_MONITOR));
  parent_class = G_OBJECT_CLASS(g_type_class_peek_parent(klass));
  obj = parent_class->constructor(type, n_construct_properties, construct_properties);
  inotify_monitor = G_INOTIFY_FILE_MONITOR(obj);
  filename = G_LOCAL_FILE_MONITOR (obj)->filename;
  g_assert(filename != NULL);
  inotify_monitor->filename = g_path_get_basename(filename);
  inotify_monitor->dirname = g_path_get_dirname(filename);
  ret_ih_startup = _ih_startup();
  g_assert(ret_ih_startup);
  pair_moves = G_LOCAL_FILE_MONITOR(obj)->flags & G_FILE_MONITOR_SEND_MOVED;
  sub = _ih_sub_new(inotify_monitor->dirname, inotify_monitor->filename, pair_moves, inotify_monitor);
  g_assert(sub != NULL);
  _ih_sub_add(sub);
  inotify_monitor->sub = sub;
  return obj;
}
static gboolean g_inotify_file_monitor_is_supported(void) {
  return _ih_startup();
}
static void g_inotify_file_monitor_class_init(GInotifyFileMonitorClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  GFileMonitorClass *file_monitor_class = G_FILE_MONITOR_CLASS(klass);
  GLocalFileMonitorClass *local_file_monitor_class = G_LOCAL_FILE_MONITOR_CLASS(klass);
  gobject_class->finalize = g_inotify_file_monitor_finalize;
  gobject_class->constructor = g_inotify_file_monitor_constructor;
  file_monitor_class->cancel = g_inotify_file_monitor_cancel;
  local_file_monitor_class->is_supported = g_inotify_file_monitor_is_supported;
}
static void g_inotify_file_monitor_init(GInotifyFileMonitor* monitor) {}
static gboolean g_inotify_file_monitor_cancel(GFileMonitor* monitor) {
  GInotifyFileMonitor *inotify_monitor = G_INOTIFY_FILE_MONITOR (monitor);
  inotify_sub *sub = inotify_monitor->sub;
  if (sub) {
      _ih_sub_cancel(sub);
      _ih_sub_free(sub);
      inotify_monitor->sub = NULL;
  }
  if (G_FILE_MONITOR_CLASS(g_inotify_file_monitor_parent_class)->cancel) (*G_FILE_MONITOR_CLASS(g_inotify_file_monitor_parent_class)->cancel)(monitor);
  return TRUE;
}
