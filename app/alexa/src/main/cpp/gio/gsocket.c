#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#ifdef G_OS_WIN32
#include <fcntl.h>
#include <unistd.h>
#endif
#include <sys/uio.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gsocket.h"
#include "gcancellable.h"
#include "gioenumtypes.h"
#include "ginetaddress.h"
#include "ginitable.h"
#include "gioerror.h"
#include "gioenums.h"
#include "gioerror.h"
#include "gnetworkingprivate.h"
#include "gsocketaddress.h"
#include "gsocketcontrolmessage.h"
#include "gcredentials.h"

static void g_socket_initable_iface_init(GInitableIface *iface);
static gboolean g_socket_initable_init(GInitable *initable, GCancellable *cancellable, GError **error);
G_DEFINE_TYPE_WITH_CODE(GSocket, g_socket, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, g_socket_initable_iface_init));
enum {
  PROP_0,
  PROP_FAMILY,
  PROP_TYPE,
  PROP_PROTOCOL,
  PROP_FD,
  PROP_BLOCKING,
  PROP_LISTEN_BACKLOG,
  PROP_KEEPALIVE,
  PROP_LOCAL_ADDRESS,
  PROP_REMOTE_ADDRESS,
  PROP_TIMEOUT
};
struct _GSocketPrivate {
  GSocketFamily family;
  GSocketType type;
  GSocketProtocol protocol;
  gint fd;
  gint listen_backlog;
  guint timeout;
  GError *construct_error;
  GSocketAddress *remote_address;
  guint inited : 1;
  guint blocking : 1;
  guint keepalive : 1;
  guint closed : 1;
  guint connected : 1;
  guint listening : 1;
  guint timed_out : 1;
  guint connect_pending : 1;
#ifdef G_OS_WIN32
  //WSAEVENT event;
  int current_events;
  int current_errors;
  int selected_events;
  GList *requested_conditions;
#endif
};
static int get_socket_errno(void) {
#ifdef G_OS_WIN32
  return errno;
#else
  return WSAGetLastError();
#endif
}
static GIOErrorEnum socket_io_error_from_errno(int err) {
#ifdef G_OS_WIN32
  return g_io_error_from_errno(err);
#else
  switch(err) {
      case WSAEADDRINUSE: return G_IO_ERROR_ADDRESS_IN_USE;
      case WSAEWOULDBLOCK: return G_IO_ERROR_WOULD_BLOCK;
      case WSAEACCES: return G_IO_ERROR_PERMISSION_DENIED;
      case WSA_INVALID_HANDLE: case WSA_INVALID_PARAMETER: case WSAEBADF: case WSAENOTSOCK: return G_IO_ERROR_INVALID_ARGUMENT;
      case WSAEPROTONOSUPPORT: return G_IO_ERROR_NOT_SUPPORTED;
      case WSAECANCELLED: return G_IO_ERROR_CANCELLED;
      case WSAESOCKTNOSUPPORT: case WSAEOPNOTSUPP: case WSAEPFNOSUPPORT: case WSAEAFNOSUPPORT: return G_IO_ERROR_NOT_SUPPORTED;
      default: return G_IO_ERROR_FAILED;
  }
#endif
}
static const char *socket_strerror(int err) {
#ifdef G_OS_WIN32
  return g_strerror(err);
#else
  static GStaticPrivate last_msg = G_STATIC_PRIVATE_INIT;
  char *msg;
  msg = g_win32_error_message(err);
  g_static_private_set(&last_msg, msg, g_free);
  return msg;
#endif
}
#ifndef G_OS_WIN32
#define win32_unset_event_mask(_socket, _mask) _win32_unset_event_mask(_socket, _mask)
static void _win32_unset_event_mask(GSocket *socket, int mask) {
  socket->priv->current_events &= ~mask;
  socket->priv->current_errors &= ~mask;
}
#else
#define win32_unset_event_mask(_socket, _mask)
#endif
static void set_fd_nonblocking(int fd) {
#ifndef G_OS_WIN32
  glong arg;
#else
  gulong arg;
#endif
#ifdef G_OS_WIN32
  if ((arg = fcntl(fd, F_GETFL, NULL)) < 0) {
      g_warning("Error getting socket status flags: %s", socket_strerror(errno));
      arg = 0;
  }
  arg = arg | O_NONBLOCK;
  if (fcntl(fd, F_SETFL, arg) < 0) g_warning("Error setting socket status flags: %s", socket_strerror(errno));
#else
  arg = TRUE;
  if (ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR) {
      int errsv = get_socket_errno();
      g_warning("Error setting socket status flags: %s", socket_strerror(errsv));
  }
#endif
}
static gboolean check_socket(GSocket *socket, GError **error) {
  if (!socket->priv->inited) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, _("Invalid socket, not initialized"));
      return FALSE;
  }
  if (socket->priv->construct_error) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, _("Invalid socket, initialization failed due to: %s"),
		          socket->priv->construct_error->message);
      return FALSE;
  }
  if (socket->priv->closed) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_CLOSED, _("Socket is already closed"));
      return FALSE;
  }
  if (socket->priv->timed_out) {
      socket->priv->timed_out = FALSE;
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT, _("Socket I/O timed out"));
      return FALSE;
  }
  return TRUE;
}
static void g_socket_details_from_fd(GSocket *socket) {
  struct sockaddr_storage address;
  gint fd;
  guint addrlen;
  guint optlen;
  int value;
  int errsv;
#ifndef G_OS_WIN32
  BOOL bool_val = FALSE;
#else
  int bool_val;
#endif
  fd = socket->priv->fd;
  optlen = sizeof value;
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, (void*)&value, &optlen) != 0) {
      errsv = get_socket_errno();
      switch(errsv) {
      #ifdef ENOTSOCK
          case ENOTSOCK:
      #endif
      #ifdef WSAENOTSOCK
          case WSAENOTSOCK:
      #endif
         case EBADF: g_error("creating GSocket from fd %d: %s\n", fd, socket_strerror(errsv));
	  }
      goto err;
  }
  g_assert(optlen == sizeof value);
  switch(value) {
      case SOCK_STREAM: socket->priv->type = G_SOCKET_TYPE_STREAM; break;
      case SOCK_DGRAM: socket->priv->type = G_SOCKET_TYPE_DATAGRAM; break;
      case SOCK_SEQPACKET: socket->priv->type = G_SOCKET_TYPE_SEQPACKET; break;
      default: socket->priv->type = G_SOCKET_TYPE_INVALID;
  }
  addrlen = sizeof address;
  if (getsockname(fd, (struct sockaddr*)&address, &addrlen) != 0) {
      errsv = get_socket_errno();
      goto err;
  }
  g_assert(G_STRUCT_OFFSET(struct sockaddr, sa_family) + sizeof address.ss_family <= addrlen);
  switch(address.ss_family) {
      case G_SOCKET_FAMILY_IPV4: case G_SOCKET_FAMILY_IPV6:
          socket->priv->family = address.ss_family;
          switch(socket->priv->type) {
              case G_SOCKET_TYPE_STREAM: socket->priv->protocol = G_SOCKET_PROTOCOL_TCP; break;
              case G_SOCKET_TYPE_DATAGRAM: socket->priv->protocol = G_SOCKET_PROTOCOL_UDP; break;
              case G_SOCKET_TYPE_SEQPACKET: socket->priv->protocol = G_SOCKET_PROTOCOL_SCTP; break;
          }
          break;
     case G_SOCKET_FAMILY_UNIX:
         socket->priv->family = G_SOCKET_FAMILY_UNIX;
         socket->priv->protocol = G_SOCKET_PROTOCOL_DEFAULT;
         break;
     default: socket->priv->family = G_SOCKET_FAMILY_INVALID;
  }
  if (socket->priv->family != G_SOCKET_FAMILY_INVALID) {
      addrlen = sizeof address;
      if (getpeername(fd, (struct sockaddr*)&address, &addrlen) >= 0) socket->priv->connected = TRUE;
  }
  optlen = sizeof bool_val;
  if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&bool_val, &optlen) == 0) {
  #ifndef G_OS_WIN32
      g_assert(optlen == sizeof bool_val);
  #endif
      socket->priv->keepalive = !!bool_val;
  } else socket->priv->keepalive = FALSE;
  return;
