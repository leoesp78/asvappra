#include <stdlib.h>
#include <stdio.h>
#include "../libcompat.h"
#include "../check.h"
#include "check_check.h"

int main (void) {
  int number_failed;
  SRunner *sr;
  fork_setup();
  setup_fixture();
  setup();
  sr = srunner_create(make_master_suite());
  srunner_add_suite(sr, make_list_suite());
  srunner_add_suite(sr, make_msg_suite());
  srunner_add_suite(sr, make_log_suite());
  srunner_add_suite(sr, make_log_internal_suite());
  srunner_add_suite(sr, make_limit_suite());
  srunner_add_suite(sr, make_fork_suite());
  srunner_add_suite(sr, make_fixture_suite());
  srunner_add_suite(sr, make_pack_suite());
  srunner_add_suite(sr, make_tag_suite());
#if defined(HAVE_FORK) && HAVE_FORK==1
  srunner_add_suite(sr, make_exit_suite());
#endif
  srunner_add_suite(sr, make_selective_suite());
  printf ("Ran %d tests in subordinate suite\n", sub_ntests);
  srunner_run_all (sr, CK_VERBOSE);
  cleanup();
  fork_teardown();
  teardown_fixture();
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}