#include "config.h"
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-auth.h"
#include "dbus-test-tap.h"
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include "dbus-auth-script.h"

static dbus_bool_t process_test_subdir(const DBusString *test_base_dir, const char *subdir) {
  DBusString test_directory;
  DBusString filename;
  DBusDirIter *dir;
  dbus_bool_t retval;
  DBusError error = DBUS_ERROR_INIT;
  retval = FALSE;
  dir = NULL;
  if (!_dbus_string_init(&test_directory)) _dbus_test_fatal("didn't allocate test_directory");
  _dbus_string_init_const(&filename, subdir);
  if (!_dbus_string_copy(test_base_dir, 0, &test_directory, 0)) _dbus_test_fatal("couldn't copy test_base_dir to test_directory");
  if (!_dbus_concat_dir_and_file(&test_directory, &filename)) _dbus_test_fatal("couldn't allocate full path");
  _dbus_string_free(&filename);
  if (!_dbus_string_init(&filename)) _dbus_test_fatal("didn't allocate filename string");
  dir = _dbus_directory_open(&test_directory, &error);
  if (dir == NULL) {
      _dbus_warn("Could not open %s: %s", _dbus_string_get_const_data(&test_directory), error.message);
      dbus_error_free(&error);
      goto failed;
  }
  _dbus_test_diag("Testing %s:", subdir);
next:
  while(_dbus_directory_get_next_file(dir, &filename, &error)) {
      DBusString full_path;
      if (!_dbus_string_init(&full_path)) _dbus_test_fatal("couldn't init string");
      if (!_dbus_string_copy(&test_directory, 0, &full_path, 0)) _dbus_test_fatal("couldn't copy dir to full_path");
      if (!_dbus_concat_dir_and_file(&full_path, &filename)) _dbus_test_fatal("couldn't concat file to dir");
      if (!_dbus_string_ends_with_c_str(&filename, ".auth-script")) {
          _dbus_verbose("Skipping non-.auth-script file %s\n", _dbus_string_get_const_data(&filename));
	      _dbus_string_free(&full_path);
          goto next;
      }
      _dbus_test_diag("    %s", _dbus_string_get_const_data(&filename));
      if (!_dbus_auth_script_run(&full_path)) {
          _dbus_string_free(&full_path);
          goto failed;
      } else _dbus_string_free(&full_path);
  }
  if (dbus_error_is_set(&error)) {
      _dbus_warn("Could not get next file in %s: %s", _dbus_string_get_const_data(&test_directory), error.message);
      dbus_error_free(&error);
      goto failed;
  }
  retval = TRUE;
failed:
  if (dir) _dbus_directory_close(dir);
  _dbus_string_free(&test_directory);
  _dbus_string_free(&filename);
  return retval;
}
static dbus_bool_t process_test_dirs(const char *test_data_dir) {
  DBusString test_directory;
  dbus_bool_t retval;
  retval = FALSE;
  _dbus_string_init_const(&test_directory, test_data_dir);
  if (!process_test_subdir(&test_directory, "auth")) goto failed;
  retval = TRUE;
failed:
  _dbus_string_free(&test_directory);
  return retval;
}
dbus_bool_t _dbus_auth_test(const char *test_data_dir) {
  if (test_data_dir == NULL) return TRUE;
  if (!process_test_dirs(test_data_dir)) return FALSE;
  return TRUE;
}
#endif