#include "config.h"
#include "dbus-address.h"
#include "dbus-internals.h"
#include "dbus-list.h"
#include "dbus-string.h"
#include "dbus-protocol.h"
#include "dbus-test-tap.h"

struct DBusAddressEntry {
  DBusString method;
  DBusList *keys;
  DBusList *values;
};
void _dbus_set_bad_address(DBusError  *error, const char *address_problem_type, const char *address_problem_field, const char *address_problem_other) {
  if (address_problem_type != NULL) {
      dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Server address of type %s was missing argument %s", address_problem_type, address_problem_field);
  } else dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"Could not parse server address: %s", address_problem_other);
}
#define _DBUS_ADDRESS_OPTIONALLY_ESCAPED_BYTE(b)   (((b) >= 'a' && (b) <= 'z') || ((b) >= 'A' && (b) <= 'Z') || ((b) >= '0' && (b) <= '9') || (b) == '-' ||                                 \
                                                    (b) == '_' || (b) == '/' || (b) == '\\' || (b) == '*' || (b) == '.')
dbus_bool_t _dbus_address_append_escaped(DBusString *escaped, const DBusString *unescaped) {
  const unsigned char *p;
  const unsigned char *end;
  dbus_bool_t ret;
  int orig_len;
  ret = FALSE;
  orig_len = _dbus_string_get_length(escaped);
  p = _dbus_string_get_const_udata(unescaped);
  end = p + _dbus_string_get_length(unescaped);
  while(p != end) {
      if (_DBUS_ADDRESS_OPTIONALLY_ESCAPED_BYTE(*p)) {
          if (!_dbus_string_append_byte(escaped, *p)) goto out;
      } else {
          if (!_dbus_string_append_byte(escaped, '%')) goto out;
          if (!_dbus_string_append_byte_as_hex(escaped, *p)) goto out;
      }
      ++p;
  }
  ret = TRUE;
out:
  if (!ret) _dbus_string_set_length(escaped, orig_len);
  return ret;
}
static void dbus_address_entry_free(DBusAddressEntry *entry) {
  DBusList *link;
  _dbus_string_free(&entry->method);
  link = _dbus_list_get_first_link(&entry->keys);
  while(link != NULL) {
      _dbus_string_free(link->data);
      dbus_free(link->data);
      link = _dbus_list_get_next_link(&entry->keys, link);
  }
  _dbus_list_clear(&entry->keys);
  link = _dbus_list_get_first_link(&entry->values);
  while(link != NULL) {
      _dbus_string_free(link->data);
      dbus_free(link->data);
      link = _dbus_list_get_next_link(&entry->values, link);
  }
  _dbus_list_clear(&entry->values);
  dbus_free(entry);
}
void dbus_address_entries_free(DBusAddressEntry **entries) {
  int i;
  for (i = 0; entries[i] != NULL; i++) dbus_address_entry_free(entries[i]);
  dbus_free(entries);
}
static DBusAddressEntry *create_entry(void) {
  DBusAddressEntry *entry;
  entry = dbus_new0(DBusAddressEntry, 1);
  if (entry == NULL) return NULL;
  if (!_dbus_string_init(&entry->method)) {
      dbus_free(entry);
      return NULL;
  }
  return entry;
}
const char *dbus_address_entry_get_method(DBusAddressEntry *entry) {
  return _dbus_string_get_const_data(&entry->method);
}
const char *dbus_address_entry_get_value(DBusAddressEntry *entry, const char *key) {
  DBusList *values, *keys;
  keys = _dbus_list_get_first_link(&entry->keys);
  values = _dbus_list_get_first_link(&entry->values);
  while(keys != NULL) {
      _dbus_assert(values != NULL);
      if (_dbus_string_equal_c_str(keys->data, key)) return _dbus_string_get_const_data(values->data);
      keys = _dbus_list_get_next_link(&entry->keys, keys);
      values = _dbus_list_get_next_link(&entry->values, values);
  }
  return NULL;
}
static dbus_bool_t append_unescaped_value(DBusString *unescaped, const DBusString *escaped, int escaped_start, int escaped_len, DBusError *error) {
  const char *p;
  const char *end;
  dbus_bool_t ret;
  ret = FALSE;
  p = _dbus_string_get_const_data(escaped) + escaped_start;
  end = p + escaped_len;
  while(p != end) {
      if (_DBUS_ADDRESS_OPTIONALLY_ESCAPED_BYTE(*p)) {
          if (!_dbus_string_append_byte(unescaped, *p)) goto out;
      } else if (*p == '%') {
          char buf[3];
          DBusString hex;
          int hex_end;
          ++p;
          if ((p + 2) > end) {
              dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"In D-Bus address, percent character was not followed by two hex digits");
              goto out;
          }
          buf[0] = *p;
          ++p;
          buf[1] = *p;
          buf[2] = '\0';
          _dbus_string_init_const(&hex, buf);
          if (!_dbus_string_hex_decode(&hex, 0, &hex_end, unescaped, _dbus_string_get_length(unescaped))) goto out;
          if (hex_end != 2) {
              dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"In D-Bus address, percent character was followed by characters other than hex digits");
              goto out;
          }
      } else {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"In D-Bus address, character '%c' should have been escaped\n", *p);
          goto out;
      }
      ++p;
  }
  ret = TRUE;
