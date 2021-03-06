#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_THREADED_SOCKET_SERVICE_H__
#define __G_THREADED_SOCKET_SERVICE_H__

#include "gsocketservice.h"

G_BEGIN_DECLS
#define G_TYPE_THREADED_SOCKET_SERVICE  (g_threaded_socket_service_get_type())
#define G_THREADED_SOCKET_SERVICE(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_THREADED_SOCKET_SERVICE, GThreadedSocketService))
#define G_THREADED_SOCKET_SERVICE_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_THREADED_SOCKET_SERVICE,  GThreadedSocketServiceClass))
#define G_IS_THREADED_SOCKET_SERVICE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_THREADED_SOCKET_SERVICE))
#define G_IS_THREADED_SOCKET_SERVICE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_THREADED_SOCKET_SERVICE))
#define G_THREADED_SOCKET_SERVICE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_THREADED_SOCKET_SERVICE, GThreadedSocketServiceClass))
typedef struct _GThreadedSocketServicePrivate GThreadedSocketServicePrivate;
typedef struct _GThreadedSocketServiceClass GThreadedSocketServiceClass;
struct _GThreadedSocketServiceClass {
  GSocketServiceClass parent_class;
  gboolean (* run)(GThreadedSocketService *service, GSocketConnection *connection, GObject *source_object);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
};
struct _GThreadedSocketService {
  GSocketService parent_instance;
  GThreadedSocketServicePrivate *priv;
};
GType g_threaded_socket_service_get_type(void);
GSocketService *g_threaded_socket_service_new(int max_threads);
G_END_DECLS

#endif