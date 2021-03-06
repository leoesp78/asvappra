#include <string.h>
#include <time.h>
#include "../glib.h"
#include "../gstdio.h"

#define ASSERT_DATE(dt,y,m,d) G_STMT_START { \
  g_assert_cmpint ((y), ==, g_date_time_get_year ((dt))); \
  g_assert_cmpint ((m), ==, g_date_time_get_month ((dt))); \
  g_assert_cmpint ((d), ==, g_date_time_get_day_of_month ((dt))); \
} G_STMT_END
#define ASSERT_TIME(dt,H,M,S) G_STMT_START { \
  g_assert_cmpint ((H), ==, g_date_time_get_hour ((dt))); \
  g_assert_cmpint ((M), ==, g_date_time_get_minute ((dt))); \
  g_assert_cmpint ((S), ==, g_date_time_get_second ((dt))); \
} G_STMT_END
static void get_localtime_tm (time_t time_, struct tm *retval) {
#ifdef HAVE_LOCALTIME_R
  localtime_r(&time_, retval);
#else
  {
      struct tm *ptm = localtime (&time_);
      if (ptm == NULL) {
      #ifndef G_DISABLE_CHECKS
          g_return_if_fail_warning (G_LOG_DOMAIN, G_STRFUNC, "ptm != NULL");
      #endif
          retval->tm_mon = 0;
          retval->tm_mday = 1;
          retval->tm_year = 100;
      } else memcpy ((void*)retval, (void*)ptm, sizeof(struct tm));
  }
#endif
}
static void test_GDateTime_now(void) {
  GDateTime *dt;
  struct tm tm;
  memset(&tm, 0, sizeof (tm));
  get_localtime_tm(time(NULL), &tm);
  dt = g_date_time_new_now_local();
  g_assert_cmpint(g_date_time_get_year(dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint(g_date_time_get_month(dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, tm.tm_mday);
  g_assert_cmpint(g_date_time_get_hour(dt), ==, tm.tm_hour);
  g_assert_cmpint(g_date_time_get_minute(dt), ==, tm.tm_min);
  g_assert_cmpint(g_date_time_get_second(dt), >=, tm.tm_sec);
  g_date_time_unref(dt);
}
static void test_GDateTime_new_from_unix (void) {
  GDateTime *dt;
  struct tm  tm;
  time_t t;
  memset(&tm, 0, sizeof(tm));
  t = time(NULL);
  get_localtime_tm(t, &tm);
  dt = g_date_time_new_from_unix_local(t);
  g_assert_cmpint(g_date_time_get_year(dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint(g_date_time_get_month(dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, tm.tm_mday);
  g_assert_cmpint(g_date_time_get_hour(dt), ==, tm.tm_hour);
  g_assert_cmpint(g_date_time_get_minute(dt), ==, tm.tm_min);
  g_assert_cmpint(g_date_time_get_second(dt), ==, tm.tm_sec);
  g_date_time_unref(dt);
  memset(&tm, 0, sizeof(tm));
  tm.tm_year = 70;
  tm.tm_mday = 1;
  tm.tm_mon = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  t = mktime(&tm);
  dt = g_date_time_new_from_unix_local(t);
  g_assert_cmpint(g_date_time_get_year(dt), ==, 1970);
  g_assert_cmpint(g_date_time_get_month(dt), ==, 1);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, 1);
  g_assert_cmpint(g_date_time_get_hour(dt), ==, 0);
  g_assert_cmpint(g_date_time_get_minute(dt), ==, 0);
  g_assert_cmpint(g_date_time_get_second(dt), ==, 0);
  g_date_time_unref(dt);
}
static void test_GDateTime_compare(void) {
  GDateTime *dt1, *dt2;
  gint i;
  dt1 = g_date_time_new_utc(2000, 1, 1, 0, 0, 0);
  for (i = 1; i < 2000; i++) {
      dt2 = g_date_time_new_utc(i, 12, 31, 0, 0, 0);
      g_assert_cmpint(1, ==, g_date_time_compare(dt1, dt2));
      g_date_time_unref(dt2);
  }
  dt2 = g_date_time_new_utc(1999, 12, 31, 23, 59, 59);
  g_assert_cmpint(1, ==, g_date_time_compare(dt1, dt2));
  g_date_time_unref(dt2);
  dt2 = g_date_time_new_utc(2000, 1, 1, 0, 0, 1);
  g_assert_cmpint(-1, ==, g_date_time_compare(dt1, dt2));
  g_date_time_unref(dt2);
  dt2 = g_date_time_new_utc(2000, 1, 1, 0, 0, 0);
  g_assert_cmpint(0, ==, g_date_time_compare(dt1, dt2));
  g_date_time_unref(dt2);
  g_date_time_unref(dt1);
}
static void test_GDateTime_equal(void) {
  GDateTime *dt1, *dt2;
  GTimeZone *tz;
  dt1 = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  dt2 = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  g_assert(g_date_time_equal(dt1, dt2));
  g_date_time_unref(dt1);
  g_date_time_unref(dt2);
  dt1 = g_date_time_new_local(2009, 10, 18, 0, 0, 0);
  dt2 = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  g_assert(!g_date_time_equal(dt1, dt2));
  g_date_time_unref(dt1);
  g_date_time_unref(dt2);
  tz = g_time_zone_new("-03:00");
  dt1 = g_date_time_new(tz, 2010, 5, 24,  8, 0, 0);
  g_time_zone_unref(tz);
  g_assert_cmpint(g_date_time_get_utc_offset (dt1) / G_USEC_PER_SEC, ==, (-3 * 3600));
  dt2 = g_date_time_new_utc(2010, 5, 24, 11, 0, 0);
  g_assert_cmpint(g_date_time_get_utc_offset (dt2), ==, 0);
  g_assert(g_date_time_equal (dt1, dt2));
  g_date_time_unref(dt1);
  tz = g_time_zone_new("America/Recife");
  dt1 = g_date_time_new(tz, 2010, 5, 24,  8, 0, 0);
  g_time_zone_unref(tz);
  g_assert_cmpint(g_date_time_get_utc_offset (dt1) / G_USEC_PER_SEC, ==, (-3 * 3600));
  g_assert(g_date_time_equal (dt1, dt2));
  g_date_time_unref(dt1);
  g_date_time_unref(dt2);
}
static void test_GDateTime_get_day_of_week (void) {
  GDateTime *dt;
  dt = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  g_assert_cmpint(1, ==, g_date_time_get_day_of_week(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_local(2000, 10, 1, 0, 0, 0);
  g_assert_cmpint(7, ==, g_date_time_get_day_of_week(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_get_day_of_month(void) {
  GDateTime *dt;
  dt = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, 19);
  g_date_time_unref(dt);
  dt = g_date_time_new_local(1400, 3, 12, 0, 0, 0);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, 12);
  g_date_time_unref(dt);
  dt = g_date_time_new_local(1800, 12, 31, 0, 0, 0);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, 31);
  g_date_time_unref(dt);
  dt = g_date_time_new_local(1000, 1, 1, 0, 0, 0);
  g_assert_cmpint(g_date_time_get_day_of_month(dt), ==, 1);
  g_date_time_unref(dt);
}
static void test_GDateTime_get_hour(void) {
  GDateTime *dt;
  dt = g_date_time_new_utc(2009, 10, 19, 15, 13, 11);
  g_assert_cmpint(15, ==, g_date_time_get_hour(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_utc(100, 10, 19, 1, 0, 0);
  g_assert_cmpint(1, ==, g_date_time_get_hour(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_utc(100, 10, 19, 0, 0, 0);
  g_assert_cmpint(0, ==, g_date_time_get_hour(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_utc(100, 10, 1, 23, 59, 59);
  g_assert_cmpint(23, ==, g_date_time_get_hour(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_get_microsecond(void) {
  GTimeVal   tv;
  GDateTime *dt;
  g_get_current_time(&tv);
  dt = g_date_time_new_from_timeval_local(&tv);
  g_assert_cmpint(tv.tv_usec, ==, g_date_time_get_microsecond(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_get_year(void) {
  GDateTime *dt;
  dt = g_date_time_new_local(2009, 1, 1, 0, 0, 0);
  g_assert_cmpint(2009, ==, g_date_time_get_year(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_local(1, 1, 1, 0, 0, 0);
  g_assert_cmpint(1, ==, g_date_time_get_year(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_local(13, 1, 1, 0, 0, 0);
  g_assert_cmpint(13, ==, g_date_time_get_year(dt));
  g_date_time_unref(dt);
  dt = g_date_time_new_local(3000, 1, 1, 0, 0, 0);
  g_assert_cmpint(3000, ==, g_date_time_get_year(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_hash(void) {
  GHashTable *h;
  h = g_hash_table_new_full(g_date_time_hash, g_date_time_equal, (GDestroyNotify)g_date_time_unref,NULL);
  g_hash_table_insert(h, g_date_time_new_now_local(), NULL);
  g_hash_table_remove_all(h);
  g_hash_table_destroy(h);
}
static void test_GDateTime_new_from_timeval(void) {
  GDateTime *dt;
  GTimeVal   tv, tv2;
  g_get_current_time(&tv);
  dt = g_date_time_new_from_timeval_local(&tv);
  if (g_test_verbose())
      g_print("\nDT%04d-%02d-%02dT%02d:%02d:%02d%s\n", g_date_time_get_year(dt), g_date_time_get_month(dt), g_date_time_get_day_of_month(dt),
              g_date_time_get_hour(dt), g_date_time_get_minute(dt), g_date_time_get_second(dt), g_date_time_get_timezone_abbreviation(dt));
  g_date_time_to_timeval(dt, &tv2);
  g_assert_cmpint(tv.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint(tv.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref(dt);
}
static void test_GDateTime_to_unix(void) {
  GDateTime *dt;
  time_t     t;
  t = time(NULL);
  dt = g_date_time_new_from_unix_local(t);
  g_assert_cmpint(g_date_time_to_unix(dt), ==, t);
  g_date_time_unref(dt);
}
static void test_GDateTime_add_years(void) {
  GDateTime *dt, *dt2;
  dt = g_date_time_new_local(2009, 10, 19, 0, 0, 0);
  dt2 = g_date_time_add_years(dt, 1);
  g_assert_cmpint(2010, ==, g_date_time_get_year(dt2));
  g_date_time_unref(dt);
  g_date_time_unref(dt2);
}
static void test_GDateTime_add_months(void) {
#define TEST_ADD_MONTHS(y,m,d,a,ny,nm,nd)         \
  G_STMT_START {                                  \
      GDateTime *dt, *dt2;                        \
      dt = g_date_time_new_utc(y, m, d, 0, 0, 0); \
      dt2 = g_date_time_add_months(dt, a);        \
      ASSERT_DATE(dt2, ny, nm, nd);               \
      g_date_time_unref(dt);                      \
      g_date_time_unref(dt2);                     \
  } G_STMT_END
  TEST_ADD_MONTHS(2009, 12, 31,    1, 2010, 1, 31);
  TEST_ADD_MONTHS(2009, 12, 31,    1, 2010, 1, 31);
  TEST_ADD_MONTHS(2009,  6, 15,    1, 2009, 7, 15);
  TEST_ADD_MONTHS(1400,  3,  1,    1, 1400, 4,  1);
  TEST_ADD_MONTHS(1400,  1, 31,    1, 1400, 2, 28);
  TEST_ADD_MONTHS(1400,  1, 31, 7200, 2000, 1, 31);
  TEST_ADD_MONTHS(2008,  2, 29,   12, 2009, 2, 28);
  TEST_ADD_MONTHS(2000,  8, 16,   -5, 2000, 3, 16);
  TEST_ADD_MONTHS(2000,  8, 16,  -12, 1999, 8, 16);
  TEST_ADD_MONTHS(2011,  2,  1,  -13, 2010, 1,  1);
  TEST_ADD_MONTHS(1776,  7,  4, 1200, 1876, 7,  4);
}
static void test_GDateTime_add_days(void) {
#define TEST_ADD_DAYS(y,m,d,a,ny,nm,nd)                           \
  G_STMT_START {                                                  \
      GDateTime *dt, *dt2;                                        \
      dt = g_date_time_new_local(y, m, d, 0, 0, 0);               \
      dt2 = g_date_time_add_days(dt, a);                          \
      g_assert_cmpint(ny, ==, g_date_time_get_year(dt2));         \
      g_assert_cmpint(nm, ==, g_date_time_get_month(dt2));        \
      g_assert_cmpint(nd, ==, g_date_time_get_day_of_month(dt2)); \
      g_date_time_unref(dt);                                      \
      g_date_time_unref(dt2);                                     \
  } G_STMT_END
  TEST_ADD_DAYS(2009, 1, 31, 1, 2009, 2, 1);
  TEST_ADD_DAYS(2009, 2, 1, -1, 2009, 1, 31);
  TEST_ADD_DAYS(2008, 2, 28, 1, 2008, 2, 29);
  TEST_ADD_DAYS(2008, 12, 31, 1, 2009, 1, 1);
  TEST_ADD_DAYS(1, 1, 1, 1, 1, 1, 2);
  TEST_ADD_DAYS(1955, 5, 24, 10, 1955, 6, 3);
  TEST_ADD_DAYS(1955, 5, 24, -10, 1955, 5, 14);
}
static void test_GDateTime_add_weeks(void) {
#define TEST_ADD_WEEKS(y,m,d,a,ny,nm,nd)                            \
  G_STMT_START {                                                    \
      GDateTime *dt, *dt2;                                          \
      dt = g_date_time_new_local(y, m, d, 0, 0, 0);                \
      dt2 = g_date_time_add_weeks(dt, a);                          \
      g_assert_cmpint(ny, ==, g_date_time_get_year(dt2));         \
      g_assert_cmpint(nm, ==, g_date_time_get_month(dt2));        \
      g_assert_cmpint(nd, ==, g_date_time_get_day_of_month(dt2)); \
      g_date_time_unref(dt);                                       \
      g_date_time_unref(dt2);                                      \
  } G_STMT_END
  TEST_ADD_WEEKS(2009, 1, 1, 1, 2009, 1, 8);
  TEST_ADD_WEEKS(2009, 8, 30, 1, 2009, 9, 6);
  TEST_ADD_WEEKS(2009, 12, 31, 1, 2010, 1, 7);
  TEST_ADD_WEEKS(2009, 1, 1, -1, 2008, 12, 25);
}
static void test_GDateTime_add_hours(void) {
#define TEST_ADD_HOURS(y,m,d,h,mi,s,a,ny,nm,nd,nh,nmi,ns)         \
  G_STMT_START {                                                  \
      GDateTime *dt, *dt2;                                        \
      dt = g_date_time_new_utc(y, m, d, h, mi, s);                \
      dt2 = g_date_time_add_hours(dt, a);                         \
      g_assert_cmpint(ny, ==, g_date_time_get_year(dt2));         \
      g_assert_cmpint(nm, ==, g_date_time_get_month(dt2));        \
      g_assert_cmpint(nd, ==, g_date_time_get_day_of_month(dt2)); \
      g_assert_cmpint(nh, ==, g_date_time_get_hour(dt2));         \
      g_assert_cmpint(nmi, ==, g_date_time_get_minute(dt2));      \
      g_assert_cmpint(ns, ==, g_date_time_get_second(dt2));       \
      g_date_time_unref(dt);                                      \
      g_date_time_unref(dt2);                                     \
  } G_STMT_END
  TEST_ADD_HOURS(2009,  1,  1,  0, 0, 0, 1, 2009, 1, 1, 1, 0, 0);
  TEST_ADD_HOURS(2008, 12, 31, 23, 0, 0, 1, 2009, 1, 1, 0, 0, 0);
}
static void test_GDateTime_add_full(void) {
  #define TEST_ADD_FULL(y,m,d,h,mi,s,ay,am,ad,ah,ami,as,ny,nm,nd,nh,nmi,ns) \
  G_STMT_START {                                                            \
      GDateTime *dt, *dt2;                                                  \
      dt = g_date_time_new_utc(y, m, d, h, mi, s);                          \
      dt2 = g_date_time_add_full(dt, ay, am, ad, ah, ami, as);              \
      g_assert_cmpint(ny, ==, g_date_time_get_year(dt2));                   \
      g_assert_cmpint(nm, ==, g_date_time_get_month(dt2));                  \
      g_assert_cmpint(nd, ==, g_date_time_get_day_of_month(dt2));           \
      g_assert_cmpint(nh, ==, g_date_time_get_hour(dt2));                   \
      g_assert_cmpint(nmi, ==, g_date_time_get_minute(dt2));                \
      g_assert_cmpint(ns, ==, g_date_time_get_second(dt2));                 \
      g_date_time_unref(dt);                                                \
      g_date_time_unref(dt2);                                               \
  } G_STMT_END
  TEST_ADD_FULL(2009, 10, 21,  0,  0, 0,1,  1,  1,  1,  1, 1,2010, 11, 22,  1,  1, 1);
  TEST_ADD_FULL(2000,  1,  1,  1,  1, 1,0,  1,  0,  0,  0, 0,2000,  2,  1,  1,  1, 1);
  TEST_ADD_FULL(2000,  1,  1,  0,  0, 0,-1,  1,  0,  0,  0, 0,1999,  2,  1,  0,  0, 0);
  TEST_ADD_FULL(2010, 10, 31,  0,  0, 0,0,  4,  0,  0,  0, 0,2011,  2, 28,  0,  0, 0);
  TEST_ADD_FULL(2010,  8, 25, 22, 45, 0,0,  1,  6,  1, 25, 0,2010, 10,  2,  0, 10, 0);
}
static void test_GDateTime_add_minutes(void) {
  #define TEST_ADD_MINUTES(i,o)                            \
  G_STMT_START {                                           \
      GDateTime *dt, *dt2;                                 \
      dt = g_date_time_new_local(2000, 1, 1, 0, 0, 0);     \
      dt2 = g_date_time_add_minutes(dt, i);                \
      g_assert_cmpint(o, ==, g_date_time_get_minute(dt2)); \
      g_date_time_unref(dt);                               \
      g_date_time_unref(dt2);                              \
  } G_STMT_END
  TEST_ADD_MINUTES(60, 0);
  TEST_ADD_MINUTES(100, 40);
  TEST_ADD_MINUTES(5, 5);
  TEST_ADD_MINUTES(1441, 1);
  TEST_ADD_MINUTES(-1441, 59);
}
static void test_GDateTime_add_seconds(void) {
#define TEST_ADD_SECONDS(i,o)                              \
  G_STMT_START {                                           \
      GDateTime *dt, *dt2;                                 \
      dt = g_date_time_new_local(2000, 1, 1, 0, 0, 0);     \
      dt2 = g_date_time_add_seconds(dt, i);                \
      g_assert_cmpint(o, ==, g_date_time_get_second(dt2)); \
      g_date_time_unref(dt);                               \
      g_date_time_unref(dt2);                              \
  } G_STMT_END
  TEST_ADD_SECONDS(1, 1);
  TEST_ADD_SECONDS(60, 0);
  TEST_ADD_SECONDS(61, 1);
  TEST_ADD_SECONDS(120, 0);
  TEST_ADD_SECONDS(-61, 59);
  TEST_ADD_SECONDS(86401, 1);
  TEST_ADD_SECONDS(-86401, 59);
  TEST_ADD_SECONDS(-31, 29);
  TEST_ADD_SECONDS(13, 13);
}
static void test_GDateTime_diff(void) {
#define TEST_DIFF(y,m,d,y2,m2,d2,u)                     \
  G_STMT_START {                                        \
      GDateTime *dt1, *dt2;                             \
      GTimeSpan  ts = 0;                                \
      dt1 = g_date_time_new_local(y, m, d, 0, 0, 0);    \
      dt2 = g_date_time_new_local(y2, m2, d2, 0, 0, 0); \
      ts = g_date_time_difference(dt2, dt1);            \
      g_assert_cmpint(ts, ==, u);                       \
      g_date_time_unref(dt1);                           \
      g_date_time_unref(dt2);                           \
  } G_STMT_END
  TEST_DIFF(2009, 1, 1, 2009, 2, 1, G_TIME_SPAN_DAY * 31);
  TEST_DIFF(2009, 1, 1, 2010, 1, 1, G_TIME_SPAN_DAY * 365);
  TEST_DIFF(2008, 2, 28, 2008, 2, 29, G_TIME_SPAN_DAY);
  TEST_DIFF(2008, 2, 29, 2008, 2, 28, -G_TIME_SPAN_DAY);
}
static void test_GDateTime_get_minute(void) {
  GDateTime *dt;
  dt = g_date_time_new_utc(2009, 12, 1, 1, 31, 0);
  g_assert_cmpint(31, ==, g_date_time_get_minute(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_get_month(void) {
  GDateTime *dt;
  dt = g_date_time_new_utc(2009, 12, 1, 1, 31, 0);
  g_assert_cmpint(12, ==, g_date_time_get_month(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_get_second(void) {
  GDateTime *dt;
  dt = g_date_time_new_utc(2009, 12, 1, 1, 31, 44);
  g_assert_cmpint(44, ==, g_date_time_get_second(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_new_full(void) {
  GTimeZone *tz;
  GDateTime *dt;
  dt = g_date_time_new_utc(2009, 12, 11, 12, 11, 10);
  g_assert_cmpint(2009, ==, g_date_time_get_year(dt));
  g_assert_cmpint(12, ==, g_date_time_get_month(dt));
  g_assert_cmpint(11, ==, g_date_time_get_day_of_month(dt));
  g_assert_cmpint(12, ==, g_date_time_get_hour(dt));
  g_assert_cmpint(11, ==, g_date_time_get_minute(dt));
  g_assert_cmpint(10, ==, g_date_time_get_second(dt));
  g_date_time_unref(dt);
  tz = g_time_zone_new("America/Recife");
  dt = g_date_time_new(tz, 2010, 5, 24, 8, 4, 0);
  g_time_zone_unref(tz);
  g_assert_cmpint(2010, ==, g_date_time_get_year(dt));
  g_assert_cmpint(5, ==, g_date_time_get_month(dt));
  g_assert_cmpint(24, ==, g_date_time_get_day_of_month(dt));
  g_assert_cmpint(8, ==, g_date_time_get_hour(dt));
  g_assert_cmpint(4, ==, g_date_time_get_minute(dt));
  g_assert_cmpint(0, ==, g_date_time_get_second(dt));
  g_assert_cmpstr("BRT", ==, g_date_time_get_timezone_abbreviation(dt));
  g_assert(!g_date_time_is_daylight_savings(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_now_utc(void) {
  GDateTime *dt;
  time_t t;
  struct tm tm;
  t = time(NULL);
#ifdef HAVE_GMTIME_R
  gmtime_r(&t, &tm);
#else
  {
      struct tm *tmp = gmtime(&t);
      memcpy (&tm, tmp, sizeof(struct tm));
  }
#endif
  dt = g_date_time_new_now_utc();
  g_assert_cmpint(tm.tm_year + 1900, ==, g_date_time_get_year(dt));
  g_assert_cmpint(tm.tm_mon + 1, ==, g_date_time_get_month(dt));
  g_assert_cmpint(tm.tm_mday, ==, g_date_time_get_day_of_month(dt));
  g_assert_cmpint(tm.tm_hour, ==, g_date_time_get_hour(dt));
  g_assert_cmpint(tm.tm_min, ==, g_date_time_get_minute(dt));
  g_assert_cmpint(tm.tm_sec, ==, g_date_time_get_second(dt));
  g_date_time_unref(dt);
}
static void test_GDateTime_new_from_unix_utc(void) {
  GDateTime *dt;
  gint64 t;
  t = g_get_real_time();
  dt = g_date_time_new_from_unix_utc(t);
  g_assert(dt == NULL);
  t = t / 1e6;
  dt = g_date_time_new_from_unix_utc(t);
  g_assert(dt != NULL);
  g_assert(dt == g_date_time_ref(dt));
  g_date_time_unref(dt);
  g_assert_cmpint(g_date_time_to_unix(dt), ==, t);
  g_date_time_unref(dt);
}
static void test_GDateTime_get_utc_offset(void) {
  GDateTime *dt;
  GTimeSpan ts;
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  get_localtime_tm(time(NULL), &tm);
  dt = g_date_time_new_now_local();
  ts = g_date_time_get_utc_offset(dt);
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
  g_assert_cmpint(ts, ==, (tm.tm_gmtoff * G_TIME_SPAN_SECOND));
#endif
#ifdef HAVE_STRUCT_TM___TM_GMTOFF
  g_assert_cmpint(ts, ==, (tm.__tm_gmtoff * G_TIME_SPAN_SECOND));
#endif
  g_date_time_unref(dt);
}
static void test_GDateTime_to_timeval(void) {
  GTimeVal tv1, tv2;
  GDateTime *dt;
  memset(&tv1, 0, sizeof(tv1));
  memset(&tv2, 0, sizeof(tv2));
  g_get_current_time(&tv1);
  dt = g_date_time_new_from_timeval_local(&tv1);
  g_date_time_to_timeval(dt, &tv2);
  g_assert_cmpint(tv1.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint(tv1.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref(dt);
}
static void test_GDateTime_to_local(void) {
  GDateTime *utc, *now, *dt;
  utc = g_date_time_new_now_utc();
  now = g_date_time_new_now_local();
  dt = g_date_time_to_local(utc);
  g_assert_cmpint(g_date_time_get_year(now), ==, g_date_time_get_year(dt));
  g_assert_cmpint(g_date_time_get_month(now), ==, g_date_time_get_month(dt));
  g_assert_cmpint(g_date_time_get_day_of_month(now), ==, g_date_time_get_day_of_month(dt));
  g_assert_cmpint(g_date_time_get_hour(now), ==, g_date_time_get_hour(dt));
  g_assert_cmpint(g_date_time_get_minute(now), ==, g_date_time_get_minute(dt));
  g_assert_cmpint(g_date_time_get_second(now), ==, g_date_time_get_second(dt));
  g_date_time_unref(now);
  g_date_time_unref(utc);
  g_date_time_unref(dt);
}
static void test_GDateTime_to_utc(void) {
  GDateTime *dt, *dt2;
  time_t t;
  struct tm  tm;
  t = time(NULL);
#ifdef HAVE_GMTIME_R
  gmtime_r(&t, &tm);
#else
  {
      struct tm *tmp = gmtime(&t);
      memcpy(&tm, tmp, sizeof(struct tm));
  }
#endif
  dt2 = g_date_time_new_now_local();
  dt = g_date_time_to_utc(dt2);
  g_assert_cmpint(tm.tm_year + 1900, ==, g_date_time_get_year(dt));
  g_assert_cmpint(tm.tm_mon + 1, ==, g_date_time_get_month(dt));
  g_assert_cmpint(tm.tm_mday, ==, g_date_time_get_day_of_month(dt));
  g_assert_cmpint(tm.tm_hour, ==, g_date_time_get_hour(dt));
  g_assert_cmpint(tm.tm_min, ==, g_date_time_get_minute(dt));
  g_assert_cmpint(tm.tm_sec, ==, g_date_time_get_second(dt));
  g_date_time_unref(dt);
  g_date_time_unref(dt2);
}
static void test_GDateTime_get_day_of_year(void) {
#define TEST_DAY_OF_YEAR(y,m,d,o)                                      \
  G_STMT_START {                                                       \
      GDateTime *__dt = g_date_time_new_local((y), (m), (d), 0, 0, 0); \
      g_assert_cmpint((o), ==, g_date_time_get_day_of_year(__dt));     \
      g_date_time_unref(__dt);                                         \
  } G_STMT_END
  TEST_DAY_OF_YEAR(2009, 1, 1, 1);
  TEST_DAY_OF_YEAR(2009, 2, 1, 32);
  TEST_DAY_OF_YEAR(2009, 8, 16, 228);
  TEST_DAY_OF_YEAR(2008, 8, 16, 229);
}
static void test_GDateTime_printf(void) {
  gchar dst[16];
  struct tm tt;
  time_t t;
  gchar t_str[16];
#define TEST_PRINTF(f,o)                                              \
  G_STMT_START {                                                      \
      GDateTime *__dt = g_date_time_new_local(2009, 10, 24, 0, 0, 0); \
      gchar *__p = g_date_time_format(__dt, (f));                     \
      g_assert_cmpstr(__p, ==, (o));                                  \
      g_date_time_unref(__dt);                                        \
      g_free(__p);                                                    \
  } G_STMT_END
#define TEST_PRINTF_DATE(y,m,d,f,o)                                \
  G_STMT_START {                                                   \
      GDateTime *dt = g_date_time_new_local(y, m, d, 0, 0, 0);     \
      gchar *p = g_date_time_format(dt, (f));                      \
      g_assert_cmpstr(p, ==, (o));                                 \
      g_date_time_unref(dt);                                       \
      g_free(p);                                                   \
  } G_STMT_END
#define TEST_PRINTF_TIME(h,m,s,f,o)                                       \
  G_STMT_START {                                                          \
      GDateTime *dt = g_date_time_new_local(2009, 10, 24, (h), (m), (s)); \
      gchar *p = g_date_time_format(dt, (f));                             \
      g_assert_cmpstr(p, ==, (o));                                        \
      g_date_time_unref(dt);                                              \
      g_free(p);                                                          \
  } G_STMT_END
  t = time(NULL);
  memset(&tt, 0, sizeof(tt));
  get_localtime_tm(t, &tt);
  tt.tm_year = 2009 - 1900;
  tt.tm_mon = 9;
  tt.tm_mday = 24;
  t = mktime(&tt);
  memset(&tt, 0, sizeof(tt));
  get_localtime_tm(t, &tt);
  strftime(dst, sizeof(dst), "%Z", &tt);
  tt.tm_sec = 0;
  tt.tm_min = 0;
  tt.tm_hour = 0;
  t = mktime(&tt);
  g_sprintf(t_str, "%ld", t);
  TEST_PRINTF("%a", "Sat");
  TEST_PRINTF("%A", "Saturday");
  TEST_PRINTF("%b", "Oct");
  TEST_PRINTF("%B", "October");
  TEST_PRINTF("%d", "24");
  TEST_PRINTF_DATE(2009, 1, 1, "%d", "01");
  TEST_PRINTF("%e", "24");
  TEST_PRINTF("%h", "Oct");
  TEST_PRINTF("%H", "00");
  TEST_PRINTF_TIME(15, 0, 0, "%H", "15");
  TEST_PRINTF("%I", "12");
  TEST_PRINTF_TIME(12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME(15, 0, 0, "%I", "03");
  TEST_PRINTF("%j", "297");
  TEST_PRINTF("%k", " 0");
  TEST_PRINTF_TIME(13, 13, 13, "%k", "13");
  TEST_PRINTF("%l", "12");
  TEST_PRINTF_TIME(12, 0, 0, "%I", "12");
  TEST_PRINTF_TIME(13, 13, 13, "%l", " 1");
  TEST_PRINTF_TIME(10, 13, 13, "%l", "10");
  TEST_PRINTF("%m", "10");
  TEST_PRINTF("%M", "00");
  TEST_PRINTF("%N", "0");
  TEST_PRINTF("%p", "AM");
  TEST_PRINTF_TIME(13, 13, 13, "%p", "PM");
  TEST_PRINTF("%P", "am");
  TEST_PRINTF_TIME(13, 13, 13, "%P", "pm");
  TEST_PRINTF("%r", "12:00:00 AM");
  TEST_PRINTF_TIME(13, 13, 13, "%r", "01:13:13 PM");
  TEST_PRINTF("%R", "00:00");
  TEST_PRINTF_TIME(13, 13, 31, "%R", "13:13");
  TEST_PRINTF("%S", "00");
  TEST_PRINTF("%t", "	");
  TEST_PRINTF("%W", "42");
  TEST_PRINTF("%u", "6");
  TEST_PRINTF("%x", "10/24/09");
  TEST_PRINTF("%X", "00:00:00");
  TEST_PRINTF_TIME(13, 14, 15, "%X", "13:14:15");
  TEST_PRINTF("%y", "09");
  TEST_PRINTF("%Y", "2009");
  TEST_PRINTF("%%", "%");
  TEST_PRINTF("%", "");
  TEST_PRINTF("%9", NULL);
  TEST_PRINTF("%Z", dst);
}
static void test_GDateTime_dst(void) {
  GDateTime *dt1, *dt2;
  GTimeZone *tz;
  tz = g_time_zone_new("Europe/London");
  dt1 = g_date_time_new(tz, 2009, 8, 15, 3, 0, 1);
  g_assert(g_date_time_is_daylight_savings(dt1));
  g_assert_cmpint(g_date_time_get_utc_offset(dt1) / G_USEC_PER_SEC, ==, 3600);
  g_assert_cmpint(g_date_time_get_hour(dt1), ==, 3);
  dt2 = g_date_time_add_months(dt1, 6);
  g_assert(!g_date_time_is_daylight_savings(dt2));
  g_assert_cmpint(g_date_time_get_utc_offset(dt2) / G_USEC_PER_SEC, ==, 0);
  g_assert_cmpint(g_date_time_get_hour (dt2), ==, 3);
  g_date_time_unref(dt2);
  g_date_time_unref(dt1);
  dt1 = g_date_time_new(tz, 2009, 2, 15, 2, 0, 1);
  g_assert(!g_date_time_is_daylight_savings(dt1));
  g_assert_cmpint(g_date_time_get_hour(dt1), ==, 2);
  dt2 = g_date_time_add_months(dt1, 6);
  g_assert(g_date_time_is_daylight_savings(dt2));
  g_assert_cmpint(g_date_time_get_hour (dt2), ==, 2);
  g_date_time_unref(dt2);
  g_date_time_unref(dt1);
  g_time_zone_unref(tz);
}
static inline gboolean is_leap_year(gint year) {
  g_assert(1 <= year && year <= 9999);
  return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}
static inline gint days_in_month(gint year, gint month) {
  const gint table[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };
  g_assert(1 <= month && month <= 12);
  return table[is_leap_year(year)][month];
}
static void test_all_dates(void) {
  gint year, month, day;
  GTimeZone *timezone;
  gint64 unix_time;
  gint day_of_year;
  gint week_year;
  gint week_num;
  gint weekday;
  timezone = g_time_zone_new_utc();
  unix_time = G_GINT64_CONSTANT(-62135596800);
  week_year = 1;
  week_num = 1;
  weekday = 1;
  for (year = 1; year <= 9999; year++) {
      day_of_year = 1;
      for (month = 1; month <= 12; month++)
          for (day = 1; day <= days_in_month (year, month); day++) {
              GDateTime *dt;
              dt = g_date_time_new(timezone, year, month, day, 0, 0, 0);
          #if 0
              g_print("%04d-%02d-%02d = %04d-W%02d-%d = %04d-%03d\n", year, month, day, week_year, week_num, weekday, year, day_of_year);
          #endif
              if G_UNLIKELY(g_date_time_get_year(dt) != year || g_date_time_get_month(dt) != month || g_date_time_get_day_of_month(dt) != day)
                  g_error("%04d-%02d-%02d comes out as %04d-%02d-%02d", year, month, day, g_date_time_get_year(dt), g_date_time_get_month(dt),
                          g_date_time_get_day_of_month(dt));
              if G_UNLIKELY(g_date_time_get_week_numbering_year (dt) != week_year || g_date_time_get_week_of_year (dt) != week_num ||
                            g_date_time_get_day_of_week (dt) != weekday)
              g_error("%04d-%02d-%02d should be %04d-W%02d-%d but comes out as %04d-W%02d-%d", year, month, day, week_year, week_num, weekday,
                      g_date_time_get_week_numbering_year (dt), g_date_time_get_week_of_year (dt), g_date_time_get_day_of_week (dt));
              if G_UNLIKELY(g_date_time_to_unix (dt) != unix_time)
                  g_error("%04d-%02d-%02d 00:00:00 UTC should have unix time %" G_GINT64_FORMAT " but comes out as %"G_GINT64_FORMAT,
                          year, month, day, unix_time, g_date_time_to_unix (dt));
              if G_UNLIKELY(g_date_time_get_day_of_year (dt) != day_of_year)
                  g_error("%04d-%02d-%02d should be day of year %d but comes out as %d", year, month, day, day_of_year, g_date_time_get_day_of_year(dt));
              if G_UNLIKELY(g_date_time_get_hour (dt) != 0 || g_date_time_get_minute (dt) != 0 || g_date_time_get_seconds (dt) != 0)
                  g_error("%04d-%02d-%02d 00:00:00 UTC comes out as %02d:%02d:%02.6f", year, month, day, g_date_time_get_hour(dt),
                          g_date_time_get_minute(dt), g_date_time_get_seconds(dt));
              unix_time += 24 * 60 * 60;
              day_of_year++;
              if (++weekday == 8) {
                  weekday = 1;
                  if (year != week_year || (month == 12 && day >= 28)) {
                      week_num = 1;
                      week_year++;
                  } else week_num++;
              }
          }
  }
  g_time_zone_unref(timezone);
}
gint main(gint argc, gchar *argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/GDateTime/add_days", test_GDateTime_add_days);
  g_test_add_func("/GDateTime/add_full", test_GDateTime_add_full);
  g_test_add_func("/GDateTime/add_hours", test_GDateTime_add_hours);
  g_test_add_func("/GDateTime/add_minutes", test_GDateTime_add_minutes);
  g_test_add_func("/GDateTime/add_months", test_GDateTime_add_months);
  g_test_add_func("/GDateTime/add_seconds", test_GDateTime_add_seconds);
  g_test_add_func("/GDateTime/add_weeks", test_GDateTime_add_weeks);
  g_test_add_func("/GDateTime/add_years", test_GDateTime_add_years);
  g_test_add_func("/GDateTime/compare", test_GDateTime_compare);
  g_test_add_func("/GDateTime/diff", test_GDateTime_diff);
  g_test_add_func("/GDateTime/equal", test_GDateTime_equal);
  g_test_add_func("/GDateTime/get_day_of_week", test_GDateTime_get_day_of_week);
  g_test_add_func("/GDateTime/get_day_of_month", test_GDateTime_get_day_of_month);
  g_test_add_func("/GDateTime/get_day_of_year", test_GDateTime_get_day_of_year);
  g_test_add_func("/GDateTime/get_hour", test_GDateTime_get_hour);
  g_test_add_func("/GDateTime/get_microsecond", test_GDateTime_get_microsecond);
  g_test_add_func("/GDateTime/get_minute", test_GDateTime_get_minute);
  g_test_add_func("/GDateTime/get_month", test_GDateTime_get_month);
  g_test_add_func("/GDateTime/get_second", test_GDateTime_get_second);
  g_test_add_func("/GDateTime/get_utc_offset", test_GDateTime_get_utc_offset);
  g_test_add_func("/GDateTime/get_year", test_GDateTime_get_year);
  g_test_add_func("/GDateTime/hash", test_GDateTime_hash);
  g_test_add_func("/GDateTime/new_from_unix", test_GDateTime_new_from_unix);
  g_test_add_func("/GDateTime/new_from_unix_utc", test_GDateTime_new_from_unix_utc);
  g_test_add_func("/GDateTime/new_from_timeval", test_GDateTime_new_from_timeval);
  g_test_add_func("/GDateTime/new_full", test_GDateTime_new_full);
  g_test_add_func("/GDateTime/now", test_GDateTime_now);
  g_test_add_func("/GDateTime/printf", test_GDateTime_printf);
  g_test_add_func("/GDateTime/to_local", test_GDateTime_to_local);
  g_test_add_func("/GDateTime/to_unix", test_GDateTime_to_unix);
  g_test_add_func("/GDateTime/to_timeval", test_GDateTime_to_timeval);
  g_test_add_func("/GDateTime/to_utc", test_GDateTime_to_utc);
  g_test_add_func("/GDateTime/now_utc", test_GDateTime_now_utc);
  g_test_add_func("/GDateTime/dst", test_GDateTime_dst);
  g_test_add_func("/GDateTime/test-all-dates", test_all_dates);
  return g_test_run();
}