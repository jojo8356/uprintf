/*
 * uprintf.h — Universal printf: portable dispatch between printf and wprintf
 *
 * Usage:
 *   #include "uprintf.h"
 *
 *   uprintf("Hello %s\n", "narrow");       // -> printf
 *   uprintf(L"Hello %ls\n", L"wide");      // -> wprintf (C11 _Generic)
 *
 *   ufprintf(stderr, "error: %s\n", msg);
 *   usnprintf(buf, sizeof(buf), "val=%d", 42);
 *
 * With TCHAR mode (C99 compatible):
 *   #define UPRINTF_UNICODE   // optional, forces wide
 *   #include "uprintf.h"
 *   uprintf(_T("Hello %s\n"), _T("world"));
 *
 * Options:
 *   UPRINTF_UNICODE      - TCHAR = wchar_t, _T() = L""
 *   UPRINTF_HEADER_ONLY  - Include implementation inline
 *   UPRINTF_AUTO_LOCALE  - Auto-init locale on Unix
 *   UPRINTF_AUTO_CONSOLE - Auto-init console on Windows
 *   UPRINTF_NO_GENERIC   - Force C99 mode (no _Generic)
 *   UPRINTF_ENABLE_N     - Allow %n specifier
 *   UPRINTF_DEBUG        - Enable internal assertions
 */

#ifndef UPRINTF_H
#define UPRINTF_H

#include "uprintf_config.h"

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#if defined(UPRINTF_WINDOWS)
    #include <io.h>
    #include <fcntl.h>
#endif

#include <locale.h>

/* ========================================================================== */
/*  TCHAR / _T() portable macros                                              */
/* ========================================================================== */

/*
 * Only define TCHAR and _T if not already provided (e.g. by <tchar.h>).
 * We guard on _TCHAR_DEFINED (MSVC) and our own flag.
 */
#if !defined(_TCHAR_DEFINED) && !defined(UPRINTF_TCHAR_DEFINED)
    #define UPRINTF_TCHAR_DEFINED 1

    #ifdef UPRINTF_UNICODE
        typedef wchar_t TCHAR;
        #define _T(x) L##x
    #else
        typedef char TCHAR;
        #define _T(x) x
    #endif
#endif

/* ========================================================================== */
/*  Security helpers (inline, zero malloc)                                    */
/* ========================================================================== */

/* --- %n scanner: reject format strings containing %n unless opted in --- */

#ifndef UPRINTF_ENABLE_N

UPRINTF_INLINE int uprintf_has_percent_n_narrow(const char *fmt) {
    const char *p;
    if (fmt == NULL) return 0;
    for (p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            if (*p == '%') continue;  /* %% literal */
            /* skip flags */
            while (*p == '-' || *p == '+' || *p == ' ' || *p == '0' || *p == '#') p++;
            /* skip width */
            if (*p == '*') { p++; } else { while (*p >= '0' && *p <= '9') p++; }
            /* skip precision */
            if (*p == '.') {
                p++;
                if (*p == '*') { p++; } else { while (*p >= '0' && *p <= '9') p++; }
            }
            /* skip length modifiers */
            while (*p == 'h' || *p == 'l' || *p == 'j' || *p == 'z' || *p == 't' || *p == 'L') p++;
            if (*p == 'n') return 1;
            if (*p == '\0') break;
        }
    }
    return 0;
}

UPRINTF_INLINE int uprintf_has_percent_n_wide(const wchar_t *fmt) {
    const wchar_t *p;
    if (fmt == NULL) return 0;
    for (p = fmt; *p; p++) {
        if (*p == L'%') {
            p++;
            if (*p == L'%') continue;
            while (*p == L'-' || *p == L'+' || *p == L' ' || *p == L'0' || *p == L'#') p++;
            if (*p == L'*') { p++; } else { while (*p >= L'0' && *p <= L'9') p++; }
            if (*p == L'.') {
                p++;
                if (*p == L'*') { p++; } else { while (*p >= L'0' && *p <= L'9') p++; }
            }
            while (*p == L'h' || *p == L'l' || *p == L'j' || *p == L'z' || *p == L't' || *p == L'L') p++;
            if (*p == L'n') return 1;
            if (*p == L'\0') break;
        }
    }
    return 0;
}

#endif /* UPRINTF_ENABLE_N */

/* --- Safe vsnprintf wrapper for MSVC (guarantees null-termination) --- */

#if defined(UPRINTF_MSVC)

UPRINTF_INLINE int uprintf_safe_vsnprintf(char *buf, size_t n, const char *fmt, va_list ap) {
    int ret;
    if (buf == NULL || n == 0) return -1;
    ret = _vsnprintf(buf, n, fmt, ap);
    buf[n - 1] = '\0';
    if (ret < 0 || (size_t)ret >= n)
        ret = (int)(n - 1);
    return ret;
}

