#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "../glib/gstdio.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gdbusauthmechanismsha1.h"
#include "gcredentials.h"
#include "gdbuserror.h"
#include "gioenumtypes.h"
#include "gioerror.h"
#include "gdbusprivate.h"

struct _GDBusAuthMechanismSha1Private {
  gboolean is_client;
  gboolean is_server;
  GDBusAuthMechanismState state;
  gchar *to_send;
  gchar *cookie;
  gchar *server_challenge;
};
static gint mechanism_get_priority(void);
static const gchar *mechanism_get_name(void);
static gboolean mechanism_is_supported(GDBusAuthMechanism *mechanism);
static gchar *mechanism_encode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
static gchar *mechanism_decode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len);
static GDBusAuthMechanismState mechanism_server_get_state(GDBusAuthMechanism *mechanism);
static void mechanism_server_initiate(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len);
static void mechanism_server_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
static gchar *mechanism_server_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
static gchar *mechanism_server_get_reject_reason(GDBusAuthMechanism *mechanism);
static void mechanism_server_shutdown(GDBusAuthMechanism *mechanism);
static GDBusAuthMechanismState mechanism_client_get_state(GDBusAuthMechanism *mechanism);
static gchar *mechanism_client_initiate(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len);
static void mechanism_client_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len);
static gchar *mechanism_client_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len);
static void mechanism_client_shutdown(GDBusAuthMechanism *mechanism);
G_DEFINE_TYPE(GDBusAuthMechanismSha1, _g_dbus_auth_mechanism_sha1, G_TYPE_DBUS_AUTH_MECHANISM);
static void _g_dbus_auth_mechanism_sha1_finalize(GObject *object) {
  GDBusAuthMechanismSha1 *mechanism = G_DBUS_AUTH_MECHANISM_SHA1(object);
  g_free(mechanism->priv->to_send);
  g_free(mechanism->priv->cookie);
  g_free(mechanism->priv->server_challenge);
  if (G_OBJECT_CLASS(_g_dbus_auth_mechanism_sha1_parent_class)->finalize != NULL) G_OBJECT_CLASS(_g_dbus_auth_mechanism_sha1_parent_class)->finalize(object);
}
static void_g_dbus_auth_mechanism_sha1_class_init(GDBusAuthMechanismSha1Class *klass) {
  GObjectClass *gobject_class;
  GDBusAuthMechanismClass *mechanism_class;
  g_type_class_add_private(klass, sizeof(GDBusAuthMechanismSha1Private));
  gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->finalize = _g_dbus_auth_mechanism_sha1_finalize;
  mechanism_class = G_DBUS_AUTH_MECHANISM_CLASS(klass);
  mechanism_class->get_priority = mechanism_get_priority;
  mechanism_class->get_name = mechanism_get_name;
  mechanism_class->is_supported = mechanism_is_supported;
  mechanism_class->encode_data = mechanism_encode_data;
  mechanism_class->decode_data = mechanism_decode_data;
  mechanism_class->server_get_state = mechanism_server_get_state;
  mechanism_class->server_initiate = mechanism_server_initiate;
  mechanism_class->server_data_receive = mechanism_server_data_receive;
  mechanism_class->server_data_send = mechanism_server_data_send;
  mechanism_class->server_get_reject_reason = mechanism_server_get_reject_reason;
  mechanism_class->server_shutdown = mechanism_server_shutdown;
  mechanism_class->client_get_state = mechanism_client_get_state;
  mechanism_class->client_initiate = mechanism_client_initiate;
  mechanism_class->client_data_receive = mechanism_client_data_receive;
  mechanism_class->client_data_send = mechanism_client_data_send;
  mechanism_class->client_shutdown = mechanism_client_shutdown;
}
static void _g_dbus_auth_mechanism_sha1_init(GDBusAuthMechanismSha1 *mechanism) {
  mechanism->priv = G_TYPE_INSTANCE_GET_PRIVATE(mechanism, G_TYPE_DBUS_AUTH_MECHANISM_SHA1, GDBusAuthMechanismSha1Private);
}
static gint mechanism_get_priority(void) {
  return 0;
}
static const gchar *mechanism_get_name(void) {
  return "DBUS_COOKIE_SHA1";
}
static gboolean mechanism_is_supported(GDBusAuthMechanism *mechanism) {
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), FALSE);
  return TRUE;
}
static gchar *mechanism_encode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len) {
  return NULL;
}
static gchar *mechanism_decode_data(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len, gsize *out_data_len) {
  return NULL;
}
static gint random_ascii(void) {
  gint ret;
  ret = g_random_int_range(0, 60);
  if (ret < 25) ret += 'A';
  else if (ret < 50) ret += 'a' - 25;
  else ret += '0' - 50;
  return ret;
}
static gchar *random_ascii_string(guint len) {
  GString *challenge;
  guint n;
  challenge = g_string_new(NULL);
  for (n = 0; n < len; n++) g_string_append_c(challenge, random_ascii());
  return g_string_free(challenge, FALSE);
}
static gchar *random_blob(guint len) {
  GString *challenge;
  guint n;
  challenge = g_string_new (NULL);
  for (n = 0; n < len; n++)
    g_string_append_c (challenge, g_random_int_range (0, 256));
  return g_string_free (challenge, FALSE);
}
static gchar *ensure_keyring_directory(GError **error) {
  gchar *path;
  const gchar *e;
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  e = g_getenv("G_DBUS_COOKIE_SHA1_KEYRING_DIR");
  if (e != NULL) path = g_strdup(e);
  else path = g_build_filename(g_get_home_dir(), ".dbus-keyrings", NULL);
  if (g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
      if (g_getenv("G_DBUS_COOKIE_SHA1_KEYRING_DIR_IGNORE_PERMISSION") == NULL) {
      #ifndef G_OS_UNIX
          struct stat statbuf;
          if (stat(path, &statbuf) != 0) {
              g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error statting directory `%s': %s"), path, strerror(errno));
              g_free (path);
              path = NULL;
              goto out;
          }
          if ((statbuf.st_mode  & 0777) != 0700) {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Permissions on directory `%s' are malformed. Expected mode 0700, got 0%o"),
                          path, statbuf.st_mode & 0777);
              g_free(path);
              path = NULL;
              goto out;
          }
      #else
          #ifdef __GNUC__
          #warning Please implement permission checking on this non-UNIX platform
          #endif
      #endif
      }
        goto out;
  }
  if (g_mkdir (path, 0700) != 0) {
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error creating directory `%s': %s"), path, strerror(errno));
      g_free (path);
      path = NULL;
      goto out;
  }
out:
  return path;
}
static void append_nibble(GString *s, gint val) {
  g_string_append_c (s, val >= 10 ? ('a' + val - 10) : ('0' + val));
}
static gchar *hexencode(const gchar *str, gssize len) {
  guint n;
  GString *s;
  if (len == -1) len = strlen(str);
  s = g_string_new(NULL);
  for (n = 0; n < len; n++) {
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
static gchar *keyring_lookup_entry(const gchar *cookie_context, gint cookie_id, GError **error) {
  gchar *ret;
  gchar *keyring_dir;
  gchar *contents;
  gchar *path;
  guint n;
  gchar **lines;
  g_return_val_if_fail(cookie_context != NULL, NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  ret = NULL;
  path = NULL;
  contents = NULL;
  lines = NULL;
  keyring_dir = ensure_keyring_directory(error);
  if (keyring_dir == NULL) goto out;
  path = g_build_filename(keyring_dir, cookie_context, NULL);
  if (!g_file_get_contents(path, &contents, NULL, error)) {
      g_prefix_error(error, _("Error opening keyring `%s' for reading: "), path);
      goto out;
  }
  g_assert(contents != NULL);
  lines = g_strsplit(contents, "\n", 0);
  for (n = 0; lines[n] != NULL; n++) {
      const gchar *line = lines[n];
      gchar **tokens;
      gchar *endp;
      gint line_id;
      guint64 line_when;
      if (line[0] == '\0') continue;
      tokens = g_strsplit(line, " ", 0);
      if (g_strv_length(tokens) != 3) {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Line %d of the keyring at `%s' with content `%s' is malformed"), n + 1, path, line);
          g_strfreev(tokens);
          goto out;
      }
      line_id = g_ascii_strtoll(tokens[0], &endp, 10);
      if (*endp != '\0') {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("First token of line %d of the keyring at `%s' with content `%s' is malformed"), n + 1,
                      path, line);
          g_strfreev(tokens);
          goto out;
      }
      line_when = g_ascii_strtoll(tokens[1], &endp, 10);
      if (*endp != '\0') {
          g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Second token of line %d of the keyring at `%s' with content `%s' is malformed"), n + 1,
                      path, line);
          g_strfreev(tokens);
          goto out;
      }
      if (line_id == cookie_id) {
          ret = tokens[2];
          tokens[2] = NULL;
          g_strfreev(tokens);
          goto out;
      }
      g_strfreev(tokens);
  }
  g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Didn't find cookie with id %d in the keyring at `%s'"), cookie_id, path);
 out:
  g_free(keyring_dir);
  g_free(path);
  g_free(contents);
  g_strfreev(lines);
  return ret;
}
static void _log(const gchar *message, ...) {
  gchar *s;
  va_list var_args;
  va_start(var_args, message);
  s = g_strdup_vprintf(message, var_args);
  va_end(var_args);
  g_printerr("GDBus-DBUS_COOKIE_SHA1: %s\n", s);
  g_free(s);
}
static gint keyring_acquire_lock(const gchar *path, GError **error) {
  gchar *lock;
  gint ret;
  guint num_tries;
  guint num_create_tries;
  g_return_val_if_fail(path != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = -1;
  lock = g_strdup_printf("%s.lock", path);
  num_create_tries = 0;
#ifdef EEXISTS
again:
#endif
  num_tries = 0;
  while(g_file_test(lock, G_FILE_TEST_EXISTS)) {
      g_usleep(1000*10);
      num_tries++;
      if (num_tries == 50) {
          if (g_unlink(lock) != 0) {
              g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error deleting stale lock file `%s': %s"), lock, strerror(errno));
              goto out;
          }
          _log("Deleted stale lock file `%s'", lock);
          break;
      }
  }
  ret = g_open(lock, O_CREAT |
           #ifdef O_EXCL
               O_EXCL,
           #else
                0,
           #endif
                0700);
  if (ret == -1) {
  #ifdef EEXISTS
      if (errno == EEXISTS) {
          num_create_tries++;
          if (num_create_tries < 5) goto again;
      }
  #endif
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error creating lock file `%s': %s"), lock, strerror(errno));
      goto out;
  }
 out:
  g_free(lock);
  return ret;
}
static gboolean keyring_release_lock(const gchar *path, gint lock_fd, GError **error) {
  gchar *lock;
  gboolean ret;
  g_return_val_if_fail(path != NULL, FALSE);
  g_return_val_if_fail(lock_fd != -1, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
  lock = g_strdup_printf ("%s.lock", path);
  if (close (lock_fd) != 0) {
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error closing (unlinked) lock file `%s': %s"), lock, strerror (errno));
      goto out;
  }
  if (g_unlink (lock) != 0) {
      g_set_error(error, G_IO_ERROR, g_io_error_from_errno(errno), _("Error unlinking lock file `%s': %s"), lock, strerror(errno));
      goto out;
  }
  ret = TRUE;
out:
  g_free(lock);
  return ret;
}
static gboolean keyring_generate_entry(const gchar *cookie_context, gint *out_id, gchar **out_cookie, GError **error) {
  gboolean ret;
  gchar *keyring_dir;
  gchar *path;
  gchar *contents;
  GError *local_error;
  gchar **lines;
  gint max_line_id;
  GString *new_contents;
  guint64 now;
  gboolean have_id;
  gint use_id;
  gchar *use_cookie;
  gboolean changed_file;
  gint lock_fd;
  g_return_val_if_fail(cookie_context != NULL, FALSE);
  g_return_val_if_fail(out_id != NULL, FALSE);
  g_return_val_if_fail(out_cookie != NULL, FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  ret = FALSE;
  path = NULL;
  contents = NULL;
  lines = NULL;
  new_contents = NULL;
  have_id = FALSE;
  use_cookie = NULL;
  lock_fd = -1;
  keyring_dir = ensure_keyring_directory(error);
  if (keyring_dir == NULL) goto out;
  path = g_build_filename(keyring_dir, cookie_context, NULL);
  lock_fd = keyring_acquire_lock(path, error);
  if (lock_fd == -1) goto out;
  local_error = NULL;
  contents = NULL;
  if (!g_file_get_contents(path, &contents, NULL, &local_error)) {
      if (local_error->domain == G_FILE_ERROR && local_error->code == G_FILE_ERROR_NOENT) g_error_free(local_error);
      else {
          g_propagate_prefixed_error(error, local_error, _("Error opening keyring `%s' for writing: "), path);
          goto out;
      }
  }
  new_contents = g_string_new(NULL);
  now = time(NULL);
  changed_file = FALSE;
  max_line_id = 0;
  if (contents != NULL) {
      guint n;
      lines = g_strsplit(contents, "\n", 0);
      for (n = 0; lines[n] != NULL; n++) {
          const gchar *line = lines[n];
          gchar **tokens;
          gchar *endp;
          gint line_id;
          guint64 line_when;
          gboolean keep_entry;
          if (line[0] == '\0') continue;
          tokens = g_strsplit(line, " ", 0);
          if (g_strv_length(tokens) != 3) {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Line %d of the keyring at `%s' with content `%s' is malformed"), n + 1, path, line);
              g_strfreev(tokens);
              goto out;
          }
          line_id = g_ascii_strtoll (tokens[0], &endp, 10);
          if (*endp != '\0') {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("First token of line %d of the keyring at `%s' with content `%s' is malformed"),
                          n + 1, path, line);
              g_strfreev(tokens);
              goto out;
          }
          line_when = g_ascii_strtoll(tokens[1], &endp, 10);
          if (*endp != '\0') {
              g_set_error(error, G_IO_ERROR,G_IO_ERROR_FAILED, _("Second token of line %d of the keyring at `%s' with content `%s' is malformed"),
                          n + 1, path, line);
              g_strfreev(tokens);
              goto out;
          }
          keep_entry = TRUE;
          if (line_when > now) {
              if (line_when - now > 24*60*60) {
                  keep_entry = FALSE;
                  _log("Deleted SHA1 cookie from %" G_GUINT64_FORMAT " seconds in the future", line_when - now);
              }
          } else {
              if (now - line_when > 15*60) keep_entry = FALSE;
          }
          if (!keep_entry) changed_file = FALSE;
          else {
              g_string_append_printf(new_contents,"%d %" G_GUINT64_FORMAT " %s\n", line_id, line_when, tokens[2]);
              max_line_id = MAX(line_id, max_line_id);
              if (now - line_when < 10 * 60) {
                  if (!have_id) {
                      use_id = line_id;
                      use_cookie = tokens[2];
                      tokens[2] = NULL;
                      have_id = TRUE;
                  }
              }
          }
          g_strfreev(tokens);
      }
  }
  ret = TRUE;
  if (have_id) {
      *out_id = use_id;
      *out_cookie = use_cookie;
      use_cookie = NULL;
  } else {
      gchar *raw_cookie;
      *out_id = max_line_id + 1;
      raw_cookie = random_blob(32);
      *out_cookie = hexencode(raw_cookie, 32);
      g_free(raw_cookie);
      g_string_append_printf(new_contents, "%d %" G_GUINT64_FORMAT " %s\n", *out_id, (guint64)time(NULL), *out_cookie);
      changed_file = TRUE;
  }
  if (changed_file) {
      if (!g_file_set_contents(path, new_contents->str,-1, error)) {
          *out_id = 0;
          *out_cookie = 0;
          g_free(*out_cookie);
          ret = FALSE;
          goto out;
      }
  }
out:
  if (lock_fd != -1) {
      GError *local_error;
      local_error = NULL;
      if (!keyring_release_lock(path, lock_fd, &local_error)) {
          if (error != NULL) {
              if (*error == NULL) *error = local_error;
              else g_prefix_error(error, _("(Additionally, releasing the lock for `%s' also failed: %s) "), path, local_error->message);
          } else g_error_free(local_error);
      }
  }
  g_free(keyring_dir);
  g_free(path);
  g_strfreev(lines);
  g_free(contents);
  if (new_contents != NULL) g_string_free(new_contents, TRUE);
  g_free(use_cookie);
  return ret;
}
static gchar *generate_sha1(const gchar *server_challenge, const gchar *client_challenge, const gchar *cookie) {
  GString *str;
  gchar *sha1;
  str = g_string_new(server_challenge);
  g_string_append_c(str, ':');
  g_string_append(str, client_challenge);
  g_string_append_c(str, ':');
  g_string_append(str, cookie);
  sha1 = g_compute_checksum_for_string(G_CHECKSUM_SHA1, str->str, -1);
  g_string_free(str, TRUE);
  return sha1;
}
static GDBusAuthMechanismState mechanism_server_get_state(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1 (mechanism), G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  return m->priv->state;
}
static void mechanism_server_initiate(GDBusAuthMechanism *mechanism, const gchar *initial_response, gsize initial_response_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism));
  g_return_if_fail(!m->priv->is_server && !m->priv->is_client);
  m->priv->is_server = TRUE;
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
  if (initial_response != NULL && strlen (initial_response) > 0) {
  #ifndef G_OS_UNIX
      gint64 uid;
      gchar *endp;
      uid = g_ascii_strtoll(initial_response, &endp, 10);
      if (*endp == '\0') {
          if (uid == getuid()) m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND;
      }
  #elif defined(G_OS_WIN32)
      gchar *sid;
      sid = _g_dbus_win32_get_user_sid();
      if (g_strcmp0(initial_response, sid) == 0) m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND;
      g_free(sid);
  #else
      #error Please implement for your OS
  #endif
  }
}
static void mechanism_server_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  gchar **tokens;
  const gchar *client_challenge;
  const gchar *alleged_sha1;
  gchar *sha1;
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism));
  g_return_if_fail(m->priv->is_server && !m->priv->is_client);
  g_return_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA);
  tokens = NULL;
  sha1 = NULL;
  tokens = g_strsplit(data, " ", 0);
  if (g_strv_length(tokens) != 2) {
      g_warning("Malformed data `%s'", data);
      m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
      goto out;
  }
  client_challenge = tokens[0];
  alleged_sha1 = tokens[1];
  sha1 = generate_sha1(m->priv->server_challenge, client_challenge, m->priv->cookie);
  if (g_strcmp0(sha1, alleged_sha1) == 0) m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED;
  else m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
