/*
 * test_wide.c — Tests for uprintf with wide (wchar_t*) format strings
 */

#define UPRINTF_HEADER_ONLY
#define UPRINTF_AUTO_LOCALE
#include "uprintf.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>

static int g_pass = 0;
static int g_fail = 0;

static void check_wstr(const char *test_name, const wchar_t *got, const wchar_t *expected) {
    printf("  [TEST] %s... ", test_name);
    if (wcscmp(got, expected) == 0) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: wide string mismatch\n"); g_fail++; }
}

static void check_true(const char *test_name, int cond) {
    printf("  [TEST] %s... ", test_name);
    if (cond) { printf("OK\n"); g_pass++; }
    else { printf("FAIL\n"); g_fail++; }
}

static void test_integers(void) {
    wchar_t buf[256];

    usnprintf_wide(buf, 256, L"%d", 42);
    check_wstr("%d positive", buf, L"42");

    usnprintf_wide(buf, 256, L"%d", -7);
    check_wstr("%d negative", buf, L"-7");

    usnprintf_wide(buf, 256, L"%u", 42u);
    check_wstr("%u", buf, L"42");

    usnprintf_wide(buf, 256, L"%x", 255);
    check_wstr("%x", buf, L"ff");

    usnprintf_wide(buf, 256, L"%X", 255);
    check_wstr("%X", buf, L"FF");

    usnprintf_wide(buf, 256, L"%o", 255);
    check_wstr("%o", buf, L"377");

    usnprintf_wide(buf, 256, L"%#x", 255);
    check_wstr("%#x", buf, L"0xff");

    usnprintf_wide(buf, 256, L"%ld", 123456789L);
    check_wstr("%ld", buf, L"123456789");

    usnprintf_wide(buf, 256, L"%lld", 9223372036854775807LL);
    check_wstr("%lld", buf, L"9223372036854775807");

    usnprintf_wide(buf, 256, L"%zu", (size_t)1024);
    check_wstr("%zu", buf, L"1024");
}

static void test_floats(void) {
    wchar_t buf[256];

    usnprintf_wide(buf, 256, L"%f", 3.14);
    check_wstr("%f", buf, L"3.140000");

    usnprintf_wide(buf, 256, L"%.2f", 3.14159);
    check_wstr("%.2f", buf, L"3.14");

    usnprintf_wide(buf, 256, L"%.2e", 3.14);
    check_wstr("%.2e", buf, L"3.14e+00");

    usnprintf_wide(buf, 256, L"%g", 3.14);
    check_wstr("%g", buf, L"3.14");

    usnprintf_wide(buf, 256, L"%.2Lf", (long double)3.14);
    check_wstr("%Lf", buf, L"3.14");
}

static void test_strings(void) {
    wchar_t buf[256];

    usnprintf_wide(buf, 256, L"%ls", L"hello");
    check_wstr("%ls wide string", buf, L"hello");

    usnprintf_wide(buf, 256, L"%.3ls", L"hello");
    check_wstr("%.3ls truncation", buf, L"hel");

    usnprintf_wide(buf, 256, L"%lc", (wint_t)L'A');
    check_wstr("%lc wide char", buf, L"A");

    usnprintf_wide(buf, 256, L"100%%");
    check_wstr("%%", buf, L"100%");
}

static void test_flags_width_precision(void) {
    wchar_t buf[256];

    usnprintf_wide(buf, 256, L"%-10d|", 42);
    check_wstr("left align %-10d", buf, L"42        |");

    usnprintf_wide(buf, 256, L"%010d", 42);
    check_wstr("zero pad %010d", buf, L"0000000042");

    usnprintf_wide(buf, 256, L"%10d", 42);
    check_wstr("width %10d", buf, L"        42");

    usnprintf_wide(buf, 256, L"%.5d", 42);
    check_wstr("precision %.5d", buf, L"00042");

    usnprintf_wide(buf, 256, L"%+010.2f", 3.14);
    check_wstr("combined %+010.2f", buf, L"+000003.14");

    usnprintf_wide(buf, 256, L"%ls is %d years old", L"Alice", 30);
    check_wstr("multiple args", buf, L"Alice is 30 years old");
}

static void test_unicode(void) {
    wchar_t buf[256];

    usnprintf_wide(buf, 256, L"%ls", L"caf\x00e9");
    check_wstr("wide accents", buf, L"caf\x00e9");

    usnprintf_wide(buf, 256, L"%ls", L"\x6f22\x5b57");
    check_wstr("wide CJK", buf, L"\x6f22\x5b57");

    usnprintf_wide(buf, 256, L"%ls", L"\U0001F680");
    check_wstr("wide emoji", buf, L"\U0001F680");
}

static void test_return_value(void) {
    wchar_t buf[256];
    int ret;

    ret = usnprintf_wide(buf, 256, L"%d", 42);
    /* swprintf returns number of wide chars written (excluding null) */
    check_true("return value > 0", ret > 0);
}

int main(void) {
    setlocale(LC_ALL, "C");

    printf("=== uprintf wide tests ===\n\n");

    printf("[Integers]\n");
    test_integers();
    printf("\n[Floats]\n");
    test_floats();
    printf("\n[Strings]\n");
    test_strings();
    printf("\n[Flags/Width/Precision]\n");
    test_flags_width_precision();
    printf("\n[Unicode]\n");
    test_unicode();
    printf("\n[Return values]\n");
    test_return_value();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
