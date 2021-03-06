#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_STRFUNCS_H__
#define __G_STRFUNCS_H__

#include <stdarg.h>
#include "gmacros.h"
#include "gtypes.h"
#include "glib-basic-types.h"

G_BEGIN_DECLS
#define gchar char
#define guchar unsigned char
#define gint int
#define gboolean gint
#define gdouble double
#define gfloat float
#define guint unsigned int
#define gpointer void*
#define gconstpointer const void*
#define gulong unsigned long
typedef enum {
  G_ASCII_ALNUM  = 1 << 0,
  G_ASCII_ALPHA  = 1 << 1,
  G_ASCII_CNTRL  = 1 << 2,
  G_ASCII_DIGIT  = 1 << 3,
  G_ASCII_GRAPH  = 1 << 4,
  G_ASCII_LOWER  = 1 << 5,
  G_ASCII_PRINT  = 1 << 6,
  G_ASCII_PUNCT  = 1 << 7,
  G_ASCII_SPACE  = 1 << 8,
  G_ASCII_UPPER  = 1 << 9,
  G_ASCII_XDIGIT = 1 << 10
} GAsciiType;
extern const guint16 ascii_table_data[256];
extern const guint16* g_ascii_table;
#define g_ascii_isalnum(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_ALNUM) != 0)
#define g_ascii_isalpha(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_ALPHA) != 0)
#define g_ascii_iscntrl(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_CNTRL) != 0)
#define g_ascii_isdigit(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_DIGIT) != 0)
#define g_ascii_isgraph(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_GRAPH) != 0)
#define g_ascii_islower(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_LOWER) != 0)
#define g_ascii_isprint(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_PRINT) != 0)
#define g_ascii_ispunct(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_PUNCT) != 0)
#define g_ascii_isspace(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_SPACE) != 0)
#define g_ascii_isupper(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_UPPER) != 0)
#define g_ascii_isxdigit(c) ((g_ascii_table[(guchar)(c)] & G_ASCII_XDIGIT) != 0)
gchar g_ascii_tolower(gchar c) G_GNUC_CONST;
gchar g_ascii_toupper(gchar c) G_GNUC_CONST;
gint g_ascii_digit_value(gchar c) G_GNUC_CONST;
gint g_ascii_xdigit_value(gchar c) G_GNUC_CONST;
#define	 G_STR_DELIMITERS	"_-|> <."
gchar* g_strdelimit(gchar *string, const gchar *delimiters, gchar new_delimiter);
gchar* g_strcanon(gchar *string, const gchar *valid_chars, gchar substitutor);
G_CONST_RETURN gchar* g_strerror(gint errnum) G_GNUC_CONST;
G_CONST_RETURN gchar* g_strsignal(gint signum) G_GNUC_CONST;
gchar* g_strreverse(gchar *string);
gsize g_strlcpy(gchar *dest, const gchar *src, gsize dest_size);
gsize g_strlcat(gchar *dest, const gchar *src, gsize dest_size);
gchar* g_strstr_len(const gchar *haystack, gssize haystack_len, const gchar *needle);
gchar* g_strrstr(const gchar *haystack, const gchar *needle);
gchar* g_strrstr_len(const gchar *haystack, gssize haystack_len, const gchar *needle);
gboolean g_str_has_suffix(const gchar *str, const gchar *suffix);
gboolean g_str_has_prefix(const gchar *str, const gchar *prefix);
gdouble	g_strtod(const gchar *nptr, gchar **endptr);
gdouble	g_ascii_strtod(const gchar *nptr, gchar **endptr);
guint64	g_ascii_strtoull(const gchar *nptr, gchar **endptr, guint base);
gint64 g_ascii_strtoll(const gchar *nptr, gchar **endptr, guint base);
#define G_ASCII_DTOSTR_BUF_SIZE (29 + 10)
gchar* g_ascii_dtostr(gchar *buffer, gint buf_len, gdouble d);
gchar* g_ascii_formatd(gchar *buffer, gint buf_len, const gchar *format, gdouble d);
gchar* g_strchug(gchar *string);
gchar* g_strchomp(gchar *string);
#define g_strstrip(string)	g_strchomp(g_strchug(string))
gint g_ascii_strcasecmp(const gchar *s1, const gchar *s2);
gint g_ascii_strncasecmp (const gchar *s1, const gchar *s2, gsize n);
gchar* g_ascii_strdown(const gchar *str, gssize len) G_GNUC_MALLOC;
gchar* g_ascii_strup(const gchar *str, gssize len) G_GNUC_MALLOC;
#ifndef G_DISABLE_DEPRECATED
gint g_strcasecmp(const gchar *s1, const gchar *s2);
gint g_strncasecmp(const gchar *s1, const gchar *s2, guint n);
gchar* g_strdown(gchar *string);
gchar* g_strup(gchar *string);
#endif
gchar* g_strdup(const gchar *str) G_GNUC_MALLOC;
gchar* g_strdup_printf(const gchar *format, ...) G_GNUC_PRINTF (1, 2) G_GNUC_MALLOC;
gchar* g_strdup_vprintf(const gchar *format, va_list args) G_GNUC_MALLOC;
gchar* g_strndup(const gchar *str, gsize n) G_GNUC_MALLOC;
gchar* g_strnfill(gsize length, gchar fill_char) G_GNUC_MALLOC;
gchar* g_strconcat(const gchar *string1, ...) G_GNUC_MALLOC G_GNUC_NULL_TERMINATED;
gchar* g_strjoin(const gchar  *separator, ...) G_GNUC_MALLOC G_GNUC_NULL_TERMINATED;
gchar* g_strcompress(const gchar *source) G_GNUC_MALLOC;
gchar* g_strescape(const gchar *source, const gchar *exceptions) G_GNUC_MALLOC;
gpointer g_memdup(gconstpointer mem, guint byte_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(2);
gchar**	g_strsplit(const gchar *string, const gchar *delimiter, gint max_tokens) G_GNUC_MALLOC;
gchar**	g_strsplit_set(const gchar *string, const gchar *delimiters, gint max_tokens) G_GNUC_MALLOC;
gchar* g_strjoinv(const gchar *separator, gchar **str_array) G_GNUC_MALLOC;
void g_strfreev(gchar **str_array);
gchar** g_strdupv(gchar **str_array) G_GNUC_MALLOC;
guint g_strv_length(gchar **str_array);
gchar* g_stpcpy(gchar *dest, const char *src);
G_CONST_RETURN gchar *g_strip_context(const gchar *msgid, const gchar *msgval) G_GNUC_FORMAT(1);
G_CONST_RETURN gchar *g_dgettext(const gchar *domain, const gchar *msgid) G_GNUC_FORMAT(2);
G_CONST_RETURN gchar *g_dcgettext(const gchar *domain, const gchar *msgid, int category) G_GNUC_FORMAT(2);
G_CONST_RETURN gchar *g_dngettext(const gchar *domain, const gchar *msgid, const gchar *msgid_plural, gulong n) G_GNUC_FORMAT(3);
G_CONST_RETURN gchar *g_dpgettext(const gchar *domain, const gchar *msgctxtid, gsize msgidoffset) G_GNUC_FORMAT(2);
G_CONST_RETURN gchar *g_dpgettext2(const gchar *domain, const gchar *context, const gchar *msgid) G_GNUC_FORMAT(3);
G_END_DECLS

#endif