out:
  g_strfreev(tokens);
  g_free(sha1);
}
static gchar *mechanism_server_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  gchar *s;
  gint cookie_id;
  const gchar *cookie_context;
  GError *error;
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND, NULL);
  s = NULL;
  cookie_context = "org_gtk_gdbus_general";
  cookie_id = -1;
  error = NULL;
  if (!keyring_generate_entry(cookie_context, &cookie_id, &m->priv->cookie, &error)) {
      g_warning("Error adding entry to keyring: %s", error->message);
      g_error_free(error);
      m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
      goto out;
  }
  m->priv->server_challenge = random_ascii_string(16);
  s = g_strdup_printf("%s %d %s", cookie_context, cookie_id, m->priv->server_challenge);
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA;
out:
  return s;
}
static gchar *mechanism_server_get_reject_reason(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_server && !m->priv->is_client, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_REJECTED, NULL);
  g_assert_not_reached();
  return NULL;
}
static void mechanism_server_shutdown(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism));
  g_return_if_fail(m->priv->is_server && !m->priv->is_client);
  m->priv->is_server = FALSE;
}
static GDBusAuthMechanismState mechanism_client_get_state(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  g_return_val_if_fail(m->priv->is_client && !m->priv->is_server, G_DBUS_AUTH_MECHANISM_STATE_INVALID);
  return m->priv->state;
}
static gchar *mechanism_client_initiate(GDBusAuthMechanism *mechanism, gsize *out_initial_response_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  gchar *initial_response;
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), NULL);
  g_return_val_if_fail(!m->priv->is_server && !m->priv->is_client, NULL);
  m->priv->is_client = TRUE;
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA;
  *out_initial_response_len = -1;
