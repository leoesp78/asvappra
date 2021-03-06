#include "../glib/glib.h"
#include "../glib/glibintl.h"
#include "config.h"
#include "gdummytlsbackend.h"
#include "gasyncresult.h"
#include "gcancellable.h"
#include "ginitable.h"
#include "gtlsbackend.h"
#include "gtlscertificate.h"
#include "gtlsclientconnection.h"
#include "gtlsserverconnection.h"
#include "gsimpleasyncresult.h"
#include "giomodule.h"
#include "giomodule-priv.h"

static GType _g_dummy_tls_certificate_get_type (void);
static GType _g_dummy_tls_connection_get_type (void);
struct _GDummyTlsBackend {
  GObject parent_instance;
};
static void g_dummy_tls_backend_iface_init(GTlsBackendInterface *iface);
#define g_dummy_tls_backend_get_type _g_dummy_tls_backend_get_type
G_DEFINE_TYPE_WITH_CODE(GDummyTlsBackend, g_dummy_tls_backend, G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE (G_TYPE_TLS_BACKEND, g_dummy_tls_backend_iface_init)
_g_io_modules_ensure_extension_points_registered();
 g_io_extension_point_implement(G_TLS_BACKEND_EXTENSION_POINT_NAME, g_define_type_id, "dummy", -100))
static void g_dummy_tls_backend_init(GDummyTlsBackend *backend) {}
static void
g_dummy_tls_backend_class_init(GDummyTlsBackendClass *backend_class) {}
static void g_dummy_tls_backend_iface_init(GTlsBackendInterface *iface) {
  iface->get_certificate_type = _g_dummy_tls_certificate_get_type;
  iface->get_client_connection_type = _g_dummy_tls_connection_get_type;
  iface->get_server_connection_type = _g_dummy_tls_connection_get_type;
}
typedef struct _GDummyTlsCertificate GDummyTlsCertificate;
typedef struct _GDummyTlsCertificateClass GDummyTlsCertificateClass;
struct _GDummyTlsCertificate {
  GTlsCertificate parent_instance;
};
struct _GDummyTlsCertificateClass {
  GTlsCertificateClass parent_class;
};
enum {
  PROP_CERTIFICATE_0,
  PROP_CERT_CERTIFICATE,
  PROP_CERT_CERTIFICATE_PEM,
  PROP_CERT_PRIVATE_KEY,
  PROP_CERT_PRIVATE_KEY_PEM,
  PROP_CERT_ISSUER
};
static void g_dummy_tls_certificate_initable_iface_init(GInitableIface *iface);
#define g_dummy_tls_certificate_get_type _g_dummy_tls_certificate_get_type
G_DEFINE_TYPE_WITH_CODE(GDummyTlsCertificate, g_dummy_tls_certificate, G_TYPE_TLS_CERTIFICATE,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,
						g_dummy_tls_certificate_initable_iface_init););
