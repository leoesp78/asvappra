#include <sys/types.h>
#include <sys/socket.h>
#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusauth.h"
#include "gdbusauthmechanismanon.h"
#include "gdbusauthmechanismexternal.h"
#include "gdbusauthmechanismsha1.h"
#include "gdbusauthobserver.h"
#include "gdbuserror.h"
#include "gdbusutils.h"
#include "gioenumtypes.h"
#include "gcredentials.h"
#include "gdbusprivate.h"
#include "giostream.h"
#include "gdatainputstream.h"
#include "gdataoutputstream.h"
#include "gunixconnection.h"
#include "gunixcredentialsmessage.h"

static void debug_print(const gchar *message, ...) {
  if (G_UNLIKELY(_g_dbus_debug_authentication())) {
      gchar *s;
      GString *str;
      va_list var_args;
      guint n;
      _g_dbus_debug_print_lock();
      va_start(var_args, message);
      s = g_strdup_vprintf(message, var_args);
      va_end(var_args);
      str = g_string_new(NULL);
      for (n = 0; s[n] != '\0'; n++) {
          if (G_UNLIKELY(s[n] == '\r')) g_string_append(str, "\\r");
          else if (G_UNLIKELY(s[n] == '\n')) g_string_append(str, "\\n");
          else g_string_append_c(str, s[n]);
      }
      g_print("GDBus-debug:Auth: %s\n", str->str);
      g_string_free(str, TRUE);
      g_free(s);
      _g_dbus_debug_print_unlock();
  }
}
typedef struct {
  const gchar *name;
  gint priority;
  GType gtype;
} Mechanism;
static void mechanism_free(Mechanism *m);
struct _GDBusAuthPrivate {
  GIOStream *stream;
  GList *available_mechanisms;
};
enum {
  PROP_0,
  PROP_STREAM
};
G_DEFINE_TYPE(GDBusAuth, _g_dbus_auth, G_TYPE_OBJECT);
static void _g_dbus_auth_finalize(GObject *object) {
  GDBusAuth *auth = G_DBUS_AUTH(object);
  if (auth->priv->stream != NULL) g_object_unref(auth->priv->stream);
  g_list_foreach(auth->priv->available_mechanisms, (GFunc)mechanism_free, NULL);
  g_list_free(auth->priv->available_mechanisms);
  if (G_OBJECT_CLASS(_g_dbus_auth_parent_class)->finalize != NULL) G_OBJECT_CLASS(_g_dbus_auth_parent_class)->finalize(object);
}
static void _g_dbus_auth_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GDBusAuth *auth = G_DBUS_AUTH(object);
  switch(prop_id) {
      case PROP_STREAM: g_value_set_object(value, auth->priv->stream); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void _g_dbus_auth_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GDBusAuth *auth = G_DBUS_AUTH(object);
  switch(prop_id) {
      case PROP_STREAM: auth->priv->stream = g_value_dup_object(value); break;
      default: G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec); break;
  }
}
static void _g_dbus_auth_class_init(GDBusAuthClass *klass) {
  GObjectClass *gobject_class;
  g_type_class_add_private(klass, sizeof(GDBusAuthPrivate));
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->get_property = _g_dbus_auth_get_property;
  gobject_class->set_property = _g_dbus_auth_set_property;
  gobject_class->finalize = _g_dbus_auth_finalize;
  g_object_class_install_property(gobject_class, PROP_STREAM, g_param_spec_object("stream", "IO Stream","The underlying GIOStream used for I/O",
                                  G_TYPE_IO_STREAM, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB |
                                  G_PARAM_STATIC_NICK));
}
static void mechanism_free(Mechanism *m) {
  g_free(m);
}
static void add_mechanism(GDBusAuth *auth, GType mechanism_type) {
  Mechanism *m;
  m = g_new0(Mechanism, 1);
  m->name = _g_dbus_auth_mechanism_get_name(mechanism_type);
  m->priority = _g_dbus_auth_mechanism_get_priority(mechanism_type);
  m->gtype = mechanism_type;
  auth->priv->available_mechanisms = g_list_prepend(auth->priv->available_mechanisms, m);
}
static gint mech_compare_func(Mechanism *a, Mechanism *b) {
  gint ret;
  ret = b->priority - a->priority;
  if (ret == 0) ret = g_strcmp0(b->name, a->name);
  return ret;
}
static void _g_dbus_auth_init(GDBusAuth *auth) {
  auth->priv = G_TYPE_INSTANCE_GET_PRIVATE(auth, G_TYPE_DBUS_AUTH, GDBusAuthPrivate);
  add_mechanism(auth, G_TYPE_DBUS_AUTH_MECHANISM_ANON);
  add_mechanism(auth, G_TYPE_DBUS_AUTH_MECHANISM_SHA1);
  add_mechanism(auth, G_TYPE_DBUS_AUTH_MECHANISM_EXTERNAL);
  auth->priv->available_mechanisms = g_list_sort(auth->priv->available_mechanisms, (GCompareFunc)mech_compare_func);
}
static GType find_mech_by_name(GDBusAuth *auth, const gchar *name) {
  GType ret;
  GList *l;
  ret = (GType)0;
  for (l = auth->priv->available_mechanisms; l != NULL; l = l->next) {
      Mechanism *m = l->data;
      if (g_strcmp0(name, m->name) == 0) {
          ret = m->gtype;
          goto out;
      }
  }
out:
  return ret;
}
GDBusAuth  *_g_dbus_auth_new(GIOStream *stream) {
  return g_object_new(G_TYPE_DBUS_AUTH, "stream", stream, NULL);
}
static gchar *_my_g_data_input_stream_read_line(GDataInputStream *dis, gsize *out_line_length, GCancellable *cancellable, GError **error) {
  gchar *ret;
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = g_data_input_stream_read_line(dis, out_line_length, cancellable, error);
  if (ret == NULL && error != NULL && *error == NULL) {
      g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected lack of content trying to read a line");
  }
  return ret;
}
static gchar *_my_g_input_stream_read_line_safe(GInputStream *i, gsize *out_line_length, GCancellable *cancellable, GError **error) {
  GString *str;
  gchar c;
  gssize num_read;
  gboolean last_was_cr;
  str = g_string_new(NULL);
  last_was_cr = FALSE;
  while(TRUE) {
      num_read = g_input_stream_read(i, &c,1, cancellable, error);
      if (num_read == -1) goto fail;
      if (num_read == 0) {
          if (error != NULL && *error == NULL) {
              g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected lack of content trying to (safely) read a line");
          }
          goto fail;
      }
      g_string_append_c(str, (gint)c);
      if (last_was_cr) {
          if (c == 0x0a) {
              g_assert(str->len >= 2);
              g_string_set_size(str, str->len - 2);
              goto out;
          }
      }
      last_was_cr = (c == 0x0d);
  }
out:
  if (out_line_length != NULL) *out_line_length = str->len;
  return g_string_free(str, FALSE);
fail:
  g_assert(error == NULL || *error != NULL);
  g_string_free(str, TRUE);
  return NULL;
}
static void append_nibble(GString *s, gint val) {
  g_string_append_c(s, val >= 10 ? ('a' + val - 10) : ('0' + val));
}
static gchar *hexdecode(const gchar *str, gsize *out_len, GError **error) {
  gchar *ret;
  GString *s;
  guint n;
  ret = NULL;
  s = g_string_new(NULL);
  for (n = 0; str[n] != '\0'; n += 2) {
      gint upper_nibble;
      gint lower_nibble;
      guint value;
      upper_nibble = g_ascii_xdigit_value(str[n]);
      lower_nibble = g_ascii_xdigit_value(str[n + 1]);
      if (upper_nibble == -1 || lower_nibble == -1) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Error hexdecoding string `%s' around position %d", str, n);
          goto out;
      }
      value = (upper_nibble<<4) | lower_nibble;
      g_string_append_c(s, value);
  }
  ret = g_string_free(s, FALSE);
  s = NULL;
out:
  if (s != NULL) g_string_free(s, TRUE);
  return ret;
}
static gchar *hexencode(const gchar *str) {
  guint n;
  GString *s;
  s = g_string_new(NULL);
  for (n = 0; str[n] != '\0'; n++) {
      gint val;
      gint upper_nibble;
      gint lower_nibble;
      val = ((const guchar*)str)[n];
      upper_nibble = val >> 4;
      lower_nibble = val & 0x0f;
      append_nibble(s, upper_nibble);
      append_nibble(s, lower_nibble);
  }
  return g_string_free(s, FALSE);
}
static GDBusAuthMechanism *client_choose_mech_and_send_initial_response(GDBusAuth *auth, GCredentials *credentials_that_were_sent,
                                                                        const gchar* const *supported_auth_mechs, GPtrArray *attempted_auth_mechs,
                                                                        GDataOutputStream *dos, GCancellable *cancellable, GError **error) {
  GDBusAuthMechanism *mech;
  GType auth_mech_to_use_gtype;
  guint n;
  guint m;
  gchar *initial_response;
  gsize initial_response_len;
  gchar *encoded;
  gchar *s;
again:
  mech = NULL;
  debug_print("CLIENT: Trying to choose mechanism");
  auth_mech_to_use_gtype = (GType)0;
  for (n = 0; supported_auth_mechs[n] != NULL; n++) {
      gboolean attempted_already;
      attempted_already = FALSE;
      for (m = 0; m < attempted_auth_mechs->len; m++) {
          if (g_strcmp0(supported_auth_mechs[n], attempted_auth_mechs->pdata[m]) == 0) {
              attempted_already = TRUE;
              break;
          }
      }
      if (!attempted_already) {
          auth_mech_to_use_gtype = find_mech_by_name(auth, supported_auth_mechs[n]);
          if (auth_mech_to_use_gtype != (GType) 0) break;
      }
  }
  if (auth_mech_to_use_gtype == (GType)0) {
      guint n;
      gchar *available;
      GString *tried_str;
      debug_print("CLIENT: Exhausted all available mechanisms");
      available = g_strjoinv(", ", (gchar**)supported_auth_mechs);
      tried_str = g_string_new(NULL);
      for (n = 0; n < attempted_auth_mechs->len; n++) {
          if (n > 0) g_string_append(tried_str, ", ");
          g_string_append(tried_str, attempted_auth_mechs->pdata[n]);
      }
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Exhausted all available authentication mechanisms (tried: %s) (available: %s)",
                  tried_str->str, available);
      g_string_free(tried_str, TRUE);
      g_free(available);
      goto out;
  }
  mech = g_object_new(auth_mech_to_use_gtype, "stream", auth->priv->stream, "credentials", credentials_that_were_sent, NULL);
  debug_print("CLIENT: Trying mechanism `%s'", _g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype));
  g_ptr_array_add(attempted_auth_mechs, (gpointer)_g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype));
  if (!_g_dbus_auth_mechanism_is_supported(mech)) {
      debug_print("CLIENT: Mechanism `%s' says it is not supported", _g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype));
      g_object_unref(mech);
      mech = NULL;
      goto again;
  }
  initial_response_len = -1;
  initial_response = _g_dbus_auth_mechanism_client_initiate(mech, &initial_response_len);
