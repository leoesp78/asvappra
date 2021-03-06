#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "../glib.h"

typedef struct {
  const gchar **input;
  const gchar **sorted;
  const gchar **file_sorted;
} CollateTest;
typedef struct {
  const gchar *key;
  const gchar *str;
} Line;
int compare_collate(const void *a, const void *b) {
  const Line *line_a = a;
  const Line *line_b = b;
  return g_utf8_collate(line_a->str, line_b->str);
}
int compare_key(const void *a, const void *b) {
  const Line *line_a = a;
  const Line *line_b = b;
  return strcmp(line_a->key, line_b->key);
}
static void do_collate(gboolean for_file, gboolean use_key, const CollateTest *test) {
  GArray *line_array = g_array_new(FALSE, FALSE, sizeof(Line));
  Line line;
  gint i;
  for (i = 0; test->input[i]; i++) {
      line.str = test->input[i];
      if (for_file) line.key = g_utf8_collate_key_for_filename(line.str, -1);
      else line.key = g_utf8_collate_key(line.str, -1);
      g_array_append_val(line_array, line);
  }
  qsort(line_array->data, line_array->len, sizeof(Line), use_key ? compare_key : compare_collate);
  for (i = 0; test->input[i]; i++) {
      const gchar *str;
      str = g_array_index(line_array, Line, i).str;
      if (for_file) { g_assert_cmpstr(str, ==, test->file_sorted[i]); }
      else { g_assert_cmpstr(str, ==, test->sorted[i]); }
  }
}
static void test_collate(gconstpointer d) {
  const CollateTest *test = d;
  do_collate(FALSE, FALSE, test);
}
static void test_collate_key(gconstpointer d) {
  const CollateTest *test = d;
  do_collate(FALSE, TRUE, test);
}
static void test_collate_file(gconstpointer d) {
  const CollateTest *test = d;
  do_collate(TRUE, TRUE, test);
}
const gchar *input0[] = {
  "z",
  "c",
  "eer34",
  "223",
  "er1",
  "foo",
  "bar",
  "baz",
  "GTK+",
  NULL
};
const gchar *sorted0[] = {
  "223",
  "bar",
  "baz",
  "c",
  "eer34",
  "er1",
  "foo",
  "GTK+",
  "z",
  NULL
};
const gchar *file_sorted0[] = {
  "223",
  "bar",
  "baz",
  "c",
  "eer34",
  "er1",
  "foo",
  "GTK+",
  "z",
  NULL
};
const gchar *input1[] = {
  "file.txt",
  "file2.bla",
  "file.c",
  "file3.xx",
  "bla001",
  "bla02",
  "bla03",
  "bla4",
  "bla10",
  "bla100",
  "event.c",
  "eventgenerator.c",
  "event.h",
  NULL
};
const gchar *sorted1[] = {
  "bla001",
  "bla02",
  "bla03",
  "bla10",
  "bla100",
  "bla4",
  "event.c",
  "eventgenerator.c",
  "event.h",
  "file2.bla",
  "file3.xx",
  "file.c",
  "file.txt",
  NULL
};
const gchar *file_sorted1[] = {
  "bla001",
  "bla02",
  "bla03",
  "bla4",
  "bla10",
  "bla100",
  "event.c",
  "event.h",
  "eventgenerator.c",
  "file.c",
  "file.txt",
  "file2.bla",
  "file3.xx",
  NULL
};
int main(int argc, char *argv[]) {
  gchar *path;
  gint i;
  const gchar *locale;
  CollateTest test[2];
  g_test_init(&argc, &argv, NULL);
  g_setenv("LC_ALL", "en_US", TRUE);
  locale = setlocale(LC_ALL, "");
  if (locale == NULL || strcmp (locale, "en_US") != 0) {
      g_test_message("No suitable locale, skipping test");
      return 0;
  }
  test[0].input = input0;
  test[0].sorted = sorted0;
  test[0].file_sorted = file_sorted0;
  test[1].input = input1;
  test[1].sorted = sorted1;
  test[1].file_sorted = file_sorted1;
  for (i = 0; i < G_N_ELEMENTS(test); i++) {
      path = g_strdup_printf("/unicode/collate/%d", i);
      g_test_add_data_func(path, &test[i], test_collate);
      g_free(path);
      path = g_strdup_printf("/unicode/collate-key/%d", i);
      g_test_add_data_func(path, test, test_collate_key);
      g_free(path);
      path = g_strdup_printf("/unicode/collate-filename/%d", i);
      g_test_add_data_func(path, test, test_collate_file);
      g_free(path);
  }
  return g_test_run();
}