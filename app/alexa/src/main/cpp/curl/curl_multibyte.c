#include "curl_setup.h"

#if defined(WIN32)
#include "curl_multibyte.h"
wchar_t *curlx_convert_UTF8_to_wchar(const char *str_utf8) {
    wchar_t *str_w = NULL;
    if(str_utf8) {
        int str_w_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str_utf8, -1, NULL, 0);
        if(str_w_len > 0) {
            str_w = malloc(str_w_len * sizeof(wchar_t));
            if(str_w) {
                if(MultiByteToWideChar(CP_UTF8, 0, str_utf8, -1, str_w, str_w_len) == 0) {
                    free(str_w);
                    return NULL;
                }
            }
        }
    }
    return str_w;
}
char *curlx_convert_wchar_to_UTF8(const wchar_t *str_w) {
    char *str_utf8 = NULL;
    if(str_w) {
        int bytes = WideCharToMultiByte(CP_UTF8, 0, str_w, -1, NULL, 0, NULL, NULL);
        if(bytes > 0) {
            str_utf8 = malloc(bytes);
            if(str_utf8) {
                if(WideCharToMultiByte(CP_UTF8, 0, str_w, -1, str_utf8, bytes, NULL, NULL) == 0) {
                    free(str_utf8);
                    return NULL;
                }
            }
        }
    }
    return str_utf8;
}
#endif
#if defined(USE_WIN32_LARGE_FILES) || defined(USE_WIN32_SMALL_FILES)
FILE *curlx_win32_fopen(const char *filename, const char *mode) {
#ifdef _UNICODE
    FILE *result = NULL;
    wchar_t *filename_w = curlx_convert_UTF8_to_wchar(filename);
    wchar_t *mode_w = curlx_convert_UTF8_to_wchar(mode);
    if (filename_w && mode_w) result = _wfopen(filename_w, mode_w);
    free(filename_w);
    free(mode_w);
    if(result)
        return result;
#endif
    return (fopen)(filename, mode);
}
int curlx_win32_stat(const char *path, struct_stat *buffer) {
    int result = -1;
#ifdef _UNICODE
    wchar_t *path_w = curlx_convert_UTF8_to_wchar(path);
#endif
#if defined(USE_WIN32_SMALL_FILES)
#if defined(_UNICODE)
    if(path_w) result = _wstat(path_w, buffer);
    else
#endif
    result = _stat(path, buffer);
#else
#if defined(_UNICODE)
    if(path_w) result = _wstati64(path_w, buffer);
    else
#endif
    result = _stati64(path, buffer);
#endif
#ifdef _UNICODE
    free(path_w);
#endif
    return result;
}
int curlx_win32_access(const char *path, int mode) {
    int result = -1;
#ifdef _UNICODE
    wchar_t *path_w = curlx_convert_UTF8_to_wchar(path);
#endif
#if defined(_UNICODE)
    if (path_w) result = _waccess(path_w, mode);
    else result = _access(path, mode);
#endif
#ifdef _UNICODE
    free(path_w);
#endif
    return result;
}
#endif