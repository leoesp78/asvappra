#include "config.h"
#include "dbus-internals.h"
#include "dbus-connection-internal.h"
#include "dbus-nonce.h"
#include "dbus-transport-socket.h"
#include "dbus-transport-protected.h"
#include "dbus-watch.h"
#include "dbus-credentials.h"

typedef struct DBusTransportSocket DBusTransportSocket;
struct DBusTransportSocket {
  DBusTransport base;
  DBusSocket fd;
  DBusWatch *read_watch;
  DBusWatch *write_watch;
  int max_bytes_read_per_iteration;
  int max_bytes_written_per_iteration;
  int message_bytes_written;
  DBusString encoded_outgoing;
  DBusString encoded_incoming;
};
static void free_watches(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  _dbus_verbose("start\n");
  if (socket_transport->read_watch) {
      if (transport->connection)
        _dbus_connection_remove_watch_unlocked(transport->connection, socket_transport->read_watch);
      _dbus_watch_invalidate(socket_transport->read_watch);
      _dbus_watch_unref(socket_transport->read_watch);
      socket_transport->read_watch = NULL;
  }
  if (socket_transport->write_watch) {
      if (transport->connection)
        _dbus_connection_remove_watch_unlocked(transport->connection, socket_transport->write_watch);
      _dbus_watch_invalidate(socket_transport->write_watch);
      _dbus_watch_unref(socket_transport->write_watch);
      socket_transport->write_watch = NULL;
  }
  _dbus_verbose("end\n");
}
static void socket_finalize(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  _dbus_verbose("\n");
  free_watches(transport);
  _dbus_string_free(&socket_transport->encoded_outgoing);
  _dbus_string_free(&socket_transport->encoded_incoming);
  _dbus_transport_finalize_base(transport);
  _dbus_assert(socket_transport->read_watch == NULL);
  _dbus_assert(socket_transport->write_watch == NULL);
  dbus_free(transport);
}
static void check_write_watch(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  dbus_bool_t needed;
  if (transport->connection == NULL) return;
  if (transport->disconnected) {
      _dbus_assert(socket_transport->write_watch == NULL);
      return;
  }
  _dbus_transport_ref(transport);
  if (_dbus_transport_try_to_authenticate (transport))needed = _dbus_connection_has_messages_to_send_unlocked(transport->connection);
  else {
      if (transport->send_credentials_pending) needed = TRUE;
      else {
          DBusAuthState auth_state;
          auth_state = _dbus_auth_do_work (transport->auth);
          if (auth_state == DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND || auth_state == DBUS_AUTH_STATE_WAITING_FOR_MEMORY) needed = TRUE;
          else needed = FALSE;
      }
  }
  _dbus_verbose("check_write_watch(): needed = %d on connection %p watch %p fd = %" DBUS_SOCKET_FORMAT " outgoing messages exist %d\n",
                needed, transport->connection, socket_transport->write_watch, _dbus_socket_printable (socket_transport->fd),
                _dbus_connection_has_messages_to_send_unlocked (transport->connection));
  _dbus_connection_toggle_watch_unlocked (transport->connection, socket_transport->write_watch, needed);
  _dbus_transport_unref (transport);
}
static void check_read_watch(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  dbus_bool_t need_read_watch;
  _dbus_verbose("fd = %" DBUS_SOCKET_FORMAT "\n", _dbus_socket_printable(socket_transport->fd));
  if (transport->connection == NULL) return;
  if (transport->disconnected) {
      _dbus_assert(socket_transport->read_watch == NULL);
      return;
  }
  _dbus_transport_ref(transport);
  if (_dbus_transport_try_to_authenticate(transport)) {
      need_read_watch = (_dbus_counter_get_size_value(transport->live_messages) < transport->max_live_messages_size) &&
                        (_dbus_counter_get_unix_fd_value(transport->live_messages) < transport->max_live_messages_unix_fds);
  } else {
      if (transport->receive_credentials_pending) need_read_watch = TRUE;
      else {
          DBusAuthState auth_state;
          auth_state = _dbus_auth_do_work (transport->auth);
          if (auth_state == DBUS_AUTH_STATE_WAITING_FOR_INPUT || auth_state == DBUS_AUTH_STATE_WAITING_FOR_MEMORY || auth_state == DBUS_AUTH_STATE_AUTHENTICATED) {
              need_read_watch = TRUE;
          } else need_read_watch = FALSE;
      }
  }
  _dbus_verbose("  setting read watch enabled = %d\n", need_read_watch);
  _dbus_connection_toggle_watch_unlocked(transport->connection, socket_transport->read_watch, need_read_watch);
  _dbus_transport_unref(transport);
}
static void do_io_error(DBusTransport *transport) {
  _dbus_transport_ref(transport);
  _dbus_transport_disconnect(transport);
  _dbus_transport_unref(transport);
}
static dbus_bool_t read_data_into_auth(DBusTransport *transport, dbus_bool_t *oom) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  DBusString *buffer;
  int bytes_read;
  int saved_errno;
  *oom = FALSE;
  _dbus_auth_get_buffer(transport->auth, &buffer);
  bytes_read = _dbus_read_socket(socket_transport->fd, buffer, socket_transport->max_bytes_read_per_iteration);
  saved_errno = _dbus_save_socket_errno();
  _dbus_auth_return_buffer(transport->auth, buffer);
  if (bytes_read > 0) {
      _dbus_verbose(" read %d bytes in auth phase\n", bytes_read);
      return TRUE;
  } else if (bytes_read < 0) {
      if (_dbus_get_is_errno_enomem(saved_errno)) *oom = TRUE;
      else if (_dbus_get_is_errno_eagain_or_ewouldblock(saved_errno));
      else {
          _dbus_verbose("Error reading from remote app: %s\n", _dbus_strerror(saved_errno));
          do_io_error(transport);
      }
      return FALSE;
  } else {
      _dbus_assert(bytes_read == 0);
      _dbus_verbose("Disconnected from remote app\n");
      do_io_error(transport);
      return FALSE;
  }
}
static dbus_bool_t write_data_from_auth(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  int bytes_written;
  int saved_errno;
  const DBusString *buffer;
  if (!_dbus_auth_get_bytes_to_send(transport->auth, &buffer)) return FALSE;
  bytes_written = _dbus_write_socket(socket_transport->fd, buffer,0, _dbus_string_get_length (buffer));
  saved_errno = _dbus_save_socket_errno();
  if (bytes_written > 0) {
      _dbus_auth_bytes_sent(transport->auth, bytes_written);
      return TRUE;
  } else if (bytes_written < 0) {
      if (_dbus_get_is_errno_eagain_or_ewouldblock(saved_errno));
      else {
          _dbus_verbose("Error writing to remote app: %s\n", _dbus_strerror(saved_errno));
          do_io_error(transport);
      }
  }
  return FALSE;
}
static dbus_bool_t exchange_credentials(DBusTransport *transport, dbus_bool_t do_reading, dbus_bool_t do_writing) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  DBusError error = DBUS_ERROR_INIT;
  _dbus_verbose("exchange_credentials: do_reading = %d, do_writing = %d\n", do_reading, do_writing);
  if (do_writing && transport->send_credentials_pending) {
      if (_dbus_send_credentials_socket(socket_transport->fd, &error)) transport->send_credentials_pending = FALSE;
      else {
          _dbus_verbose("Failed to write credentials: %s\n", error.message);
          dbus_error_free(&error);
          do_io_error(transport);
      }
  }
  if (do_reading && transport->receive_credentials_pending) {
      if (_dbus_read_credentials_socket(socket_transport->fd, transport->credentials, &error)) transport->receive_credentials_pending = FALSE;
      else {
          _dbus_verbose("Failed to read credentials %s\n", error.message);
          dbus_error_free(&error);
          do_io_error(transport);
      }
  }
  if (!(transport->send_credentials_pending || transport->receive_credentials_pending)) {
      if (!_dbus_auth_set_credentials(transport->auth, transport->credentials)) return FALSE;
  }
  return TRUE;
}
static dbus_bool_t do_authentication(DBusTransport *transport, dbus_bool_t do_reading, dbus_bool_t do_writing, dbus_bool_t *auth_completed) {
  dbus_bool_t oom;
  dbus_bool_t orig_auth_state;
  oom = FALSE;
  orig_auth_state = _dbus_transport_try_to_authenticate(transport);
  if (orig_auth_state) {
      if (auth_completed) *auth_completed = FALSE;
      return TRUE;
  }
  _dbus_transport_ref(transport);
  while(!_dbus_transport_try_to_authenticate(transport) && _dbus_transport_get_is_connected(transport)) {
      if (!exchange_credentials(transport, do_reading, do_writing)) {
          oom = TRUE;
          goto out;
      }
      if (transport->send_credentials_pending || transport->receive_credentials_pending) {
          _dbus_verbose("send_credentials_pending = %d receive_credentials_pending = %d\n", transport->send_credentials_pending,
                        transport->receive_credentials_pending);
          goto out;
      }
      #define TRANSPORT_SIDE(t) ((t)->is_server ? "server" : "client")
      switch(_dbus_auth_do_work (transport->auth)) {
          case DBUS_AUTH_STATE_WAITING_FOR_INPUT:
              _dbus_verbose(" %s auth state: waiting for input\n", TRANSPORT_SIDE(transport));
              if (!do_reading || !read_data_into_auth(transport, &oom)) goto out;
              break;
          case DBUS_AUTH_STATE_WAITING_FOR_MEMORY:
              _dbus_verbose(" %s auth state: waiting for memory\n", TRANSPORT_SIDE(transport));
              oom = TRUE;
              goto out;
              break;
          case DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND:
              _dbus_verbose(" %s auth state: bytes to send\n", TRANSPORT_SIDE(transport));
              if (!do_writing || !write_data_from_auth(transport)) goto out;
              break;
          case DBUS_AUTH_STATE_NEED_DISCONNECT:
              _dbus_verbose(" %s auth state: need to disconnect\n", TRANSPORT_SIDE(transport));
              do_io_error(transport);
              break;
          case DBUS_AUTH_STATE_AUTHENTICATED: _dbus_verbose(" %s auth state: authenticated\n", TRANSPORT_SIDE(transport)); break;
          default: _dbus_assert_not_reached("invalid auth state");
      }
  }
out:
  if (auth_completed) *auth_completed = (orig_auth_state != _dbus_transport_try_to_authenticate(transport));
  check_read_watch(transport);
  check_write_watch(transport);
  _dbus_transport_unref(transport);
  if (oom) return FALSE;
  else return TRUE;
}
static dbus_bool_t do_writing(DBusTransport *transport) {
  int total;
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  dbus_bool_t oom;
  if (!_dbus_transport_try_to_authenticate(transport)) {
      _dbus_verbose("Not authenticated, not writing anything\n");
      return TRUE;
  }
  if (transport->disconnected) {
      _dbus_verbose("Not connected, not writing anything\n");
      return TRUE;
  }
#if 1
  _dbus_verbose("do_writing(), have_messages = %d, fd = %" DBUS_SOCKET_FORMAT "\n", _dbus_connection_has_messages_to_send_unlocked(transport->connection),
                _dbus_socket_printable(socket_transport->fd));
#endif
  oom = FALSE;
  total = 0;
  while(!transport->disconnected && _dbus_connection_has_messages_to_send_unlocked(transport->connection)) {
      int bytes_written;
      DBusMessage *message;
      const DBusString *header;
      const DBusString *body;
      int header_len, body_len;
      int total_bytes_to_write;
      int saved_errno;
      if (total > socket_transport->max_bytes_written_per_iteration) {
          _dbus_verbose("%d bytes exceeds %d bytes written per iteration, returning\n", total, socket_transport->max_bytes_written_per_iteration);
          goto out;
      }
      message = _dbus_connection_get_message_to_send(transport->connection);
      _dbus_assert(message != NULL);
      dbus_message_lock(message);
  #if 0
      _dbus_verbose("writing message %p\n", message);
  #endif
      _dbus_message_get_network_data(message, &header, &body);
      header_len = _dbus_string_get_length(header);
      body_len = _dbus_string_get_length(body);
      if (_dbus_auth_needs_encoding(transport->auth)) {
          _dbus_assert(!DBUS_TRANSPORT_CAN_SEND_UNIX_FD(transport));
          if (_dbus_string_get_length(&socket_transport->encoded_outgoing) == 0) {
              if (!_dbus_auth_encode_data(transport->auth, header, &socket_transport->encoded_outgoing)) {
                  oom = TRUE;
                  goto out;
              }
              if (!_dbus_auth_encode_data(transport->auth, body, &socket_transport->encoded_outgoing)) {
                  _dbus_string_set_length(&socket_transport->encoded_outgoing, 0);
                  oom = TRUE;
                  goto out;
              }
          }
          total_bytes_to_write = _dbus_string_get_length(&socket_transport->encoded_outgoing);
      #if 0
          _dbus_verbose("encoded message is %d bytes\n", total_bytes_to_write);
      #endif
          bytes_written = _dbus_write_socket(socket_transport->fd, &socket_transport->encoded_outgoing, socket_transport->message_bytes_written,
                                        total_bytes_to_write - socket_transport->message_bytes_written);
          saved_errno = _dbus_save_socket_errno();
      } else {
          total_bytes_to_write = header_len + body_len;
      #if 0
          _dbus_verbose("message is %d bytes\n", total_bytes_to_write);
      #endif
      #ifdef HAVE_UNIX_FD_PASSING
          if (socket_transport->message_bytes_written <= 0 && DBUS_TRANSPORT_CAN_SEND_UNIX_FD(transport)) {
              const int *unix_fds;
              unsigned n;
              _dbus_message_get_unix_fds(message, &unix_fds, &n);
              bytes_written = _dbus_write_socket_with_unix_fds_two(socket_transport->fd, header, socket_transport->message_bytes_written,
                                                             header_len - socket_transport->message_bytes_written, body,0, body_len, unix_fds, n);
              saved_errno = _dbus_save_socket_errno();
              if (bytes_written > 0 && n > 0) _dbus_verbose("Wrote %i unix fds\n", n);
          } else
      #endif
          {
              if (socket_transport->message_bytes_written < header_len) {
                  bytes_written = _dbus_write_socket_two(socket_transport->fd, header, socket_transport->message_bytes_written,
                                                    header_len - socket_transport->message_bytes_written, body,0, body_len);
              } else {
                  bytes_written = _dbus_write_socket(socket_transport->fd, body,(socket_transport->message_bytes_written - header_len),
                                                body_len - (socket_transport->message_bytes_written - header_len));
              }
              saved_errno = _dbus_save_socket_errno();
          }
      }
      if (bytes_written < 0) {
          if (_dbus_get_is_errno_eagain_or_ewouldblock(saved_errno) || _dbus_get_is_errno_epipe(saved_errno)) goto out;
          else if (_dbus_get_is_errno_etoomanyrefs(saved_errno)) {
              _dbus_assert(socket_transport->message_bytes_written == 0);
              _dbus_verbose(" discard message of %d bytes due to ETOOMANYREFS\n", total_bytes_to_write);
              socket_transport->message_bytes_written = 0;
              _dbus_string_set_length(&socket_transport->encoded_outgoing, 0);
              _dbus_string_compact(&socket_transport->encoded_outgoing, 2048);
              _dbus_connection_message_sent_unlocked(transport->connection, message);
          } else {
              _dbus_verbose("Error writing to remote app: %s\n", _dbus_strerror(saved_errno));
              do_io_error(transport);
              goto out;
          }
      } else {
          _dbus_verbose(" wrote %d bytes of %d\n", bytes_written, total_bytes_to_write);
          total += bytes_written;
          socket_transport->message_bytes_written += bytes_written;
          _dbus_assert(socket_transport->message_bytes_written <= total_bytes_to_write);
          if (socket_transport->message_bytes_written == total_bytes_to_write) {
              socket_transport->message_bytes_written = 0;
              _dbus_string_set_length(&socket_transport->encoded_outgoing, 0);
              _dbus_string_compact(&socket_transport->encoded_outgoing, 2048);
              _dbus_connection_message_sent_unlocked(transport->connection, message);
          }
      }
  }
out:
  if (oom) return FALSE;
  else return TRUE;
}
static dbus_bool_t do_reading(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  DBusString *buffer;
  int bytes_read;
  int total;
  dbus_bool_t oom;
  int saved_errno;
  _dbus_verbose("fd = %" DBUS_SOCKET_FORMAT "\n", _dbus_socket_printable(socket_transport->fd));
  if (!_dbus_transport_try_to_authenticate(transport)) return TRUE;
  oom = FALSE;
  total = 0;
again:
  check_read_watch(transport);
  if (total > socket_transport->max_bytes_read_per_iteration) {
      _dbus_verbose("%d bytes exceeds %d bytes read per iteration, returning\n", total, socket_transport->max_bytes_read_per_iteration);
      goto out;
  }
  _dbus_assert(socket_transport->read_watch != NULL || transport->disconnected);
  if (transport->disconnected) goto out;
  if (!dbus_watch_get_enabled(socket_transport->read_watch)) return TRUE;
  if (_dbus_auth_needs_decoding(transport->auth)) {
      _dbus_assert(!DBUS_TRANSPORT_CAN_SEND_UNIX_FD(transport));
      if (_dbus_string_get_length(&socket_transport->encoded_incoming) > 0) bytes_read = _dbus_string_get_length(&socket_transport->encoded_incoming);
      else bytes_read = _dbus_read_socket(socket_transport->fd, &socket_transport->encoded_incoming, socket_transport->max_bytes_read_per_iteration);
      saved_errno = _dbus_save_socket_errno();
      _dbus_assert(_dbus_string_get_length(&socket_transport->encoded_incoming) == bytes_read);
      if (bytes_read > 0) {
          _dbus_message_loader_get_buffer(transport->loader, &buffer,NULL,NULL);
          if (!_dbus_auth_decode_data(transport->auth, &socket_transport->encoded_incoming, buffer)) {
              _dbus_verbose("Out of memory decoding incoming data\n");
              _dbus_message_loader_return_buffer(transport->loader, buffer);
              oom = TRUE;
              goto out;
          }
          _dbus_message_loader_return_buffer (transport->loader, buffer);
          _dbus_string_set_length (&socket_transport->encoded_incoming, 0);
          _dbus_string_compact (&socket_transport->encoded_incoming, 2048);
      }
  } else {
      int max_to_read = DBUS_MAXIMUM_MESSAGE_LENGTH;
      dbus_bool_t may_read_unix_fds = TRUE;
      _dbus_message_loader_get_buffer (transport->loader, &buffer, &max_to_read, &may_read_unix_fds);
      if (max_to_read > socket_transport->max_bytes_read_per_iteration) max_to_read = socket_transport->max_bytes_read_per_iteration;
  #ifdef HAVE_UNIX_FD_PASSING
      if (DBUS_TRANSPORT_CAN_SEND_UNIX_FD(transport) && may_read_unix_fds) {
          int *fds;
          unsigned int n_fds;
          if (!_dbus_message_loader_get_unix_fds(transport->loader, &fds, &n_fds)) {
              _dbus_verbose("Out of memory reading file descriptors\n");
              _dbus_message_loader_return_buffer(transport->loader, buffer);
              oom = TRUE;
              goto out;
          }
          bytes_read = _dbus_read_socket_with_unix_fds(socket_transport->fd, buffer, max_to_read, fds, &n_fds);
          saved_errno = _dbus_save_socket_errno();
          if (bytes_read >= 0 && n_fds > 0) _dbus_verbose("Read %i unix fds\n", n_fds);
          _dbus_message_loader_return_unix_fds(transport->loader, fds, bytes_read < 0 ? 0 : n_fds);
      } else
  #endif
      {
          bytes_read = _dbus_read_socket(socket_transport->fd, buffer, max_to_read);
          saved_errno = _dbus_save_socket_errno();
      }
      _dbus_message_loader_return_buffer(transport->loader, buffer);
  }
  if (bytes_read < 0) {
      if (_dbus_get_is_errno_enomem(saved_errno)) {
          _dbus_verbose("Out of memory in read()/do_reading()\n");
          oom = TRUE;
          goto out;
      } else if (_dbus_get_is_errno_eagain_or_ewouldblock(saved_errno)) goto out;
      else {
          _dbus_verbose("Error reading from remote app: %s\n", _dbus_strerror(saved_errno));
          do_io_error(transport);
          goto out;
      }
  } else if (bytes_read == 0) {
      _dbus_verbose("Disconnected from remote app\n");
      do_io_error(transport);
      goto out;
  } else {
      _dbus_verbose(" read %d bytes\n", bytes_read);
      total += bytes_read;
      if (!_dbus_transport_queue_messages(transport)) {
          oom = TRUE;
          _dbus_verbose(" out of memory when queueing messages we just read in the transport\n");
          goto out;
      }
      goto again;
  }
out:
  if (oom) return FALSE;
  else return TRUE;
}
static dbus_bool_t unix_error_with_read_to_come(DBusTransport *itransport, DBusWatch *watch, unsigned int flags) {
  DBusTransportSocket *transport = (DBusTransportSocket*)itransport;
  if (!(flags & DBUS_WATCH_HANGUP || flags & DBUS_WATCH_ERROR)) return FALSE;
  if (watch != transport->read_watch && _dbus_watch_get_enabled(transport->read_watch)) return FALSE;
  return TRUE; 
}
static dbus_bool_t socket_handle_watch(DBusTransport *transport, DBusWatch *watch, unsigned int flags) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  _dbus_assert(watch == socket_transport->read_watch || watch == socket_transport->write_watch);
  _dbus_assert(watch != NULL);
  if (!(flags & DBUS_WATCH_READABLE) && unix_error_with_read_to_come(transport, watch, flags)) {
      _dbus_verbose("Hang up or error on watch\n");
      _dbus_transport_disconnect(transport);
      return TRUE;
  }
  if (watch == socket_transport->read_watch && (flags & DBUS_WATCH_READABLE)) {
      dbus_bool_t auth_finished;
  #if 1
      _dbus_verbose("handling read watch %p flags = %x\n", watch, flags);
  #endif
      if (!do_authentication(transport, TRUE, FALSE, &auth_finished)) return FALSE;
      if (!auth_finished) {
          if (!do_reading(transport)) {
              _dbus_verbose("no memory to read\n");
              return FALSE;
          }
	  } else { _dbus_verbose("Not reading anything since we just completed the authentication\n"); }
  } else if (watch == socket_transport->write_watch && (flags & DBUS_WATCH_WRITABLE)) {
  #if 1
      _dbus_verbose("handling write watch, have_outgoing_messages = %d\n", _dbus_connection_has_messages_to_send_unlocked(transport->connection));
  #endif
      if (!do_authentication(transport, FALSE, TRUE, NULL)) return FALSE;
      if (!do_writing(transport)) {
          _dbus_verbose("no memory to write\n");
          return FALSE;
      }
      check_write_watch(transport);
  }
