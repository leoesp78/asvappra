#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_SOCKET_SERVICE_H__
#define __G_SOCKET_SERVICE_H__

#include "gsocketlistener.h"

G_BEGIN_DECLS
#define G_TYPE_SOCKET_SERVICE  (g_socket_service_get_type())
#define G_SOCKET_SERVICE(inst)  (G_TYPE_CHECK_INSTANCE_CAST((inst), G_TYPE_SOCKET_SERVICE, GSocketService))
#define G_SOCKET_SERVICE_CLASS(class)  (G_TYPE_CHECK_CLASS_CAST((class), G_TYPE_SOCKET_SERVICE, GSocketServiceClass))
#define G_IS_SOCKET_SERVICE(inst)  (G_TYPE_CHECK_INSTANCE_TYPE((inst), G_TYPE_SOCKET_SERVICE))
#define G_IS_SOCKET_SERVICE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE((class), G_TYPE_SOCKET_SERVICE))
#define G_SOCKET_SERVICE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS((inst), G_TYPE_SOCKET_SERVICE, GSocketServiceClass))
typedef struct _GSocketServicePrivate GSocketServicePrivate;
typedef struct _GSocketServiceClass GSocketServiceClass;
struct _GSocketServiceClass {
  GSocketListenerClass parent_class;
  gboolean (*incoming)(GSocketService *service, GSocketConnection *connection, GObject *source_object);
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
};
struct _GSocketService {
  GSocketListener parent_instance;
  GSocketServicePrivate *priv;
};
GType g_socket_service_get_type(void);
GSocketService *g_socket_service_new(void);
void g_socket_service_start(GSocketService *service);
void g_socket_service_stop(GSocketService *service);
gboolean g_socket_service_is_active(GSocketService *service);
G_END_DECLS

#endif