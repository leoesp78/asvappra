#define __KERNEL_STRICT_NAMES
#include <sys/inotify.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../config.h"
#include "inotify-kernel.h"
#include "inotify-path.h"
#include "inotify-missing.h"

#define IP_INOTIFY_MASK (IN_MODIFY|IN_ATTRIB|IN_MOVED_FROM|IN_MOVED_TO|IN_DELETE|IN_CREATE|IN_DELETE_SELF|IN_UNMOUNT|IN_MOVE_SELF|IN_CLOSE_WRITE)
#ifndef IN_ONLYDIR
#define IN_ONLYDIR 0  
#endif
typedef struct ip_watched_dir_s {
  char *path;
  struct ip_watched_dir_s* parent;
  GList* children;
  gint32 wd;
  GList *subs;
} ip_watched_dir_t;
static gboolean ip_debug_enabled = FALSE;
#define IP_W if (ip_debug_enabled) g_warning
static GHashTable *path_dir_hash = NULL;
static GHashTable *sub_dir_hash = NULL;
static GHashTable *wd_dir_hash = NULL;
static ip_watched_dir_t *ip_watched_dir_new(const char *path, int wd);
static void ip_watched_dir_free(ip_watched_dir_t *dir);
static void ip_event_callback(ik_event_t *event);
static void (*event_callback)(ik_event_t *event, inotify_sub *sub);
gboolean _ip_startup(void (*cb)(ik_event_t *event, inotify_sub *sub)) {
  static gboolean initialized = FALSE;
  static gboolean result = FALSE;
  if (initialized == TRUE) return result;
  event_callback = cb;
  result = _ik_startup(ip_event_callback);
  if (!result) return FALSE;
  path_dir_hash = g_hash_table_new(g_str_hash, g_str_equal);
  sub_dir_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  wd_dir_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialized = TRUE;
  return TRUE;
}
static void ip_map_path_dir(const char *path, ip_watched_dir_t *dir) {
  g_assert(path && dir);
  g_hash_table_insert(path_dir_hash, dir->path, dir);
}
static void ip_map_sub_dir(inotify_sub *sub, ip_watched_dir_t *dir) {
  g_assert(dir && sub);
  g_hash_table_insert(sub_dir_hash, sub, dir);
  dir->subs = g_list_prepend(dir->subs, sub);
}
static void ip_map_wd_dir(gint32 wd, ip_watched_dir_t *dir) {
  GList *dir_list;
  g_assert(wd >= 0 && dir);
  dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(wd));
  dir_list = g_list_prepend(dir_list, dir);
  g_hash_table_replace(wd_dir_hash, GINT_TO_POINTER(dir->wd), dir_list);
}
gboolean _ip_start_watching(inotify_sub *sub) {
  gint32 wd;
  int err;
  ip_watched_dir_t *dir;
  g_assert(sub);
  g_assert(!sub->cancelled);
  g_assert(sub->dirname);
  IP_W("Starting to watch %s\n", sub->dirname);
  dir = g_hash_table_lookup(path_dir_hash, sub->dirname);
  if (dir) {
      IP_W("Already watching\n");
      goto out;
  }
  IP_W("Trying to add inotify watch ");
  wd = _ik_watch(sub->dirname, IP_INOTIFY_MASK | IN_ONLYDIR, &err);
  if (wd < 0) {
      IP_W("Failed\n");
      return FALSE;
  } else {
      IP_W("Success\n");
      dir = ip_watched_dir_new(sub->dirname, wd);
      ip_map_wd_dir(wd, dir);
      ip_map_path_dir(sub->dirname, dir);
  }
  out:
  ip_map_sub_dir(sub, dir);
  return TRUE;
}
static void ip_unmap_path_dir(const char *path, ip_watched_dir_t *dir) {
  g_assert(path && dir);
  g_hash_table_remove(path_dir_hash, dir->path);
}
static void ip_unmap_wd_dir(gint32 wd, ip_watched_dir_t *dir) {
  GList *dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(wd));
  if (!dir_list) return;
  g_assert(wd >= 0 && dir);
  dir_list = g_list_remove(dir_list, dir);
  if (dir_list == NULL) g_hash_table_remove(wd_dir_hash, GINT_TO_POINTER(dir->wd));
  else g_hash_table_replace(wd_dir_hash, GINT_TO_POINTER(dir->wd), dir_list);
}
static void ip_unmap_wd(gint32 wd) {
  GList *dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(wd));
  if (!dir_list) return;
  g_assert(wd >= 0);
  g_hash_table_remove(wd_dir_hash, GINT_TO_POINTER(wd));
  g_list_free(dir_list);
}
static void ip_unmap_sub_dir(inotify_sub *sub, ip_watched_dir_t *dir) {
  g_assert(sub && dir);
  g_hash_table_remove(sub_dir_hash, sub);
  dir->subs = g_list_remove(dir->subs, sub);
}
static void ip_unmap_all_subs(ip_watched_dir_t *dir) {
  GList *l = NULL;
  for (l = dir->subs; l; l = l->next) {
      inotify_sub *sub = l->data;
      g_hash_table_remove(sub_dir_hash, sub);
  }
  g_list_free(dir->subs);
  dir->subs = NULL;
}
gboolean _ip_stop_watching(inotify_sub *sub) {
  ip_watched_dir_t *dir = NULL;
  dir = g_hash_table_lookup(sub_dir_hash, sub);
  if (!dir) return TRUE;
  ip_unmap_sub_dir(sub, dir);
  if (dir->subs == NULL) {
      _ik_ignore(dir->path, dir->wd);
      ip_unmap_wd_dir(dir->wd, dir);
      ip_unmap_path_dir(dir->path, dir);
      ip_watched_dir_free(dir);
  }
  return TRUE;
}
static ip_watched_dir_t *ip_watched_dir_new(const char *path, gint32 wd) {
  ip_watched_dir_t *dir = g_new0(ip_watched_dir_t, 1);
  dir->path = g_strdup(path);
  dir->wd = wd;
  return dir;
}
static void ip_watched_dir_free(ip_watched_dir_t *dir) {
  g_assert(dir->subs == NULL);
  g_free(dir->path);
  g_free(dir);
}
static void ip_wd_delete(gpointer data, gpointer user_data) {
  ip_watched_dir_t *dir = data;
  GList *l = NULL;
  for (l = dir->subs; l; l = l->next) {
      inotify_sub *sub = l->data;
      _im_add(sub);
  }
  ip_unmap_all_subs(dir);
  ip_unmap_path_dir(dir->path, dir);
  ip_watched_dir_free(dir);
}
static void ip_event_dispatch(GList *dir_list, GList *pair_dir_list, ik_event_t *event) {
  GList *dirl;
  if (!event) return;
  for (dirl = dir_list; dirl; dirl = dirl->next) {
      GList *subl;
      ip_watched_dir_t *dir = dirl->data;
      for (subl = dir->subs; subl; subl = subl->next) {
          inotify_sub *sub = subl->data;
          if (sub->filename && event->name && strcmp(sub->filename, event->name)) continue;
          if (sub->filename && !event->name) continue;
          event_callback(event, sub);
	  }
  }
  if (!event->pair) return;
  for (dirl = pair_dir_list; dirl; dirl = dirl->next) {
      GList *subl;
      ip_watched_dir_t *dir = dirl->data;
      for (subl = dir->subs; subl; subl = subl->next) {
          inotify_sub *sub = subl->data;
          if (sub->filename && event->pair->name && strcmp(sub->filename, event->pair->name)) continue;
          if (sub->filename && !event->pair->name) continue;
          event_callback (event->pair, sub);
	  }
  }
}
static void ip_event_callback(ik_event_t *event) {
  GList* dir_list = NULL;
  GList* pair_dir_list = NULL;
  dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(event->wd));
  if (event->mask & IN_IGNORED) {
      _ik_event_free(event);
      return;
  }
  if (event->pair) pair_dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(event->pair->wd));
  if (event->mask & IP_INOTIFY_MASK) ip_event_dispatch (dir_list, pair_dir_list, event);
  if (event->mask & IN_DELETE_SELF || event->mask & IN_MOVE_SELF || event->mask & IN_UNMOUNT) {
      g_list_foreach(dir_list, ip_wd_delete, NULL);
      ip_unmap_wd(event->wd);
  }
  _ik_event_free(event);
}
const char *_ip_get_path_for_wd(gint32 wd) {
  GList *dir_list;
  ip_watched_dir_t *dir;
  g_assert(wd >= 0);
  dir_list = g_hash_table_lookup(wd_dir_hash, GINT_TO_POINTER(wd));
  if (dir_list) {
      dir = dir_list->data;
      if (dir) return dir->path;
  }
  return NULL;
}