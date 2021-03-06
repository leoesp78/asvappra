#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <zconf.h>
#include "libcompat.h"
#include "check_error.h"
#include "check.h"
#include "check_list.h"
#include "check_impl.h"
#include "check_msg.h"
#include "check_pack.h"
#include "check_str.h"

static FILE *send_file1;
static char *send_file1_name;
static FILE *send_file2;
static char *send_file2_name;
static FILE *get_pipe(void);
static void setup_pipe(void);
static void teardown_pipe(void);
static TestResult *construct_test_result(RcvMsg * rmsg, int waserror);
static void tr_set_loc_by_ctx(TestResult * tr, enum ck_result_ctx ctx, RcvMsg * rmsg);
static FILE *get_pipe(void) {
    if(send_file2 != 0) return send_file2;
    if(send_file1 != 0) return send_file1;
    eprintf("Unable to report test progress or a failure; was an ck_assert or ck_abort function called while not running tests?",
            __FILE__, __LINE__);
    return NULL;
}
void send_failure_info(const char *msg) {
    FailMsg fmsg;
    fmsg.msg = strdup(msg);
    ppack(get_pipe(), CK_MSG_FAIL, (CheckMsg *) & fmsg);
    free(fmsg.msg);
}
void send_duration_info(int duration) {
    DurationMsg dmsg;
    dmsg.duration = duration;
    ppack(get_pipe(), CK_MSG_DURATION, (CheckMsg *) & dmsg);
}
void send_loc_info(const char *file, int line) {
    LocMsg lmsg;
    lmsg.file = strdup(file);
    lmsg.line = line;
    ppack(get_pipe(), CK_MSG_LOC, (CheckMsg *) & lmsg);
    free(lmsg.file);
}
void send_ctx_info(enum ck_result_ctx ctx) {
    CtxMsg cmsg;
    cmsg.ctx = ctx;
    ppack(get_pipe(), CK_MSG_CTX, (CheckMsg *) & cmsg);
}
TestResult *receive_test_result(int waserror) {
    FILE *fp;
    RcvMsg *rmsg;
    TestResult *result;
    fp = get_pipe();
    if(fp == NULL) eprintf("Error in call to get_pipe", __FILE__, __LINE__ - 2);
    rewind(fp);
    rmsg = punpack(fp);
    if(rmsg == NULL) eprintf("Error in call to punpack", __FILE__, __LINE__ - 4);
    teardown_pipe();
    setup_pipe();
    result = construct_test_result(rmsg, waserror);
    rcvmsg_free(rmsg);
    return result;
}
static void tr_set_loc_by_ctx(TestResult * tr, enum ck_result_ctx ctx, RcvMsg * rmsg) {
    if(ctx == CK_CTX_TEST) {
        tr->file = rmsg->test_file;
        tr->line = rmsg->test_line;
        rmsg->test_file = NULL;
        rmsg->test_line = -1;
    } else {
        tr->file = rmsg->fixture_file;
        tr->line = rmsg->fixture_line;
        rmsg->fixture_file = NULL;
        rmsg->fixture_line = -1;
    }
}
static TestResult *construct_test_result(RcvMsg * rmsg, int waserror) {
    TestResult *tr;
    if(rmsg == NULL) return NULL;
    tr = tr_create();
    if(rmsg->msg != NULL || waserror) {
        if(rmsg->failctx != CK_CTX_INVALID) tr->ctx = rmsg->failctx;
        else tr->ctx = rmsg->lastctx;
        tr->msg = rmsg->msg;
        rmsg->msg = NULL;
        tr_set_loc_by_ctx(tr, tr->ctx, rmsg);
    } else if(rmsg->lastctx == CK_CTX_SETUP) {
        tr->ctx = CK_CTX_SETUP;
        tr->msg = NULL;
        tr_set_loc_by_ctx(tr, CK_CTX_SETUP, rmsg);
    } else {
        tr->ctx = CK_CTX_TEST;
        tr->msg = NULL;
        tr->duration = rmsg->duration;
        tr_set_loc_by_ctx(tr, CK_CTX_TEST, rmsg);
    }
    return tr;
}
void setup_messaging(void) {
    setup_pipe();
}
void teardown_messaging(void) {
    teardown_pipe();
}
FILE *open_tmp_file(char **name) {
    FILE *file = NULL;
    *name = NULL;
#if !HAVE_MKSTEMP
    file = tmpfile();
    if(file == NULL) {
        char *tmp = getenv("TEMP");
        char *tmp_file = tempnam(tmp, "check_");
        char *uniq_tmp_file = ck_strdup_printf("%s.%d", tmp_file, getpid());
        file = fopen(uniq_tmp_file, "w+b");
        *name = uniq_tmp_file;
        free(tmp_file);
    }
#else
    int fd = -1;
    const char *tmp_dir = getenv("TEMP");
#ifdef P_tmpdir
    if (tmp_dir == NULL) tmp_dir = P_tmpdir;
#endif
    if (tmp_dir == NULL) tmp_dir = getenv("TMPDIR");
    if (tmp_dir == NULL) tmp_dir = ".";
    *name = ck_strdup_printf("%s/check_XXXXXX", tmp_dir);
    if (-1 < (fd = mkstemp(*name))) {
        file = fdopen (fd, "w+b");
        if (0 == unlink (*name) || NULL == file) {
            free (*name);
            *name = NULL;
        }
    }
#endif
    return file;
}
static void setup_pipe(void) {
    if(send_file1 == NULL) {
        send_file1 = open_tmp_file(&send_file1_name);
        if(send_file1 == NULL) {
            eprintf("Unable to create temporary file for communication; may not have permissions to do so", __FILE__, __LINE__ -3);
        }
        return;
    }
    if(send_file2 == NULL) {
        send_file2 = open_tmp_file(&send_file2_name);
        if(send_file2 == NULL) {
            eprintf("Unable to create temporary file for communication; may not have permissions to do so", __FILE__, __LINE__ -3);
        }
        return;
    }
    eprintf("Only one nesting of suite runs supported", __FILE__, __LINE__);
}
static void teardown_pipe(void) {
    if(send_file2 != 0) {
        fclose(send_file2);
        send_file2 = 0;
        if(send_file2_name != NULL) {
            unlink(send_file2_name);
            free(send_file2_name);
            send_file2_name = NULL;
        }
    } else if(send_file1 != 0) {
        fclose(send_file1);
        send_file1 = 0;
        if(send_file1_name != NULL) {
            unlink(send_file1_name);
            free(send_file1_name);
            send_file1_name = NULL;
        }
    } else eprintf("No messaging setup", __FILE__, __LINE__);
}