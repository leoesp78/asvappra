#include <stdio.h>
#include <stdarg.h>
#include "libcompat.h"
#include "check.h"
#include "check_list.h"
#include "check_error.h"
#include "check_impl.h"
#include "check_str.h"

static const char *tr_type_str(TestResult * tr);
static int percent_passed(TestStats * t);
char *tr_str(TestResult * tr) {
    const char *exact_msg;
    char *rstr;
    exact_msg = (tr->rtype == CK_ERROR) ? "(after this point) " : "";
    rstr = ck_strdup_printf("%s:%d:%s:%s:%s:%d: %s%s", tr->file, tr->line, tr_type_str(tr), tr->tcname, tr->tname, tr->iter,
                            exact_msg, tr->msg);
    return rstr;
}
char *tr_short_str(TestResult *tr) {
    const char *exact_msg;
    char *rstr;
    exact_msg = (tr->rtype == CK_ERROR) ? "(after this point) " : "";
    rstr = ck_strdup_printf("%s:%d: %s%s", tr->file, tr->line, exact_msg, tr->msg);
    return rstr;
}
char *sr_stat_str(SRunner *sr) {
    char *str;
    TestStats *ts;
    ts = sr->stats;
    str = ck_strdup_printf("%d%%: Checks: %d, Failures: %d, Errors: %d", percent_passed(ts), ts->n_checked, ts->n_failed, ts->n_errors);
    return str;
}
char *ck_strdup_printf(const char *fmt, ...) {
    size_t size = 100;
    char *p;
    va_list ap;
    p = (char*)emalloc(size);
    while(1) {
        int n;
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);
        if(n > -1 && n < (int)size) return p;
        if(n > -1) size = (size_t) n + 1;
        else size *= 2;
        p = (char *)erealloc(p, size);
    }
}
static const char *tr_type_str(TestResult *tr) {
    if(tr->ctx == CK_CTX_TEST) {
        if(tr->rtype == CK_PASS) return "P";
        if(tr->rtype == CK_FAILURE) return "F";
        if(tr->rtype == CK_ERROR) return "E";
        return NULL;
    }
    return "S";
}
static int percent_passed(TestStats *t) {
    if(t->n_failed == 0 && t->n_errors == 0) return 100;
    if(t->n_checked == 0) return 0;
    return (int)((float)(t->n_checked - (t->n_failed + t->n_errors)) / (float)t->n_checked * 100);
}