err:
  g_set_error(&socket->priv->construct_error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("creating GSocket from fd: %s"), socket_strerror(errsv));
}
static gint g_socket_create_socket(GSocketFamily family, GSocketType type, int protocol, GError **error) {
  gint native_type;
  gint fd;
  switch(type) {
     case G_SOCKET_TYPE_STREAM: native_type = SOCK_STREAM; break;
     case G_SOCKET_TYPE_DATAGRAM: native_type = SOCK_DGRAM; break;
     case G_SOCKET_TYPE_SEQPACKET: native_type = SOCK_SEQPACKET; break;
     default: g_assert_not_reached();
  }
  if (protocol == -1) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Unable to create socket: %s"), _("Unknown protocol was specified"));
      return -1;
  }
#ifdef SOCK_CLOEXEC
  fd = socket(family, native_type | SOCK_CLOEXEC, protocol);
  if (fd < 0 && errno == EINVAL)
#endif
  fd = socket(family, native_type, protocol);
  if (fd < 0) {
      int errsv = get_socket_errno();
      g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Unable to create socket: %s"), socket_strerror(errsv));
  }
#ifndef G_OS_WIN32
  {
      int flags;
      flags = fcntl(fd, F_GETFD, 0);
      if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
          flags |= FD_CLOEXEC;
          fcntl(fd, F_SETFD, flags);
      }
  }
