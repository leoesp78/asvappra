#ifndef CHECK_CHECK_H
#define CHECK_CHECK_H

#include "../check.h"

#ifndef TIMEOUT_TESTS_ENABLED
#define TIMEOUT_TESTS_ENABLED 1
#endif
#ifndef MEMORY_LEAKING_TESTS_ENABLED
#define MEMORY_LEAKING_TESTS_ENABLED 1
#endif
extern int sub_ntests;
void fork_setup (void);
void fork_teardown (void);
void setup_fixture(void);
void teardown_fixture (void);
void setup (void);
void cleanup (void);
Suite *make_sub_suite(void);
Suite *make_sub2_suite(void);
Suite *make_master_suite(void);
Suite *make_list_suite(void);
Suite *make_msg_suite(void);
Suite *make_log_suite(void);
Suite *make_log_internal_suite(void);
Suite *make_limit_suite(void);
Suite *make_fork_suite(void);
Suite *make_fixture_suite(void);
Suite *make_pack_suite(void);
Suite *make_exit_suite(void);
Suite *make_selective_suite(void);
Suite *make_tag_suite(void);
extern int master_tests_lineno[];
void init_master_tests_lineno(int num_master_tests);
void record_test_name(const char* test_name);
char* get_next_test_name(FILE *file);
void record_failure_line_num(const int line);
long get_next_failure_line_num(FILE *file);

#endif