static void g_dummy_tls_certificate_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {}
static void g_dummy_tls_certificate_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {}
static void g_dummy_tls_certificate_class_init(GDummyTlsCertificateClass *certificate_class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(certificate_class);
  gobject_class->get_property = g_dummy_tls_certificate_get_property;
  gobject_class->set_property = g_dummy_tls_certificate_set_property;
  g_object_class_override_property(gobject_class, PROP_CERT_CERTIFICATE, "certificate");
  g_object_class_override_property(gobject_class, PROP_CERT_CERTIFICATE_PEM, "certificate-pem");
  g_object_class_override_property(gobject_class, PROP_CERT_PRIVATE_KEY, "private-key");
  g_object_class_override_property(gobject_class, PROP_CERT_PRIVATE_KEY_PEM, "private-key-pem");
  g_object_class_override_property(gobject_class, PROP_CERT_ISSUER, "issuer");
}
static void g_dummy_tls_certificate_init(GDummyTlsCertificate *certificate) {}
static gboolean g_dummy_tls_certificate_initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  g_set_error_literal(error, G_TLS_ERROR, G_TLS_ERROR_UNAVAILABLE, _("TLS support is not available"));
  return FALSE;
}
static void g_dummy_tls_certificate_initable_iface_init(GInitableIface *iface) {
  iface->init = g_dummy_tls_certificate_initable_init;
}
typedef struct _GDummyTlsConnection GDummyTlsConnection;
typedef struct _GDummyTlsConnectionClass GDummyTlsConnectionClass;
struct _GDummyTlsConnection {
  GTlsConnection parent_instance;
};
struct _GDummyTlsConnectionClass {
  GTlsConnectionClass parent_class;
};
enum {
  PROP_CONNECTION_0,
  PROP_CONN_BASE_IO_STREAM,
  PROP_CONN_USE_SYSTEM_CERTDB,
  PROP_CONN_REQUIRE_CLOSE_NOTIFY,
  PROP_CONN_REHANDSHAKE_MODE,
  PROP_CONN_CERTIFICATE,
  PROP_CONN_PEER_CERTIFICATE,
  PROP_CONN_PEER_CERTIFICATE_ERRORS,
  PROP_CONN_VALIDATION_FLAGS,
  PROP_CONN_SERVER_IDENTITY,
  PROP_CONN_USE_SSL3,
  PROP_CONN_ACCEPTED_CAS,
  PROP_CONN_AUTHENTICATION_MODE
};
static void g_dummy_tls_connection_initable_iface_init(GInitableIface *iface);
#define g_dummy_tls_connection_get_type _g_dummy_tls_connection_get_type
G_DEFINE_TYPE_WITH_CODE(GDummyTlsConnection, g_dummy_tls_connection, G_TYPE_TLS_CONNECTION,G_IMPLEMENT_INTERFACE (G_TYPE_TLS_CLIENT_CONNECTION, NULL);
G_IMPLEMENT_INTERFACE(G_TYPE_TLS_SERVER_CONNECTION, NULL);
G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,
g_dummy_tls_connection_initable_iface_init););
static void g_dummy_tls_connection_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {}
static void g_dummy_tls_connection_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {}
static gboolean g_dummy_tls_connection_close(GIOStream *stream, GCancellable *cancellable, GError **error) {
  return TRUE;
}
static void g_dummy_tls_connection_class_init(GDummyTlsConnectionClass *connection_class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(connection_class);
  GIOStreamClass *io_stream_class = G_IO_STREAM_CLASS(connection_class);
  gobject_class->get_property = g_dummy_tls_connection_get_property;
  gobject_class->set_property = g_dummy_tls_connection_set_property;
  io_stream_class->close_fn = g_dummy_tls_connection_close;
  g_object_class_override_property(gobject_class, PROP_CONN_BASE_IO_STREAM, "base-io-stream");
  g_object_class_override_property(gobject_class, PROP_CONN_USE_SYSTEM_CERTDB, "use-system-certdb");
  g_object_class_override_property(gobject_class, PROP_CONN_REQUIRE_CLOSE_NOTIFY, "require-close-notify");
  g_object_class_override_property(gobject_class, PROP_CONN_REHANDSHAKE_MODE, "rehandshake-mode");
  g_object_class_override_property(gobject_class, PROP_CONN_CERTIFICATE, "certificate");
  g_object_class_override_property(gobject_class, PROP_CONN_PEER_CERTIFICATE, "peer-certificate");
  g_object_class_override_property(gobject_class, PROP_CONN_PEER_CERTIFICATE_ERRORS, "peer-certificate-errors");
  g_object_class_override_property(gobject_class, PROP_CONN_VALIDATION_FLAGS, "validation-flags");
  g_object_class_override_property(gobject_class, PROP_CONN_SERVER_IDENTITY, "server-identity");
  g_object_class_override_property(gobject_class, PROP_CONN_USE_SSL3, "use-ssl3");
  g_object_class_override_property(gobject_class, PROP_CONN_ACCEPTED_CAS, "accepted-cas");
  g_object_class_override_property(gobject_class, PROP_CONN_AUTHENTICATION_MODE, "authentication-mode");
}
static void g_dummy_tls_connection_init(GDummyTlsConnection *connection) {}
static gboolean g_dummy_tls_connection_initable_init(GInitable *initable, GCancellable *cancellable, GError **error) {
  g_set_error_literal(error, G_TLS_ERROR, G_TLS_ERROR_UNAVAILABLE, _("TLS support is not available"));
  return FALSE;
}
static void g_dummy_tls_connection_initable_iface_init(GInitableIface  *iface) {
  iface->init = g_dummy_tls_connection_initable_init;
}