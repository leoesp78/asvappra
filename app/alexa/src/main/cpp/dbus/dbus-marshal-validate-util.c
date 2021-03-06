#include "config.h"
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdio.h>
#include "dbus-internals.h"
#include "dbus-marshal-validate.h"
#include "dbus-marshal-recursive.h"
#include "dbus-test-tap.h"
#include "dbus-test.h"

typedef struct {
  const char *data;
  DBusValidity expected;
} ValidityTest;
static void run_validity_tests(const ValidityTest *tests, int n_tests, DBusValidity (*func)(const DBusString*,int,int)) {
  int i;
  for (i = 0; i < n_tests; i++) {
      DBusString str;
      DBusValidity v;
      _dbus_string_init_const(&str, tests[i].data);
      v = (*func)(&str, 0, _dbus_string_get_length(&str));
      if (v != tests[i].expected) _dbus_test_fatal("Improper validation result %d for '%s'", v, tests[i].data);
  }
}
static const ValidityTest signature_tests[] = {
  { "", DBUS_VALID },
  { "sss", DBUS_VALID },
  { "i", DBUS_VALID },
  { "b", DBUS_VALID },
  { "ai", DBUS_VALID },
  { "(i)", DBUS_VALID },
  { "w", DBUS_INVALID_UNKNOWN_TYPECODE },
  { "a", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "aaaaaa", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "ii(ii)a", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "ia", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",DBUS_INVALID_EXCEEDED_MAXIMUM_ARRAY_RECURSION },
  { "((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((ii))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))",DBUS_INVALID_EXCEEDED_MAXIMUM_STRUCT_RECURSION },
  { ")", DBUS_INVALID_STRUCT_ENDED_BUT_NOT_STARTED },
  { "i)", DBUS_INVALID_STRUCT_ENDED_BUT_NOT_STARTED },
  { "a)", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "(", DBUS_INVALID_STRUCT_STARTED_BUT_NOT_ENDED },
  { "(i", DBUS_INVALID_STRUCT_STARTED_BUT_NOT_ENDED },
  { "(iiiii", DBUS_INVALID_STRUCT_STARTED_BUT_NOT_ENDED },
  { "(ai", DBUS_INVALID_STRUCT_STARTED_BUT_NOT_ENDED },
  { "()", DBUS_INVALID_STRUCT_HAS_NO_FIELDS },
  { "(())", DBUS_INVALID_STRUCT_HAS_NO_FIELDS },
  { "a()", DBUS_INVALID_STRUCT_HAS_NO_FIELDS },
  { "i()", DBUS_INVALID_STRUCT_HAS_NO_FIELDS },
  { "()i", DBUS_INVALID_STRUCT_HAS_NO_FIELDS },
  { "(a)", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "a{ia}", DBUS_INVALID_MISSING_ARRAY_ELEMENT_TYPE },
  { "a{}", DBUS_INVALID_DICT_ENTRY_HAS_NO_FIELDS },
  { "a{aii}", DBUS_INVALID_DICT_KEY_MUST_BE_BASIC_TYPE },
  { " ", DBUS_INVALID_UNKNOWN_TYPECODE },
  { "not a valid signature", DBUS_INVALID_UNKNOWN_TYPECODE },
  { "123", DBUS_INVALID_UNKNOWN_TYPECODE },
  { ".", DBUS_INVALID_UNKNOWN_TYPECODE },
  { "a{(ii)i}", DBUS_INVALID_DICT_KEY_MUST_BE_BASIC_TYPE },
  { "a{i}", DBUS_INVALID_DICT_ENTRY_HAS_ONLY_ONE_FIELD },
  { "{is}", DBUS_INVALID_DICT_ENTRY_NOT_INSIDE_ARRAY },
  { "a{isi}", DBUS_INVALID_DICT_ENTRY_HAS_TOO_MANY_FIELDS },
};
dbus_bool_t _dbus_marshal_validate_test(void) {
  DBusString str;
  int i;
  const char *valid_paths[] = {
    "/",
    "/foo/bar",
    "/foo",
    "/foo/bar/baz"
  };
  const char *invalid_paths[] = {
    "bar",
    "bar/baz",
    "/foo/bar/",
    "/foo/",
    "foo/",
    "boo//blah",
    "//",
    "///",
    "foo///blah/",
    "Hello World",
    "",
    "   ",
    "foo bar"
  };
  const char *valid_interfaces[] = {
    "org.freedesktop.Foo",
    "Bar.Baz",
    "Blah.Blah.Blah.Blah.Blah",
    "a.b",
    "a.b.c.d.e.f.g",
    "a0.b1.c2.d3.e4.f5.g6",
    "abc123.foo27"
  };
  const char *invalid_interfaces[] = {
    ".",
    "",
    "..",
    ".Foo.Bar",
    "..Foo.Bar",
    "Foo.Bar.",
    "Foo.Bar..",
    "Foo",
    "9foo.bar.baz",
    "foo.bar..baz",
    "foo.bar...baz",
    "foo.bar.b..blah",
    ":",
    ":0-1",
    "10",
    ":11.34324",
    "0.0.0",
    "0..0",
    "foo.Bar.%",
    "foo.Bar!!",
    "!Foo.bar.bz",
    "foo.$.blah",
    "",
    "   ",
    "foo bar"
  };
  const char *valid_unique_names[] = {
    ":0",
    ":a",
    ":",
    ":.a",
    ":.1",
    ":0.1",
    ":000.2222",
    ":.blah",
    ":abce.freedesktop.blah"
  };
  const char *invalid_unique_names[] = {
    //":-",
    ":!",
    //":0-10",
    ":blah.",
    ":blah.",
    ":blah..org",
    ":blah.org..",
    ":..blah.org",
    "",
    "   ",
    "foo bar"
  };
  const char *valid_members[] = {
    "Hello",
    "Bar",
    "foobar",
    "_foobar",
    "foo89"
  };
  const char *invalid_members[] = {
    "9Hello",
    "10",
    "1",
    "foo-bar",
    "blah.org",
    ".blah",
    "blah.",
    "Hello.",
    "!foo",
    "",
    "   ",
    "foo bar"
  };
  run_validity_tests(signature_tests, _DBUS_N_ELEMENTS(signature_tests), _dbus_validate_signature_with_reason);
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_paths)) {
      _dbus_string_init_const(&str, valid_paths[i]);
      if (!_dbus_validate_path(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Path \"%s\" should have been valid", valid_paths[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(invalid_paths)) {
      _dbus_string_init_const(&str, invalid_paths[i]);
      if (_dbus_validate_path(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Path \"%s\" should have been invalid", invalid_paths[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_interfaces)) {
      _dbus_string_init_const(&str, valid_interfaces[i]);
      if (!_dbus_validate_interface(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Interface \"%s\" should have been valid", valid_interfaces[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(invalid_interfaces)) {
      _dbus_string_init_const(&str, invalid_interfaces[i]);
      if (_dbus_validate_interface(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Interface \"%s\" should have been invalid", invalid_interfaces[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_interfaces)) {
      _dbus_string_init_const(&str, valid_interfaces[i]);
      if (!_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Bus name \"%s\" should have been valid", valid_interfaces[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(invalid_interfaces)) {
      if (invalid_interfaces[i][0] != ':') {
          _dbus_string_init_const(&str, invalid_interfaces[i]);
          if (_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str)))
              _dbus_test_fatal("Bus name \"%s\" should have been invalid", invalid_interfaces[i]);
      }
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_unique_names)) {
      _dbus_string_init_const(&str, valid_unique_names[i]);
      if (!_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Bus name \"%s\" should have been valid", valid_unique_names[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(invalid_unique_names)) {
      _dbus_string_init_const(&str, invalid_unique_names[i]);
      if (_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Bus name \"%s\" should have been invalid", invalid_unique_names[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_interfaces)) {
      _dbus_string_init_const(&str, valid_interfaces[i]);
      if (!_dbus_validate_error_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Error name \"%s\" should have been valid", valid_interfaces[i]);
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(invalid_interfaces)) {
      if (invalid_interfaces[i][0] != ':') {
          _dbus_string_init_const(&str, invalid_interfaces[i]);
          if (_dbus_validate_error_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Error name \"%s\" should have been invalid", invalid_interfaces[i]);
      }
      ++i;
  }
  i = 0;
  while(i < (int)_DBUS_N_ELEMENTS(valid_members)) {
      _dbus_string_init_const(&str, valid_members[i]);
      if (!_dbus_validate_member(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Member \"%s\" should have been valid", valid_members[i]);
      ++i;
  }
  i = 0;
  while (i < (int)_DBUS_N_ELEMENTS(invalid_members)) {
      _dbus_string_init_const(&str, invalid_members[i]);
      if (_dbus_validate_member(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("Member \"%s\" should have been invalid", invalid_members[i]);
      ++i;
  }
  _dbus_string_init_const(&str, "abc.efg");
  if (_dbus_validate_bus_name(&str, 0, 8)) _dbus_test_fatal("validated too-long string");
  if (_dbus_validate_interface(&str, 0, 8)) _dbus_test_fatal("validated too-long string");
  if (_dbus_validate_error_name(&str, 0, 8)) _dbus_test_fatal("validated too-long string");
  _dbus_string_init_const(&str, "abc");
  if (_dbus_validate_member(&str, 0, 4)) _dbus_test_fatal("validated too-long string");
  if (!_dbus_string_init(&str)) _dbus_test_fatal("no memory");
  while(_dbus_string_get_length(&str) <= DBUS_MAXIMUM_NAME_LENGTH)
      if (!_dbus_string_append(&str, "abc.def")) _dbus_test_fatal("no memory");
  if (_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("validated overmax string");
  if (_dbus_validate_interface(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("validated overmax string");
  if (_dbus_validate_error_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("validated overmax string");
  _dbus_string_set_length(&str, 0);
  while(_dbus_string_get_length(&str) <= DBUS_MAXIMUM_NAME_LENGTH)
      if (!_dbus_string_append(&str, "abc")) _dbus_test_fatal("no memory");
  if (_dbus_validate_member(&str, 0, _dbus_string_get_length(&str)))_dbus_test_fatal("validated overmax string");
  _dbus_string_set_length(&str, 0);
  _dbus_string_append(&str, ":");
  while(_dbus_string_get_length(&str) <= DBUS_MAXIMUM_NAME_LENGTH)
      if (!_dbus_string_append(&str, "abc")) _dbus_test_fatal("no memory");
  if (_dbus_validate_bus_name(&str, 0, _dbus_string_get_length(&str))) _dbus_test_fatal("validated overmax string");
  _dbus_string_free(&str);
  {
      int sequence;
      DBusString signature;
      DBusString body;
      if (!_dbus_string_init(&signature) || !_dbus_string_init(&body)) _dbus_test_fatal("oom");
      sequence = 0;
      while(dbus_internal_do_not_use_generate_bodies(sequence, DBUS_LITTLE_ENDIAN, &signature, &body)) {
          DBusValidity validity;
          validity = _dbus_validate_body_with_reason(&signature, 0, DBUS_LITTLE_ENDIAN,NULL, &body, 0,
                                                     _dbus_string_get_length(&body));
          if (validity != DBUS_VALID) {
              _dbus_warn("invalid code %d expected valid on sequence %d little endian", validity, sequence);
              _dbus_verbose_bytes_of_string(&signature, 0, _dbus_string_get_length(&signature));
              _dbus_verbose_bytes_of_string(&body, 0, _dbus_string_get_length(&body));
              _dbus_test_fatal("test failed");
          }
          _dbus_string_set_length(&signature, 0);
          _dbus_string_set_length(&body, 0);
          ++sequence;
      }
      sequence = 0;
      while(dbus_internal_do_not_use_generate_bodies(sequence, DBUS_BIG_ENDIAN, &signature, &body)) {
          DBusValidity validity;
          validity = _dbus_validate_body_with_reason(&signature, 0, DBUS_BIG_ENDIAN,NULL, &body, 0,
                                                     _dbus_string_get_length(&body));
          if (validity != DBUS_VALID) {
              _dbus_warn("invalid code %d expected valid on sequence %d big endian", validity, sequence);
              _dbus_verbose_bytes_of_string(&signature, 0, _dbus_string_get_length(&signature));
              _dbus_verbose_bytes_of_string(&body, 0, _dbus_string_get_length(&body));
              _dbus_test_fatal("test failed");
          }
          _dbus_string_set_length(&signature, 0);
          _dbus_string_set_length(&body, 0);
          ++sequence;
      }
      _dbus_string_free(&signature);
      _dbus_string_free(&body);
  }
  return TRUE;
}
#endif
#endif