#endif
  return fd;
}
static void g_socket_constructed(GObject *object) {
  GSocket *socket = G_SOCKET(object);
  if (socket->priv->fd >= 0) g_socket_details_from_fd(socket);
  else socket->priv->fd = g_socket_create_socket(socket->priv->family, socket->priv->type, socket->priv->protocol, &socket->priv->construct_error);
  if (socket->priv->fd != -1) set_fd_nonblocking(socket->priv->fd);
}
static void g_socket_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GSocket *socket = G_SOCKET(object);
  GSocketAddress *address;
  switch(prop_id) {
      case PROP_FAMILY: g_value_set_enum(value, socket->priv->family); break;
      case PROP_TYPE: g_value_set_enum(value, socket->priv->type); break;
      case PROP_PROTOCOL: g_value_set_enum(value, socket->priv->protocol); break;
      case PROP_FD: g_value_set_int(value, socket->priv->fd); break;
      case PROP_BLOCKING: g_value_set_boolean(value, socket->priv->blocking); break;
      case PROP_LISTEN_BACKLOG: g_value_set_int(value, socket->priv->listen_backlog); break;
      case PROP_KEEPALIVE: g_value_set_boolean(value, socket->priv->keepalive); break;
      case PROP_LOCAL_ADDRESS:
          address = g_socket_get_local_address(socket, NULL);
          g_value_take_object(value, address);
          break;
      case PROP_REMOTE_ADDRESS:
          address = g_socket_get_remote_address(socket, NULL);
          g_value_take_object(value, address);
          break;
      case PROP_TIMEOUT: g_value_set_uint(value, socket->priv->timeout); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GSocket *socket = G_SOCKET(object);
  switch(prop_id) {
      case PROP_FAMILY: socket->priv->family = g_value_get_enum(value); break;
      case PROP_TYPE: socket->priv->type = g_value_get_enum(value); break;
      case PROP_PROTOCOL: socket->priv->protocol = g_value_get_enum(value); break;
      case PROP_FD: socket->priv->fd = g_value_get_int(value); break;
      case PROP_BLOCKING: g_socket_set_blocking(socket, g_value_get_boolean(value)); break;
      case PROP_LISTEN_BACKLOG: g_socket_set_listen_backlog(socket, g_value_get_int(value)); break;
      case PROP_KEEPALIVE: g_socket_set_keepalive(socket, g_value_get_boolean(value)); break;
      case PROP_TIMEOUT: g_socket_set_timeout(socket, g_value_get_uint(value)); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_socket_finalize(GObject *object) {
  GSocket *socket = G_SOCKET(object);
  g_clear_error(&socket->priv->construct_error);
  if (socket->priv->fd != -1 && !socket->priv->closed) g_socket_close(socket, NULL);
  if (socket->priv->remote_address) g_object_unref(socket->priv->remote_address);
#ifndef G_OS_WIN32
  if (socket->priv->event != WSA_INVALID_EVENT) {
      WSACloseEvent(socket->priv->event);
      socket->priv->event = WSA_INVALID_EVENT;
  }
  g_assert(socket->priv->requested_conditions == NULL);
#endif
  if (G_OBJECT_CLASS(g_socket_parent_class)->finalize) (*G_OBJECT_CLASS(g_socket_parent_class)->finalize)(object);
}
static void g_socket_class_init(GSocketClass *klass) {
  GObjectClass *gobject_class G_GNUC_UNUSED = G_OBJECT_CLASS(klass);
  volatile GType type;
  type = g_inet_address_get_type();
#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif
  g_type_class_add_private(klass, sizeof(GSocketPrivate));
  gobject_class->finalize = g_socket_finalize;
  gobject_class->constructed = g_socket_constructed;
  gobject_class->set_property = g_socket_set_property;
  gobject_class->get_property = g_socket_get_property;
  g_object_class_install_property(gobject_class, PROP_FAMILY, g_param_spec_enum("family", P_("Socket family"), P_("The sockets address family"),
						          G_TYPE_SOCKET_FAMILY, G_SOCKET_FAMILY_INVALID, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TYPE, g_param_spec_enum("type", P_("Socket type"), P_("The sockets type"),
						          G_TYPE_SOCKET_TYPE, G_SOCKET_TYPE_STREAM, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_PROTOCOL, g_param_spec_enum("protocol", P_("Socket protocol"), P_("The id of the protocol "
                                  "to use, or -1 for unknown"), G_TYPE_SOCKET_PROTOCOL, G_SOCKET_PROTOCOL_UNKNOWN, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_FD,g_param_spec_int("fd", P_("File descriptor"), P_("The sockets file"
                                  " descriptor"),G_MININT, G_MAXINT,-1,G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_BLOCKING,g_param_spec_boolean("blocking", P_("blocking"), P_("Whether"
                                  " or not I/O on this socket is blocking"), TRUE,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_LISTEN_BACKLOG,g_param_spec_int("listen-backlog", P_("Listen backlog"),
                                  P_("Outstanding connections in the listen queue"),0, SOMAXCONN,10,G_PARAM_READWRITE |
                                  G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_KEEPALIVE,g_param_spec_boolean("keepalive", P_("Keep connection alive"),
							      P_("Keep connection alive by sending periodic pings"),FALSE,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_LOCAL_ADDRESS, g_param_spec_object("local-address", P_("Local address"), P_("The local address"
                                  " the socket is bound to"), G_TYPE_SOCKET_ADDRESS, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_REMOTE_ADDRESS, g_param_spec_object("remote-address", P_("Remote address"), P_("The remote "
                                  "address the socket is connected to"), G_TYPE_SOCKET_ADDRESS, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_TIMEOUT,g_param_spec_uint("timeout", P_("Timeout"), P_("The timeout "
                                  "in seconds on socket I/O"),0,G_MAXUINT,0,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
static void g_socket_initable_iface_init(GInitableIface *iface) {
  iface->init = g_socket_initable_init;
}
static void g_socket_init(GSocket *socket) {
  socket->priv = G_TYPE_INSTANCE_GET_PRIVATE(socket, G_TYPE_SOCKET, GSocketPrivate);
  socket->priv->fd = -1;
  socket->priv->blocking = TRUE;
  socket->priv->listen_backlog = 10;
  socket->priv->construct_error = NULL;
#ifndef G_OS_WIN32
  socket->priv->event = WSA_INVALID_EVENT;
#endif
}
static gboolean g_socket_initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  GSocket *socket;
  g_return_val_if_fail(G_IS_SOCKET(initable), FALSE);
  socket = G_SOCKET(initable);
  if (cancellable != NULL) {
      g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("Cancellable initialization not supported"));
      return FALSE;
  }
  socket->priv->inited = TRUE;
  if (socket->priv->construct_error) {
      if (error) *error = g_error_copy(socket->priv->construct_error);
      return FALSE;
  }
  return TRUE;
}
GSocket *g_socket_new(GSocketFamily family, GSocketType type, GSocketProtocol protocol, GError **error) {
  return G_SOCKET(g_initable_new(G_TYPE_SOCKET, NULL, error, "family", family, "type", type, "protocol", protocol, NULL));
}
GSocket *g_socket_new_from_fd(gint fd, GError **error) {
  return G_SOCKET(g_initable_new(G_TYPE_SOCKET, NULL, error, "fd", fd, NULL));
}
void g_socket_set_blocking(GSocket *socket, gboolean blocking) {
  g_return_if_fail(G_IS_SOCKET(socket));
  blocking = !!blocking;
  if (socket->priv->blocking == blocking) return;
  socket->priv->blocking = blocking;
  g_object_notify(G_OBJECT(socket), "blocking");
}
gboolean g_socket_get_blocking(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), FALSE);
  return socket->priv->blocking;
}
void g_socket_set_keepalive(GSocket *socket, gboolean keepalive) {
  int value;
  g_return_if_fail(G_IS_SOCKET(socket));
  keepalive = !!keepalive;
  if (socket->priv->keepalive == keepalive) return;
  value = (gint) keepalive;
  if (setsockopt(socket->priv->fd, SOL_SOCKET, SO_KEEPALIVE, (gpointer)&value, sizeof(value)) < 0) {
      int errsv = get_socket_errno();
      g_warning("error setting keepalive: %s", socket_strerror(errsv));
      return;
  }
  socket->priv->keepalive = keepalive;
  g_object_notify(G_OBJECT(socket), "keepalive");
}
gboolean g_socket_get_keepalive(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), FALSE);
  return socket->priv->keepalive;
}
gint g_socket_get_listen_backlog(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), 0);
  return socket->priv->listen_backlog;
}
void g_socket_set_listen_backlog(GSocket *socket, gint backlog) {
  g_return_if_fail(G_IS_SOCKET(socket));
  g_return_if_fail(!socket->priv->listening);
  if (backlog != socket->priv->listen_backlog) {
      socket->priv->listen_backlog = backlog;
      g_object_notify(G_OBJECT(socket), "listen-backlog");
  }
}
guint g_socket_get_timeout(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), 0);
  return socket->priv->timeout;
}
void g_socket_set_timeout(GSocket *socket, guint timeout) {
  g_return_if_fail(G_IS_SOCKET(socket));
  if (timeout != socket->priv->timeout) {
      socket->priv->timeout = timeout;
      g_object_notify(G_OBJECT(socket), "timeout");
  }
}
GSocketFamily g_socket_get_family(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), G_SOCKET_FAMILY_INVALID);
  return socket->priv->family;
}
GSocketType g_socket_get_socket_type(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), G_SOCKET_TYPE_INVALID);
  return socket->priv->type;
}
GSocketProtocol g_socket_get_protocol(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), -1);
  return socket->priv->protocol;
}
int g_socket_get_fd(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), -1);
  return socket->priv->fd;
}
GSocketAddress *g_socket_get_local_address(GSocket *socket, GError **error) {
  struct sockaddr_storage buffer;
  guint32 len = sizeof(buffer);
  g_return_val_if_fail(G_IS_SOCKET(socket), NULL);
  if (getsockname(socket->priv->fd, (struct sockaddr*)&buffer, &len) < 0) {
      int errsv = get_socket_errno();
      g_set_error (error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("could not get local address: %s"), socket_strerror(errsv));
      return NULL;
  }
  return g_socket_address_new_from_native(&buffer, len);
}
GSocketAddress *g_socket_get_remote_address(GSocket *socket, GError **error) {
  struct sockaddr_storage buffer;
  guint32 len = sizeof(buffer);
  g_return_val_if_fail(G_IS_SOCKET(socket), NULL);
  if (socket->priv->connect_pending) {
      if (!g_socket_check_connect_result(socket, error)) return NULL;
      else socket->priv->connect_pending = FALSE;
  }
  if (!socket->priv->remote_address) {
      if (getpeername(socket->priv->fd, (struct sockaddr*)&buffer, &len) < 0) {
          int errsv = get_socket_errno();
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("could not get remote address: %s"), socket_strerror(errsv));
          return NULL;
	  }
      socket->priv->remote_address = g_socket_address_new_from_native(&buffer, len);
  }
  return g_object_ref(socket->priv->remote_address);
}
gboolean g_socket_is_connected(GSocket *socket) {
  g_return_val_if_fail(G_IS_SOCKET(socket), FALSE);
  return socket->priv->connected;
}
gboolean g_socket_listen(GSocket *socket, GError **error) {
  g_return_val_if_fail(G_IS_SOCKET(socket), FALSE);
  if (!check_socket(socket, error)) return FALSE;
  if (listen(socket->priv->fd, socket->priv->listen_backlog) < 0) {
      int errsv = get_socket_errno();
      g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("could not listen: %s"), socket_strerror(errsv));
      return FALSE;
  }
  socket->priv->listening = TRUE;
  return TRUE;
}
gboolean g_socket_bind(GSocket *socket, GSocketAddress *address, gboolean reuse_address, GError **error) {
  struct sockaddr_storage addr;
  g_return_val_if_fail(G_IS_SOCKET(socket) && G_IS_SOCKET_ADDRESS(address), FALSE);
  if (!check_socket(socket, error)) return FALSE;
#ifndef G_OS_WIN32
  {
      int value;
      value = (int)!!reuse_address;
      setsockopt(socket->priv->fd, SOL_SOCKET, SO_REUSEADDR, (gpointer)&value, sizeof(value));
  }
#endif
  if (!g_socket_address_to_native(address, &addr, sizeof addr, error)) return FALSE;
  if (bind (socket->priv->fd, (struct sockaddr*)&addr, g_socket_address_get_native_size(address)) < 0) {
      int errsv = get_socket_errno();
      g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error binding to address: %s"), socket_strerror(errsv));
      return FALSE;
  }
  return TRUE;
}
gboolean g_socket_speaks_ipv4(GSocket *socket) {
  switch(socket->priv->family) {
      case G_SOCKET_FAMILY_IPV4: return TRUE;
      case G_SOCKET_FAMILY_IPV6:
      #if defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
          {
              guint sizeof_int = sizeof(int);
              gint v6_only;
              if (getsockopt(socket->priv->fd,IPPROTO_IPV6, IPV6_V6ONLY, &v6_only, &sizeof_int) != 0)
              return FALSE;
              return !v6_only;
          }
      #else
          return FALSE;
  #endif
      default: return FALSE;
  }
}
GSocket *g_socket_accept(GSocket *socket, GCancellable *cancellable, GError **error) {
  GSocket *new_socket;
  gint ret;
  g_return_val_if_fail(G_IS_SOCKET(socket), NULL);
  if (!check_socket(socket, error)) return NULL;
  while(TRUE) {
      if (socket->priv->blocking && !g_socket_condition_wait(socket,G_IO_IN, cancellable, error)) return NULL;
      if ((ret = accept(socket->priv->fd, NULL, 0)) < 0) {
          int errsv = get_socket_errno ();
          win32_unset_event_mask(socket, FD_ACCEPT);
          if (errsv == EINTR) continue;
          if (socket->priv->blocking) {
          #ifdef WSAEWOULDBLOCK
              if (errsv == WSAEWOULDBLOCK) continue;
          #else
              if (errsv == EWOULDBLOCK || errsv == EAGAIN) continue;
          #endif
          }
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error accepting connection: %s"), socket_strerror(errsv));
          return NULL;
	  }
      break;
  }
  win32_unset_event_mask(socket, FD_ACCEPT);
#ifndef G_OS_WIN32
  {
      WSAEventSelect(ret, NULL, 0);
  }
#else
  {
      int flags;
      flags = fcntl(ret, F_GETFD, 0);
      if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
          flags |= FD_CLOEXEC;
          fcntl(ret, F_SETFD, flags);
      }
  }
