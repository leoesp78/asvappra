#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#include "dbus-protocol.h"
#include "dbus-errors.h"
#include "dbus-file.h"
#include "dbus-internals.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-unix.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif
dbus_bool_t _dbus_file_get_contents(DBusString *str, const DBusString *filename, DBusError *error) {
  int fd;
  struct stat sb;
  int orig_len;
  int total;
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  fd = open(filename_c, O_RDONLY | O_BINARY);
  if (fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to open \"%s\": %s", filename_c, _dbus_strerror(errno));
      return FALSE;
  }
  _dbus_verbose("file fd %d opened\n", fd);
  if (fstat(fd, &sb) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Failed to stat \"%s\": %s", filename_c, _dbus_strerror(errno));
      _dbus_verbose("fstat() failed: %s", _dbus_strerror(errno));
      _dbus_close(fd, NULL);
      return FALSE;
  }
  if (sb.st_size > _DBUS_ONE_MEGABYTE) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"File size %lu of \"%s\" is too large.", (unsigned long)sb.st_size, filename_c);
      _dbus_close(fd, NULL);
      return FALSE;
  }
  total = 0;
  orig_len = _dbus_string_get_length(str);
  if (sb.st_size > 0 && S_ISREG(sb.st_mode)) {
      int bytes_read;
      while(total < (int)sb.st_size) {
          bytes_read = _dbus_read(fd, str,sb.st_size - total);
          if (bytes_read <= 0) {
              dbus_set_error(error, _dbus_error_from_errno(errno),"Error reading \"%s\": %s", filename_c, _dbus_strerror(errno));
              _dbus_verbose("read() failed: %s", _dbus_strerror(errno));
              _dbus_close(fd, NULL);
              _dbus_string_set_length(str, orig_len);
              return FALSE;
          } else total += bytes_read;
      }
      _dbus_close(fd, NULL);
      return TRUE;
  } else if (sb.st_size != 0) {
      _dbus_verbose("Can only open regular files at the moment.\n");
      dbus_set_error(error, DBUS_ERROR_FAILED,"\"%s\" is not a regular file", filename_c);
      _dbus_close(fd, NULL);
      return FALSE;
  } else {
      _dbus_close(fd, NULL);
      return TRUE;
  }
}
dbus_bool_t _dbus_string_save_to_file(const DBusString *str, const DBusString *filename, dbus_bool_t world_readable, DBusError *error) {
  int fd;
  int bytes_to_write;
  const char *filename_c;
  DBusString tmp_filename;
  const char *tmp_filename_c;
  int total;
  dbus_bool_t need_unlink;
  dbus_bool_t retval;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  fd = -1;
  retval = FALSE;
  need_unlink = FALSE;
  if (!_dbus_string_init(&tmp_filename)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return FALSE;
  }
  if (!_dbus_string_copy(filename, 0, &tmp_filename, 0)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&tmp_filename);
      return FALSE;
  }
  if (!_dbus_string_append(&tmp_filename, ".")) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_string_free(&tmp_filename);
      return FALSE;
  }
#define N_TMP_FILENAME_RANDOM_BYTES 8
  if (!_dbus_generate_random_ascii(&tmp_filename, N_TMP_FILENAME_RANDOM_BYTES, error)) {
      _dbus_string_free(&tmp_filename);
      return FALSE;
  }
  filename_c = _dbus_string_get_const_data(filename);
  tmp_filename_c = _dbus_string_get_const_data(&tmp_filename);
  fd = open(tmp_filename_c, O_WRONLY | O_BINARY | O_EXCL | O_CREAT,world_readable ? 0644 : 0600);
  if (fd < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not create %s: %s", tmp_filename_c, _dbus_strerror(errno));
      goto out;
  }
  if (world_readable) {
      if (fchmod(fd, 0644) < 0) {
          dbus_set_error(error, _dbus_error_from_errno(errno),"Could not chmod %s: %s", tmp_filename_c, _dbus_strerror(errno));
          goto out;
      }
  }
  _dbus_verbose("tmp file fd %d opened\n", fd);
  need_unlink = TRUE;
  total = 0;
  bytes_to_write = _dbus_string_get_length(str);
  while(total < bytes_to_write) {
      int bytes_written;
      bytes_written = _dbus_write(fd, str, total,bytes_to_write - total);
      if (bytes_written <= 0) {
          dbus_set_error(error, _dbus_error_from_errno(errno),"Could not write to %s: %s", tmp_filename_c, _dbus_strerror(errno));
          goto out;
      }
      total += bytes_written;
  }
  if (fsync(fd)) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not synchronize file %s: %s", tmp_filename_c, _dbus_strerror(errno));
      goto out;
  }
  if (!_dbus_close(fd, NULL)) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not close file %s: %s", tmp_filename_c, _dbus_strerror(errno));
      goto out;
  }
  fd = -1;
  if (rename(tmp_filename_c, filename_c) < 0) {
      dbus_set_error(error, _dbus_error_from_errno(errno),"Could not rename %s to %s: %s", tmp_filename_c, filename_c, _dbus_strerror(errno));
      goto out;
  }
  need_unlink = FALSE;
  retval = TRUE;
out:
  if (fd >= 0) _dbus_close(fd, NULL);
  if (need_unlink && unlink(tmp_filename_c) < 0) _dbus_verbose("Failed to unlink temp file %s: %s\n", tmp_filename_c, _dbus_strerror(errno));
  _dbus_string_free(&tmp_filename);
  if (!retval) _DBUS_ASSERT_ERROR_IS_SET(error);
  return retval;
}
dbus_bool_t _dbus_make_file_world_readable(const DBusString *filename, DBusError *error) {
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (chmod(filename_c, 0644) == -1) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Could not change permissions of file %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_create_file_exclusively(const DBusString *filename, DBusError *error) {
  int fd;
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  fd = open(filename_c, O_WRONLY | O_BINARY | O_EXCL | O_CREAT,0600);
  if (fd < 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Could not create file %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  }
  _dbus_verbose("exclusive file fd %d opened\n", fd);
  if (!_dbus_close(fd, NULL)) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Could not close file %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  }
  return TRUE;
}
dbus_bool_t _dbus_delete_file(const DBusString *filename, DBusError *error) {
  const char *filename_c;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  filename_c = _dbus_string_get_const_data(filename);
  if (unlink(filename_c) < 0) {
      dbus_set_error(error, DBUS_ERROR_FAILED,"Failed to delete file %s: %s\n", filename_c, _dbus_strerror(errno));
      return FALSE;
  } else return TRUE;
}