#if 0
  g_printerr("using auth mechanism with name `%s' of type `%s' with initial response `%s'\n", _g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype),
             g_type_name(G_TYPE_FROM_INSTANCE(mech)), initial_response);
#endif
  if (initial_response != NULL) {
      encoded = hexencode(initial_response);
      s = g_strdup_printf("AUTH %s %s\r\n", _g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype), encoded);
      g_free(initial_response);
      g_free(encoded);
  } else s = g_strdup_printf("AUTH %s\r\n", _g_dbus_auth_mechanism_get_name(auth_mech_to_use_gtype));
  debug_print("CLIENT: writing `%s'", s);
  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
      g_object_unref(mech);
      mech = NULL;
      g_free(s);
      goto out;
  }
  g_free(s);
out:
  return mech;
}
typedef enum {
  CLIENT_STATE_WAITING_FOR_DATA,
  CLIENT_STATE_WAITING_FOR_OK,
  CLIENT_STATE_WAITING_FOR_REJECT,
  CLIENT_STATE_WAITING_FOR_AGREE_UNIX_FD
} ClientState;
gchar *_g_dbus_auth_run_client(GDBusAuth *auth, GDBusCapabilityFlags offered_capabilities, GDBusCapabilityFlags *out_negotiated_capabilities,
                               GCancellable *cancellable, GError **error) {
  gchar *s;
  GDataInputStream *dis;
  GDataOutputStream *dos;
  GCredentials *credentials;
  gchar *ret_guid;
  gchar *line;
  gsize line_length;
  gchar **supported_auth_mechs;
  GPtrArray *attempted_auth_mechs;
  GDBusAuthMechanism *mech;
  ClientState state;
  GDBusCapabilityFlags negotiated_capabilities;
  debug_print("CLIENT: initiating");
  ret_guid = NULL;
  supported_auth_mechs = NULL;
  attempted_auth_mechs = g_ptr_array_new();
  mech = NULL;
  negotiated_capabilities = 0;
  credentials = NULL;
  dis = G_DATA_INPUT_STREAM(g_data_input_stream_new(g_io_stream_get_input_stream(auth->priv->stream)));
  dos = G_DATA_OUTPUT_STREAM(g_data_output_stream_new(g_io_stream_get_output_stream(auth->priv->stream)));
  g_filter_input_stream_set_close_base_stream(G_FILTER_INPUT_STREAM(dis), FALSE);
  g_filter_output_stream_set_close_base_stream(G_FILTER_OUTPUT_STREAM(dos), FALSE);
  g_data_input_stream_set_newline_type(dis, G_DATA_STREAM_NEWLINE_TYPE_CR_LF);
#ifndef G_OS_UNIX
  if (G_IS_UNIX_CONNECTION(auth->priv->stream) && g_unix_credentials_message_is_supported()) {
      credentials = g_credentials_new();
      if (!g_unix_connection_send_credentials(G_UNIX_CONNECTION(auth->priv->stream), cancellable, error)) goto out;
  } else {
      if (!g_data_output_stream_put_byte(dos, '\0', cancellable, error)) goto out;
  }
#else
  if (!g_data_output_stream_put_byte(dos, '\0', cancellable, error)) goto out;
#endif
  if (credentials != NULL) {
      if (G_UNLIKELY(_g_dbus_debug_authentication())) {
          s = g_credentials_to_string(credentials);
          debug_print("CLIENT: sent credentials `%s'", s);
          g_free(s);
      }
  } else debug_print("CLIENT: didn't send any credentials");
  s = "AUTH\r\n";
  debug_print("CLIENT: writing `%s'", s);
  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
  state = CLIENT_STATE_WAITING_FOR_REJECT;
  while(TRUE) {
      switch(state) {
          case CLIENT_STATE_WAITING_FOR_REJECT:
              debug_print ("CLIENT: WaitingForReject");
              line = _my_g_data_input_stream_read_line (dis, &line_length, cancellable, error);
              if (line == NULL) goto out;
              debug_print ("CLIENT: WaitingForReject, read '%s'", line);
              foobar:
              if (!g_str_has_prefix(line, "REJECTED ")) {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"In WaitingForReject: Expected `REJECTED am1 am2 ... amN', got `%s'", line);
                  g_free(line);
                  goto out;
              }
              if (supported_auth_mechs == NULL) {
                  supported_auth_mechs = g_strsplit(line + sizeof ("REJECTED ") - 1, " ", 0);
              #if 0
                  for (n = 0; supported_auth_mechs != NULL && supported_auth_mechs[n] != NULL; n++)
                      g_printerr ("supported_auth_mechs[%d] = `%s'\n", n, supported_auth_mechs[n]);
              #endif
              }
              g_free (line);
              mech = client_choose_mech_and_send_initial_response(auth, credentials, (const gchar* const*)supported_auth_mechs, attempted_auth_mechs, dos,
                                                                  cancellable, error);
              if (mech == NULL) goto out;
              if (_g_dbus_auth_mechanism_client_get_state(mech) == G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA) state = CLIENT_STATE_WAITING_FOR_DATA;
              else state = CLIENT_STATE_WAITING_FOR_OK;
              break;
          case CLIENT_STATE_WAITING_FOR_OK:
              debug_print("CLIENT: WaitingForOK");
              line = _my_g_data_input_stream_read_line(dis, &line_length, cancellable, error);
              if (line == NULL) goto out;
              debug_print("CLIENT: WaitingForOK, read `%s'", line);
              if (g_str_has_prefix(line, "OK ")) {
                  if (!g_dbus_is_guid(line + 3)) {
                      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Invalid OK response `%s'", line);
                      g_free(line);
                      goto out;
                  }
                  ret_guid = g_strdup(line + 3);
                  g_free(line);
                  if (offered_capabilities & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING) {
                      s = "NEGOTIATE_UNIX_FD\r\n";
                      debug_print("CLIENT: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                      state = CLIENT_STATE_WAITING_FOR_AGREE_UNIX_FD;
                  } else {
                      s = "BEGIN\r\n";
                      debug_print("CLIENT: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                      goto out;
                  }
              } else if (g_str_has_prefix(line, "REJECTED ")) goto foobar;
              else {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"In WaitingForOk: unexpected response `%s'", line);
                  g_free(line);
                  goto out;
              }
              break;
          case CLIENT_STATE_WAITING_FOR_AGREE_UNIX_FD:
              debug_print("CLIENT: WaitingForAgreeUnixFD");
              line = _my_g_data_input_stream_read_line(dis, &line_length, cancellable, error);
              if (line == NULL) goto out;
              debug_print("CLIENT: WaitingForAgreeUnixFD, read=`%s'", line);
              if (g_strcmp0(line, "AGREE_UNIX_FD") == 0) {
                  g_free(line);
                  negotiated_capabilities |= G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING;
                  s = "BEGIN\r\n";
                  debug_print("CLIENT: writing `%s'", s);
                  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                  goto out;
              } else if (g_str_has_prefix(line, "ERROR") && (line[5] == 0 || g_ascii_isspace(line[5]))) {
                  g_free(line);
                  s = "BEGIN\r\n";
                  debug_print("CLIENT: writing `%s'", s);
                  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                  goto out;
              } else {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"In WaitingForAgreeUnixFd: unexpected response `%s'", line);
                  g_free(line);
                  goto out;
              }
              break;
          case CLIENT_STATE_WAITING_FOR_DATA:
              debug_print("CLIENT: WaitingForData");
              line = _my_g_data_input_stream_read_line(dis, &line_length, cancellable, error);
              if (line == NULL) goto out;
              debug_print("CLIENT: WaitingForData, read=`%s'", line);
              if (g_str_has_prefix(line, "DATA ")) {
                  gchar *encoded;
                  gchar *decoded_data;
                  gsize decoded_data_len = 0;
                  encoded = g_strdup(line + 5);
                  g_free(line);
                  g_strstrip(encoded);
                  decoded_data = hexdecode(encoded, &decoded_data_len, error);
                  g_free(encoded);
                  if (decoded_data == NULL) {
                      g_prefix_error(error, "DATA response is malformed: ");
                      goto out;
                  }
                  _g_dbus_auth_mechanism_client_data_receive(mech, decoded_data, decoded_data_len);
                  g_free(decoded_data);
                  if (_g_dbus_auth_mechanism_client_get_state(mech) == G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND) {
                      gchar *data;
                      gsize data_len;
                      gchar *encoded_data;
                      data = _g_dbus_auth_mechanism_client_data_send(mech, &data_len);
                      encoded_data = hexencode(data);
                      s = g_strdup_printf("DATA %s\r\n", encoded_data);
                      g_free(encoded_data);
                      g_free(data);
                      debug_print("CLIENT: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                          g_free(s);
                          goto out;
                      }
                      g_free (s);
                  }
                  state = CLIENT_STATE_WAITING_FOR_OK;
              }else {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, "In WaitingForData: unexpected response `%s'", line);
                  g_free(line);
                  goto out;
              }
              break;
          default: g_assert_not_reached(); break;
      }
  };