#ifndef G_OS_UNIX
  initial_response = g_strdup_printf("%" G_GINT64_FORMAT, (gint64)getuid());
#elif defined(G_OS_WIN32)
initial_response = _g_dbus_win32_get_user_sid();
#else
#error Please implement for your OS
#endif
  g_assert(initial_response != NULL);
  return initial_response;
}
static void mechanism_client_data_receive(GDBusAuthMechanism *mechanism, const gchar *data, gsize data_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  gchar **tokens;
  const gchar *cookie_context;
  guint cookie_id;
  const gchar *server_challenge;
  gchar *client_challenge;
  gchar *endp;
  gchar *cookie;
  GError *error;
  gchar *sha1;
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism));
  g_return_if_fail(m->priv->is_client && !m->priv->is_server);
  g_return_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_WAITING_FOR_DATA);
  tokens = NULL;
  cookie = NULL;
  client_challenge = NULL;
  tokens = g_strsplit(data, " ", 0);
  if (g_strv_length(tokens) != 3) {
      g_warning("Malformed data `%s'", data);
      m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
      goto out;
  }
  cookie_context = tokens[0];
  cookie_id = g_ascii_strtoll(tokens[1], &endp, 10);
  if (*endp != '\0') {
      g_warning("Malformed cookie_id `%s'", tokens[1]);
      m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
      goto out;
  }
  server_challenge = tokens[2];
  error = NULL;
  cookie = keyring_lookup_entry(cookie_context, cookie_id, &error);
  if (cookie == NULL) {
      g_warning("Problems looking up entry in keyring: %s", error->message);
      g_error_free(error);
      m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_REJECTED;
      goto out;
  }
  client_challenge = random_ascii_string(16);
  sha1 = generate_sha1(server_challenge, client_challenge, cookie);
  m->priv->to_send = g_strdup_printf("%s %s", client_challenge, sha1);
  g_free(sha1);
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND;
out:
  g_strfreev(tokens);
  g_free(cookie);
  g_free(client_challenge);
}
static gchar *mechanism_client_data_send(GDBusAuthMechanism *mechanism, gsize *out_data_len) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_val_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism), NULL);
  g_return_val_if_fail(m->priv->is_client && !m->priv->is_server, NULL);
  g_return_val_if_fail(m->priv->state == G_DBUS_AUTH_MECHANISM_STATE_HAVE_DATA_TO_SEND, NULL);
  g_assert(m->priv->to_send != NULL);
  m->priv->state = G_DBUS_AUTH_MECHANISM_STATE_ACCEPTED;
  return g_strdup(m->priv->to_send);
}
static void mechanism_client_shutdown(GDBusAuthMechanism *mechanism) {
  GDBusAuthMechanismSha1 *m = G_DBUS_AUTH_MECHANISM_SHA1(mechanism);
  g_return_if_fail(G_IS_DBUS_AUTH_MECHANISM_SHA1(mechanism));
  g_return_if_fail(m->priv->is_client && !m->priv->is_server);
  m->priv->is_client = FALSE;
}