#endif
  new_socket = g_socket_new_from_fd(ret, error);
  if (new_socket == NULL) {
  #ifndef G_OS_WIN32
      closesocket(ret);
  #else
      close(ret);
  #endif
  } else new_socket->priv->protocol = socket->priv->protocol;
  return new_socket;
}
gboolean g_socket_connect(GSocket *socket, GSocketAddress *address, GCancellable *cancellable, GError **error) {
  struct sockaddr_storage buffer;
  g_return_val_if_fail(G_IS_SOCKET(socket) && G_IS_SOCKET_ADDRESS(address), FALSE);
  if (!check_socket(socket, error)) return FALSE;
  if (!g_socket_address_to_native(address, &buffer, sizeof buffer, error)) return FALSE;
  if (socket->priv->remote_address) g_object_unref(socket->priv->remote_address);
  socket->priv->remote_address = g_object_ref(address);
  while(1) {
      if (connect(socket->priv->fd, (struct sockaddr*)&buffer, g_socket_address_get_native_size (address)) < 0) {
          int errsv = get_socket_errno ();
          if (errsv == EINTR) continue;
      #ifdef G_OS_WIN32
          if (errsv == EINPROGRESS)
      #else
          if (errsv == WSAEWOULDBLOCK)
      #endif
          {
              if (socket->priv->blocking) {
                  if (g_socket_condition_wait(socket, G_IO_OUT, cancellable, error)) {
                      if (g_socket_check_connect_result(socket, error)) break;
                  }
                  g_prefix_error(error, _("Error connecting: "));
              } else {
                  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_PENDING, _("Connection in progress"));
                  socket->priv->connect_pending = TRUE;
              }
          } else g_set_error(error, G_IO_ERROR, socket_io_error_from_errno (errsv), _("Error connecting: %s"), socket_strerror(errsv));
          return FALSE;
	  }
      break;
  }
  win32_unset_event_mask(socket, FD_CONNECT);
  socket->priv->connected = TRUE;
  return TRUE;
}
gboolean g_socket_check_connect_result(GSocket *socket, GError **error) {
  guint optlen;
  int value;
  if (!check_socket(socket, error)) return FALSE;
  optlen = sizeof(value);
  if (getsockopt(socket->priv->fd, SOL_SOCKET, SO_ERROR, (void*)&value, &optlen) != 0) {
      int errsv = get_socket_errno();
      g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Unable to get pending error: %s"), socket_strerror(errsv));
      return FALSE;
  }
  if (value != 0) {
      g_set_error_literal(error, G_IO_ERROR, socket_io_error_from_errno(value), socket_strerror(value));
      if (socket->priv->remote_address) {
          g_object_unref(socket->priv->remote_address);
          socket->priv->remote_address = NULL;
      }
      return FALSE;
  }
  return TRUE;
}
gssize g_socket_receive(GSocket *socket, gchar *buffer, gsize size, GCancellable *cancellable, GError **error) {
  return g_socket_receive_with_blocking(socket, buffer, size, socket->priv->blocking, cancellable, error);
}
gssize g_socket_receive_with_blocking(GSocket *socket, gchar *buffer, gsize size, gboolean blocking, GCancellable *cancellable, GError **error) {
  gssize ret;
  g_return_val_if_fail(G_IS_SOCKET(socket) && buffer != NULL, FALSE);
  if (!check_socket(socket, error)) return -1;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  while (1) {
      if (blocking && !g_socket_condition_wait(socket,G_IO_IN, cancellable, error)) return -1;
      if ((ret = recv(socket->priv->fd, buffer, size, 0)) < 0) {
          int errsv = get_socket_errno();
          if (errsv == EINTR) continue;
          if (blocking) {
          #ifdef WSAEWOULDBLOCK
              if (errsv == WSAEWOULDBLOCK) continue;
          #else
              if (errsv == EWOULDBLOCK || errsv == EAGAIN) continue;
          #endif
          }
          win32_unset_event_mask(socket, FD_READ);
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno (errsv), _("Error receiving data: %s"), socket_strerror(errsv));
          return -1;
	  }
      win32_unset_event_mask(socket, FD_READ);
      break;
  }
  return ret;
}
gssize g_socket_receive_from(GSocket *socket, GSocketAddress **address, gchar *buffer, gsize size, GCancellable *cancellable, GError **error) {
  GInputVector v;
  v.buffer = buffer;
  v.size = size;
  return g_socket_receive_message(socket, address, &v,1,NULL,0,NULL, cancellable, error);
}
#ifdef MSG_NOSIGNAL
#define G_SOCKET_DEFAULT_SEND_FLAGS MSG_NOSIGNAL
#else
#define G_SOCKET_DEFAULT_SEND_FLAGS 0
#endif
gssize g_socket_send(GSocket *socket, const gchar *buffer, gsize size, GCancellable *cancellable, GError **error) {
  return g_socket_send_with_blocking(socket, buffer, size, socket->priv->blocking, cancellable, error);
}
gssize g_socket_send_with_blocking(GSocket *socket, const gchar *buffer, gsize size, gboolean blocking, GCancellable *cancellable, GError **error) {
  gssize ret;
  g_return_val_if_fail(G_IS_SOCKET(socket) && buffer != NULL, FALSE);
  if (!check_socket(socket, error)) return -1;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  while(1) {
      if (blocking && !g_socket_condition_wait(socket,G_IO_OUT, cancellable, error)) return -1;
      if ((ret = send(socket->priv->fd, buffer, size, G_SOCKET_DEFAULT_SEND_FLAGS)) < 0) {
          int errsv = get_socket_errno();
          if (errsv == EINTR) continue;
      #ifdef WSAEWOULDBLOCK
          if (errsv == WSAEWOULDBLOCK) win32_unset_event_mask(socket, FD_WRITE);
      #endif
          if (blocking) {
          #ifdef WSAEWOULDBLOCK
              if (errsv == WSAEWOULDBLOCK) continue;
          #else
              if (errsv == EWOULDBLOCK || errsv == EAGAIN) continue;
          #endif
          }
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error sending data: %s"), socket_strerror(errsv));
          return -1;
	  }
      break;
  }
  return ret;
}
gssize g_socket_send_to(GSocket *socket, GSocketAddress *address, const gchar *buffer, gsize size, GCancellable *cancellable, GError **error) {
  GOutputVector v;
  v.buffer = buffer;
  v.size = size;
  return g_socket_send_message(socket, address, &v,1,NULL,0,0, cancellable, error);
}
gboolean g_socket_shutdown(GSocket *socket, gboolean shutdown_read, gboolean shutdown_write, GError **error) {
  int how;
  g_return_val_if_fail(G_IS_SOCKET(socket), TRUE);
  if (!check_socket(socket, NULL)) return FALSE;
  if (!shutdown_read && !shutdown_write) return TRUE;
#ifdef G_OS_WIN32
  if (shutdown_read && shutdown_write) how = SHUT_RDWR;
  else if (shutdown_read) how = SHUT_RD;
  else how = SHUT_WR;
#else
  if (shutdown_read && shutdown_write) how = SD_BOTH;
  else if (shutdown_read) how = SD_RECEIVE;
  else how = SD_SEND;
#endif
  if (shutdown(socket->priv->fd, how) != 0) {
      int errsv = get_socket_errno();
      g_set_error (error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Unable to create socket: %s"), socket_strerror(errsv));
      return FALSE;
  }
  if (shutdown_read && shutdown_write) socket->priv->connected = FALSE;
  return TRUE;
}
gboolean g_socket_close(GSocket *socket, GError **error) {
  int res;
  g_return_val_if_fail(G_IS_SOCKET(socket), TRUE);
  if (socket->priv->closed) return TRUE;
  if (!check_socket(socket, NULL)) return FALSE;
  while (1) {
  #ifndef G_OS_WIN32
      res = closesocket(socket->priv->fd);
  #else
      res = close(socket->priv->fd);
  #endif
      if (res == -1) {
          int errsv = get_socket_errno();
          if (errsv == EINTR) continue;
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno (errsv), _("Error closing socket: %s"), socket_strerror(errsv));
          return FALSE;
	  }
      break;
  }
  socket->priv->connected = FALSE;
  socket->priv->closed = TRUE;
  if (socket->priv->remote_address) {
      g_object_unref(socket->priv->remote_address);
      socket->priv->remote_address = NULL;
  }
  return TRUE;
}
gboolean g_socket_is_closed(GSocket *socket) {
  return socket->priv->closed;
}
#ifdef G_OS_WIN32
static gboolean broken_prepare(GSource *source, gint *timeout) {
  return FALSE;
}
static gboolean broken_check(GSource *source) {
  return FALSE;
}
static gboolean broken_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  return TRUE;
}
static GSourceFuncs broken_funcs = {
  broken_prepare,
  broken_check,
  broken_dispatch,
  NULL
};
static gint network_events_for_condition(GIOCondition condition) {
  int event_mask = 0;
  if (condition & G_IO_IN) event_mask |= (FD_READ | FD_ACCEPT);
  if (condition & G_IO_OUT) event_mask |= (FD_WRITE | FD_CONNECT);
  event_mask |= FD_CLOSE;
  return event_mask;
}
static void ensure_event(GSocket *socket) {
  //if (socket->priv->event == WSA_INVALID_EVENT) socket->priv->event = WSACreateEvent();
}
static void update_select_events(GSocket *socket) {
  /*int event_mask;
  GIOCondition *ptr;
  GList *l;
  WSAEVENT event;
  ensure_event (socket);
  event_mask = 0;
  for (l = socket->priv->requested_conditions; l != NULL; l = l->next) {
      ptr = l->data;
      event_mask |= network_events_for_condition (*ptr);
  }
  if (event_mask != socket->priv->selected_events) {
      if (event_mask == 0) event = NULL;
      else event = socket->priv->event;
      if (WSAEventSelect(socket->priv->fd, event, event_mask) == 0) socket->priv->selected_events = event_mask;
  }*/
}
static void add_condition_watch(GSocket *socket, GIOCondition *condition) {
  g_assert(g_list_find (socket->priv->requested_conditions, condition) == NULL);
  socket->priv->requested_conditions = g_list_prepend(socket->priv->requested_conditions, condition);
  update_select_events(socket);
}
static void remove_condition_watch(GSocket *socket, GIOCondition *condition) {
  g_assert (g_list_find(socket->priv->requested_conditions, condition) != NULL);
  socket->priv->requested_conditions = g_list_remove(socket->priv->requested_conditions, condition);
  update_select_events(socket);
}
static GIOCondition update_condition(GSocket *socket) {
  /*WSANETWORKEVENTS events;
  GIOCondition condition;
  if (WSAEnumNetworkEvents(socket->priv->fd, socket->priv->event, &events) == 0) {
      socket->priv->current_events |= events.lNetworkEvents;
      if (events.lNetworkEvents & FD_WRITE && events.iErrorCode[FD_WRITE_BIT] != 0) socket->priv->current_errors |= FD_WRITE;
      if (events.lNetworkEvents & FD_CONNECT && events.iErrorCode[FD_CONNECT_BIT] != 0) socket->priv->current_errors |= FD_CONNECT;
  }
  condition = 0;
  if (socket->priv->current_events & (FD_READ | FD_ACCEPT)) condition |= G_IO_IN;
  if (socket->priv->current_events & FD_CLOSE || socket->priv->closed) condition |= G_IO_HUP;
  if ((condition & G_IO_HUP) == 0 && socket->priv->current_events & FD_WRITE) {
      if (socket->priv->current_errors & FD_WRITE) condition |= G_IO_ERR;
      else condition |= G_IO_OUT;
  } else {
      if (socket->priv->current_events & FD_CONNECT) {
          if (socket->priv->current_errors & FD_CONNECT) condition |= (G_IO_HUP | G_IO_ERR);
          else condition |= G_IO_OUT;
	  }
  }
  return condition;*/
  return 0;
}
#endif
typedef struct {
  GSource source;
  GPollFD pollfd;
  GSocket *socket;
  GIOCondition condition;
  GCancellable *cancellable;
  GPollFD cancel_pollfd;
  gint64 timeout_time;
} GSocketSource;
static gboolean socket_source_prepare(GSource *source, gint *timeout) {
  GSocketSource *socket_source = (GSocketSource*)source;
  if (g_cancellable_is_cancelled(socket_source->cancellable)) return TRUE;
  if (socket_source->timeout_time) {
      gint64 now;
      now = g_source_get_time(source);
      *timeout = (socket_source->timeout_time - now + 999) / 1000;
      if (*timeout < 0) {
          socket_source->socket->priv->timed_out = TRUE;
          socket_source->pollfd.revents = socket_source->condition & (G_IO_IN | G_IO_OUT);
          *timeout = 0;
          return TRUE;
      }
  } else *timeout = -1;
#ifdef G_OS_WIN32
  socket_source->pollfd.revents = update_condition(socket_source->socket);
#endif
  if ((socket_source->condition & socket_source->pollfd.revents) != 0) return TRUE;
  return FALSE;
}
static gboolean socket_source_check(GSource *source) {
  int timeout;
  return socket_source_prepare(source, &timeout);
}
static gboolean socket_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
  GSocketSourceFunc func = (GSocketSourceFunc)callback;
  GSocketSource *socket_source = (GSocketSource*)source;