out:
  if (mech != NULL) g_object_unref(mech);
  g_ptr_array_unref(attempted_auth_mechs);
  g_strfreev(supported_auth_mechs);
  g_object_unref(dis);
  g_object_unref(dos);
  if (error != NULL && *error != NULL) {
      g_free(ret_guid);
      ret_guid = NULL;
  }
  if (ret_guid != NULL) {
      if (out_negotiated_capabilities != NULL) *out_negotiated_capabilities = negotiated_capabilities;
  }
  if (credentials != NULL) g_object_unref(credentials);
  debug_print("CLIENT: Done, authenticated=%d", ret_guid != NULL);
  return ret_guid;
}
static gchar *get_auth_mechanisms(GDBusAuth *auth, gboolean allow_anonymous, const gchar *prefix, const gchar *suffix, const gchar *separator) {
  GList *l;
  GString *str;
  gboolean need_sep;
  str = g_string_new(prefix);
  need_sep = FALSE;
  for (l = auth->priv->available_mechanisms; l != NULL; l = l->next) {
      Mechanism *m = l->data;
      if (!allow_anonymous && g_strcmp0(m->name, "ANONYMOUS") == 0) continue;
      if (need_sep) g_string_append(str, separator);
      g_string_append(str, m->name);
      need_sep = TRUE;
  }
  g_string_append(str, suffix);
  return g_string_free(str, FALSE);
}
typedef enum {
  SERVER_STATE_WAITING_FOR_AUTH,
  SERVER_STATE_WAITING_FOR_DATA,
  SERVER_STATE_WAITING_FOR_BEGIN
} ServerState;
gboolean _g_dbus_auth_run_server(GDBusAuth *auth, GDBusAuthObserver *observer, const gchar *guid, gboolean allow_anonymous, GDBusCapabilityFlags offered_capabilities,
                                 GDBusCapabilityFlags *out_negotiated_capabilities, GCredentials **out_received_credentials, GCancellable *cancellable,
                                 GError **error) {
  gboolean ret;
  ServerState state;
  GDataInputStream *dis;
  GDataOutputStream *dos;
  GError *local_error;
  guchar byte;
  gchar *line;
  gsize line_length;
  GDBusAuthMechanism *mech;
  gchar *s;
  GDBusCapabilityFlags negotiated_capabilities;
  GCredentials *credentials;
  debug_print("SERVER: initiating");
  ret = FALSE;
  dis = NULL;
  dos = NULL;
  mech = NULL;
  negotiated_capabilities = 0;
  credentials = NULL;
  if (!g_dbus_is_guid(guid)) {
      g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"The given guid `%s' is not valid", guid);
      goto out;
  }
  dis = G_DATA_INPUT_STREAM(g_data_input_stream_new(g_io_stream_get_input_stream(auth->priv->stream)));
  dos = G_DATA_OUTPUT_STREAM(g_data_output_stream_new(g_io_stream_get_output_stream(auth->priv->stream)));
  g_filter_input_stream_set_close_base_stream(G_FILTER_INPUT_STREAM(dis), FALSE);
  g_filter_output_stream_set_close_base_stream(G_FILTER_OUTPUT_STREAM(dos), FALSE);
  g_data_input_stream_set_newline_type(dis, G_DATA_STREAM_NEWLINE_TYPE_CR_LF);
