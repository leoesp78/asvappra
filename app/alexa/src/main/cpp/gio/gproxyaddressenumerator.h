#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_PROXY_ADDRESS_ENUMERATOR_H__
#define __G_PROXY_ADDRESS_ENUMERATOR_H__

#include "../gobject/gtype.h"
#include "giotypes.h"
#include "gsocketaddressenumerator.h"

G_BEGIN_DECLS
#define G_TYPE_PROXY_ADDRESS_ENUMERATOR  (g_proxy_address_enumerator_get_type())
#define G_PROXY_ADDRESS_ENUMERATOR(o)  (G_TYPE_CHECK_INSTANCE_CAST((o), G_TYPE_PROXY_ADDRESS_ENUMERATOR, GProxyAddressEnumerator))
#define G_PROXY_ADDRESS_ENUMERATOR_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_PROXY_ADDRESS_ENUMERATOR, GProxyAddressEnumeratorClass))
#define G_IS_PROXY_ADDRESS_ENUMERATOR(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), G_TYPE_PROXY_ADDRESS_ENUMERATOR))
#define G_IS_PROXY_ADDRESS_ENUMERATOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), G_TYPE_PROXY_ADDRESS_ENUMERATOR))
#define G_PROXY_ADDRESS_ENUMERATOR_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), G_TYPE_PROXY_ADDRESS_ENUMERATOR, GProxyAddressEnumeratorClass))
typedef struct _GProxyAddressEnumeratorClass GProxyAddressEnumeratorClass;
typedef struct _GProxyAddressEnumeratorPrivate GProxyAddressEnumeratorPrivate;
struct _GProxyAddressEnumerator {
  GSocketAddressEnumerator parent_instance;
  GProxyAddressEnumeratorPrivate *priv;
};
struct _GProxyAddressEnumeratorClass {
  GSocketAddressEnumeratorClass parent_class;
  void (*_g_reserved1)(void);
  void (*_g_reserved2)(void);
  void (*_g_reserved3)(void);
  void (*_g_reserved4)(void);
  void (*_g_reserved5)(void);
  void (*_g_reserved6)(void);
  void (*_g_reserved7)(void);
};
GType g_proxy_address_enumerator_get_type(void) G_GNUC_CONST;
G_END_DECLS

#endif