#ifdef DBUS_ENABLE_VERBOSE_MODE
  else {
      if (watch == socket_transport->read_watch) _dbus_verbose("asked to handle read watch with non-read condition 0x%x\n", flags);
      else if (watch == socket_transport->write_watch) _dbus_verbose("asked to handle write watch with non-write condition 0x%x\n", flags);
      else _dbus_verbose("asked to handle watch %p on fd %" DBUS_SOCKET_FORMAT " that we don't recognize\n", watch, dbus_watch_get_socket(watch));
  }
#endif
  return TRUE;
}
static void socket_disconnect(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  _dbus_verbose("\n");
  free_watches(transport);
  _dbus_close_socket(socket_transport->fd, NULL);
  _dbus_socket_invalidate(&socket_transport->fd);
}
static dbus_bool_t socket_connection_set(DBusTransport *transport) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  _dbus_watch_set_handler(socket_transport->write_watch, _dbus_connection_handle_watch, transport->connection, NULL);
  _dbus_watch_set_handler(socket_transport->read_watch, _dbus_connection_handle_watch, transport->connection, NULL);
  if (!_dbus_connection_add_watch_unlocked(transport->connection, socket_transport->write_watch)) return FALSE;
  if (!_dbus_connection_add_watch_unlocked(transport->connection, socket_transport->read_watch)) {
      _dbus_connection_remove_watch_unlocked(transport->connection, socket_transport->write_watch);
      return FALSE;
  }
  check_read_watch(transport);
  check_write_watch(transport);
  return TRUE;
}
static  void socket_do_iteration(DBusTransport *transport, unsigned int flags, int timeout_milliseconds) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  DBusPollFD poll_fd;
  int poll_res;
  int poll_timeout;
  _dbus_verbose(" iteration flags = %s%s timeout = %d read_watch = %p write_watch = %p fd = %" DBUS_SOCKET_FORMAT "\n", flags & DBUS_ITERATION_DO_READING ?
                "read" : "", flags & DBUS_ITERATION_DO_WRITING ? "write" : "", timeout_milliseconds, socket_transport->read_watch, socket_transport->write_watch,
                _dbus_socket_printable(socket_transport->fd));
  poll_fd.fd = _dbus_socket_get_pollable(socket_transport->fd);
  poll_fd.events = 0;
  if (_dbus_transport_try_to_authenticate(transport)) {
      if ((flags & DBUS_ITERATION_DO_WRITING) && !(flags & (DBUS_ITERATION_DO_READING | DBUS_ITERATION_BLOCK)) && !transport->disconnected &&
          _dbus_connection_has_messages_to_send_unlocked(transport->connection)) {
          do_writing(transport);
          if (transport->disconnected || !_dbus_connection_has_messages_to_send_unlocked(transport->connection)) goto out;
      }
      _dbus_assert(socket_transport->read_watch);
      if (flags & DBUS_ITERATION_DO_READING) poll_fd.events |= _DBUS_POLLIN;
      _dbus_assert(socket_transport->write_watch);
      if (flags & DBUS_ITERATION_DO_WRITING) poll_fd.events |= _DBUS_POLLOUT;
  } else {
      DBusAuthState auth_state;
      auth_state = _dbus_auth_do_work(transport->auth);
      if (transport->receive_credentials_pending || auth_state == DBUS_AUTH_STATE_WAITING_FOR_INPUT) poll_fd.events |= _DBUS_POLLIN;
      if (transport->send_credentials_pending || auth_state == DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND) poll_fd.events |= _DBUS_POLLOUT;
  }
  if (poll_fd.events) {
      int saved_errno;
      if (flags & DBUS_ITERATION_BLOCK) poll_timeout = timeout_milliseconds;
      else poll_timeout = 0;
      if (flags & DBUS_ITERATION_BLOCK) {
          _dbus_verbose("unlock pre poll\n");
          _dbus_connection_unlock(transport->connection);
      }
  again:
      poll_res = _dbus_poll(&poll_fd, 1, poll_timeout);
      saved_errno = _dbus_save_socket_errno();
      if (poll_res < 0 && _dbus_get_is_errno_eintr(saved_errno)) goto again;
      if (flags & DBUS_ITERATION_BLOCK) {
          _dbus_verbose("lock post poll\n");
          _dbus_connection_lock(transport->connection);
      }
      if (poll_res >= 0) {
          if (poll_res == 0) poll_fd.revents = 0;
          if (poll_fd.revents & _DBUS_POLLERR) do_io_error(transport);
          else {
              dbus_bool_t need_read = (poll_fd.revents & _DBUS_POLLIN) > 0;
              dbus_bool_t need_write = (poll_fd.revents & _DBUS_POLLOUT) > 0;
	          dbus_bool_t authentication_completed;
              _dbus_verbose("in iteration, need_read=%d need_write=%d\n", need_read, need_write);
              do_authentication(transport, need_read, need_write, &authentication_completed);
	          if (authentication_completed) goto out;
              if (need_read && (flags & DBUS_ITERATION_DO_READING)) do_reading(transport);
              if (need_write && (flags & DBUS_ITERATION_DO_WRITING)) do_writing(transport);
          }
      } else { _dbus_verbose("Error from _dbus_poll(): %s\n", _dbus_strerror(saved_errno)); }
  }