#ifndef G_OS_UNIX
  if (G_IS_UNIX_CONNECTION(auth->priv->stream) && g_unix_credentials_message_is_supported()) {
      local_error = NULL;
      credentials = g_unix_connection_receive_credentials(G_UNIX_CONNECTION(auth->priv->stream), cancellable, &local_error);
      if (credentials == NULL) {
          g_propagate_error(error, local_error);
          goto out;
      }
  } else {
      local_error = NULL;
      byte = g_data_input_stream_read_byte(dis, cancellable, &local_error);
      if (local_error != NULL) {
          g_propagate_error(error, local_error);
          goto out;
      }
  }
#else
  local_error = NULL;
  byte = g_data_input_stream_read_byte(dis, cancellable, &local_error);
  if (local_error != NULL) {
      g_propagate_error(error, local_error);
      goto out;
  }
#endif
  if (credentials != NULL) {
      if (G_UNLIKELY(_g_dbus_debug_authentication())) {
          s = g_credentials_to_string(credentials);
          debug_print("SERVER: received credentials `%s'", s);
          g_free(s);
      }
  } else debug_print("SERVER: didn't receive any credentials");
  state = SERVER_STATE_WAITING_FOR_AUTH;
  while(TRUE) {
      switch(state) {
          case SERVER_STATE_WAITING_FOR_AUTH:
              debug_print("SERVER: WaitingForAuth");
              line = _my_g_data_input_stream_read_line(dis, &line_length, cancellable, error);
              debug_print("SERVER: WaitingForAuth, read `%s'", line);
              if (line == NULL) goto out;
              if (g_strcmp0(line, "AUTH") == 0) {
                  s = get_auth_mechanisms(auth, allow_anonymous, "REJECTED ", "\r\n", " ");
                  debug_print("SERVER: writing `%s'", s);
                  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                      g_free(s);
                      goto out;
                  }
                  g_free(s);
                  g_free(line);
              } else if (g_str_has_prefix(line, "AUTH ")) {
                  gchar **tokens;
                  const gchar *encoded;
                  const gchar *mech_name;
                  GType auth_mech_to_use_gtype;
                  tokens = g_strsplit(line, " ", 0);
                  g_free(line);
                  switch(g_strv_length(tokens)) {
                      case 2:
                          mech_name = tokens[1];
                          encoded = NULL;
                          break;
                      case 3:
                          mech_name = tokens[1];
                          encoded = tokens[2];
                          break;
                      default:
                          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected line `%s' while in WaitingForAuth state", line);
                          g_strfreev (tokens);
                          goto out;
                  }
                  auth_mech_to_use_gtype = find_mech_by_name (auth, mech_name);
                  if ((auth_mech_to_use_gtype == (GType)0) || (!allow_anonymous && g_strcmp0(mech_name, "ANONYMOUS") == 0)) {
                      g_strfreev(tokens);
                      s = get_auth_mechanisms(auth, allow_anonymous, "REJECTED ", "\r\n", " ");
                      debug_print("SERVER: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                          g_free(s);
                          goto out;
                      }
                      g_free(s);
                      state = SERVER_STATE_WAITING_FOR_AUTH;
                  } else {
                      gchar *initial_response;
                      gsize initial_response_len;
                      mech = g_object_new(auth_mech_to_use_gtype, "stream", auth->priv->stream, "credentials", credentials, NULL);
                      initial_response = NULL;
                      initial_response_len = 0;
                      if (encoded != NULL) {
                          initial_response = hexdecode (encoded, &initial_response_len, error);
                          if (initial_response == NULL) {
                              g_prefix_error(error, "Initial response is malformed: ");
                              g_strfreev(tokens);
                              goto out;
                          }
                      }
                      _g_dbus_auth_mechanism_server_initiate(mech, initial_response, initial_response_len);
                      g_free(initial_response);
                      g_strfreev(tokens);
                  change_state:
                      switch(_g_dbus_auth_mechanism_server_get_state(mech)) {
                          case G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED:
                          if (observer != NULL && !g_dbus_auth_observer_authorize_authenticated_peer(observer, auth->priv->stream, credentials)) {
                              g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Cancelled via GDBusAuthObserver::authorize-authenticated-peer");
                              goto out;
                          } else {
                              s = g_strdup_printf("OK %s\r\n", guid);
                              debug_print("SERVER: writing `%s'", s);
                              if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                                  g_free(s);
                                  goto out;
                              }
                              g_free(s);
                              state = SERVER_STATE_WAITING_FOR_BEGIN;
                          }
                          break;
                          case G_DBUS_AUTH_MECHANISM_STATE_REJECTED:
                              s = get_auth_mechanisms(auth, allow_anonymous, "REJECTED ", "\r\n", " ");
                              debug_print("SERVER: writing `%s'", s);
                              if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                                  g_free(s);
                                  goto out;
                              }
                              g_free(s);
                              state = SERVER_STATE_WAITING_FOR_AUTH;
                              break;
                          case G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA: state = SERVER_STATE_WAITING_FOR_DATA; break;
                          case G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND: {
                                  gchar *data;
                                  gsize data_len;
                                  gchar *encoded_data;
                                  data = _g_dbus_auth_mechanism_server_data_send(mech, &data_len);
                                  encoded_data = hexencode(data);
                                  s = g_strdup_printf("DATA %s\r\n", encoded_data);
                                  g_free(encoded_data);
                                  g_free(data);
                                  debug_print("SERVER: writing `%s'", s);
                                  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) {
                                      g_free(s);
                                      goto out;
                                  }
                                  g_free(s);
                              }
                              goto change_state;
                              break;
                          default: g_assert_not_reached(); break;
                      }
                  }
              } else {
                  g_set_error (error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected line `%s' while in WaitingForAuth state", line);
                  g_free (line);
                  goto out;
              }
              break;
          case SERVER_STATE_WAITING_FOR_DATA:
              debug_print("SERVER: WaitingForData");
              line = _my_g_data_input_stream_read_line(dis, &line_length, cancellable, error);
              debug_print("SERVER: WaitingForData, read `%s'", line);
              if (line == NULL) goto out;
              if (g_str_has_prefix(line, "DATA ")) {
                  gchar *encoded;
                  gchar *decoded_data;
                  gsize decoded_data_len = 0;
                  encoded = g_strdup(line + 5);
                  g_free(line);
                  g_strstrip(encoded);
                  decoded_data = hexdecode(encoded, &decoded_data_len, error);
                  g_free(encoded);
                  if (decoded_data == NULL) {
                      g_prefix_error(error, "DATA response is malformed: ");
                      goto out;
                  }
                  _g_dbus_auth_mechanism_server_data_receive(mech, decoded_data, decoded_data_len);
                  g_free(decoded_data);
                  goto change_state;
              } else {
                  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Unexpected line `%s' while in WaitingForData state", line);
                  g_free(line);
              }
              goto out;
          case SERVER_STATE_WAITING_FOR_BEGIN:
              debug_print("SERVER: WaitingForBegin");
              line = _my_g_input_stream_read_line_safe(g_io_stream_get_input_stream(auth->priv->stream), &line_length, cancellable, error);
              debug_print("SERVER: WaitingForBegin, read `%s'", line);
              if (line == NULL) goto out;
              if (g_strcmp0(line, "BEGIN") == 0) {
                  ret = TRUE;
                  g_free(line);
                  goto out;
              } else if (g_strcmp0(line, "NEGOTIATE_UNIX_FD") == 0) {
                  g_free(line);
                  if (offered_capabilities & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING) {
                      negotiated_capabilities |= G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING;
                      s = "AGREE_UNIX_FD\r\n";
                      debug_print("SERVER: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                  } else {
                      s = "ERROR \"fd passing not offered\"\r\n";
                      debug_print("SERVER: writing `%s'", s);
                      if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
                  }
              } else {
                  g_debug("Unexpected line `%s' while in WaitingForBegin state", line);
                  g_free(line);
                  s = "ERROR \"Unknown Command\"\r\n";
                  debug_print("SERVER: writing `%s'", s);
                  if (!g_data_output_stream_put_string(dos, s, cancellable, error)) goto out;
              }
              break;
          default: g_assert_not_reached(); break;
      }
  }
  g_set_error_literal(error, G_IO_ERROR,G_IO_ERROR_FAILED,"Not implemented (server)");
out:
  if (mech != NULL) g_object_unref(mech);
  if (dis != NULL) g_object_unref(dis);
  if (dos != NULL) g_object_unref(dos);
  if (error != NULL && *error != NULL) ret = FALSE;
  if (ret) {
      if (out_negotiated_capabilities != NULL) *out_negotiated_capabilities = negotiated_capabilities;
      if (out_received_credentials != NULL) *out_received_credentials = credentials != NULL ? g_object_ref(credentials) : NULL;
  }
  if (credentials != NULL) g_object_unref(credentials);
  debug_print("SERVER: Done, authenticated=%d", ret);
  return ret;
}