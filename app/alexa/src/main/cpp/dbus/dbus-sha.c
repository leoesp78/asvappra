#include <string.h>
#include "config.h"
#include "dbus-internals.h"
#include "dbus-sha.h"
#include "dbus-marshal-basic.h"
#include "dbus-test-tap.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define SHA_DATASIZE    64
#define SHA_DIGESTSIZE  20
//#define f1(x, y, z) ((x & y) | (~x & z))
#define f1(x, y, z)  (z ^ (x & (y ^ z )))
#define f2(x, y, z)  (x ^ y ^ z)
//#define f3(x, y, z) (( x & y) | (x & z) | (y & z))
#define f3(x, y, z)  (( x & y) | (z & (x | y)))
#define f4(x, y, z)  (x ^ y ^ z)
#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L
#define ROTL(n, X)  (((X) << n) | ((X) >> (32 - n)))
#define expand(W, i)  (W[ i & 15 ] = ROTL(1, (W[ i & 15 ] ^ W[(i - 14) & 15 ] ^ W[ (i - 8) & 15 ] ^ W[ (i - 3) & 15 ])))
#define subRound(a, b, c, d, e, f, k, data)  (e += ROTL(5, a) + f(b, c, d) + k + data, b = ROTL(30, b))
#endif
static void SHATransform(dbus_uint32_t *digest, dbus_uint32_t *data) {
  dbus_uint32_t A, B, C, D, E;
  dbus_uint32_t eData[16];
  A = digest[0];
  B = digest[1];
  C = digest[2];
  D = digest[3];
  E = digest[4];
  memmove(eData, data, SHA_DATASIZE);
  subRound(A, B, C, D, E, f1, K1, eData[0]);
  subRound(E, A, B, C, D, f1, K1, eData[1]);
  subRound(D, E, A, B, C, f1, K1, eData[2]);
  subRound(C, D, E, A, B, f1, K1, eData[3]);
  subRound(B, C, D, E, A, f1, K1, eData[4]);
  subRound(A, B, C, D, E, f1, K1, eData[5]);
  subRound(E, A, B, C, D, f1, K1, eData[6]);
  subRound(D, E, A, B, C, f1, K1, eData[7]);
  subRound(C, D, E, A, B, f1, K1, eData[8]);
  subRound(B, C, D, E, A, f1, K1, eData[9]);
  subRound(A, B, C, D, E, f1, K1, eData[10]);
  subRound(E, A, B, C, D, f1, K1, eData[11]);
  subRound(D, E, A, B, C, f1, K1, eData[12]);
  subRound(C, D, E, A, B, f1, K1, eData[13]);
  subRound(B, C, D, E, A, f1, K1, eData[14]);
  subRound(A, B, C, D, E, f1, K1, eData[15]);
  subRound(E, A, B, C, D, f1, K1, expand(eData, 16));
  subRound(D, E, A, B, C, f1, K1, expand(eData, 17));
  subRound(C, D, E, A, B, f1, K1, expand(eData, 18));
  subRound(B, C, D, E, A, f1, K1, expand(eData, 19));
  subRound(A, B, C, D, E, f2, K2, expand(eData, 20));
  subRound(E, A, B, C, D, f2, K2, expand(eData, 21));
  subRound(D, E, A, B, C, f2, K2, expand(eData, 22));
  subRound(C, D, E, A, B, f2, K2, expand(eData, 23));
  subRound(B, C, D, E, A, f2, K2, expand(eData, 24));
  subRound(A, B, C, D, E, f2, K2, expand(eData, 25));
  subRound(E, A, B, C, D, f2, K2, expand(eData, 26));
  subRound(D, E, A, B, C, f2, K2, expand(eData, 27));
  subRound(C, D, E, A, B, f2, K2, expand(eData, 28));
  subRound(B, C, D, E, A, f2, K2, expand(eData, 29));
  subRound(A, B, C, D, E, f2, K2, expand(eData, 30));
  subRound(E, A, B, C, D, f2, K2, expand(eData, 31));
  subRound(D, E, A, B, C, f2, K2, expand(eData, 32));
  subRound(C, D, E, A, B, f2, K2, expand(eData, 33));
  subRound(B, C, D, E, A, f2, K2, expand(eData, 34));
  subRound(A, B, C, D, E, f2, K2, expand(eData, 35));
  subRound(E, A, B, C, D, f2, K2, expand(eData, 36));
  subRound(D, E, A, B, C, f2, K2, expand(eData, 37));
  subRound(C, D, E, A, B, f2, K2, expand(eData, 38));
  subRound(B, C, D, E, A, f2, K2, expand(eData, 39));
  subRound(A, B, C, D, E, f3, K3, expand(eData, 40));
  subRound(E, A, B, C, D, f3, K3, expand(eData, 41));
  subRound(D, E, A, B, C, f3, K3, expand(eData, 42));
  subRound(C, D, E, A, B, f3, K3, expand(eData, 43));
  subRound(B, C, D, E, A, f3, K3, expand(eData, 44));
  subRound(A, B, C, D, E, f3, K3, expand(eData, 45));
  subRound(E, A, B, C, D, f3, K3, expand(eData, 46));
  subRound(D, E, A, B, C, f3, K3, expand(eData, 47));
  subRound(C, D, E, A, B, f3, K3, expand(eData, 48));
  subRound(B, C, D, E, A, f3, K3, expand(eData, 49));
  subRound(A, B, C, D, E, f3, K3, expand(eData, 50));
  subRound(E, A, B, C, D, f3, K3, expand(eData, 51));
  subRound(D, E, A, B, C, f3, K3, expand(eData, 52));
  subRound(C, D, E, A, B, f3, K3, expand(eData, 53));
  subRound(B, C, D, E, A, f3, K3, expand(eData, 54));
  subRound(A, B, C, D, E, f3, K3, expand(eData, 55));
  subRound(E, A, B, C, D, f3, K3, expand(eData, 56));
  subRound(D, E, A, B, C, f3, K3, expand(eData, 57));
  subRound(C, D, E, A, B, f3, K3, expand(eData, 58));
  subRound(B, C, D, E, A, f3, K3, expand(eData, 59));
  subRound(A, B, C, D, E, f4, K4, expand(eData, 60));
  subRound(E, A, B, C, D, f4, K4, expand(eData, 61));
  subRound(D, E, A, B, C, f4, K4, expand(eData, 62));
  subRound(C, D, E, A, B, f4, K4, expand(eData, 63));
  subRound(B, C, D, E, A, f4, K4, expand(eData, 64));
  subRound(A, B, C, D, E, f4, K4, expand(eData, 65));
  subRound(E, A, B, C, D, f4, K4, expand(eData, 66));
  subRound(D, E, A, B, C, f4, K4, expand(eData, 67));
  subRound(C, D, E, A, B, f4, K4, expand(eData, 68));
  subRound(B, C, D, E, A, f4, K4, expand(eData, 69));
  subRound(A, B, C, D, E, f4, K4, expand(eData, 70));
  subRound(E, A, B, C, D, f4, K4, expand(eData, 71));
  subRound(D, E, A, B, C, f4, K4, expand(eData, 72));
  subRound(C, D, E, A, B, f4, K4, expand(eData, 73));
  subRound(B, C, D, E, A, f4, K4, expand(eData, 74));
  subRound(A, B, C, D, E, f4, K4, expand(eData, 75));
  subRound(E, A, B, C, D, f4, K4, expand(eData, 76));
  subRound(D, E, A, B, C, f4, K4, expand(eData, 77));
  subRound(C, D, E, A, B, f4, K4, expand(eData, 78));
  subRound(B, C, D, E, A, f4, K4, expand(eData, 79));
  digest[0] += A;
  digest[1] += B;
  digest[2] += C;
  digest[3] += D;
  digest[4] += E;
}
#ifdef WORDS_BIGENDIAN
#define swap_words(buffer, byte_count)
#else
static void swap_words(dbus_uint32_t *buffer, int byte_count) {
  byte_count /= sizeof(dbus_uint32_t);
  while(byte_count--) {
      *buffer = DBUS_UINT32_SWAP_LE_BE(*buffer);
      ++buffer;
  }
}
#endif
static void sha_init(DBusSHAContext *context) {
  context->digest[0] = h0init;
  context->digest[1] = h1init;
  context->digest[2] = h2init;
  context->digest[3] = h3init;
  context->digest[4] = h4init;
  context->count_lo = context->count_hi = 0;
}
static void sha_append(DBusSHAContext *context, const unsigned char *buffer, unsigned int count) {
  dbus_uint32_t tmp;
  unsigned int dataCount;
  tmp = context->count_lo;
  if ((context->count_lo = tmp + ((dbus_uint32_t)count << 3)) < tmp)
    context->count_hi++;
  context->count_hi += count >> 29;
  dataCount = (int)(tmp >> 3) & 0x3F;
  if (dataCount) {
      unsigned char *p = (unsigned char*)context->data + dataCount;
      dataCount = SHA_DATASIZE - dataCount;
      if (count < dataCount) {
          memmove(p, buffer, count);
          return;
      }
      memmove(p, buffer, dataCount);
      swap_words(context->data, SHA_DATASIZE);
      SHATransform(context->digest, context->data);
      buffer += dataCount;
      count -= dataCount;
  }
  while(count >= SHA_DATASIZE) {
      memmove(context->data, buffer, SHA_DATASIZE);
      swap_words(context->data, SHA_DATASIZE);
      SHATransform(context->digest, context->data);
      buffer += SHA_DATASIZE;
      count -= SHA_DATASIZE;
  }
  memmove(context->data, buffer, count);
}
static void sha_finish(DBusSHAContext *context, unsigned char digest[20]) {
  int count;
  unsigned char *data_p;
  count = (int)context->count_lo;
  count = (count >> 3) & 0x3F;
  data_p = (unsigned char*)context->data + count;
  *data_p++ = 0x80;
  count = SHA_DATASIZE - 1 - count;
  if (count < 8) {
      memset(data_p, 0, count);
      swap_words(context->data, SHA_DATASIZE);
      SHATransform(context->digest, context->data);
      memset(context->data, 0, SHA_DATASIZE - 8);
  } else memset(data_p, 0, count - 8);
  context->data[14] = context->count_hi;
  context->data[15] = context->count_lo;
  swap_words(context->data, SHA_DATASIZE - 8);
  SHATransform(context->digest, context->data);
  swap_words(context->digest, SHA_DIGESTSIZE);
  memmove(digest, context->digest, SHA_DIGESTSIZE);
}
void _dbus_sha_init(DBusSHAContext *context) {
  sha_init(context);
}
void _dbus_sha_update(DBusSHAContext *context, const DBusString *data) {
  unsigned int inputLen;
  const unsigned char *input;
  input = (const unsigned char*)_dbus_string_get_const_data(data);
  inputLen = _dbus_string_get_length(data);
  sha_append(context, input, inputLen);
}
dbus_bool_t _dbus_sha_final(DBusSHAContext *context, DBusString *results) {
  unsigned char digest[20];
  sha_finish(context, digest);
  if (!_dbus_string_append_len(results, (const char*)digest, 20)) return FALSE;
  _DBUS_ZERO(*context);
  return TRUE;
}
dbus_bool_t _dbus_sha_compute(const DBusString *data, DBusString *ascii_output) {
  DBusSHAContext context;
  DBusString digest;
  _dbus_sha_init(&context);
  _dbus_sha_update(&context, data);
  if (!_dbus_string_init(&digest)) return FALSE;
  if (!_dbus_sha_final(&context, &digest)) goto error;
  if (!_dbus_string_hex_encode(&digest, 0, ascii_output, _dbus_string_get_length(ascii_output))) goto error;
  _dbus_string_free(&digest);
  return TRUE;
error:
  _dbus_string_free(&digest);
  return FALSE;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include "dbus-test.h"

static dbus_bool_t check_sha_binary(const unsigned char *input, int input_len, const char *expected) {
  DBusString input_str;
  DBusString expected_str;
  DBusString results;
  _dbus_string_init_const_len(&input_str, (const char*)input, input_len);
  _dbus_string_init_const(&expected_str, expected);
  if (!_dbus_string_init(&results)) _dbus_test_fatal("no memory for SHA-1 results");
  if (!_dbus_sha_compute(&input_str, &results)) _dbus_test_fatal("no memory for SHA-1 results");
  if (!_dbus_string_equal(&expected_str, &results)) {
      _dbus_warn("Expected hash %s got %s for SHA-1 sum", expected, _dbus_string_get_const_data(&results));
      _dbus_string_free(&results);
      return FALSE;
  }
  _dbus_string_free(&results);
  return TRUE;
}
static dbus_bool_t check_sha_str(const char *input, const char *expected) {
  return check_sha_binary((unsigned char*)input, strlen(input), expected);
}
static dbus_bool_t decode_compact_string(const DBusString *line, DBusString *decoded) {
  int n_bits;
  dbus_bool_t current_b;
  int offset;
  int next;
  long val;
  int length_bytes;
  offset = 0;
  next = 0;
  if (!_dbus_string_parse_int(line, offset, &val, &next)) {
      fprintf(stderr, "could not parse length at start of compact string: %s\n", _dbus_string_get_const_data(line));
      return FALSE;
  }
  _dbus_string_skip_blank(line, next, &next);
  offset = next;
  if (!_dbus_string_parse_int(line, offset, &val, &next)) {
      fprintf(stderr, "could not parse start bit 'b' in compact string: %s\n", _dbus_string_get_const_data(line));
      return FALSE;
  }
  if (!(val == 0 || val == 1)) {
      fprintf(stderr, "the value 'b' must be 0 or 1, see sha-1/Readme.txt\n");
      return FALSE;
  }
  _dbus_string_skip_blank(line, next, &next);
  current_b = val;
  n_bits = 0;
  while(next < _dbus_string_get_length(line)) {
      int total_bits;
      offset = next;
      if (_dbus_string_get_byte(line, offset) == '^') break;
      if (!_dbus_string_parse_int(line, offset, &val, &next)) {
          fprintf(stderr, "could not parse bit count in compact string\n");
          return FALSE;
      }
      total_bits = n_bits + val;
      while(n_bits < total_bits) {
          int byte_containing_next_bit = n_bits / 8;
          int bit_containing_next_bit = 7 - (n_bits % 8);
          unsigned char old_byte;
          if (byte_containing_next_bit >= _dbus_string_get_length(decoded)) {
              if (!_dbus_string_set_length(decoded, byte_containing_next_bit + 1))
                _dbus_test_fatal("no memory to extend to next byte");
          }
          old_byte = _dbus_string_get_byte(decoded, byte_containing_next_bit);
          old_byte |= current_b << bit_containing_next_bit;
      #if 0
          _dbus_test_diag("Appending bit %d to byte %d at bit %d resulting in byte 0x%x", current_b, byte_containing_next_bit, bit_containing_next_bit, old_byte);
      #endif
          _dbus_string_set_byte(decoded, byte_containing_next_bit, old_byte);
          ++n_bits;
      }
      _dbus_string_skip_blank(line, next, &next);
      current_b = !current_b;
  }
  length_bytes = (n_bits / 8 + ((n_bits % 8) ? 1 : 0));
  if (_dbus_string_get_length(decoded) != length_bytes) {
      fprintf(stderr, "Expected length %d bytes %d bits for compact string, got %d bytes\n", length_bytes, n_bits, _dbus_string_get_length(decoded));
      return FALSE;
  } else return TRUE;
}
static dbus_bool_t get_next_expected_result(DBusString *results, DBusString *result) {
  DBusString line;
  dbus_bool_t retval;
  retval = FALSE;
  if (!_dbus_string_init(&line)) _dbus_test_fatal("no memory");
next_iteration:
  while(_dbus_string_pop_line(results, &line)) {
      _dbus_string_delete_leading_blanks(&line);
      if (_dbus_string_get_length(&line) == 0) goto next_iteration;
      else if (_dbus_string_starts_with_c_str(&line, "#")) goto next_iteration;
      else if (_dbus_string_starts_with_c_str(&line, "H>"));
      else if (_dbus_string_starts_with_c_str(&line, "D>") || _dbus_string_starts_with_c_str(&line, "<D")) goto next_iteration;
      else {
          int i;
          if (!_dbus_string_move(&line, 0, result, 0)) _dbus_test_fatal("no memory");
          i = 0;
          while(i < _dbus_string_get_length(result)) {
              unsigned char c = _dbus_string_get_byte(result, i);
              switch (c) {
                  case 'A': _dbus_string_set_byte(result, i, 'a'); break;
                  case 'B': _dbus_string_set_byte(result, i, 'b'); break;
                  case 'C': _dbus_string_set_byte(result, i, 'c'); break;
                  case 'D': _dbus_string_set_byte(result, i, 'd'); break;
                  case 'E': _dbus_string_set_byte(result, i, 'e'); break;
                  case 'F': _dbus_string_set_byte(result, i, 'f'); break;
                  case '^': case ' ':
                      _dbus_string_delete(result, i, 1);
                      --i;
                      break;
                  default: if ((c < '0' || c > '9') && (c < 'a' || c > 'f')) _dbus_test_fatal("invalid SHA-1 test script");
              }
              ++i;
          }
          break;
      }
  }
  retval = TRUE;
  _dbus_string_free(&line);
  return retval;
}
static dbus_bool_t process_test_data(const char *test_data_dir) {
  DBusString tests_file;
  DBusString results_file;
  DBusString tests;
  DBusString results;
  DBusString line;
  DBusString tmp;
  int line_no;
  dbus_bool_t retval;
  int success_count;
  DBusError error = DBUS_ERROR_INIT;
  retval = FALSE;
  if (!_dbus_string_init(&tests_file)) _dbus_test_fatal("no memory");
  if (!_dbus_string_init(&results_file)) _dbus_test_fatal("no memory");
  if (!_dbus_string_init(&tests)) _dbus_test_fatal("no memory");
  if (!_dbus_string_init(&results)) _dbus_test_fatal("no memory");
  if (!_dbus_string_init(&line)) _dbus_test_fatal("no memory");
  if (!_dbus_string_append(&tests_file, test_data_dir)) _dbus_test_fatal("no memory");
  if (!_dbus_string_append(&results_file, test_data_dir)) _dbus_test_fatal("no memory");
  _dbus_string_init_const(&tmp, "sha-1/byte-messages.sha1");
  if (!_dbus_concat_dir_and_file(&tests_file, &tmp)) _dbus_test_fatal("no memory");
  _dbus_string_init_const(&tmp, "sha-1/byte-hashes.sha1");
  if (!_dbus_concat_dir_and_file(&results_file, &tmp)) _dbus_test_fatal("no memory");
  if (!_dbus_file_get_contents(&tests, &tests_file, &error)) {
      fprintf(stderr, "could not load test data file %s: %s\n", _dbus_string_get_const_data(&tests_file), error.message);
      dbus_error_free(&error);
      goto out;
  }
  if (!_dbus_file_get_contents(&results, &results_file, &error)) {
      fprintf(stderr, "could not load results data file %s: %s\n", _dbus_string_get_const_data(&results_file), error.message);
      dbus_error_free(&error);
      goto out;
  }
  success_count = 0;
  line_no = 0;
next_iteration:
  while (_dbus_string_pop_line(&tests, &line)) {
      line_no += 1;
      _dbus_string_delete_leading_blanks(&line);
      if (_dbus_string_get_length(&line) == 0) goto next_iteration;
      else if (_dbus_string_starts_with_c_str(&line, "#")) goto next_iteration;
      else if (_dbus_string_starts_with_c_str(&line, "H>")) {
          _dbus_test_diag("SHA-1: %s", _dbus_string_get_const_data(&line));
          if (_dbus_string_find(&line, 0, "Type 3", NULL)) {
              _dbus_test_diag(" (ending tests due to Type 3 tests seen - this is normal)");
              break;
          }
      } else if (_dbus_string_starts_with_c_str(&line, "D>") || _dbus_string_starts_with_c_str(&line, "<D")) goto next_iteration;
      else {
          DBusString test;
          DBusString result;
          DBusString next_line;
          DBusString expected;
          dbus_bool_t success;
          success = FALSE;
          if (!_dbus_string_init(&next_line)) _dbus_test_fatal("no memory");
          if (!_dbus_string_init(&expected)) _dbus_test_fatal("no memory");
          if (!_dbus_string_init(&test)) _dbus_test_fatal("no memory");
          if (!_dbus_string_init(&result)) _dbus_test_fatal("no memory");
          while (!_dbus_string_find(&line, 0, "^", NULL) && _dbus_string_pop_line(&tests, &next_line)) {
              if (!_dbus_string_append_byte(&line, ' ') || !_dbus_string_move(&next_line, 0, &line, _dbus_string_get_length(&line)))
                  _dbus_test_fatal("no memory");
          }
          if (!decode_compact_string(&line, &test)) {
              fprintf(stderr, "Failed to decode line %d as a compact string\n", line_no);
              goto failure;
          }
          if (!_dbus_sha_compute(&test, &result)) _dbus_test_fatal("no memory for SHA-1 result");
          if (!get_next_expected_result(&results, &expected)) {
              fprintf(stderr, "Failed to read an expected result\n");
              goto failure;
          }
          if (!_dbus_string_equal(&result, &expected)) {
              fprintf(stderr," for line %d got hash %s expected %s\n", line_no, _dbus_string_get_const_data(&result), _dbus_string_get_const_data(&expected));
              goto failure;
          } else success_count += 1;
          success = TRUE;
      failure:
          _dbus_string_free(&test);
          _dbus_string_free(&result);
          _dbus_string_free(&next_line);
          _dbus_string_free(&expected);
          if (!success) goto out;
      }
  }
  retval = TRUE;
  _dbus_test_diag("Passed the %d SHA-1 tests in the test file", success_count);
out:
  _dbus_string_free(&tests_file);
  _dbus_string_free(&results_file);
  _dbus_string_free(&tests);
  _dbus_string_free(&results);
  _dbus_string_free(&line);
  return retval;
}
dbus_bool_t _dbus_sha_test(const char *test_data_dir) {
  unsigned char all_bytes[256];
  int i;
  if (test_data_dir != NULL) {
      if (!process_test_data(test_data_dir)) return FALSE;
  } else _dbus_test_diag("No test data dir");
  i = 0;
  while(i < 256) {
      all_bytes[i] = i;
      ++i;
  }
  if (!check_sha_binary(all_bytes, 256,"4916d6bdb7f78e6803698cab32d1586ea457dfc8")) return FALSE;
#define CHECK(input, expected)  if (!check_sha_str(input, expected)) return FALSE
  CHECK("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
  CHECK("a", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8");
  CHECK("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
  CHECK("message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
  CHECK("abcdefghijklmnopqrstuvwxyz", "32d10c7b8cf96570ca04ce37f2a19d84240d3a89");
  CHECK("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789","761c457bf73b14d27e9e9265c46f4b4dda11f940");
  CHECK("12345678901234567890123456789012345678901234567890123456789012345678901234567890","50abf5706a150990a08b2c5ea40fa0e585554732");
  return TRUE;
}
#endif