UPRINTF_INLINE int uprintf_safe_vsnwprintf(wchar_t *buf, size_t n, const wchar_t *fmt, va_list ap) {
    int ret;
    if (buf == NULL || n == 0) return -1;
    ret = _vsnwprintf(buf, n, fmt, ap);
    buf[n - 1] = L'\0';
    if (ret < 0 || (size_t)ret >= n)
        ret = (int)(n - 1);
    return ret;
}

#endif /* UPRINTF_MSVC */

/* ========================================================================== */
/*  Core narrow functions                                                     */
/* ========================================================================== */

UPRINTF_INLINE int uprintf_narrow(const char *fmt, ...)
#if defined(UPRINTF_GCC) || defined(UPRINTF_CLANG)
    __attribute__((format(printf, 1, 2)))
#endif
;

UPRINTF_INLINE int uprintf_narrow(const char *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "uprintf: format string is NULL");
    if (fmt == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_narrow(fmt)) return -1;
#endif
    va_start(ap, fmt);
    ret = vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

UPRINTF_INLINE int ufprintf_narrow(FILE *stream, const char *fmt, ...)
#if defined(UPRINTF_GCC) || defined(UPRINTF_CLANG)
    __attribute__((format(printf, 2, 3)))
#endif
;

UPRINTF_INLINE int ufprintf_narrow(FILE *stream, const char *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "ufprintf: format string is NULL");
    if (fmt == NULL || stream == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_narrow(fmt)) return -1;
#endif
    va_start(ap, fmt);
    ret = vfprintf(stream, fmt, ap);
    va_end(ap);
    return ret;
}

UPRINTF_INLINE int usnprintf_narrow(char *buf, size_t n, const char *fmt, ...)
#if defined(UPRINTF_GCC) || defined(UPRINTF_CLANG)
    __attribute__((format(printf, 3, 4)))
#endif
;

UPRINTF_INLINE int usnprintf_narrow(char *buf, size_t n, const char *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "usnprintf: format string is NULL");
    UPRINTF_ASSERT(buf != NULL || n == 0, "usnprintf: buf is NULL");
    if (fmt == NULL) return -1;
    if (buf == NULL || n == 0) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_narrow(fmt)) return -1;
#endif
    va_start(ap, fmt);
#if defined(UPRINTF_MSVC)
    ret = uprintf_safe_vsnprintf(buf, n, fmt, ap);
#else
    ret = vsnprintf(buf, n, fmt, ap);
#endif
    va_end(ap);
    /* Guarantee null-termination */
    if (n > 0) buf[n - 1] = '\0';
    return ret;
}

UPRINTF_INLINE int usprintf_narrow(char *buf, const char *fmt, ...)
#if defined(UPRINTF_GCC) || defined(UPRINTF_CLANG)
    __attribute__((format(printf, 2, 3)))
#endif
;

UPRINTF_INLINE int usprintf_narrow(char *buf, const char *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "usprintf: format string is NULL");
    UPRINTF_ASSERT(buf != NULL, "usprintf: buf is NULL");
    if (fmt == NULL || buf == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_narrow(fmt)) return -1;
#endif
    va_start(ap, fmt);
    ret = vsprintf(buf, fmt, ap);
    va_end(ap);
    return ret;
}

/* ========================================================================== */
/*  Core wide functions                                                       */
/* ========================================================================== */

UPRINTF_INLINE int uprintf_wide(const wchar_t *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "uprintf: format string is NULL");
    if (fmt == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_wide(fmt)) return -1;
#endif
    va_start(ap, fmt);
    ret = vwprintf(fmt, ap);
    va_end(ap);
    return ret;
}

UPRINTF_INLINE int ufprintf_wide(FILE *stream, const wchar_t *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "ufprintf: format string is NULL");
    if (fmt == NULL || stream == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_wide(fmt)) return -1;
#endif
    va_start(ap, fmt);
    ret = vfwprintf(stream, fmt, ap);
    va_end(ap);
    return ret;
}

UPRINTF_INLINE int usnprintf_wide(wchar_t *buf, size_t n, const wchar_t *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "usnprintf: format string is NULL");
    UPRINTF_ASSERT(buf != NULL || n == 0, "usnprintf: buf is NULL");
    if (fmt == NULL) return -1;
    if (buf == NULL || n == 0) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_wide(fmt)) return -1;
#endif
    va_start(ap, fmt);
#if defined(UPRINTF_MSVC)
    ret = uprintf_safe_vsnwprintf(buf, n, fmt, ap);
#else
    ret = vswprintf(buf, n, fmt, ap);
#endif
    va_end(ap);
    if (n > 0) buf[n - 1] = L'\0';
    return ret;
}