out:
  check_write_watch(transport);
  _dbus_verbose(" ... leaving do_iteration()\n");
}
static void socket_live_messages_changed(DBusTransport *transport) {
  check_read_watch(transport);
}
static dbus_bool_t socket_get_socket_fd(DBusTransport *transport, DBusSocket *fd_p) {
  DBusTransportSocket *socket_transport = (DBusTransportSocket*)transport;
  *fd_p = socket_transport->fd;
  return TRUE;
}
static const DBusTransportVTable socket_vtable = {
  socket_finalize,
  socket_handle_watch,
  socket_disconnect,
  socket_connection_set,
  socket_do_iteration,
  socket_live_messages_changed,
  socket_get_socket_fd
};
DBusTransport* _dbus_transport_new_for_socket(DBusSocket fd, const DBusString *server_guid, const DBusString *address) {
  DBusTransportSocket *socket_transport;
  DBusString invalid = _DBUS_STRING_INIT_INVALID;
  socket_transport = dbus_new0(DBusTransportSocket, 1);
  if (socket_transport == NULL) return NULL;
  socket_transport->encoded_outgoing = invalid;
  socket_transport->encoded_incoming = invalid;
  if (!_dbus_string_init(&socket_transport->encoded_outgoing)) goto failed;
  if (!_dbus_string_init(&socket_transport->encoded_incoming)) goto failed;
  socket_transport->write_watch = _dbus_watch_new(_dbus_socket_get_pollable(fd),DBUS_WATCH_WRITABLE,FALSE,NULL, NULL,NULL);
  if (socket_transport->write_watch == NULL) goto failed;
  socket_transport->read_watch = _dbus_watch_new(_dbus_socket_get_pollable(fd),DBUS_WATCH_READABLE,FALSE,NULL, NULL, NULL);
  if (socket_transport->read_watch == NULL) goto failed;
  if (!_dbus_transport_init_base(&socket_transport->base, &socket_vtable, server_guid, address)) goto failed;
#ifdef HAVE_UNIX_FD_PASSING
  _dbus_auth_set_unix_fd_possible(socket_transport->base.auth, _dbus_socket_can_pass_unix_fd(fd));
#endif
  socket_transport->fd = fd;
  socket_transport->message_bytes_written = 0;
  socket_transport->max_bytes_read_per_iteration = 2048;
  socket_transport->max_bytes_written_per_iteration = 2048;
  return (DBusTransport*)socket_transport;
failed:
  if (socket_transport->read_watch != NULL) {
      _dbus_watch_invalidate(socket_transport->read_watch);
      _dbus_watch_unref(socket_transport->read_watch);
  }
  if (socket_transport->write_watch != NULL) {
      _dbus_watch_invalidate(socket_transport->write_watch);
      _dbus_watch_unref(socket_transport->write_watch);
  }
  _dbus_string_free(&socket_transport->encoded_incoming);
  _dbus_string_free(&socket_transport->encoded_outgoing);
  dbus_free(socket_transport);
  return NULL;
}
DBusTransport* _dbus_transport_new_for_tcp_socket(const char *host, const char *port, const char *family, const char *noncefile, DBusError *error) {
  DBusSocket fd;
  DBusTransport *transport;
  DBusString address;
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  if (!_dbus_string_init(&address)) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      return NULL;
  }
  if (host == NULL) host = "localhost";
  if (!_dbus_string_append(&address, noncefile ? "nonce-tcp:" : "tcp:")) goto error;
  if (!_dbus_string_append(&address, "host=") || !_dbus_string_append (&address, host)) goto error;
  if (!_dbus_string_append(&address, ",port=") || !_dbus_string_append (&address, port)) goto error;
  if (family != NULL && (!_dbus_string_append(&address, ",family=") || !_dbus_string_append(&address, family))) goto error;
  if (noncefile != NULL && (!_dbus_string_append(&address, ",noncefile=") || !_dbus_string_append(&address, noncefile))) goto error;
  fd = _dbus_connect_tcp_socket_with_nonce(host, port, family, noncefile, error);
  if (!_dbus_socket_is_valid(fd)) {
      _DBUS_ASSERT_ERROR_IS_SET(error);
      _dbus_string_free(&address);
      return NULL;
  }
  _dbus_verbose("Successfully connected to tcp socket %s:%s\n", host, port);
  transport = _dbus_transport_new_for_socket(fd, NULL, &address);
  _dbus_string_free(&address);
  if (transport == NULL) {
      dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
      _dbus_close_socket(fd, NULL);
      _dbus_socket_invalidate(&fd);
  }
  return transport;