out:
  if (!ret && error && !dbus_error_is_set(error)) _DBUS_SET_OOM(error);
  _dbus_assert(ret || error == NULL || dbus_error_is_set(error));
  return ret;
}
dbus_bool_t dbus_parse_address(const char *address, DBusAddressEntry ***entry_result, int *array_len, DBusError *error) {
  DBusString str;
  int pos, end_pos, len, i;
  DBusList *entries, *link;
  DBusAddressEntry **entry_array;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  _dbus_string_init_const(&str, address);
  entries = NULL;
  pos = 0;
  len = _dbus_string_get_length(&str);
  if (len == 0) {
      dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS, "Empty address '%s'", address);
      goto error;
  }
  while(pos < len) {
      DBusAddressEntry *entry;
      int found_pos;
      entry = create_entry();
      if (!entry) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          goto error;
	  }
      if (!_dbus_list_append(&entries, entry)) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          dbus_address_entry_free(entry);
          goto error;
	  }
      if (!_dbus_string_find(&str, pos, ";", &end_pos)) end_pos = len;
      if (!_dbus_string_find_to(&str, pos, end_pos, ":", &found_pos)) {
          dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS, "Address does not contain a colon");
          goto error;
	  }
      if (!_dbus_string_copy_len(&str, pos, found_pos - pos, &entry->method, 0)) {
          dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
          goto error;
	  }
      pos = found_pos + 1;
      while(pos < end_pos) {
          int comma_pos, equals_pos;
          if (!_dbus_string_find_to(&str, pos, end_pos, ",", &comma_pos)) comma_pos = end_pos;
          if (!_dbus_string_find_to(&str, pos, comma_pos, "=", &equals_pos) || equals_pos == pos || equals_pos + 1 == comma_pos) {
              dbus_set_error(error, DBUS_ERROR_BAD_ADDRESS,"'=' character not found or has no value following it");
              goto error;
          } else {
              DBusString *key;
              DBusString *value;
              key = dbus_new0(DBusString, 1);
              if (!key) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  goto error;
              }
              value = dbus_new0(DBusString, 1);
              if (!value) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  dbus_free(key);
                  goto error;
              }
              if (!_dbus_string_init(key)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  dbus_free(key);
                  dbus_free(value);
                  goto error;
              }
              if (!_dbus_string_init(value)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  _dbus_string_free(key);
                  dbus_free(key);
                  dbus_free(value);
                  goto error;
		      }
              if (!_dbus_string_copy_len(&str, pos, equals_pos - pos, key, 0)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  _dbus_string_free(key);
                  _dbus_string_free(value);
                  dbus_free(key);
                  dbus_free(value);
                  goto error;
              }
              if (!append_unescaped_value(value, &str, equals_pos + 1,comma_pos - equals_pos - 1, error)) {
                  _dbus_assert(error == NULL || dbus_error_is_set(error));
                  _dbus_string_free(key);
                  _dbus_string_free(value);
                  dbus_free(key);
                  dbus_free(value);
                  goto error;
              }
              if (!_dbus_list_append(&entry->keys, key)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  _dbus_string_free(key);
                  _dbus_string_free(value);
                  dbus_free(key);
                  dbus_free(value);
                  goto error;
              }
              if (!_dbus_list_append(&entry->values, value)) {
                  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
                  _dbus_string_free(value);
                  dbus_free(value);
                  goto error;
              }
          }
	      pos = comma_pos + 1;
      }
      pos = end_pos + 1;
  }
  *array_len = _dbus_list_get_length(&entries);
  entry_array = dbus_new(DBusAddressEntry *, *array_len + 1);
  if (!entry_array) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      goto error;
  }
  entry_array[*array_len] = NULL;
  link = _dbus_list_get_first_link(&entries);
  i = 0;
  while(link != NULL) {
      entry_array[i] = link->data;
      i++;
      link = _dbus_list_get_next_link(&entries, link);
  }
  _dbus_list_clear(&entries);
  *entry_result = entry_array;
  return TRUE;
