/*
 * test_snprintf.c — Tests for usnprintf buffer operations and edge cases
 */

#define UPRINTF_HEADER_ONLY
#include "uprintf.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

static int g_pass = 0;
static int g_fail = 0;

static void check_str(const char *test_name, const char *got, const char *expected) {
    printf("  [TEST] %s... ", test_name);
    if (strcmp(got, expected) == 0) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: got \"%s\", expected \"%s\"\n", got, expected); g_fail++; }
}

static void check_wstr(const char *test_name, const wchar_t *got, const wchar_t *expected) {
    printf("  [TEST] %s... ", test_name);
    if (wcscmp(got, expected) == 0) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: wide string mismatch\n"); g_fail++; }
}

static void check_ret(const char *test_name, int got, int expected) {
    printf("  [TEST] %s... ", test_name);
    if (got == expected) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: got %d, expected %d\n", got, expected); g_fail++; }
}

static void check_true(const char *test_name, int cond) {
    printf("  [TEST] %s... ", test_name);
    if (cond) { printf("OK\n"); g_pass++; }
    else { printf("FAIL\n"); g_fail++; }
}

static void test_narrow_truncation(void) {
    char buf[10];

    usnprintf_narrow(buf, 5, "%s", "hello world");
    check_str("truncation to 5 chars", buf, "hell");

    usnprintf_narrow(buf, 6, "%s", "hello");
    check_str("exact fit", buf, "hello");

    usnprintf_narrow(buf, 1, "%s", "hello");
    check_str("size 1 -> empty string", buf, "");

    memset(buf, 'X', sizeof(buf));
    usnprintf_narrow(buf, 5, "%s", "abcdefghij");
    check_true("null-termination guaranteed", buf[4] == '\0');
}

static void test_wide_truncation(void) {
    wchar_t buf[10];

    usnprintf_wide(buf, 5, L"%ls", L"hello world");
    check_wstr("wide truncation to 5", buf, L"hell");

    usnprintf_wide(buf, 6, L"%ls", L"hello");
    check_wstr("wide exact fit", buf, L"hello");

    usnprintf_wide(buf, 1, L"%ls", L"hello");
    check_wstr("wide size 1 -> empty", buf, L"");
}

static void test_null_and_zero(void) {
    char nbuf[10];
    wchar_t wbuf[10];
    int ret;

    ret = usnprintf_narrow(NULL, 10, "hello");
    check_ret("NULL buf returns -1", ret, -1);

    ret = usnprintf_narrow(nbuf, 0, "hello");
    check_ret("size 0 returns -1", ret, -1);

    ret = usnprintf_narrow(nbuf, 10, NULL);
    check_ret("NULL fmt returns -1", ret, -1);

    ret = usnprintf_wide(NULL, 10, L"hello");
    check_ret("wide NULL buf returns -1", ret, -1);

    ret = usnprintf_wide(wbuf, 0, L"hello");
    check_ret("wide size 0 returns -1", ret, -1);

    ret = usnprintf_wide(wbuf, 10, NULL);
    check_ret("wide NULL fmt returns -1", ret, -1);
}

static void test_return_values(void) {
    char buf[256];
    int ret;

    ret = usnprintf_narrow(buf, sizeof(buf), "abc");
    check_ret("return = length of output", ret, 3);

    ret = usnprintf_narrow(buf, sizeof(buf), "%d", 12345);
    check_ret("return with format", ret, 5);

    ret = usnprintf_narrow(buf, sizeof(buf), "%s", "");
    check_ret("empty string arg return 0", ret, 0);
}

int main(void) {
    printf("=== usnprintf buffer tests ===\n\n");

    printf("[Narrow truncation]\n");
    test_narrow_truncation();
    printf("\n[Wide truncation]\n");
    test_wide_truncation();
    printf("\n[NULL and zero edge cases]\n");
    test_null_and_zero();
    printf("\n[Return values]\n");
    test_return_values();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
