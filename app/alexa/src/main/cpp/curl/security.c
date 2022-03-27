#include "curl_setup.h"

#ifndef CURL_DISABLE_FTP
#ifdef HAVE_GSSAPI
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <limits.h>
#include "urldata.h"
#include "curl_base64.h"
#include "curl_memory.h"
#include "curl_sec.h"
#include "ftp.h"
#include "sendf.h"
#include "strcase.h"
#include "warnless.h"
#include "strdup.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

static const struct {
    enum protection_level level;
    const char *name;
} level_names[] = {
    { PROT_CLEAR, "clear" },
    { PROT_SAFE, "safe" },
    { PROT_CONFIDENTIAL, "confidential" },
    { PROT_PRIVATE, "private" }
};
static enum protection_level name_to_level(const char *name) {
    for(int i = 0; i < (int)sizeof(level_names)/(int)sizeof(level_names[0]); i++)
        if(checkprefix(name, level_names[i].name)) return level_names[i].level;
    return PROT_NONE;
}
static char level_to_char(int level) {
    switch(level) {
        case PROT_CLEAR: return 'C';
        case PROT_SAFE: return 'S';
        case PROT_CONFIDENTIAL: return 'E';
        case PROT_PRIVATE: return 'P';
        case PROT_CMD: default: break;
    }
    DEBUGASSERT(0);
    return 'P';
}
static int ftp_send_command(struct connectdata *conn, const char *message, ...) {
    int ftp_code;
    ssize_t nread = 0;
    va_list args;
    char print_buffer[50];
    va_start(args, message);
    mvsnprintf(print_buffer, sizeof(print_buffer), message, args);
    va_end(args);
    if(Curl_ftpsend(conn, print_buffer)) ftp_code = -1;
    else if(Curl_GetFTPResponse(&nread, conn, &ftp_code)) ftp_code = -1;
    (void)nread;
    return ftp_code;
}
static CURLcode socket_read(curl_socket_t fd, void *to, size_t len) {
    char *to_p = to;
    CURLcode result;
    ssize_t nread = 0;
    while(len > 0) {
        result = Curl_read_plain(fd, to_p, len, &nread);
        if(!result) {
            len -= nread;
            to_p += nread;
        } else {
            if(result == CURLE_AGAIN) continue;
            return result;
        }
    }
    return CURLE_OK;
}
static CURLcode socket_write(struct connectdata *conn, curl_socket_t fd, const void *to, size_t len) {
    const char *to_p = to;
    CURLcode result;
    ssize_t written;
    while(len > 0) {
        result = Curl_write_plain(conn, fd, to_p, len, &written);
        if(!result) {
            len -= written;
            to_p += written;
        } else {
            if(result == CURLE_AGAIN) continue;
            return result;
        }
    }
    return CURLE_OK;
}
static CURLcode read_data(struct connectdata *conn, curl_socket_t fd, struct krb5buffer *buf) {
    int len;
    CURLcode result;
    result = socket_read(fd, &len, sizeof(len));
    if(result) return result;
    if(len) {
        len = ntohl(len);
        buf->data = Curl_saferealloc(buf->data, len);
    }
    if(!len || !buf->data) return CURLE_OUT_OF_MEMORY;
    result = socket_read(fd, buf->data, len);
    if(result) return result;
    buf->size = conn->mech->decode(conn->app_data, buf->data, len, conn->data_prot, conn);
    buf->index = 0;
    return CURLE_OK;
}
static size_t buffer_read(struct krb5buffer *buf, void *data, size_t len) {
    if(buf->size - buf->index < len) len = buf->size - buf->index;
    memcpy(data, (char *)buf->data + buf->index, len);
    buf->index += len;
    return len;
}
static ssize_t sec_recv(struct connectdata *conn, int sockindex, char *buffer, size_t len, CURLcode *err) {
    size_t bytes_read;
    size_t total_read = 0;
    curl_socket_t fd = conn->sock[sockindex];
    *err = CURLE_OK;
    if(conn->sec_complete == 0 || conn->data_prot == PROT_CLEAR) return sread(fd, buffer, len);
    if(conn->in_buffer.eof_flag) {
        conn->in_buffer.eof_flag = 0;
        return 0;
    }
    bytes_read = buffer_read(&conn->in_buffer, buffer, len);
    len -= bytes_read;
    total_read += bytes_read;
    buffer += bytes_read;
    while(len > 0) {
        if(read_data(conn, fd, &conn->in_buffer)) return -1;
        if(conn->in_buffer.size == 0) {
            if(bytes_read > 0) conn->in_buffer.eof_flag = 1;
            return bytes_read;
        }
        bytes_read = buffer_read(&conn->in_buffer, buffer, len);
        len -= bytes_read;
        total_read += bytes_read;
        buffer += bytes_read;
    }
    return total_read;
}
static void do_sec_send(struct connectdata *conn, curl_socket_t fd, const char *from, int length) {
    int bytes, htonl_bytes;
    char *buffer = NULL;
    char *cmd_buffer;
    size_t cmd_size = 0;
    CURLcode error;
    enum protection_level prot_level = conn->data_prot;
    bool iscmd = (prot_level == PROT_CMD)?TRUE:FALSE;
    DEBUGASSERT(prot_level > PROT_NONE && prot_level < PROT_LAST);
    if(iscmd) {
        if(!strncmp(from, "PASS ", 5) || !strncmp(from, "ACCT ", 5)) prot_level = PROT_PRIVATE;
        else prot_level = conn->command_prot;
    }
    bytes = conn->mech->encode(conn->app_data, from, length, prot_level, (void **)&buffer);
    if(!buffer || bytes <= 0) return;
    if(iscmd) {
        error = Curl_base64_encode(conn->data, buffer, curlx_sitouz(bytes), &cmd_buffer, &cmd_size);
        if(error) {
            free(buffer);
          return;
        }
        if(cmd_size > 0) {
            static const char *enc = "ENC ";
            static const char *mic = "MIC ";
            if(prot_level == PROT_PRIVATE) socket_write(conn, fd, enc, 4);
            else socket_write(conn, fd, mic, 4);
            socket_write(conn, fd, cmd_buffer, cmd_size);
            socket_write(conn, fd, "\r\n", 2);
            infof(conn->data, "Send: %s%s\n", prot_level == PROT_PRIVATE?enc:mic, cmd_buffer);
            free(cmd_buffer);
        }
    } else {
        htonl_bytes = htonl(bytes);
        socket_write(conn, fd, &htonl_bytes, sizeof(htonl_bytes));
        socket_write(conn, fd, buffer, curlx_sitouz(bytes));
    }
    free(buffer);
}
static ssize_t sec_write(struct connectdata *conn, curl_socket_t fd, const char *buffer, size_t length) {
    ssize_t tx = 0, len = conn->buffer_size;
    len -= conn->mech->overhead(conn->app_data, conn->data_prot, curlx_sztosi(len));
    if(len <= 0) len = length;
    while(length) {
        if(length < (size_t)len) len = length;
        do_sec_send(conn, fd, buffer, curlx_sztosi(len));
        length -= len;
        buffer += len;
        tx += len;
    }
    return tx;
}
static ssize_t sec_send(struct connectdata *conn, int sockindex, const void *buffer, size_t len, CURLcode *err) {
    curl_socket_t fd = conn->sock[sockindex];
    *err = CURLE_OK;
    return sec_write(conn, fd, buffer, len);
}
int Curl_sec_read_msg(struct connectdata *conn, char *buffer, enum protection_level level) {
    int decoded_len;
    char *buf;
    int ret_code = 0;
    size_t decoded_sz = 0;
    CURLcode error;
    if(!conn->mech) return -1;
    DEBUGASSERT(level > PROT_NONE && level < PROT_LAST);
    error = Curl_base64_decode(buffer + 4, (unsigned char **)&buf, &decoded_sz);
    if(error || decoded_sz == 0) return -1;
    if(decoded_sz > (size_t)INT_MAX) {
        free(buf);
        return -1;
    }
    decoded_len = curlx_uztosi(decoded_sz);
    decoded_len = conn->mech->decode(conn->app_data, buf, decoded_len, level, conn);
    if(decoded_len <= 0) {
        free(buf);
        return -1;
    }
    if(conn->data->set.verbose) {
        buf[decoded_len] = '\n';
        Curl_debug(conn->data, CURLINFO_HEADER_IN, buf, decoded_len + 1);
    }
    buf[decoded_len] = '\0';
    if(decoded_len <= 3) return 0;
    if(buf[3] != '-') (void)sscanf(buf, "%d", &ret_code);
    if(buf[decoded_len - 1] == '\n') buf[decoded_len - 1] = '\0';
    strcpy(buffer, buf);
    free(buf);
    return ret_code;
}
static int sec_set_protection_level(struct connectdata *conn) {
    int code;
    enum protection_level level = conn->request_data_prot;
    DEBUGASSERT(level > PROT_NONE && level < PROT_LAST);
    if(!conn->sec_complete) {
        infof(conn->data, "Trying to change the protection level after the" " completion of the data exchange.\n");
        return -1;
    }
    if(conn->data_prot == level) return 0;
    if(level) {
        char *pbsz;
        static unsigned int buffer_size = 1 << 20;
        code = ftp_send_command(conn, "PBSZ %u", buffer_size);
        if(code < 0) return -1;
        if(code/100 != 2) {
            failf(conn->data, "Failed to set the protection's buffer size.");
            return -1;
        }
        conn->buffer_size = buffer_size;
        pbsz = strstr(conn->data->state.buffer, "PBSZ=");
        if(pbsz) {
            (void)sscanf(pbsz, "PBSZ=%u", &buffer_size);
            if(buffer_size < conn->buffer_size) conn->buffer_size = buffer_size;
        }
    }
    code = ftp_send_command(conn, "PROT %c", level_to_char(level));
    if(code < 0) return -1;
    if(code/100 != 2) {
        failf(conn->data, "Failed to set the protection level.");
        return -1;
    }
    conn->data_prot = level;
    if(level == PROT_PRIVATE) conn->command_prot = level;
    return 0;
}
int Curl_sec_request_prot(struct connectdata *conn, const char *level) {
    enum protection_level l = name_to_level(level);
    if(l == PROT_NONE) return -1;
    DEBUGASSERT(l > PROT_NONE && l < PROT_LAST);
    conn->request_data_prot = l;
    return 0;
}
static CURLcode choose_mech(struct connectdata *conn) {
    int ret;
    struct Curl_easy *data = conn->data;
    void *tmp_allocation;
    const struct Curl_sec_client_mech *mech = &Curl_krb5_client_mech;
    tmp_allocation = realloc(conn->app_data, mech->size);
    if(tmp_allocation == NULL) {
        failf(data, "Failed realloc of size %zu", mech->size);
        mech = NULL;
        return CURLE_OUT_OF_MEMORY;
    }
    conn->app_data = tmp_allocation;
    if(mech->init) {
        ret = mech->init(conn->app_data);
        if(ret) {
            infof(data, "Failed initialization for %s. Skipping it.\n", mech->name);
            return CURLE_FAILED_INIT;
        }
    }
    infof(data, "Trying mechanism %s...\n", mech->name);
    ret = ftp_send_command(conn, "AUTH %s", mech->name);
    if(ret < 0) return CURLE_COULDNT_CONNECT;
    if(ret/100 != 3) {
        switch(ret) {
            case 504:
                infof(data, "Mechanism %s is not supported by the server (server " "returned ftp code: 504).\n", mech->name);
                break;
            case 534:
                infof(data, "Mechanism %s was rejected by the server (server returned " "ftp code: 534).\n", mech->name);
                break;
            default:
                if(ret/100 == 5) {
                    infof(data, "server does not support the security extensions\n");
                    return CURLE_USE_SSL_FAILED;
                }
        }
        return CURLE_LOGIN_DENIED;
    }
    ret = mech->auth(conn->app_data, conn);
    if(ret != AUTH_CONTINUE) {
        if(ret != AUTH_OK) return -1;
        DEBUGASSERT(ret == AUTH_OK);
        conn->mech = mech;
        conn->sec_complete = 1;
        conn->recv[FIRSTSOCKET] = sec_recv;
        conn->send[FIRSTSOCKET] = sec_send;
        conn->recv[SECONDARYSOCKET] = sec_recv;
        conn->send[SECONDARYSOCKET] = sec_send;
        conn->command_prot = PROT_SAFE;
        (void)sec_set_protection_level(conn);
    }
    return CURLE_OK;
}
CURLcode Curl_sec_login(struct connectdata *conn) {
    return choose_mech(conn);
}
void Curl_sec_end(struct connectdata *conn) {
    if(conn->mech != NULL && conn->mech->end) conn->mech->end(conn->app_data);
    free(conn->app_data);
    conn->app_data = NULL;
    if(conn->in_buffer.data) {
        free(conn->in_buffer.data);
        conn->in_buffer.data = NULL;
        conn->in_buffer.size = 0;
        conn->in_buffer.index = 0;
        conn->in_buffer.eof_flag = 0;
    }
    conn->sec_complete = 0;
    conn->data_prot = PROT_CLEAR;
    conn->mech = NULL;
}
#endif
#endif