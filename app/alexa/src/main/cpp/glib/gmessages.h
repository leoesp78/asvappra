#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_MESSAGES_H__
#define __G_MESSAGES_H__

#include <stdarg.h>
#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

#if (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#pragma GCC system_header
#endif

G_BEGIN_DECLS
gsize	g_printf_string_upper_bound (const gchar* format, va_list	  args);
#define G_LOG_LEVEL_USER_SHIFT  (8)
typedef enum {
  G_LOG_FLAG_RECURSION          = 1 << 0,
  G_LOG_FLAG_FATAL              = 1 << 1,
  G_LOG_LEVEL_ERROR             = 1 << 2,
  G_LOG_LEVEL_CRITICAL          = 1 << 3,
  G_LOG_LEVEL_WARNING           = 1 << 4,
  G_LOG_LEVEL_MESSAGE           = 1 << 5,
  G_LOG_LEVEL_INFO              = 1 << 6,
  G_LOG_LEVEL_DEBUG             = 1 << 7,
  G_LOG_LEVEL_MASK              = ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;
#define g_log_level_debug 1 << 7
#define G_LOG_FATAL_MASK        (G_LOG_FLAG_RECURSION | G_LOG_LEVEL_ERROR)
typedef void (*GLogFunc)(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
guint g_log_set_handler(const gchar *log_domain, GLogLevelFlags log_levels, GLogFunc log_func, gpointer user_data);
void g_log_remove_handler(const gchar *log_domain, guint handler_id);
void g_log_default_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data);
GLogFunc g_log_set_default_handler(GLogFunc log_func, gpointer user_data);
void g_log(const gchar *log_domain, GLogLevelFlags log_level, const gchar *format, ...) G_GNUC_PRINTF(3, 4);
void g_logv(const gchar *log_domain, GLogLevelFlags log_level, const gchar *format, va_list args);
GLogLevelFlags g_log_set_fatal_mask(const gchar *log_domain, GLogLevelFlags fatal_mask);
GLogLevelFlags g_log_set_always_fatal(GLogLevelFlags fatal_mask);
G_GNUC_INTERNAL void _g_log_fallback_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data);
void g_return_if_fail_warning(const char *log_domain, const char *pretty_function, const char *expression);
void g_warn_message(const char *domain, const char *file, int line, const char *func, const char *warnexpr);
#ifndef G_DISABLE_DEPRECATED
void g_assert_warning(const char *log_domain, const char *file, const int line, const char *pretty_function, const char *expression) G_GNUC_NORETURN;
#endif
#define G_LOG(log_domain, log_level, format, ...) g_log(log_domain, log_level, format, ...)
#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN ((gchar*) 0)
#endif
#ifdef G_HAVE_ISO_VARARGS
#define g_error(...)  G_STMT_START {                       \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, __VA_ARGS__);  \
    for ( ; ; );                                             \
} G_STMT_END
#define g_message(...)  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)
#define g_critical(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)
#define g_warning(...)  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define g_debug(...)    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define g_error(format...)    G_STMT_START {          \
	g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format);  \
	for ( ; ; );                                        \
} G_STMT_END
#define g_message(format...) g_log (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format)
#define g_critical(format...) g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, format)
#define g_warning(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format)
#define g_debug(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format)
#else
static void g_error(const gchar *format, ...) {
  va_list args;
  va_start (args, format);
  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format, args);
  va_end(args);
  for( ; ; );
}
static void g_message(const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, args);
  va_end(args);
}
static void g_critical(const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, format, args);
  va_end(args);
}
static void g_warning(const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format, args);
  va_end(args);
}
static void g_debug(const gchar *format, ...) {
  va_list args;
  va_start(args, format);
  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format, args);
  va_end(args);
}
#endif
typedef void (*GPrintFunc)(const gchar *string);
void g_print(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
GPrintFunc g_set_print_handler(GPrintFunc func);
void g_printerr(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
GPrintFunc g_set_printerr_handler(GPrintFunc func);
#define g_warn_if_reached() do { g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, NULL); } while(0);
#define g_warn_if_fail(expr) 	                                             \
do {	                                                	                 \
    if (G_LIKELY(expr)); 	                                            	 \
    else g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, #expr); \
} while(0);
#ifdef G_DISABLE_CHECKS
#define g_return_if_fail(expr)	G_STMT_START{ (void)0; }G_STMT_END
#define g_return_val_if_fail(expr,val)	G_STMT_START{ (void)0; }G_STMT_END
#define g_return_if_reached()	G_STMT_START{ return; }G_STMT_END
#define g_return_val_if_reached(val)  G_STMT_START{ return (val); }G_STMT_END
#else
#ifdef __GNUC__
#define g_return_if_fail(expr) \
G_STMT_START { \
    if (G_LIKELY(expr)) {} else { \
        g_return_if_fail_warning(G_LOG_DOMAIN, __PRETTY_FUNCTION__, #expr);  \
        return; \
    } \
} G_STMT_END
#define g_return_val_if_fail(expr, val)	\
G_STMT_START { \
    if (G_LIKELY(expr)); \
    else { \
        g_return_if_fail_warning(G_LOG_DOMAIN, __PRETTY_FUNCTION__, #expr); \
        return (val); \
    } \
} G_STMT_END
#define g_return_if_reached() \
G_STMT_START { \
     g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,	"file %s: line %d (%s): should not be reached", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
     return; \
} G_STMT_END
#define g_return_val_if_reached(val) \
G_STMT_START { \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "file %s: line %d (%s): should not be reached", __FILE__,  __LINE__,  __PRETTY_FUNCTION__); \
    return (val); \
} G_STMT_END
#else
#define g_return_if_fail(expr) \
G_STMT_START { \
    if (expr); \
    else { \
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "file %s: line %d: assertion `%s' failed", __FILE__, __LINE__, #expr); \
        return;	\
    }; \
}G_STMT_END
#define g_return_val_if_fail(expr, val)	G_STMT_START{		                                                              \
    if (expr);                                                                                                            \
    else {							                                                                                      \
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "file %s: line %d: assertion `%s' failed", __FILE__, __LINE__, #expr); \
        return (val);						                                                                              \
    };                                                                                                                    \
}G_STMT_END
#define g_return_if_reached()		G_STMT_START{		                                                        \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "file %s: line %d: should not be reached", __FILE__,  __LINE__);	\
    return;                                                                                                     \
}G_STMT_END
#define g_return_val_if_reached(val)	G_STMT_START{		                                                   \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "file %s: line %d: should not be reached", __FILE__, __LINE__); \
    return (val);                                                                                              \
}G_STMT_END
#endif
#endif
G_END_DECLS
#endif