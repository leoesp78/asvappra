#include "config.h"
#include "dbus-spawn.h"
#include "dbus-sysdeps.h"
#include "dbus-test.h"

static char *get_test_exec(const char *exe, DBusString *scratch_space) {
  const char *dbus_test_exec;
  dbus_test_exec = _dbus_getenv("DBUS_TEST_EXEC");
  if (dbus_test_exec == NULL) return NULL;
  if (!_dbus_string_init(scratch_space)) return NULL;
  if (!_dbus_string_append_printf(scratch_space, "%s/%s%s", dbus_test_exec, exe, DBUS_EXEEXT)) {
      _dbus_string_free(scratch_space);
      return NULL;
  }
  return _dbus_string_get_data(scratch_space);
}
static dbus_bool_t check_spawn_nonexistent(void *data, dbus_bool_t have_memory) {
  static const char arg_does_not_exist[] = "/this/does/not/exist/32542sdgafgafdg";
  const char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter = NULL;
  DBusError error = DBUS_ERROR_INIT;
  argv[0] = arg_does_not_exist;
  if (_dbus_spawn_async_with_babysitter(&sitter,"spawn_nonexistent", (char * const*)argv,NULL,DBUS_SPAWN_NONE,NULL,NULL,
      &error)) {
      _dbus_babysitter_block_for_child_exit(sitter);
      _dbus_babysitter_set_child_exit_error(sitter, &error);
  }
  if (sitter) _dbus_babysitter_unref(sitter);
  if (!dbus_error_is_set(&error)) {
      _dbus_warn("Did not get an error launching nonexistent executable");
      return FALSE;
  }
  if (!(dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY) || dbus_error_has_name(&error, DBUS_ERROR_SPAWN_EXEC_FAILED))) {
      _dbus_warn("Not expecting error when launching nonexistent executable: %s: %s", error.name, error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  dbus_error_free(&error);
  return TRUE;
}
static dbus_bool_t check_spawn_segfault(void *data, dbus_bool_t have_memory) {
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusString argv0;
  argv[0] = get_test_exec("test-segfault", &argv0);
  if (argv[0] == NULL) return TRUE;
  if (_dbus_spawn_async_with_babysitter(&sitter, "spawn_segfault", argv,NULL, DBUS_SPAWN_NONE, NULL, NULL, &error)) {
      _dbus_babysitter_block_for_child_exit(sitter);
      _dbus_babysitter_set_child_exit_error(sitter, &error);
  }
  _dbus_string_free(&argv0);
  if (sitter) _dbus_babysitter_unref(sitter);
  if (!dbus_error_is_set(&error)) {
      _dbus_warn("Did not get an error launching segfaulting binary");
      return FALSE;
  }
  if (!(dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY) ||
  #ifdef DBUS_WIN
      dbus_error_has_name(&error, DBUS_ERROR_SPAWN_CHILD_EXITED)))
  #else
      dbus_error_has_name(&error, DBUS_ERROR_SPAWN_CHILD_SIGNALED)))
  #endif
  {
      _dbus_warn("Not expecting error when launching segfaulting executable: %s: %s", error.name, error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  dbus_error_free(&error);
  return TRUE;
}
static dbus_bool_t check_spawn_exit(void *data, dbus_bool_t have_memory) {
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusString argv0;
  argv[0] = get_test_exec("test-exit", &argv0);
  if (argv[0] == NULL) return TRUE;
  if (_dbus_spawn_async_with_babysitter(&sitter, "spawn_exit", argv,NULL, DBUS_SPAWN_NONE, NULL, NULL, &error)) {
      _dbus_babysitter_block_for_child_exit(sitter);
      _dbus_babysitter_set_child_exit_error(sitter, &error);
  }
  _dbus_string_free(&argv0);
  if (sitter) _dbus_babysitter_unref(sitter);
  if (!dbus_error_is_set(&error)) {
      _dbus_warn("Did not get an error launching binary that exited with failure code");
      return FALSE;
  }
  if (!(dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY) || dbus_error_has_name(&error, DBUS_ERROR_SPAWN_CHILD_EXITED))) {
      _dbus_warn("Not expecting error when launching exiting executable: %s: %s", error.name, error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  dbus_error_free(&error);
  return TRUE;
}
static dbus_bool_t check_spawn_and_kill(void *data, dbus_bool_t have_memory) {
  char *argv[4] = { NULL, NULL, NULL, NULL };
  DBusBabysitter *sitter = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusString argv0;
  argv[0] = get_test_exec("test-sleep-forever", &argv0);
  if (argv[0] == NULL) return TRUE;
  if (_dbus_spawn_async_with_babysitter(&sitter, "spawn_and_kill", argv,NULL, DBUS_SPAWN_NONE, NULL, NULL, &error)) {
      _dbus_babysitter_kill_child(sitter);
      _dbus_babysitter_block_for_child_exit(sitter);
      _dbus_babysitter_set_child_exit_error(sitter, &error);
  }
  _dbus_string_free(&argv0);
  if (sitter) _dbus_babysitter_unref(sitter);
  if (!dbus_error_is_set(&error)) {
      _dbus_warn("Did not get an error after killing spawned binary");
      return FALSE;
  }
  if (!(dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY) ||
  #ifdef DBUS_WIN
      dbus_error_has_name(&error, DBUS_ERROR_SPAWN_CHILD_EXITED)))
  #else
      dbus_error_has_name(&error, DBUS_ERROR_SPAWN_CHILD_SIGNALED)))
  #endif
  {
      _dbus_warn("Not expecting error when killing executable: %s: %s", error.name, error.message);
      dbus_error_free(&error);
      return FALSE;
  }
  dbus_error_free(&error);
  return TRUE;
}
dbus_bool_t _dbus_spawn_test(const char *test_data_dir) {
  if (!_dbus_test_oom_handling("spawn_nonexistent", check_spawn_nonexistent, NULL)) return FALSE;
  if (!_dbus_test_oom_handling("spawn_segfault", check_spawn_segfault, NULL)) return FALSE;
  if (!_dbus_test_oom_handling("spawn_exit", check_spawn_exit, NULL)) return FALSE;
  if (!_dbus_test_oom_handling("spawn_and_kill", check_spawn_and_kill,NULL)) return FALSE;
  return TRUE;
}