error:
  link = _dbus_list_get_first_link(&entries);
  while(link != NULL) {
      dbus_address_entry_free(link->data);
      link = _dbus_list_get_next_link(&entries, link);
  }
  _dbus_list_clear(&entries);
  return FALSE;
}
char* dbus_address_escape_value(const char *value) {
  DBusString escaped;
  DBusString unescaped;
  char *ret;
  ret = NULL;
  _dbus_string_init_const(&unescaped, value);
  if (!_dbus_string_init(&escaped)) return NULL;
  if (!_dbus_address_append_escaped(&escaped, &unescaped)) goto out;
  if (!_dbus_string_steal_data(&escaped, &ret)) goto out;
out:
  _dbus_string_free(&escaped);
  return ret;
}
char* dbus_address_unescape_value(const char *value, DBusError *error) {
  DBusString unescaped;
  DBusString escaped;
  char *ret;
  ret = NULL;
  _dbus_string_init_const(&escaped, value);
  if (!_dbus_string_init(&unescaped)) return NULL;
  if (!append_unescaped_value(&unescaped, &escaped,0, _dbus_string_get_length(&escaped), error)) goto out;
  if (!_dbus_string_steal_data(&unescaped, &ret)) goto out;
out:
  if (ret == NULL && error && !dbus_error_is_set(error)) _DBUS_SET_OOM(error);
  _dbus_assert(ret != NULL || error == NULL || dbus_error_is_set(error));
  _dbus_string_free(&unescaped);
  return ret;
}
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "dbus-test.h"
#include <stdlib.h>

typedef struct {
  const char *escaped;
  const char *unescaped;
} EscapeTest;
static const EscapeTest escape_tests[] = {
  { "abcde", "abcde" },
  { "", "" },
  { "%20%20", "  " },
  { "%24", "$" },
  { "%25", "%" },
  { "abc%24", "abc$" },
  { "%24abc", "$abc" },
  { "abc%24abc", "abc$abc" },
  { "/", "/" },
  { "-", "-" },
  { "_", "_" },
  { "A", "A" },
  { "I", "I" },
  { "Z", "Z" },
  { "a", "a" },
  { "i", "i" },
  { "z", "z" },
  { "%c3%b6", "\xc3\xb6" }
};
static const char* invalid_escaped_values[] = {
  "%a",
  "%q",
  "%az",
  "%%",
  "%$$",
  "abc%a",
  "%axyz",
  "%",
  "$",
  " ",
};
dbus_bool_t _dbus_address_test(void) {
  DBusAddressEntry **entries;
  int len;
  DBusError error = DBUS_ERROR_INIT;
  int i;
  i = 0;
  while(i < _DBUS_N_ELEMENTS(escape_tests)) {
      const EscapeTest *test = &escape_tests[i];
      char *escaped;
      char *unescaped;
      escaped = dbus_address_escape_value(test->unescaped);
      if (escaped == NULL) _dbus_test_fatal("oom");
      if (strcmp(escaped, test->escaped) != 0) {
          _dbus_warn("Escaped '%s' as '%s' should have been '%s'", test->unescaped, escaped, test->escaped);
          exit(1);
      }
      dbus_free(escaped);
      unescaped = dbus_address_unescape_value(test->escaped, &error);
      if (unescaped == NULL) {
          _dbus_warn("Failed to unescape '%s': %s", test->escaped, error.message);
          dbus_error_free(&error);
          exit(1);
      }
      if (strcmp(unescaped, test->unescaped) != 0) {
          _dbus_warn("Unescaped '%s' as '%s' should have been '%s'", test->escaped, unescaped, test->unescaped);
          exit(1);
      }
      dbus_free(unescaped);
      ++i;
  }
  i = 0;
  while(i < _DBUS_N_ELEMENTS(invalid_escaped_values)) {
      char *unescaped;
      unescaped = dbus_address_unescape_value(invalid_escaped_values[i], &error);
      if (unescaped != NULL) {
          _dbus_warn("Should not have successfully unescaped '%s' to '%s'", invalid_escaped_values[i], unescaped);
          dbus_free(unescaped);
          exit(1);
      }
      _dbus_assert(dbus_error_is_set(&error));
      dbus_error_free(&error);
      ++i;
  }
  if (!dbus_parse_address("unix:path=/tmp/foo;debug:name=test,sliff=sloff;", &entries, &len, &error)) _dbus_test_fatal("could not parse address");
  _dbus_assert(len == 2);
  _dbus_assert(strcmp(dbus_address_entry_get_value(entries[0], "path"), "/tmp/foo") == 0);
  _dbus_assert(strcmp(dbus_address_entry_get_value(entries[1], "name"), "test") == 0);
  _dbus_assert(strcmp(dbus_address_entry_get_value(entries[1], "sliff"), "sloff") == 0);
  dbus_address_entries_free(entries);
  if (dbus_parse_address("", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:bar", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:bar,baz", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:bar=foo,baz", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:bar=foo;baz", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:=foo", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:foo=", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  if (dbus_parse_address("foo:foo,bar=baz", &entries, &len, &error)) _dbus_test_fatal("Parsed incorrect address.");
  else dbus_error_free(&error);
  return TRUE;
}
#endif /* !DOXYGEN_SHOULD_SKIP_THIS */
#endif