#ifdef G_OS_WIN32
  socket_source->pollfd.revents = update_condition(socket_source->socket);
#endif
  return (*func)(socket_source->socket, socket_source->pollfd.revents & socket_source->condition, user_data);
}
static void socket_source_finalize(GSource *source) {
  GSocketSource *socket_source = (GSocketSource*)source;
  GSocket *socket;
  socket = socket_source->socket;
#ifdef G_OS_WIN32
  remove_condition_watch(socket, &socket_source->condition);
#endif
  g_object_unref(socket);
  if (socket_source->cancellable) {
      g_cancellable_release_fd(socket_source->cancellable);
      g_object_unref(socket_source->cancellable);
  }
}
static gboolean socket_source_closure_callback(GSocket *socket, GIOCondition condition, gpointer data) {
  GClosure *closure = data;
  GValue params[2] = { { 0, }, { 0, } };
  GValue result_value = { 0, };
  gboolean result;
  g_value_init(&result_value, G_TYPE_BOOLEAN);
  g_value_init(&params[0], G_TYPE_SOCKET);
  g_value_set_object(&params[0], socket);
  g_value_init (&params[1], G_TYPE_IO_CONDITION);
  g_value_set_flags(&params[1], condition);
  g_closure_invoke(closure, &result_value, 2, params, NULL);
  result = g_value_get_boolean(&result_value);
  g_value_unset(&result_value);
  g_value_unset(&params[0]);
  g_value_unset(&params[1]);
  return result;
}
static GSourceFuncs socket_source_funcs = {
  socket_source_prepare,
  socket_source_check,
  socket_source_dispatch,
  socket_source_finalize,
  (GSourceFunc)socket_source_closure_callback,
  (GSourceDummyMarshal)NULL
};
static GSource *socket_source_new(GSocket *socket, GIOCondition condition, GCancellable *cancellable) {
  GSource *source;
  GSocketSource *socket_source;
#ifndef G_OS_WIN32
  ensure_event(socket);
  if (socket->priv->event == WSA_INVALID_EVENT) {
      g_warning("Failed to create WSAEvent");
      return g_source_new(&broken_funcs, sizeof (GSource));
  }
#endif
  condition |= G_IO_HUP | G_IO_ERR;
  source = g_source_new(&socket_source_funcs, sizeof(GSocketSource));
  g_source_set_name(source, "GSocket");
  socket_source = (GSocketSource*)source;
  socket_source->socket = g_object_ref(socket);
  socket_source->condition = condition;
  if (g_cancellable_make_pollfd(cancellable, &socket_source->cancel_pollfd)) {
      socket_source->cancellable = g_object_ref(cancellable);
      g_source_add_poll(source, &socket_source->cancel_pollfd);
  }
#ifndef G_OS_WIN32
  add_condition_watch(socket, &socket_source->condition);
  socket_source->pollfd.fd = (gintptr)socket->priv->event;
#else
  socket_source->pollfd.fd = socket->priv->fd;
#endif
  socket_source->pollfd.events = condition;
  socket_source->pollfd.revents = 0;
  g_source_add_poll(source, &socket_source->pollfd);
  if (socket->priv->timeout) socket_source->timeout_time = g_get_monotonic_time() + socket->priv->timeout * 1000000;
  else socket_source->timeout_time = 0;
  return source;
}
GSource *g_socket_create_source(GSocket *socket, GIOCondition condition, GCancellable *cancellable) {
  g_return_val_if_fail(G_IS_SOCKET (socket) && (cancellable == NULL || G_IS_CANCELLABLE(cancellable)), NULL);
  return socket_source_new(socket, condition, cancellable);
}
GIOCondition g_socket_condition_check(GSocket *socket, GIOCondition condition) {
  if (!check_socket(socket, NULL)) return 0;
#ifdef G_OS_WIN32
  {
      GIOCondition current_condition;
      condition |= G_IO_ERR | G_IO_HUP;
      add_condition_watch(socket, &condition);
      current_condition = update_condition(socket);
      remove_condition_watch(socket, &condition);
      return condition & current_condition;
  }
#else
  {
      GPollFD poll_fd;
      gint result;
      poll_fd.fd = socket->priv->fd;
      poll_fd.events = condition;
      do {
          result = g_poll(&poll_fd, 1, 0);
      } while(result == -1 && get_socket_errno() == EINTR);
      return poll_fd.revents;
  }
#endif
}
gboolean g_socket_condition_wait(GSocket *socket, GIOCondition condition, GCancellable *cancellable, GError **error) {
  if (!check_socket(socket, error)) return FALSE;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return FALSE;
#ifndef G_OS_WIN32
  {
      GIOCondition current_condition;
      WSAEVENT events[2];
      DWORD res, timeout;
      GPollFD cancel_fd;
      int num_events;
      condition |=  G_IO_ERR | G_IO_HUP;
      add_condition_watch (socket, &condition);
      num_events = 0;
      events[num_events++] = socket->priv->event;
      if (g_cancellable_make_pollfd(cancellable, &cancel_fd)) events[num_events++] = (WSAEVENT)cancel_fd.fd;
      if (socket->priv->timeout) timeout = socket->priv->timeout * 1000;
      else timeout = WSA_INFINITE;
      current_condition = update_condition(socket);
      while((condition & current_condition) == 0) {
          res = WSAWaitForMultipleEvents(num_events, events, FALSE, timeout, FALSE);
          if (res == WSA_WAIT_FAILED) {
              int errsv = get_socket_errno();
              g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Waiting for socket condition: %s"), socket_strerror(errsv));
              break;
          } else if (res == WSA_WAIT_TIMEOUT) {
              g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT, _("Socket I/O timed out"));
              break;
          }
          if (g_cancellable_set_error_if_cancelled(cancellable, error)) break;
          current_condition = update_condition(socket);
      }
      remove_condition_watch(socket, &condition);
      if (num_events > 1) g_cancellable_release_fd(cancellable);
      return (condition & current_condition) != 0;
  }
