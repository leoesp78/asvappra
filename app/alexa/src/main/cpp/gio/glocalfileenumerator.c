#include <string.h>
#include <stdlib.h>
#include "../glib/glibintl.h"
#include "../glib/glib.h"
#include "config.h"
#include "glocalfileenumerator.h"
#include "glocalfileinfo.h"
#include "glocalfile.h"
#include "gioerror.h"

#define CHUNK_SIZE 1000
#ifndef G_OS_WIN32
#define USE_GDIR
#endif
#ifndef USE_GDIR
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

typedef struct {
  char *name;
  long inode;
} DirEntry;
#endif
struct _GLocalFileEnumerator {
  GFileEnumerator parent;
  GFileAttributeMatcher *matcher;
  char *filename;
  char *attributes;
  GFileQueryInfoFlags flags;
  gboolean got_parent_info;
  GLocalParentFileInfo parent_info;
#ifdef USE_GDIR
  GDir *dir;
#else
  DIR *dir;
  DirEntry *entries;
  int entries_pos;
  gboolean at_end;
#endif
  gboolean follow_symlinks;
};
#define g_local_file_enumerator_get_type _g_local_file_enumerator_get_type
G_DEFINE_TYPE (GLocalFileEnumerator, g_local_file_enumerator, G_TYPE_FILE_ENUMERATOR);
static GFileInfo *g_local_file_enumerator_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static gboolean g_local_file_enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static void free_entries(GLocalFileEnumerator *local) {
#ifndef USE_GDIR
  int i;
  if (local->entries != NULL) {
      for (i = 0; local->entries[i].name != NULL; i++) g_free (local->entries[i].name);
      g_free (local->entries);
  }
#endif
}
static void g_local_file_enumerator_finalize(GObject *object) {
  GLocalFileEnumerator *local;
  local = G_LOCAL_FILE_ENUMERATOR(object);
  if (local->got_parent_info) _g_local_file_info_free_parent_info(&local->parent_info);
  g_free(local->filename);
  g_file_attribute_matcher_unref(local->matcher);
  if (local->dir) {
  #ifdef USE_GDIR
      g_dir_close(local->dir);
  #else
      closedir(local->dir);
  #endif
      local->dir = NULL;
  }
  free_entries(local);
  G_OBJECT_CLASS(g_local_file_enumerator_parent_class)->finalize(object);
}
static void g_local_file_enumerator_class_init(GLocalFileEnumeratorClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GFileEnumeratorClass *enumerator_class = G_FILE_ENUMERATOR_CLASS(klass);
  gobject_class->finalize = g_local_file_enumerator_finalize;
  enumerator_class->next_file = g_local_file_enumerator_next_file;
  enumerator_class->close_fn = g_local_file_enumerator_close;
}
static void g_local_file_enumerator_init(GLocalFileEnumerator *local) {}
#ifdef USE_GDIR
static void convert_file_to_io_error(GError **error, GError *file_error) {
  int new_code;
  if (file_error == NULL) return;
  new_code = G_IO_ERROR_FAILED;
  if (file_error->domain == G_FILE_ERROR) {
      switch(file_error->code) {
          case G_FILE_ERROR_NOENT: new_code = G_IO_ERROR_NOT_FOUND; break;
          case G_FILE_ERROR_ACCES: new_code = G_IO_ERROR_PERMISSION_DENIED; break;
          case G_FILE_ERROR_NOTDIR: new_code = G_IO_ERROR_NOT_DIRECTORY; break;
          case G_FILE_ERROR_MFILE: new_code = G_IO_ERROR_TOO_MANY_OPEN_FILES; break;
      }
  }
  g_set_error_literal(error, G_IO_ERROR, new_code, file_error->message);
}
#endif
GFileEnumerator *_g_local_file_enumerator_new(GLocalFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error) {
  GLocalFileEnumerator *local;
  char *filename = g_file_get_path(G_FILE(file));
#ifdef USE_GDIR
  GError *dir_error;
  GDir *dir;
  dir_error = NULL;
  dir = g_dir_open(filename, 0, error != NULL ? &dir_error : NULL);
  if (dir == NULL) {
      if (error != NULL) {
          convert_file_to_io_error(error, dir_error);
          g_error_free(dir_error);
	  }
      g_free(filename);
      return NULL;
  }
#else
  DIR *dir;
  int errsv;
  dir = opendir(filename);
  if (dir == NULL) {
      errsv = errno;
      g_set_error_literal(error, G_IO_ERROR, g_io_error_from_errno(errsv), g_strerror(errsv));
      g_free(filename);
      return NULL;
  }
#endif
  local = g_object_new(G_TYPE_LOCAL_FILE_ENUMERATOR,"container", file, NULL);
  local->dir = dir;
  local->filename = filename;
  local->matcher = g_file_attribute_matcher_new(attributes);
  local->flags = flags;
  return G_FILE_ENUMERATOR(local);
}
#ifndef USE_GDIR
static int sort_by_inode(const void *_a, const void *_b) {
  const DirEntry *a, *b;
  a = _a;
  b = _b;
  return a->inode - b->inode;
}
static const char *next_file_helper(GLocalFileEnumerator *local) {
  struct dirent *entry;
  const char *filename;
  int i;
  if (local->at_end) return NULL;
  if (local->entries == NULL || (local->entries[local->entries_pos].name == NULL)) {
      if (local->entries == NULL) local->entries = g_new (DirEntry, CHUNK_SIZE + 1);
      else {
	      for (i = 0; local->entries[i].name != NULL; i++) g_free(local->entries[i].name);
	  }
      for (i = 0; i < CHUNK_SIZE; i++) {
          entry = readdir(local->dir);
          while(entry && (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, ".."))) entry = readdir(local->dir);
          if (entry) {
              local->entries[i].name = g_strdup(entry->d_name);
              local->entries[i].inode = entry->d_ino;
          } else break;
	  }
      local->entries[i].name = NULL;
      local->entries_pos = 0;
      qsort (local->entries, i, sizeof(DirEntry), sort_by_inode);
  }
  filename = local->entries[local->entries_pos++].name;
  if (filename == NULL) local->at_end = TRUE;
  return filename;
}
#endif
static GFileInfo *g_local_file_enumerator_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error) {
  GLocalFileEnumerator *local = G_LOCAL_FILE_ENUMERATOR(enumerator);
  const char *filename;
  char *path;
  GFileInfo *info;
  GError *my_error;
  if (!local->got_parent_info) {
      _g_local_file_info_get_parent_info(local->filename, local->matcher, &local->parent_info);
      local->got_parent_info = TRUE;
  }
  next_file:
#ifdef USE_GDIR
  filename = g_dir_read_name(local->dir);
#else
  filename = next_file_helper(local);
#endif
  if (filename == NULL) return NULL;
  my_error = NULL;
  path = g_build_filename(local->filename, filename, NULL);
  info = _g_local_file_info_get(filename, path, local->matcher, local->flags, &local->parent_info, &my_error);
  g_free(path);
  if (info == NULL) {
      if (g_error_matches(my_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
          g_error_free(my_error);
          goto next_file;
	  } else g_propagate_error(error, my_error);
  }
  return info;
}
static gboolean g_local_file_enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error) {
  GLocalFileEnumerator *local = G_LOCAL_FILE_ENUMERATOR(enumerator);
  if (local->dir) {
  #ifdef USE_GDIR
      g_dir_close(local->dir);
  #else
      closedir(local->dir);
  #endif
      local->dir = NULL;
  }
  return TRUE;
}