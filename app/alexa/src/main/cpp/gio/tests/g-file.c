#include <stdlib.h>
#include <string.h>
#include "../../glib/glib.h"
#include "../gio.h"

struct TestPathsWithOper {
  const char *path1;
  gboolean equal;
  gboolean use_uri;
  const char *path2;
  const char *path3;
};
static void test_g_file_new_null(void) {
  const char *paths[] = { "/", "/tmp///", "/non-existent-file", "/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88",
			              NULL };
  const char *uris[] = { "file:///", "file:///tmp///", "non-existent-uri:///some-dir/", "file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD"
                         "%20k%C5%AF%C5%88", NULL };
  GFile *file = NULL;
  int i = 0;
  while(paths[i]) {
      file = g_file_new_for_path(paths[i++]);
      g_assert(file != NULL);
      g_object_unref(file);
  }
  i = 0;
  while(uris[i]) {
      file = g_file_new_for_uri(uris[i++]);
      g_assert(file != NULL);
      g_object_unref(file);
  }
}
static gboolean compare_two_files(const gboolean use_uri, const char *path1, const char *path2) {
  GFile *file1 = NULL;
  GFile *file2 = NULL;
  gboolean equal;
  if (use_uri) {
      file1 = g_file_new_for_uri(path1);
      file2 = g_file_new_for_uri(path2);
  } else {
      file1 = g_file_new_for_path(path1);
      file2 = g_file_new_for_path(path2);
  }
  g_assert(file1 != NULL);
  g_assert(file2 != NULL);
  equal = g_file_equal(file1, file2);
  g_object_unref(file1);
  g_object_unref(file2);
  return equal;
}
static void test_g_file_new_for_path(void) {
  const struct TestPathsWithOper cmp_paths[] = {
      {"/", TRUE, 0, "/./"},
      {"//", TRUE, 0, "//"},
      {"//", TRUE, 0, "//./"},
      {"/", TRUE, 0, "/.//"},
      {"/", TRUE, 0, "/././"},
      {"/tmp", TRUE, 0, "/tmp/d/../"},
      {"/", TRUE, 0, "/somedir/../"},
      {"/", FALSE, 0, "/somedir/.../"},
      {"//tmp/dir1", TRUE, 0, "//tmp/dir1"},
      {"/tmp/dir1", TRUE, 0, "///tmp/dir1"},
      {"/tmp/dir1", TRUE, 0, "////tmp/dir1"},
      {"/tmp/dir1", TRUE, 0, "/tmp/./dir1"},
      {"/tmp/dir1", TRUE, 0, "/tmp//dir1"},
      {"/tmp/dir1", TRUE, 0, "/tmp///dir1///"},
      {"/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", TRUE, 0, "/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88/"}
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS(cmp_paths); i++) {
      gboolean equal = compare_two_files(FALSE, cmp_paths[i].path1, cmp_paths[i].path2);
      g_assert_cmpint(equal, ==, cmp_paths[i].equal);
  }
}
static void test_g_file_new_for_uri(void) {
  const struct TestPathsWithOper cmp_uris[] = {
      {"file:///", TRUE, 0, "file:///./"},
      {"file:////", TRUE, 0, "file:////"},
      {"file:////", TRUE, 0, "file:////./"},
      {"file:///", TRUE, 0, "file:///.//"},
      {"file:///", TRUE, 0, "file:///././"},
      {"file:///tmp", TRUE, 0, "file:///tmp/d/../"},
      {"file:///", TRUE, 0, "file:///somedir/../"},
      {"file:///", FALSE, 0, "file:///somedir/.../"},
      {"file:////tmp/dir1", TRUE, 0, "file:////tmp/dir1"},
      {"file:///tmp/dir1", TRUE, 0, "file:///tmp/./dir1"},
      {"file:///tmp/dir1", TRUE, 0, "file:///tmp//dir1"},
      {"file:///tmp/dir1", TRUE, 0, "file:///tmp///dir1///"},
      {"file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88", TRUE, 0, "file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88/"}
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS(cmp_uris); i++) {
      gboolean equal = compare_two_files(TRUE, cmp_uris[i].path1, cmp_uris[i].path2);
      g_assert_cmpint(equal, ==, cmp_uris[i].equal);
  }
}
static gboolean dup_equals(const gboolean use_uri, const char *path) {
  GFile *file1 = NULL;
  GFile *file2 = NULL;
  gboolean equal;
  if (use_uri) file1 = g_file_new_for_uri(path);
  else file1 = g_file_new_for_path(path);
  g_assert(file1 != NULL);
  file2 = g_file_dup(file1);
  g_assert(file2 != NULL);
  equal = g_file_equal(file1, file2);
  g_object_unref(file1);
  g_object_unref(file2);
  return equal;
}
static void test_g_file_dup(void) {
  const struct TestPathsWithOper dup_paths[] = {
      {"/", 0, FALSE, ""},
      {"file:///", 0, TRUE, ""},
      {"totalnonsense", 0, FALSE, ""},
      {"/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", 0, FALSE, ""},
      {"file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88", 0, TRUE, ""},
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS(dup_paths); i++) {
      gboolean equal = dup_equals(dup_paths[i].use_uri, dup_paths[i].path1);
      g_assert(equal == TRUE);
  }
}
static gboolean parse_check_utf8(const gboolean use_uri, const char *path, const char *result_parse_name) {
  GFile *file1 = NULL;
  GFile *file2 = NULL;
  char *parsed_name;
  gboolean is_utf8_valid;
  gboolean equal;
  if (use_uri) file1 = g_file_new_for_uri(path);
  else file1 = g_file_new_for_path(path);
  g_assert(file1 != NULL);
  parsed_name = g_file_get_parse_name(file1);
  g_assert(parsed_name != NULL);
  is_utf8_valid = g_utf8_validate(parsed_name, -1, NULL);
  g_assert(is_utf8_valid == TRUE);
  if (result_parse_name) g_assert_cmpstr(parsed_name, ==, result_parse_name);
  file2 = g_file_parse_name(parsed_name);
  g_assert(file2 != NULL);
  equal = g_file_equal(file1, file2);
  g_object_unref(file1);
  g_object_unref(file2);
  g_free(parsed_name);
  return equal;
}
static void test_g_file_get_parse_name_utf8 (void) {
  const struct TestPathsWithOper strings[] = {
      {"/", 0, FALSE, "/"},
      {"file:///", 0, TRUE, "/"},
      {"totalnonsense", 0, FALSE, NULL},
      {"/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", 0, FALSE, NULL },
      {"file:///invalid%08/UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88/", 0, TRUE, "file:///invalid%08/UTF-8%20p\xc5\x99\xc3\xadli\xc5\xa1%20\xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd%20k\xc5\xaf\xc5\x88"},
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS(strings); i++) {
      gboolean equal = parse_check_utf8(strings[i].use_uri, strings[i].path1, strings[i].path2);
      g_assert(equal == TRUE);
  }
}
static char *resolve_arg(const gboolean is_uri_only, const char *arg) {
  GFile *file1 = NULL;
  char *uri = NULL;
  char *path = NULL;
  char *s = NULL;
  file1 = g_file_new_for_commandline_arg(arg);
  g_assert(file1 != NULL);
  uri = g_file_get_uri (file1);
  g_assert_cmpstr(uri, !=, NULL);
  g_print("%s\n",uri);
  path = g_file_get_path(file1);
  if (is_uri_only) { g_assert_cmpstr(path, ==, NULL); }
  else { g_assert (g_path_is_absolute(path) == TRUE); }
  s = g_file_get_uri_scheme(file1);
  g_object_unref(file1);
  g_free(uri);
  g_free(path);
  return s;
}
static void test_g_file_new_for_commandline_arg(void) {
  const struct TestPathsWithOper arg_data[] = {
      {"./", 0, FALSE, "file" },
      {"../", 0, FALSE, "file" },
      {"/tmp", 0, FALSE, "file" },
      {"//UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", 0, FALSE, "file" },
      {"file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88/", 0, FALSE, "file" },
  #if 0
      { "http://www.gtk.org/", 0, TRUE, "http" },
      { "ftp://user:pass@ftp.gimp.org/", 0, TRUE, "ftp" },
  #endif
  };
  GFile *file;
  char *resolved;
  char *cwd;
  guint i;
  for (i = 0; i < G_N_ELEMENTS(arg_data); i++) {
      char *s = resolve_arg(arg_data[i].use_uri, arg_data[i].path1);
      g_assert_cmpstr(s, ==, arg_data[i].path2);
      g_free(s);
  }
  file = g_file_new_for_commandline_arg("./");
  resolved = g_file_get_path(file);
  cwd = g_get_current_dir();
  g_assert_cmpstr(resolved, ==, cwd);
  g_object_unref(file);
  g_free(resolved);
  g_free(cwd);
}
static char* get_relative_path(const gboolean use_uri, const gboolean should_have_prefix, const char *dir1, const char *dir2) {
  GFile *file1 = NULL;
  GFile *file2 = NULL;
  GFile *file3 = NULL;
  gboolean has_prefix = FALSE;
  char *relative_path = NULL;
  if (use_uri) {
      file1 = g_file_new_for_uri(dir1);
      file2 = g_file_new_for_uri(dir2);
  } else {
      file1 = g_file_new_for_path(dir1);
      file2 = g_file_new_for_path(dir2);
  }
  g_assert(file1 != NULL);
  g_assert(file2 != NULL);
  has_prefix = g_file_has_prefix(file2, file1);
  g_print("%s %s\n", dir1, dir2);
  g_assert(has_prefix == should_have_prefix);
  relative_path = g_file_get_relative_path(file1, file2);
  if (should_have_prefix) {
      g_assert(relative_path != NULL);
      file3 = g_file_resolve_relative_path(file1, relative_path);
      g_assert(g_file_equal(file2, file3) == TRUE);
  }
  if (file1) g_object_unref(file1);
  if (file2) g_object_unref(file2);
  if (file3) g_object_unref(file3);
  return relative_path;
}
static void test_g_file_has_prefix (void) {
  const struct TestPathsWithOper dirs[] = {
      {"/dir1", TRUE, FALSE, "/dir1/dir2/dir3/", "dir2/dir3" },
      {"/dir1/", TRUE, FALSE, "/dir1/dir2/dir3/", "dir2/dir3" },
      {"/dir1", TRUE, FALSE, "/dir1/dir2/dir3", "dir2/dir3" },
      {"/dir1/", TRUE, FALSE, "/dir1/dir2/dir3", "dir2/dir3" },
      {"/tmp/", FALSE, FALSE, "/something/", NULL },
      {"/dir1/dir2", FALSE, FALSE, "/dir1/", NULL },
      {"//dir1/new", TRUE, FALSE, "//dir1/new/dir2/dir3", "dir2/dir3" },
      {"/dir/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", TRUE, FALSE, "/dir/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88/dir2", "dir2" },
      {"file:///dir1", TRUE, TRUE, "file:///dir1/dir2/dir3/", "dir2/dir3" },
      {"file:///dir1/", TRUE, TRUE, "file:///dir1/dir2/dir3/", "dir2/dir3" },
      {"file:///dir1", TRUE, TRUE, "file:///dir1/dir2/dir3", "dir2/dir3" },
      {"file:///dir1/", TRUE, TRUE, "file:///dir1/dir2/dir3", "dir2/dir3" },
      {"file:///tmp/", FALSE, TRUE, "file:///something/", NULL },
      {"file:///dir1/dir2", FALSE, TRUE, "file:///dir1/", NULL },
      {"file:////dir1/new", TRUE, TRUE, "file:////dir1/new/dir2/dir3", "dir2/dir3" },
      {"file:///dir/UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88", TRUE, TRUE, "file:///dir/UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88/dir2", "dir2" },
  #if 0
      { "dav://www.gtk.org/plan/", TRUE, TRUE, "dav://www.gtk.org/plan/meetings/20071218.txt", "meetings/20071218.txt" },
      { "dav://www.gtk.org/plan/meetings", TRUE, TRUE, "dav://www.gtk.org/plan/meetings/20071218.txt", "20071218.txt" },
  #endif
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS (dirs); i++) {
      char *s = get_relative_path(dirs[i].use_uri, dirs[i].equal, dirs[i].path1, dirs[i].path2);
      if (dirs[i].equal) g_assert_cmpstr(s, ==, dirs[i].path3);
      g_free(s);
  }
}
static void roundtrip_parent_child(const gboolean use_uri, const gboolean under_root_descending, const char *path, const char *dir_holder) {
  GFile *files[6] = { NULL };
  guint i;
  if (use_uri) {
      files[0] = g_file_new_for_uri(path);
      files[1] = g_file_new_for_uri(path);
  } else {
      files[0] = g_file_new_for_path(path);
      files[1] = g_file_new_for_path(path);
  }
  g_assert(files[0] != NULL);
  g_assert(files[1] != NULL);
  files[2] = g_file_get_child(files[1], dir_holder);
  g_assert(files[2] != NULL);
  files[3] = g_file_get_parent(files[2]);
  g_assert(files[3] != NULL);
  g_assert(g_file_equal(files[3], files[0]) == TRUE);
  files[4] = g_file_get_parent(files[3]);
  if (under_root_descending) { g_assert(files[4] == NULL); }
  else {
      g_assert(files[4] != NULL);
      files[5] = g_file_get_child(files[4], dir_holder);
      g_assert(files[5] != NULL);
      g_assert(g_file_equal(files[5], files[0]) == TRUE);
  }
  for (i = 0; i < G_N_ELEMENTS(files); i++) {
      if (files[i]) g_object_unref(files[i]);
  }
}
static void test_g_file_get_parent_child(void) {
  const struct TestPathsWithOper paths[] = {
      {"/dir1/dir", FALSE, FALSE, "dir" },
      {"/dir", FALSE, FALSE, "dir" },
      {"/", TRUE, FALSE, "dir" },
      {"/UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88/", FALSE, FALSE, "UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88" },
      {"file:///dir1/dir", FALSE, TRUE, "dir" },
      {"file:///dir", FALSE, TRUE, "dir" },
      {"file:///", TRUE, TRUE, "dir" },
      {"file:///UTF-8%20p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88/", FALSE, TRUE, "UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88" },
      {"dav://www.gtk.org/plan/meetings", FALSE, TRUE, "meetings" },
  };
  guint i;
  for (i = 0; i < G_N_ELEMENTS(paths); i++) roundtrip_parent_child(paths[i].use_uri, paths[i].equal, paths[i].path1, paths[i].path2);
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/g-file/test_g_file_new_null", test_g_file_new_null);
  g_test_add_func("/g-file/test_g_file_new_for_path", test_g_file_new_for_path);
  g_test_add_func("/g-file/test_g_file_new_for_uri", test_g_file_new_for_uri);
  g_test_add_func("/g-file/test_g_file_dup", test_g_file_dup);
  g_test_add_func("/g-file/test_g_file_get_parse_name_utf8", test_g_file_get_parse_name_utf8);
  g_test_add_func("/g-file/test_g_file_new_for_commandline_arg", test_g_file_new_for_commandline_arg);
  g_test_add_func("/g-file/test_g_file_has_prefix", test_g_file_has_prefix);
  g_test_add_func("/g-file/test_g_file_get_parent_child", test_g_file_get_parent_child);
  return g_test_run();
}