#else
  {
      GPollFD poll_fd[2];
      gint result;
      gint num;
      gint timeout;
      poll_fd[0].fd = socket->priv->fd;
      poll_fd[0].events = condition;
      num = 1;
      if (g_cancellable_make_pollfd(cancellable, &poll_fd[1])) num++;
      if (socket->priv->timeout) timeout = socket->priv->timeout * 1000;
      else timeout = -1;
      do {
          result = g_poll(poll_fd, num, timeout);
      } while(result == -1 && get_socket_errno() == EINTR);
      if (num > 1) g_cancellable_release_fd(cancellable);
      if (result == 0) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT, _("Socket I/O timed out"));
          return FALSE;
      }
      return !g_cancellable_set_error_if_cancelled(cancellable, error);
  }
  #endif
}
gssize g_socket_send_message(GSocket *socket, GSocketAddress *address, GOutputVector *vectors, gint num_vectors, GSocketControlMessage **messages, gint num_messages,
                             gint flags, GCancellable *cancellable, GError **error) {
  GOutputVector one_vector;
  char zero;
  if (!check_socket(socket, error)) return -1;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  if (num_vectors == -1) {
      for (num_vectors = 0; vectors[num_vectors].buffer != NULL; num_vectors++);
  }
  if (num_messages == -1) {
      for (num_messages = 0; messages != NULL && messages[num_messages] != NULL; num_messages++);
  }
  if (num_vectors == 0) {
      zero = '\0';
      one_vector.buffer = &zero;
      one_vector.size = 1;
      num_vectors = 1;
      vectors = &one_vector;
  }
#ifdef G_OS_WIN32
  {
      struct msghdr msg;
      gssize result;
      msg.msg_flags = 0;
      if (address) {
          msg.msg_namelen = g_socket_address_get_native_size(address);
          msg.msg_name = g_alloca(msg.msg_namelen);
          if (!g_socket_address_to_native(address, msg.msg_name, msg.msg_namelen, error)) return -1;
      } else {
          msg.msg_name = NULL;
          msg.msg_namelen = 0;
      }
      {
          if (sizeof *msg.msg_iov == sizeof *vectors && sizeof msg.msg_iov->iov_base == sizeof vectors->buffer &&
              G_STRUCT_OFFSET(struct iovec, iov_base) == G_STRUCT_OFFSET(GOutputVector, buffer) &&
              sizeof msg.msg_iov->iov_len == sizeof vectors->size && G_STRUCT_OFFSET(struct iovec, iov_len) == G_STRUCT_OFFSET(GOutputVector, size)) {
              msg.msg_iov = (struct iovec*)vectors;
              msg.msg_iovlen = num_vectors;
          } else {
              gint i;
              msg.msg_iov = g_newa(struct iovec, num_vectors);
              for (i = 0; i < num_vectors; i++) {
                  msg.msg_iov[i].iov_base = (void*)vectors[i].buffer;
                  msg.msg_iov[i].iov_len = vectors[i].size;
              }
              msg.msg_iovlen = num_vectors;
          }
      }
      {
          struct cmsghdr *cmsg;
          gint i;
          msg.msg_controllen = 0;
          for (i = 0; i < num_messages; i++) msg.msg_controllen += CMSG_SPACE(g_socket_control_message_get_size(messages[i]));
          if (msg.msg_controllen == 0) msg.msg_control = NULL;
          else {
              msg.msg_control = g_alloca(msg.msg_controllen);
              memset(msg.msg_control, '\0', msg.msg_controllen);
          }
          cmsg = CMSG_FIRSTHDR(&msg);
          for (i = 0; i < num_messages; i++) {
              cmsg->cmsg_level = g_socket_control_message_get_level(messages[i]);
              cmsg->cmsg_type = g_socket_control_message_get_msg_type(messages[i]);
              cmsg->cmsg_len = CMSG_LEN(g_socket_control_message_get_size(messages[i]));
              g_socket_control_message_serialize(messages[i],CMSG_DATA(cmsg));
              cmsg = CMSG_NXTHDR(&msg, cmsg);
          }
          g_assert(cmsg == NULL);
      }
      while(1) {
          if (socket->priv->blocking && !g_socket_condition_wait(socket,G_IO_OUT, cancellable, error)) return -1;
          result = sendmsg(socket->priv->fd, &msg, flags | G_SOCKET_DEFAULT_SEND_FLAGS);
          if (result < 0) {
              int errsv = get_socket_errno();
              if (errsv == EINTR) continue;
              if (socket->priv->blocking && (errsv == EWOULDBLOCK || errsv == EAGAIN)) continue;
              g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error sending message: %s"), socket_strerror(errsv));
              return -1;
          }
          break;
      }
      return result;
  }
#else
  {
      struct sockaddr_storage addr;
      guint addrlen;
      DWORD bytes_sent;
      int result;
      WSABUF *bufs;
      gint i;
      if (num_messages != 0) {
          g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("GSocketControlMessage not supported on windows"));
          return -1;
      }
      bufs = g_newa(WSABUF, num_vectors);
      for (i = 0; i < num_vectors; i++) {
          bufs[i].buf = (char*)vectors[i].buffer;
          bufs[i].len = (gulong)vectors[i].size;
      }
      addrlen = 0;
      if (address) {
          addrlen = g_socket_address_get_native_size(address);
          if (!g_socket_address_to_native(address, &addr, sizeof addr, error)) return -1;
      }
      while(1) {
          if (socket->priv->blocking && !g_socket_condition_wait(socket, G_IO_OUT, cancellable, error)) return -1;
          if (address) result = WSASendTo(socket->priv->fd, bufs, num_vectors, &bytes_sent, flags, (const struct sockaddr*)&addr, addrlen, NULL, NULL);
          else result = WSASend(socket->priv->fd, bufs, num_vectors, &bytes_sent, flags, NULL, NULL);
          if (result != 0) {
              int errsv = get_socket_errno();
              if (errsv == WSAEINTR) continue;
              if (errsv == WSAEWOULDBLOCK) win32_unset_event_mask(socket, FD_WRITE);
              if (socket->priv->blocking && errsv == WSAEWOULDBLOCK) continue;
              g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error sending message: %s"), socket_strerror(errsv));
              return -1;
          }
          break;
      }
      return bytes_sent;
  }