error:
  _dbus_string_free(&address);
  dbus_set_error(error, DBUS_ERROR_NO_MEMORY, NULL);
  return NULL;
}
DBusTransportOpenResult _dbus_transport_open_socket(DBusAddressEntry *entry, DBusTransport **transport_p, DBusError *error) {
  const char *method;
  dbus_bool_t isTcp;
  dbus_bool_t isNonceTcp;
  method = dbus_address_entry_get_method(entry);
  _dbus_assert(method != NULL);
  isTcp = strcmp(method, "tcp") == 0;
  isNonceTcp = strcmp(method, "nonce-tcp") == 0;
  if (isTcp || isNonceTcp) {
      const char *host = dbus_address_entry_get_value(entry, "host");
      const char *port = dbus_address_entry_get_value(entry, "port");
      const char *family = dbus_address_entry_get_value(entry, "family");
      const char *noncefile = dbus_address_entry_get_value(entry, "noncefile");
      if ((isNonceTcp == TRUE) != (noncefile != NULL)) {
          _dbus_set_bad_address(error, method, "noncefile", NULL);
          return DBUS_TRANSPORT_OPEN_BAD_ADDRESS;
      }
      if (port == NULL) {
          _dbus_set_bad_address(error, method, "port", NULL);
          return DBUS_TRANSPORT_OPEN_BAD_ADDRESS;
      }
      *transport_p = _dbus_transport_new_for_tcp_socket(host, port, family, noncefile, error);
      if (*transport_p == NULL) {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return DBUS_TRANSPORT_OPEN_DID_NOT_CONNECT;
      } else {
          _DBUS_ASSERT_ERROR_IS_CLEAR(error);
          return DBUS_TRANSPORT_OPEN_OK;
      }
  } else {
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return DBUS_TRANSPORT_OPEN_NOT_HANDLED;
  }
}