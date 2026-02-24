/*
 * uprintf_config.h — Platform detection, compiler detection, security constants
 * Part of the uprintf library (universal printf)
 *
 * This file is included automatically by uprintf.h. Do not include directly.
 */

#ifndef UPRINTF_CONFIG_H
#define UPRINTF_CONFIG_H

/* ========================================================================== */
/*  Platform detection                                                        */
/* ========================================================================== */

#if defined(_WIN32) || defined(_WIN64)
    #define UPRINTF_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define UPRINTF_MACOS 1
#elif defined(__linux__)
    #define UPRINTF_LINUX 1
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #define UPRINTF_BSD 1
#else
    #define UPRINTF_UNKNOWN_PLATFORM 1
#endif

/* ========================================================================== */
/*  Compiler detection                                                        */
/* ========================================================================== */

#if defined(_MSC_VER)
    #define UPRINTF_MSVC 1
    #define UPRINTF_MSVC_VER _MSC_VER
#endif

#if defined(__clang__)
    #define UPRINTF_CLANG 1
    #define UPRINTF_CLANG_VER (__clang_major__ * 100 + __clang_minor__)
#elif defined(__GNUC__)
    #define UPRINTF_GCC 1
    #define UPRINTF_GCC_VER (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

/* ========================================================================== */
/*  C standard version detection                                              */
/* ========================================================================== */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define UPRINTF_C11 1
#endif

#if defined(UPRINTF_C11) && !defined(UPRINTF_NO_GENERIC)
    #define UPRINTF_HAS_GENERIC 1
#endif

/* ========================================================================== */
/*  Compiler attributes                                                       */
/* ========================================================================== */

#if defined(UPRINTF_GCC) || defined(UPRINTF_CLANG)
    #define UPRINTF_FORMAT_CHECK(fmt_idx, args_idx) \
        __attribute__((format(printf, fmt_idx, args_idx)))
    #define UPRINTF_WFORMAT_CHECK
    #define UPRINTF_UNUSED __attribute__((unused))
#else
    #define UPRINTF_FORMAT_CHECK(fmt_idx, args_idx)
    #define UPRINTF_WFORMAT_CHECK
    #define UPRINTF_UNUSED
#endif

#if defined(UPRINTF_MSVC)
    #include <sal.h>
#endif

/* ========================================================================== */
/*  Inline keyword portability                                                */
/* ========================================================================== */

#if defined(UPRINTF_MSVC) && !defined(UPRINTF_C11)
    #define UPRINTF_INLINE __inline
#else
    #define UPRINTF_INLINE static inline
#endif

/* ========================================================================== */
/*  Security constants                                                        */
/* ========================================================================== */

#ifndef UPRINTF_STACK_BUF_MAX
    #define UPRINTF_STACK_BUF_MAX 4096
#endif

#ifndef UPRINTF_MAX_WIDTH
    #define UPRINTF_MAX_WIDTH 1048576
#endif

#ifndef UPRINTF_MAX_PRECISION
    #define UPRINTF_MAX_PRECISION 1048576
#endif

/* ========================================================================== */
/*  Debug assertions                                                          */
/* ========================================================================== */

#ifdef UPRINTF_DEBUG
    #include <assert.h>
    #define UPRINTF_ASSERT(cond, msg) assert((cond) && (msg))
#else
    #define UPRINTF_ASSERT(cond, msg) ((void)0)
#endif

#endif /* UPRINTF_CONFIG_H */
