#include "config.h"
#ifdef __linux__
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 1
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 1
#else
#define G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED 0
#endif
#include <string.h>
#include <errno.h>
#include "../glib/glibintl.h"
#include "../glib/glib-object.h"
#include "gunixcredentialsmessage.h"
#include "gcredentials.h"

struct _GUnixCredentialsMessagePrivate {
  GCredentials *credentials;
};
enum {
  PROP_0,
  PROP_CREDENTIALS
};
G_DEFINE_TYPE(GUnixCredentialsMessage, g_unix_credentials_message, G_TYPE_SOCKET_CONTROL_MESSAGE);
static gsize g_unix_credentials_message_get_size(GSocketControlMessage *message) {
#ifdef __linux__
  return sizeof(struct ucred);
#elif defined(__FreeBSD__)
  return sizeof(struct cmsgcred);
#else
  return 0;
#endif
}
static int g_unix_credentials_message_get_level(GSocketControlMessage *message) {
#ifdef __linux__
  return SOL_SOCKET;
#elif defined(__FreeBSD__)
  return SOL_SOCKET;
#else
  return 0;
#endif
}
static int g_unix_credentials_message_get_msg_type(GSocketControlMessage *message) {
#ifdef __linux__
  return SCM_CREDENTIALS;
#elif defined(__FreeBSD__)
  return SCM_CREDS;
#else
  return 0;
#endif
}
static GSocketControlMessage *g_unix_credentials_message_deserialize(gint level, gint type, gsize size, gpointer data) {
  GSocketControlMessage *message;
  message = NULL;
#ifdef __linux__
  {
      GCredentials *credentials;
      struct ucred *ucred;
      if (level != SOL_SOCKET || type != SCM_CREDENTIALS) return message;
      if (size != sizeof(struct ucred)) {
          g_warning("Expected a struct ucred (%" G_GSIZE_FORMAT " bytes) but got %" G_GSIZE_FORMAT " bytes of data", sizeof(struct ucred), size);
          return message;
      }
      ucred = data;
      credentials = g_credentials_new();
      g_credentials_set_native(credentials, G_CREDENTIALS_TYPE_LINUX_UCRED, ucred);
      message = g_unix_credentials_message_new_with_credentials(credentials);
      g_object_unref(credentials);
  }
#elif defined(__FreeBSD__)
  {
      GCredentials *credentials;
      struct cmsgcred *cred;
      if (level != SOL_SOCKET || type != SCM_CREDS) return message;
      if (size < CMSG_LEN (sizeof *cred)) {
          g_warning("Expected a struct cmsgcred (%" G_GSIZE_FORMAT " bytes) but got %" G_GSIZE_FORMAT " bytes of data", CMSG_LEN (sizeof *cred), size);
          return message;
      }
      cred = data;
      credentials = g_credentials_new();
      g_credentials_set_native(credentials, G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED, cred);
      message = g_unix_credentials_message_new_with_credentials(credentials);
      g_object_unref(credentials);
  }
#endif
  return message;
}
static void g_unix_credentials_message_serialize(GSocketControlMessage *_message, gpointer data) {
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE(_message);
#ifdef __linux__
  memcpy(data, g_credentials_get_native(message->priv->credentials,G_CREDENTIALS_TYPE_LINUX_UCRED), sizeof(struct ucred));
#elif defined(__FreeBSD__)
  memcpy(data, g_credentials_get_native(message->priv->credentials, G_CREDENTIALS_TYPE_FREEBSD_CMSGCRED), sizeof(struct cmsgcred));
#endif
}
static void g_unix_credentials_message_finalize (GObject *object) {
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE(object);
  if (message->priv->credentials != NULL) g_object_unref(message->priv->credentials);
  G_OBJECT_CLASS(g_unix_credentials_message_parent_class)->finalize(object);
}
static void g_unix_credentials_message_init(GUnixCredentialsMessage *message) {
  message->priv = G_TYPE_INSTANCE_GET_PRIVATE(message, G_TYPE_UNIX_CREDENTIALS_MESSAGE, GUnixCredentialsMessagePrivate);
}
static void g_unix_credentials_message_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE(object);
  switch(prop_id) {
      case PROP_CREDENTIALS: g_value_set_object(value, message->priv->credentials); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_unix_credentials_message_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE(object);
  switch(prop_id) {
      case PROP_CREDENTIALS: message->priv->credentials = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void g_unix_credentials_message_constructed(GObject *object) {
  GUnixCredentialsMessage *message = G_UNIX_CREDENTIALS_MESSAGE(object);
  if (message->priv->credentials == NULL) message->priv->credentials = g_credentials_new();
  if (G_OBJECT_CLASS(g_unix_credentials_message_parent_class)->constructed != NULL) G_OBJECT_CLASS(g_unix_credentials_message_parent_class)->constructed(object);
}
static void g_unix_credentials_message_class_init(GUnixCredentialsMessageClass *class) {
  GSocketControlMessageClass *scm_class;
  GObjectClass *gobject_class;
  g_type_class_add_private(class, sizeof(GUnixCredentialsMessagePrivate));
  gobject_class = G_OBJECT_CLASS(class);
  gobject_class->get_property = g_unix_credentials_message_get_property;
  gobject_class->set_property = g_unix_credentials_message_set_property;
  gobject_class->finalize = g_unix_credentials_message_finalize;
  gobject_class->constructed = g_unix_credentials_message_constructed;
  scm_class = G_SOCKET_CONTROL_MESSAGE_CLASS(class);
  scm_class->get_size = g_unix_credentials_message_get_size;
  scm_class->get_level = g_unix_credentials_message_get_level;
  scm_class->get_type = g_unix_credentials_message_get_msg_type;
  scm_class->serialize = g_unix_credentials_message_serialize;
  scm_class->deserialize = g_unix_credentials_message_deserialize;
  g_object_class_install_property(gobject_class, PROP_CREDENTIALS, g_param_spec_object ("credentials", P_("Credentials"), P_("The credentials stored "
                                  "in the message"), G_TYPE_CREDENTIALS, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME |
                                  G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));

}
gboolean g_unix_credentials_message_is_supported(void) {
  return G_UNIX_CREDENTIALS_MESSAGE_SUPPORTED;
}
GSocketControlMessage *g_unix_credentials_message_new(void) {
  g_return_val_if_fail(g_unix_credentials_message_is_supported(), NULL);
  return g_object_new(G_TYPE_UNIX_CREDENTIALS_MESSAGE,NULL);
}
GSocketControlMessage *g_unix_credentials_message_new_with_credentials(GCredentials *credentials) {
  g_return_val_if_fail(G_IS_CREDENTIALS(credentials), NULL);
  g_return_val_if_fail(g_unix_credentials_message_is_supported(), NULL);
  return g_object_new(G_TYPE_UNIX_CREDENTIALS_MESSAGE,"credentials", credentials, NULL);
}
GCredentials *g_unix_credentials_message_get_credentials(GUnixCredentialsMessage *message) {
  g_return_val_if_fail(G_IS_UNIX_CREDENTIALS_MESSAGE(message), NULL);
  return message->priv->credentials;
}