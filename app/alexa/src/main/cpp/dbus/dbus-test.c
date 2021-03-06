#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "dbus-test.h"
#include "dbus-sysdeps.h"
#include "dbus-internals.h"
#include "dbus-test-tap.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
typedef dbus_bool_t (*TestFunc)(void);
typedef dbus_bool_t (*TestDataFunc)(const char *data);
static void run_test(const char *test_name, const char *specific_test, TestFunc test) {
  if (specific_test != NULL && strcmp(specific_test, test_name) != 0) {
      _dbus_test_skip("%s - Only intending to run %s", test_name, specific_test);
      return;
  }
  _dbus_test_diag("%s: running %s tests", "test-dbus", test_name);
  if (test()) _dbus_test_ok("%s", test_name);
  else _dbus_test_not_ok("%s", test_name);
  _dbus_test_check_memleaks(test_name);
}
static void run_data_test(const char *test_name, const char *specific_test, TestDataFunc test, const char *test_data_dir) {
  if (specific_test != NULL && strcmp(specific_test, test_name) != 0) {
      _dbus_test_skip("%s - Only intending to run %s", test_name, specific_test);
      return;
  }
  _dbus_test_diag("%s: running %s tests", "test-dbus", test_name);
  if (test(test_data_dir)) _dbus_test_ok("%s", test_name);
  else _dbus_test_not_ok("%s", test_name);
  _dbus_test_check_memleaks(test_name);
}
void _dbus_run_tests(const char *test_data_dir, const char *specific_test) {
  if (!_dbus_threads_init_debug()) _dbus_test_fatal("debug threads init failed");
  if (test_data_dir == NULL) test_data_dir = _dbus_getenv("DBUS_TEST_DATA");
  if (test_data_dir != NULL) _dbus_test_diag("Test data in %s", test_data_dir);
  else _dbus_test_diag("No test data!");
  run_test("string", specific_test, _dbus_string_test);
  run_test("sysdeps", specific_test, _dbus_sysdeps_test);
  run_test("data-slot", specific_test, _dbus_data_slot_test);
  run_test("misc", specific_test, _dbus_misc_test);
  run_test("address", specific_test, _dbus_address_test);
  run_test("server", specific_test, _dbus_server_test);
  run_test("object-tree", specific_test, _dbus_object_tree_test);
  run_test("signature", specific_test, _dbus_signature_test);
  run_test("marshalling", specific_test, _dbus_marshal_test);
  run_test("marshal-recursive", specific_test, _dbus_marshal_recursive_test);
  run_test("byteswap", specific_test, _dbus_marshal_byteswap_test);
  run_test("memory", specific_test, _dbus_memory_test);
#if 1
  run_test("mem-pool", specific_test, _dbus_mem_pool_test);
#endif
  run_test("list", specific_test, _dbus_list_test);
  run_test("marshal-validate", specific_test, _dbus_marshal_validate_test);
  run_data_test("message", specific_test, _dbus_message_test, test_data_dir);
  run_test("hash", specific_test, _dbus_hash_test);
#if !defined(DBUS_WINCE)
  run_data_test("spawn", specific_test, _dbus_spawn_test, test_data_dir);
#endif
  run_data_test("credentials", specific_test, _dbus_credentials_test, test_data_dir);
#ifdef DBUS_UNIX
  run_data_test("userdb", specific_test, _dbus_userdb_test, test_data_dir);
  run_test("transport-unix", specific_test, _dbus_transport_unix_test);
#endif
  run_test("keyring", specific_test, _dbus_keyring_test);
  run_data_test("sha", specific_test, _dbus_sha_test, test_data_dir);
  run_data_test("auth", specific_test, _dbus_auth_test, test_data_dir);
}
#endif