#endif
}
gssize g_socket_receive_message(GSocket *socket, GSocketAddress **address, GInputVector *vectors, gint num_vectors, GSocketControlMessage ***messages,
                                gint *num_messages, gint *flags, GCancellable *cancellable, GError **error) {
  GInputVector one_vector;
  char one_byte;
  if (!check_socket(socket, error)) return -1;
  if (g_cancellable_set_error_if_cancelled(cancellable, error)) return -1;
  if (num_vectors == -1) {
      for (num_vectors = 0; vectors[num_vectors].buffer != NULL; num_vectors++);
  }
  if (num_vectors == 0) {
      one_vector.buffer = &one_byte;
      one_vector.size = 1;
      num_vectors = 1;
      vectors = &one_vector;
  }
#ifdef G_OS_WIN32
  {
      struct msghdr msg;
      gssize result;
      struct sockaddr_storage one_sockaddr;
      if (address) {
          msg.msg_name = &one_sockaddr;
          msg.msg_namelen = sizeof(struct sockaddr_storage);
      } else {
          msg.msg_name = NULL;
          msg.msg_namelen = 0;
      }
      if (sizeof *msg.msg_iov == sizeof *vectors && sizeof msg.msg_iov->iov_base == sizeof vectors->buffer &&
          G_STRUCT_OFFSET(struct iovec, iov_base) == G_STRUCT_OFFSET(GInputVector, buffer) &&
          sizeof msg.msg_iov->iov_len == sizeof vectors->size && G_STRUCT_OFFSET(struct iovec, iov_len) == G_STRUCT_OFFSET(GInputVector, size)) {
          msg.msg_iov = (struct iovec*)vectors;
          msg.msg_iovlen = num_vectors;
      } else {
          gint i;
          msg.msg_iov = g_newa(struct iovec, num_vectors);
          for (i = 0; i < num_vectors; i++) {
              msg.msg_iov[i].iov_base = vectors[i].buffer;
              msg.msg_iov[i].iov_len = vectors[i].size;
	      }
          msg.msg_iovlen = num_vectors;
      }
      msg.msg_control = g_alloca(2048);
      msg.msg_controllen = 2048;
      if (flags != NULL) msg.msg_flags = *flags;
      else msg.msg_flags = 0;
      while(1) {
          if (socket->priv->blocking && !g_socket_condition_wait(socket,G_IO_IN, cancellable, error)) return -1;
          result = recvmsg(socket->priv->fd, &msg, msg.msg_flags);
          if (result < 0) {
              int errsv = get_socket_errno();
              if (errsv == EINTR) continue;
              if (socket->priv->blocking && (errsv == EWOULDBLOCK || errsv == EAGAIN)) continue;
              g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error receiving message: %s"), socket_strerror(errsv));
              return -1;
          }
          break;
      }
      if (address != NULL) {
          if (msg.msg_namelen > 0) *address = g_socket_address_new_from_native(msg.msg_name, msg.msg_namelen);
          else *address = NULL;
      }
      {
          GPtrArray *my_messages = NULL;
          const gchar *scm_pointer;
          struct cmsghdr *cmsg;
          gsize scm_size;
          scm_pointer = (const gchar*)msg.msg_control;
          scm_size = msg.msg_controllen;
          for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
              GSocketControlMessage *message;
              message = g_socket_control_message_deserialize(cmsg->cmsg_level, cmsg->cmsg_type,cmsg->cmsg_len - ((char*)CMSG_DATA(cmsg) - (char*)cmsg),CMSG_DATA(cmsg));
              if (message == NULL) continue;
              if (messages == NULL) g_object_unref(message);
              else {
                  if (my_messages == NULL) my_messages = g_ptr_array_new();
                  g_ptr_array_add(my_messages, message);
              }
          }
          if (num_messages) *num_messages = my_messages != NULL ? my_messages->len : 0;
          if (messages) {
              if (my_messages == NULL) *messages = NULL;
              else {
                  g_ptr_array_add(my_messages, NULL);
                  *messages = (GSocketControlMessage**)g_ptr_array_free(my_messages, FALSE);
              }
          } else { g_assert(my_messages == NULL); }
      }
      if (flags != NULL) *flags = msg.msg_flags;
      return result;
  }
