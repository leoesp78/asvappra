#ifndef HEADER_CURL_TOOL_OPERATE_H
#define HEADER_CURL_TOOL_OPERATE_H

#include "tool_setup.h"
#include "tool_cb_hdr.h"
#include "tool_cb_prg.h"
#include "tool_sdecls.h"

struct per_transfer {
    struct per_transfer *next;
    struct per_transfer *prev;
    struct OperationConfig *config;
    CURL *curl;
    long retry_numretries;
    long retry_sleep_default;
    long retry_sleep;
    struct timeval retrystart;
    bool metalink;
    bool metalink_next_res;
    struct metalinkfile *mlfile;
    struct metalink_resource *mlres;
    char *this_url;
    char *outfile;
    bool infdopen;
    int infd;
    bool noprogress;
    struct ProgressData progressbar;
    struct OutStruct outs;
    struct OutStruct heads;
    struct OutStruct etag_save;
    struct InStruct input;
    struct HdrCbData hdrcbdata;
    char errorbuffer[CURL_ERROR_SIZE];
    bool added;
    curl_off_t dltotal;
    curl_off_t dlnow;
    curl_off_t ultotal;
    curl_off_t ulnow;
    bool dltotal_added;
    bool ultotal_added;
    char *separator_err;
    char *separator;
    char *uploadfile;
};
CURLcode operate(struct GlobalConfig *config, int argc, argv_item_t argv[]);
extern struct per_transfer *transfers;
#endif