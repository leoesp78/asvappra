#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../glib.h"

typedef struct _CmdlineTest CmdlineTest;
struct _CmdlineTest {
  const gchar *cmdline;
  gint argc;
  const gchar *argv[10];
  gint error_code;
};
static CmdlineTest cmdline_tests[] = {
  { "foo bar", 2, { "foo", "bar", NULL }, -1 },
  { "foo 'bar'", 2, { "foo", "bar", NULL }, -1 },
  { "foo \"bar\"", 2, { "foo", "bar", NULL }, -1 },
  { "foo '' 'bar'", 3, { "foo", "", "bar", NULL }, -1 },
  { "foo \"bar\"'baz'blah'foo'\\''blah'\"boo\"", 2, { "foo", "barbazblahfoo'blahboo", NULL }, -1 },
  { "foo \t \tblah\tfoo\t\tbar  baz", 5, { "foo", "blah", "foo", "bar", "baz", NULL }, -1 },
  { "foo '    spaces more spaces lots of     spaces in this   '  \t", 2, { "foo", "    spaces more spaces lots of     spaces in this   ", NULL }, -1 },
  { "foo \\\nbar", 2, { "foo", "bar", NULL }, -1 },
  { "foo '' ''", 3, { "foo", "", "", NULL }, -1 },
  { "foo \\\" la la la", 5, { "foo", "\"", "la", "la", "la", NULL }, -1 },
  { "foo \\ foo woo woo\\ ", 4, { "foo", " foo", "woo", "woo ", NULL }, -1 },
  { "foo \"yada yada \\$\\\"\"", 2, { "foo", "yada yada $\"", NULL }, -1 },
  { "foo \"c:\\\\\"", 2, { "foo", "c:\\", NULL }, -1 },
  { "foo # bla bla bla\n bar", 2, { "foo", "bar", NULL }, -1 },
  { "foo bar \\", 0, { NULL }, G_SHELL_ERROR_BAD_QUOTING },
  { "foo 'bar baz", 0, { NULL }, G_SHELL_ERROR_BAD_QUOTING },
  { "foo '\"bar\" baz", 0, { NULL }, G_SHELL_ERROR_BAD_QUOTING },
  { "", 0, { NULL }, G_SHELL_ERROR_EMPTY_STRING },
  { "  ", 0, { NULL }, G_SHELL_ERROR_EMPTY_STRING },
  { "# foo bar", 0, { NULL }, G_SHELL_ERROR_EMPTY_STRING }
};
static gboolean strv_equal(gchar **a, gchar **b) {
  gint i;
  if (g_strv_length(a) != g_strv_length(b)) return FALSE;
  for (i = 0; a[i]; i++)
      if (g_strcmp0(a[i], b[i]) != 0) return FALSE;
  return TRUE;
}
static void do_cmdline_test(gconstpointer d) {
  const CmdlineTest *test = d;
  gint argc;
  gchar **argv;
  GError *err;
  gboolean res;
  err = NULL;
  res = g_shell_parse_argv(test->cmdline, &argc, &argv, &err);
  if (test->error_code == -1) {
      g_assert(res);
      g_assert_cmpint(argc, ==, test->argc);
      g_assert(strv_equal(argv, (gchar**)test->argv));
      g_assert_no_error(err);
  } else {
      g_assert(!res);
      g_assert_error(err, G_SHELL_ERROR, test->error_code);
  }
  if (err) g_error_free(err);
  if (res) g_strfreev(argv);
}
typedef struct _QuoteTest QuoteTest;
struct _QuoteTest {
  const gchar *in;
  const gchar *out;
};
static QuoteTest quote_tests[] = {
  { "", "''" },
  { "a", "'a'" },
  { "(", "'('" },
  { "'", "''\\'''" },
  { "'a", "''\\''a'" },
  { "a'", "'a'\\'''" },
  { "a'a", "'a'\\''a'" }
};
static void do_quote_test(gconstpointer d) {
  const QuoteTest *test = d;
  gchar *out;
  out = g_shell_quote(test->in);
  g_assert_cmpstr(out, ==, test->out);
  g_free(out);
}
typedef struct _UnquoteTest UnquoteTest;
struct _UnquoteTest {
  const gchar *in;
  const gchar *out;
  gint error_code;
};
static UnquoteTest unquote_tests[] = {
  { "", "", -1 },
  { "a", "a", -1 },
  { "'a'", "a", -1 },
  { "'('", "(", -1 },
  { "''\\'''", "'", -1 },
  { "''\\''a'", "'a", -1 },
  { "'a'\\'''", "a'", -1 },
  { "'a'\\''a'", "a'a", -1 },
  { "\\\\", "\\", -1 },
  { "\\\n", "", -1 },
  { "'\\''", NULL, G_SHELL_ERROR_BAD_QUOTING },
  { "\"\\\"\"", "\"", -1 },
  { "\"", NULL, G_SHELL_ERROR_BAD_QUOTING },
  { "'", NULL, G_SHELL_ERROR_BAD_QUOTING },
  { "\x22\\\\\"", "\\", -1 },
  { "\x22\\`\"", "`", -1 },
  { "\x22\\$\"", "$", -1 },
  { "\x22\\\n\"", "\n", -1 },
  { "\"\\'\"", "\\'", -1 },
  { "\x22\\\r\"", "\\\r", -1 },
  { "\x22\\n\"", "\\n", -1 }
};
static void do_unquote_test (gconstpointer d) {
  const UnquoteTest *test = d;
  gchar *out;
  GError *error;
  error = NULL;
  out = g_shell_unquote (test->in, &error);
  g_assert_cmpstr(out, ==, test->out);
  if (test->error_code == -1) { g_assert_no_error(error); }
  else { g_assert_error(error, G_SHELL_ERROR, test->error_code); }
  g_free (out);
  if (error) g_error_free(error);
}
int main(int argc, char *argv[]) {
  gint i;
  gchar *path;
  g_test_init(&argc, &argv, NULL);
  for (i = 0; i < G_N_ELEMENTS(cmdline_tests); i++) {
      path = g_strdup_printf("/shell/cmdline/%d", i);
      g_test_add_data_func(path, &cmdline_tests[i], do_cmdline_test);
      g_free(path);
  }
  for (i = 0; i < G_N_ELEMENTS(quote_tests); i++) {
      path = g_strdup_printf("/shell/quote/%d", i);
      g_test_add_data_func(path, &quote_tests[i], do_quote_test);
      g_free(path);
  }
  for (i = 0; i < G_N_ELEMENTS (unquote_tests); i++) {
      path = g_strdup_printf("/shell/unquote/%d", i);
      g_test_add_data_func(path, &unquote_tests[i], do_unquote_test);
      g_free(path);
  }
  return g_test_run();
}