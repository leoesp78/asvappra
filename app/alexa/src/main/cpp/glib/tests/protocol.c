#include <errno.h>
#include <unistd.h>
#include "../glib.h"

static void debug(void) {
  if (g_test_verbose()) g_debug("this is a regular g_debug() from the test suite");
}
static void info(void) {
#ifdef g_info
#error "rewrite this to use g_info()"
#endif
  if (g_test_verbose()) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "this is a regular g_log(..., G_LOG_LEVEL_INFO, ...) from the test suite");
}
static void message(void) {
  if (g_test_verbose()) g_message("this is a regular g_message() from the test suite");
}
static void warning(void) {
  if (g_test_verbose()) g_warning("this is a regular g_warning() from the test suite");
}
static void critical(void) {
  if (g_test_verbose()) g_critical("this is a regular g_critical() from the test suite");
}
static void _error(void) {
  if (g_test_verbose()) g_error("this is a regular g_error() from the test suite");
}
static void gtest_message(void) {
  if (g_test_verbose ()) g_test_message ("this is a regular g_test_message() from the test suite");
}
static gboolean test_message_cb1(GIOChannel *channel, GIOCondition condition, gpointer user_data) {
  GIOStatus  status;
  guchar buf[512];
  gsize read_bytes = 0;
  g_assert_cmpuint(condition, ==, G_IO_IN);
  status = g_io_channel_read_chars(channel, (gchar*)buf, sizeof(buf), &read_bytes, NULL);
  for ( ; status == G_IO_STATUS_NORMAL; status = g_io_channel_read_chars(channel, (gchar*)buf, sizeof(buf), &read_bytes, NULL)) {
      g_test_log_buffer_push (user_data, read_bytes, buf);
  }
  g_assert_cmpuint(status, ==, G_IO_STATUS_AGAIN);
  return TRUE;
}
static void test_message_cb2(GPid pid, gint status, gpointer user_data) {
  g_spawn_close_pid (pid);
  g_main_loop_quit (user_data);
}
static void test_message(void) {
  gchar* argv[] = { "./protocol", NULL, "--verbose", "-p", "/glib/testing/protocol/debug", "-p", "/glib/testing/protocol/message",
                    "-p", "/glib/testing/protocol/gtest-message", NULL };
  GTestLogBuffer* tlb;
  GTestLogMsg* msg;
  GIOChannel* channel;
  GMainLoop* loop;
  GError* error = NULL;
  gulong child_source;
  gulong io_source;
  GPid pid = 0;
  int pipes[2];
  int passed = 0;
  int messages = 0;
  if (0 > pipe (pipes)) g_error ("error creating pipe: %s", g_strerror (errno));
  argv[1] = g_strdup_printf ("--GTestLogFD=%u", pipes[1]);
  if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_LEAVE_DESCRIPTORS_OPEN | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
                     NULL, NULL, &pid, &error)) {
      g_error("error spawning the test: %s", error->message);
  }
  tlb = g_test_log_buffer_new();
  loop = g_main_loop_new(NULL, FALSE);
  channel = g_io_channel_unix_new(pipes[0]);
  g_io_channel_set_close_on_unref(channel, TRUE);
  g_io_channel_set_encoding(channel, NULL, NULL);
  g_io_channel_set_buffered(channel, FALSE);
  g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
  io_source = g_io_add_watch(channel, G_IO_IN, test_message_cb1, tlb);
  child_source = g_child_watch_add(pid, test_message_cb2, loop);
  g_main_loop_run(loop);
  test_message_cb1(channel, G_IO_IN, tlb);
  g_assert(!g_source_remove(child_source));
  g_assert(g_source_remove(io_source));
  g_io_channel_unref(channel);
  for (msg = g_test_log_buffer_pop(tlb); msg; msg = g_test_log_buffer_pop(tlb)) {
      switch(msg->log_type) {
          case G_TEST_LOG_START_BINARY: case G_TEST_LOG_START_CASE: break;
          case G_TEST_LOG_STOP_CASE: passed++; break;
          case G_TEST_LOG_MESSAGE: {
              gchar const* known_messages[] = {
                  "this is a regular g_test_message() from the test suite",
                  "MESSAGE: this is a regular g_message() from the test suite",
                  "DEBUG: this is a regular g_debug() from the test suite"
              };
              g_assert_cmpint(messages, <, G_N_ELEMENTS (known_messages));
              g_assert_cmpstr(msg->strings[0], ==, known_messages[messages]);
              messages++;
          }
          break;
          case G_TEST_LOG_ERROR: g_assert_not_reached(); break;
          default: g_error("unexpected log message type: %s", g_test_log_type_name(msg->log_type));
      }
  }
  g_assert_cmpint(passed, ==, 3);
  g_assert_cmpint(messages, ==, 3);
}
static void test_error(void) {
  gchar* tests[] = { "/glib/testing/protocol/warning", "/glib/testing/protocol/critical", "/glib/testing/protocol/error" };
  gint i;
  int messages = 0;
  for (i = 0; i < G_N_ELEMENTS(tests); i++) {
      gchar* argv[] = { "./protocol", NULL, "--verbose", "-p", tests[i], NULL };
      GTestLogBuffer* tlb;
      GTestLogMsg* msg;
      GIOChannel* channel;
      GMainLoop* loop;
      GError* error = NULL;
      gulong child_source;
      gulong io_source;
      GPid pid = 0;
      int pipes[2];
      if (0 > pipe(pipes)) g_error("error creating pipe: %s", g_strerror(errno));
      argv[1] = g_strdup_printf("--GTestLogFD=%u", pipes[1]);
      if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_LEAVE_DESCRIPTORS_OPEN | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
                         NULL, NULL, &pid, &error)) {
          g_error("error spawning the test: %s", error->message);
      }
      tlb = g_test_log_buffer_new();
      loop = g_main_loop_new(NULL, FALSE);
      channel = g_io_channel_unix_new(pipes[0]);
      g_io_channel_set_close_on_unref(channel, TRUE);
      g_io_channel_set_encoding(channel, NULL, NULL);
      g_io_channel_set_buffered(channel, FALSE);
      g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);
      io_source = g_io_add_watch(channel, G_IO_IN, test_message_cb1, tlb);
      child_source = g_child_watch_add(pid, test_message_cb2, loop);
      g_main_loop_run(loop);
      test_message_cb1(channel, G_IO_IN, tlb);
      g_assert(!g_source_remove(child_source));
      g_assert(g_source_remove(io_source));
      g_io_channel_unref(channel);
      for (msg = g_test_log_buffer_pop(tlb); msg; msg = g_test_log_buffer_pop(tlb)) {
          switch(msg->log_type) {
              case G_TEST_LOG_START_BINARY: case G_TEST_LOG_START_CASE:  break;
              case G_TEST_LOG_STOP_CASE: case G_TEST_LOG_MESSAGE: g_assert_not_reached(); break;
              case G_TEST_LOG_ERROR: {
                      gchar const* known_messages[] = {
                           "FATAL-WARNING: this is a regular g_warning() from the test suite",
                           "FATAL-CRITICAL: this is a regular g_critical() from the test suite",
                           "FATAL-ERROR: this is a regular g_error() from the test suite"
                      };
                      g_assert_cmpint(messages, <, G_N_ELEMENTS (known_messages));
                      g_assert_cmpstr(msg->strings[0], ==, known_messages[messages]);
                      messages++;
                  }
                  break;
              default: g_error("unexpected log message type: %s", g_test_log_type_name(msg->log_type));
          }
      }
  }
  g_assert_cmpint(messages, ==, 3);
}
int main(int argc, char**argv) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/glib/testing/protocol/debug", debug);
  g_test_add_func("/glib/testing/protocol/info", info);
  g_test_add_func("/glib/testing/protocol/message", message);
  g_test_add_func("/glib/testing/protocol/warning", warning);
  g_test_add_func("/glib/testing/protocol/critical", critical);
  g_test_add_func("/glib/testing/protocol/error", _error);
  g_test_add_func("/glib/testing/protocol/gtest-message", gtest_message);
  g_test_add_func("/glib/testing/protocol/test-message", test_message);
  g_test_add_func("/glib/testing/protocol/test-error", test_error);
  return g_test_run();
}