UPRINTF_INLINE int usprintf_wide(wchar_t *buf, const wchar_t *fmt, ...) {
    va_list ap;
    int ret;
    UPRINTF_ASSERT(fmt != NULL, "usprintf: format string is NULL");
    UPRINTF_ASSERT(buf != NULL, "usprintf: buf is NULL");
    if (fmt == NULL || buf == NULL) return -1;
#ifndef UPRINTF_ENABLE_N
    if (uprintf_has_percent_n_wide(fmt)) return -1;
#endif
    va_start(ap, fmt);
#if defined(UPRINTF_MSVC)
    /* MSVC swprintf without size is deprecated; use a large limit */
    ret = uprintf_safe_vsnwprintf(buf, UPRINTF_STACK_BUF_MAX / sizeof(wchar_t), fmt, ap);
#else
    ret = vswprintf(buf, UPRINTF_STACK_BUF_MAX / sizeof(wchar_t), fmt, ap);
#endif
    va_end(ap);
    return ret;
}

/* ========================================================================== */
/*  Public API macros — C11 _Generic dispatch                                 */
/* ========================================================================== */

#if defined(UPRINTF_HAS_GENERIC)

#define uprintf(fmt, ...) _Generic((fmt),               \
    char*:          uprintf_narrow,                     \
    const char*:    uprintf_narrow,                     \
    wchar_t*:       uprintf_wide,                       \
    const wchar_t*: uprintf_wide                        \
)(fmt, ##__VA_ARGS__)

#define ufprintf(stream, fmt, ...) _Generic((fmt),      \
    char*:          ufprintf_narrow,                    \
    const char*:    ufprintf_narrow,                    \
    wchar_t*:       ufprintf_wide,                      \
    const wchar_t*: ufprintf_wide                       \
)(stream, fmt, ##__VA_ARGS__)

#define usnprintf(buf, n, fmt, ...) _Generic((fmt),     \
    char*:          usnprintf_narrow,                  \
    const char*:    usnprintf_narrow,                  \
    wchar_t*:       usnprintf_wide,                     \
    const wchar_t*: usnprintf_wide                      \
)(buf, n, fmt, ##__VA_ARGS__)

#define usprintf(buf, fmt, ...) _Generic((fmt),         \
    char*:          usprintf_narrow,                   \
    const char*:    usprintf_narrow,                   \
    wchar_t*:       usprintf_wide,                      \
    const wchar_t*: usprintf_wide                       \
)(buf, fmt, ##__VA_ARGS__)

/* ========================================================================== */
/*  Public API macros — C99 fallback (static dispatch via UPRINTF_UNICODE)    */
/* ========================================================================== */

#else /* No _Generic */

#ifdef UPRINTF_UNICODE
    #define uprintf     uprintf_wide
    #define ufprintf    ufprintf_wide
    #define usnprintf   usnprintf_wide
    #define usprintf    usprintf_wide
#else
    #define uprintf     uprintf_narrow
    #define ufprintf    ufprintf_narrow
    #define usnprintf   usnprintf_narrow
    #define usprintf    usprintf_narrow
#endif

#endif /* UPRINTF_HAS_GENERIC */

/* ========================================================================== */
/*  uprintf_init() — Platform initialization                                  */
/* ========================================================================== */

#if defined(UPRINTF_HEADER_ONLY) || defined(UPRINTF_IMPLEMENTATION)

UPRINTF_INLINE void uprintf_init(void) {
#if defined(UPRINTF_WINDOWS)
    #if defined(UPRINTF_AUTO_CONSOLE) || defined(UPRINTF_UNICODE)
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    #endif
    #if defined(UPRINTF_ENABLE_N)
    _set_printf_count_output(1);
    #endif
#else
    #if defined(UPRINTF_AUTO_LOCALE)
    setlocale(LC_ALL, "");
    #endif
#endif
}

#else /* Declaration only */

void uprintf_init(void);

#endif

/* ========================================================================== */
/*  Wide inttypes macros (UPRI*)                                              */
/* ========================================================================== */

#ifdef UPRINTF_UNICODE
    #define UPRI_d8   L"d"
    #define UPRI_d16  L"d"
    #define UPRI_d32  L"d"
    #define UPRI_d64  L"lld"
    #define UPRI_u8   L"u"
    #define UPRI_u16  L"u"
    #define UPRI_u32  L"u"
    #define UPRI_u64  L"llu"
    #define UPRI_x32  L"x"
    #define UPRI_x64  L"llx"
    #define UPRI_X32  L"X"
    #define UPRI_X64  L"llX"
#else
    #include <inttypes.h>
    #define UPRI_d8   PRId8
    #define UPRI_d16  PRId16
    #define UPRI_d32  PRId32
    #define UPRI_d64  PRId64
    #define UPRI_u8   PRIu8
    #define UPRI_u16  PRIu16
    #define UPRI_u32  PRIu32
    #define UPRI_u64  PRIu64
    #define UPRI_x32  PRIx32
    #define UPRI_x64  PRIx64
    #define UPRI_X32  PRIX32
    #define UPRI_X64  PRIX64
#endif

#endif /* UPRINTF_H */