#else
  {
      struct sockaddr_storage addr;
      int addrlen;
      DWORD bytes_received;
      DWORD win_flags;
      int result;
      WSABUF *bufs;
      gint i;
      bufs = g_newa(WSABUF, num_vectors);
      for (i = 0; i < num_vectors; i++) {
          bufs[i].buf = (char*)vectors[i].buffer;
          bufs[i].len = (gulong)vectors[i].size;
      }
      if (flags != NULL) win_flags = *flags;
      else win_flags = 0;
      while(1) {
          if (socket->priv->blocking && !g_socket_condition_wait(socket, G_IO_IN, cancellable, error)) return -1;
          addrlen = sizeof addr;
          if (address) result = WSARecvFrom(socket->priv->fd, bufs, num_vectors, &bytes_received, &win_flags, (struct sockaddr*)&addr, &addrlen, NULL, NULL);
          else result = WSARecv(socket->priv->fd, bufs, num_vectors, &bytes_received, &win_flags, NULL, NULL);
          if (result != 0) {
              int errsv = get_socket_errno();
              if (errsv == WSAEINTR) continue;
              win32_unset_event_mask(socket, FD_READ);
              if (socket->priv->blocking && errsv == WSAEWOULDBLOCK) continue;
              g_set_error(error, G_IO_ERROR, socket_io_error_from_errno(errsv), _("Error receiving message: %s"), socket_strerror(errsv));
              return -1;
          }
          win32_unset_event_mask(socket, FD_READ);
          break;
      }
      if (address != NULL) {
          if (addrlen > 0) *address = g_socket_address_new_from_native (&addr, addrlen);
          else *address = NULL;
      }
      if (flags != NULL) *flags = win_flags;
      if (messages != NULL) *messages = NULL;
      if (num_messages != NULL) *num_messages = 0;
      return bytes_received;
  }
#endif
}
GCredentials *g_socket_get_credentials(GSocket *socket, GError **error) {
  GCredentials *ret;
  g_return_val_if_fail(G_IS_SOCKET(socket), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = NULL;
#ifdef __linux__
  {
      struct ucred native_creds;
      socklen_t optlen;
      optlen = sizeof(struct ucred);
      if (getsockopt(socket->priv->fd, SOL_SOCKET, SO_PEERCRED, (void*)&native_creds, &optlen) != 0) {
          int errsv = get_socket_errno();
          g_set_error(error, G_IO_ERROR, socket_io_error_from_errno (errsv), _("Unable to get pending error: %s"), socket_strerror(errsv));
      } else {
          ret = g_credentials_new();
          g_credentials_set_native(ret,G_CREDENTIALS_TYPE_LINUX_UCRED, &native_creds);
      }
  }
#else
  g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, _("g_socket_get_credentials not implemented for this OS"));
#endif
  return ret;
}