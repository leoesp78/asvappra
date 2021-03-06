#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_TEST_UTILS_H__
#define __G_TEST_UTILS_H__

#include "gmessages.h"
#include "gstring.h"
#include "gerror.h"
#include "gslist.h"

G_BEGIN_DECLS
typedef struct GTestCase  GTestCase;
typedef struct GTestSuite GTestSuite;
typedef void (*GTestFunc)(void);
typedef void (*GTestDataFunc)(gconstpointer user_data);
typedef void (*GTestFixtureFunc)(gpointer fixture, gconstpointer user_data);
#define g_assert_cmpstr(s1, cmp, s2)                                                                                      \
do {                                                                                                                      \
    const char *__s1 = (s1), *__s2 = (s2);                                                                                \
    if (g_strcmp0(__s1, __s2) cmp 0);                                                                                     \
    else g_assertion_message_cmpstr(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #s1 " " #cmp " " #s2, __s1, #cmp, __s2); \
} while(0);
#define g_assert_cmpint(n1, cmp, n2)                                                                                            \
do {                                                                                                                            \
    gint64 __n1 = (n1), __n2 = (n2);                                                                                            \
    if (__n1 cmp __n2);                                                                                                         \
    else g_assertion_message_cmpnum(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'i');  \
} while(0);
#define g_assert_cmpuint(n1, cmp, n2)                                                                                           \
do {                                                                                                                            \
    guint64 __n1 = (n1), __n2 = (n2);                                                                                           \
    if (__n1 cmp __n2);                                                                                                         \
    else g_assertion_message_cmpnum(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'i');  \
} while(0);
#define g_assert_cmphex(n1, cmp, n2)                                                                                            \
do {                                                                                                                            \
    guint64 __n1 = (n1), __n2 = (n2);                                                                                           \
    if (__n1 cmp __n2);                                                                                                         \
    else g_assertion_message_cmpnum(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'x');  \
} while(0);
#define g_assert_cmpfloat(n1,cmp,n2)                                                                                            \
do {                                                                                                                            \
    long double __n1 = (n1), __n2 = (n2);                                                                                       \
    if (__n1 cmp __n2);                                                                                                         \
    else g_assertion_message_cmpnum(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'f');  \
} while(0);
#define g_assert_no_error(err)                                                                         \
do {                                                                                                   \
    if (err) g_assertion_message_error(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #err, err, 0, 0);  \
} while(0);
#define g_assert_error(err, dom, c)	                                                               \
do {                                                                                               \
    if (!err || (err)->domain != dom || (err)->code != c)                                          \
        g_assertion_message_error(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #err, err, dom, c); \
} while(0);
#ifdef G_DISABLE_ASSERT
#define g_assert_not_reached() do { (void) 0; } while(0);
#define g_assert(expr) do { (void) 0; } while(0);
#else
#define g_assert_not_reached()  do { g_assertion_message (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, NULL); } while(0);
#define g_assert(expr)                                                                 \
do {                                                                                   \
    if G_LIKELY(expr);                                                                 \
    else g_assertion_message_expr(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #expr); \
} while (0);
#endif
int g_strcmp0(const char *str1, const char *str2);
void g_test_minimized_result(double minimized_quantity, const char *format, ...) G_GNUC_PRINTF(2, 3);
void g_test_maximized_result(double maximized_quantity, const char *format, ...) G_GNUC_PRINTF(2, 3);
void g_test_init(int *argc, char ***argv, ...);
#define g_test_quick()  (g_test_config_vars->test_quick)
#define g_test_slow()  (!g_test_config_vars->test_quick)
#define g_test_thorough() (!g_test_config_vars->test_quick)
#define g_test_perf()  (g_test_config_vars->test_perf)
#define g_test_verbose()  (g_test_config_vars->test_verbose)
#define g_test_quiet()  (g_test_config_vars->test_quiet)
int g_test_run(void);
void g_test_add_func(const char *testpath, GTestFunc test_func);
void g_test_add_data_func(const char *testpath, gconstpointer test_data, GTestDataFunc test_func);
#define g_test_add(testpath, Fixture, tdata, fsetup, ftest, fteardown)                                                                        \
G_STMT_START {			                                                                                                                      \
    void (*add_vtable)(const char*, gsize, gconstpointer, void (*)(Fixture*, gconstpointer), void (*)(Fixture*, gconstpointer),               \
                       void (*)(Fixture*, gconstpointer)) = (void (*)(const gchar *, gsize, gconstpointer, void (*)(Fixture*, gconstpointer), \
                       void (*) (Fixture*, gconstpointer), void (*) (Fixture*, gconstpointer))) g_test_add_vtable;                            \
add_vtable(testpath, sizeof (Fixture), tdata, fsetup, ftest, fteardown);                                                                      \
} G_STMT_END
void g_test_message(const char *format, ...) G_GNUC_PRINTF(1, 2);
void g_test_bug_base(const char *uri_pattern);
void g_test_bug(const char *bug_uri_snippet);
void g_test_timer_start(void);
double g_test_timer_elapsed(void);
double g_test_timer_last(void);
void g_test_queue_free(gpointer gfree_pointer);
void g_test_queue_destroy(GDestroyNotify destroy_func, gpointer destroy_data);
#define g_test_queue_unref(gobject)  g_test_queue_destroy(g_object_unref, gobject)
typedef enum {
  G_TEST_TRAP_SILENCE_STDOUT = 1 << 7,
  G_TEST_TRAP_SILENCE_STDERR = 1 << 8,
  G_TEST_TRAP_INHERIT_STDIN = 1 << 9
} GTestTrapFlags;
gboolean g_test_trap_fork(guint64 usec_timeout, GTestTrapFlags test_trap_flags);
gboolean g_test_trap_has_passed(void);
gboolean g_test_trap_reached_timeout(void);
#define g_test_trap_assert_passed()  g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 0, 0)
#define g_test_trap_assert_failed()  g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 1, 0)
#define g_test_trap_assert_stdout(soutpattern)  g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 2, soutpattern)
#define g_test_trap_assert_stdout_unmatched(soutpattern) g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 3, soutpattern)
#define g_test_trap_assert_stderr(serrpattern) g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 4, serrpattern)
#define g_test_trap_assert_stderr_unmatched(serrpattern) g_test_trap_assertions(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, 5, serrpattern)
#define g_test_rand_bit() (0 != (g_test_rand_int() & (1 << 15)))
gint32 g_test_rand_int(void);
gint32 g_test_rand_int_range(gint32 begin, gint32 end);
double g_test_rand_double(void);
double g_test_rand_double_range(double range_start, double range_end);
GTestCase* g_test_create_case(const char *test_name, gsize data_size, gconstpointer test_data, GTestFixtureFunc data_setup, GTestFixtureFunc data_test,
                              GTestFixtureFunc data_teardown);
GTestSuite* g_test_create_suite(const char *suite_name);
GTestSuite* g_test_get_root(void);
void g_test_suite_add(GTestSuite *suite, GTestCase *test_case);
void g_test_suite_add_suite(GTestSuite *suite, GTestSuite *nestedsuite);
int g_test_run_suite(GTestSuite *suite);
void g_test_trap_assertions(const char *domain, const char *file, int line, const char *func, guint64 assertion_flags, const char *pattern);
void g_assertion_message(const char *domain, const char *file, int line, const char *func, const char *message) G_GNUC_NORETURN;
void g_assertion_message_expr(const char *domain, const char *file, int line, const char *func, const char *expr) G_GNUC_NORETURN;
void g_assertion_message_cmpstr(const char *domain, const char *file, int line, const char *func, const char *expr, const char *arg1, const char *cmp,
                                const char *arg2) G_GNUC_NORETURN;
void g_assertion_message_cmpnum(const char *domain, const char *file, int line, const char *func, const char *expr, long double arg1, const char *cmp,
                                long double arg2, char numtype) G_GNUC_NORETURN;
void g_assertion_message_error(const char *domain, const char *file, int line, const char *func, const char *expr, const GError *error, GQuark error_domain,
                               int error_code) G_GNUC_NORETURN;
void g_test_add_vtable(const char *testpath, gsize data_size, gconstpointer test_data, GTestFixtureFunc data_setup, GTestFixtureFunc data_test,
                       GTestFixtureFunc data_teardown);
typedef struct {
  gboolean test_initialized;
  gboolean test_quick;
  gboolean test_perf;
  gboolean test_verbose;
  gboolean test_quiet;
} GTestConfig;
GLIB_VAR const GTestConfig* const g_test_config_vars;
typedef enum {
  G_TEST_LOG_NONE,
  G_TEST_LOG_ERROR,
  G_TEST_LOG_START_BINARY,
  G_TEST_LOG_LIST_CASE,
  G_TEST_LOG_SKIP_CASE,
  G_TEST_LOG_START_CASE,
  G_TEST_LOG_STOP_CASE,
  G_TEST_LOG_MIN_RESULT,
  G_TEST_LOG_MAX_RESULT,
  G_TEST_LOG_MESSAGE
} GTestLogType;
typedef struct {
  GTestLogType log_type;
  guint n_strings;
  gchar **strings;
  guint n_nums;
  long double *nums;
} GTestLogMsg;
typedef struct {
  GString *data;
  GSList *msgs;
} GTestLogBuffer;
const char* g_test_log_type_name(GTestLogType log_type);
GTestLogBuffer* g_test_log_buffer_new(void);
void g_test_log_buffer_free(GTestLogBuffer *tbuffer);
void g_test_log_buffer_push(GTestLogBuffer *tbuffer, guint n_bytes, const guint8 *bytes);
GTestLogMsg* g_test_log_buffer_pop(GTestLogBuffer *tbuffer);
void g_test_log_msg_free(GTestLogMsg *tmsg);
typedef gboolean(*GTestLogFatalFunc)(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
void g_test_log_set_fatal_handler(GTestLogFatalFunc log_func, gpointer user_data);
G_END_DECLS

#endif