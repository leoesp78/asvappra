#include "config.h"
#include "dbus-marshal-byteswap.h"
#include "dbus-marshal-basic.h"
#include "dbus-signature.h"

static void byteswap_body_helper(DBusTypeReader *reader, dbus_bool_t walk_reader_to_end, int old_byte_order, int new_byte_order, unsigned char *p,
                                 unsigned char **new_p) {
  int current_type;
  while((current_type = _dbus_type_reader_get_current_type(reader)) != DBUS_TYPE_INVALID) {
      switch(current_type) {
          case DBUS_TYPE_BYTE: ++p; break;
          case DBUS_TYPE_INT16: case DBUS_TYPE_UINT16: {
                  p = _DBUS_ALIGN_ADDRESS(p, 2);
                  *((dbus_uint16_t*)p) = DBUS_UINT16_SWAP_LE_BE(*((dbus_uint16_t*)p));
                  p += 2;
              }
              break;
          case DBUS_TYPE_BOOLEAN: case DBUS_TYPE_INT32: case DBUS_TYPE_UINT32: {
                  p = _DBUS_ALIGN_ADDRESS(p, 4);
                  *((dbus_uint32_t*)p) = DBUS_UINT32_SWAP_LE_BE(*((dbus_uint32_t*)p));
                  p += 4;
              }
              break;
          case DBUS_TYPE_INT64: case DBUS_TYPE_UINT64: case DBUS_TYPE_DOUBLE: {
                  p = _DBUS_ALIGN_ADDRESS(p, 8);
                  *((dbus_uint64_t*)p) = DBUS_UINT64_SWAP_LE_BE(*((dbus_uint64_t*)p));
                  p += 8;
              }
              break;
          case DBUS_TYPE_ARRAY: case DBUS_TYPE_STRING: case DBUS_TYPE_OBJECT_PATH: {
                  dbus_uint32_t array_len;
                  p = _DBUS_ALIGN_ADDRESS(p, 4);
                  array_len = _dbus_unpack_uint32(old_byte_order, p);
                  *((dbus_uint32_t*)p) = DBUS_UINT32_SWAP_LE_BE(*((dbus_uint32_t*)p));
                  p += 4;
                  if (current_type == DBUS_TYPE_ARRAY) {
                      int elem_type;
                      int alignment;
                      elem_type = _dbus_type_reader_get_element_type(reader);
                      alignment = _dbus_type_get_alignment(elem_type);
                      _dbus_assert((array_len / alignment) < DBUS_MAXIMUM_ARRAY_LENGTH);
                      p = _DBUS_ALIGN_ADDRESS(p, alignment);
                      if (dbus_type_is_fixed(elem_type)) {
                          if (alignment > 1) _dbus_swap_array(p, array_len / alignment, alignment);
                          p += array_len;
                      } else {
                          DBusTypeReader sub;
                          const unsigned char *array_end;
                          array_end = p + array_len;
                          _dbus_type_reader_recurse(reader, &sub);
                          while(p < array_end) byteswap_body_helper(&sub, FALSE, old_byte_order, new_byte_order, p, &p);
                      }
                  } else {
                      _dbus_assert(current_type == DBUS_TYPE_STRING || current_type == DBUS_TYPE_OBJECT_PATH);
                      p += (array_len + 1);
                  }
              }
              break;
          case DBUS_TYPE_SIGNATURE: {
                  dbus_uint32_t sig_len;
                  sig_len = *p;
                  p += (sig_len + 2);
              }
              break;
          case DBUS_TYPE_VARIANT: {
                  dbus_uint32_t sig_len;
                  DBusString sig;
                  DBusTypeReader sub;
                  int contained_alignment;
                  sig_len = *p;
                  ++p;
                  _dbus_string_init_const_len(&sig, (const char*)p, sig_len);
                  p += (sig_len + 1);
                  contained_alignment = _dbus_type_get_alignment(_dbus_first_type_in_signature(&sig, 0));
                  p = _DBUS_ALIGN_ADDRESS(p, contained_alignment);
                  _dbus_type_reader_init_types_only(&sub, &sig, 0);
                  byteswap_body_helper(&sub, FALSE, old_byte_order, new_byte_order, p, &p);
              }
              break;
          case DBUS_TYPE_STRUCT: case DBUS_TYPE_DICT_ENTRY: {
                  DBusTypeReader sub;
                  p = _DBUS_ALIGN_ADDRESS(p, 8);
                  _dbus_type_reader_recurse(reader, &sub);
                  byteswap_body_helper(&sub, TRUE, old_byte_order, new_byte_order, p, &p);
              }
              break;
          case DBUS_TYPE_UNIX_FD: _dbus_assert_not_reached("attempted to byteswap unix fds which makes no sense"); break;
          default: _dbus_assert_not_reached("invalid typecode in supposedly-validated signature");
      }
      if (walk_reader_to_end) _dbus_type_reader_next(reader);
      else break;
  }
  if (new_p) *new_p = p;
}
void _dbus_marshal_byteswap(const DBusString *signature, int signature_start, int old_byte_order, int new_byte_order, DBusString *value_str, int value_pos) {
  DBusTypeReader reader;
  _dbus_assert(value_pos >= 0);
  _dbus_assert(value_pos <= _dbus_string_get_length(value_str));
  if (old_byte_order == new_byte_order) return;
  _dbus_type_reader_init_types_only(&reader, signature, signature_start);
  byteswap_body_helper(&reader, TRUE, old_byte_order, new_byte_order,_dbus_string_get_udata_len(value_str, value_pos, 0), NULL);
}