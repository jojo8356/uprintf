/*
 * test_security.c — Security tests: %n rejection, NULL handling
 *
 * Compiled WITHOUT UPRINTF_ENABLE_N to test %n rejection.
 */

#define UPRINTF_HEADER_ONLY
#include "uprintf.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

static int g_pass = 0;
static int g_fail = 0;

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

static void test_percent_n_rejection(void) {
    char buf[256];
    int ret;

    ret = usnprintf_narrow(buf, sizeof(buf), "hello%n", (int*)NULL);
    check_ret("%n in narrow rejected", ret, -1);

    ret = usnprintf_narrow(buf, sizeof(buf), "%10n", (int*)NULL);
    check_ret("%n with width rejected", ret, -1);

    ret = usnprintf_narrow(buf, sizeof(buf), "%ln", (long*)NULL);
    check_ret("%n with length rejected", ret, -1);

    ret = usnprintf_narrow(buf, sizeof(buf), "%-n", (int*)NULL);
    check_ret("%n with flags rejected", ret, -1);

    ret = usnprintf_narrow(buf, sizeof(buf), "100%%");
    check_true("%%%% is NOT %n", ret >= 0);

    ret = usnprintf_narrow(buf, sizeof(buf), "%d", 42);
    check_true("%d is not %n", ret >= 0);
}

static void test_percent_n_rejection_wide(void) {
    wchar_t buf[256];
    int ret;

    ret = usnprintf_wide(buf, 256, L"hello%n", (int*)NULL);
    check_ret("wide %n rejected", ret, -1);

    ret = usnprintf_wide(buf, 256, L"%ln", (long*)NULL);
    check_ret("wide %ln rejected", ret, -1);

    ret = usnprintf_wide(buf, 256, L"100%%");
    check_true("wide %%%% is NOT %n", ret >= 0);
}

static void test_null_format(void) {
    int ret;

    ret = uprintf_narrow(NULL);
    check_ret("uprintf_narrow(NULL) returns -1", ret, -1);

    ret = uprintf_wide(NULL);
    check_ret("uprintf_wide(NULL) returns -1", ret, -1);

    ret = ufprintf_narrow(NULL, "hello");
    check_ret("ufprintf_narrow(NULL stream) returns -1", ret, -1);

    ret = ufprintf_narrow(stdout, NULL);
    check_ret("ufprintf_narrow(NULL fmt) returns -1", ret, -1);

    ret = ufprintf_wide(NULL, L"hello");
    check_ret("ufprintf_wide(NULL stream) returns -1", ret, -1);
}

static void test_percent_n_scanner(void) {
    check_true("scanner: simple %n", uprintf_has_percent_n_narrow("hello%n") == 1);
    check_true("scanner: no %n", uprintf_has_percent_n_narrow("hello %d %s") == 0);
    check_true("scanner: %n after flags", uprintf_has_percent_n_narrow("%+010n") == 1);
    check_true("scanner: %n after precision", uprintf_has_percent_n_narrow("%.5n") == 1);
    check_true("scanner: %n after length", uprintf_has_percent_n_narrow("%lln") == 1);
    check_true("scanner: %%%% not confused", uprintf_has_percent_n_narrow("%%n") == 0);
    check_true("scanner: NULL fmt", uprintf_has_percent_n_narrow(NULL) == 0);
    check_true("scanner: empty string", uprintf_has_percent_n_narrow("") == 0);
    check_true("scanner wide: %n", uprintf_has_percent_n_wide(L"hello%n") == 1);
    check_true("scanner wide: no %n", uprintf_has_percent_n_wide(L"hello %d") == 0);
    check_true("scanner wide: %%%% safe", uprintf_has_percent_n_wide(L"%%n") == 0);
}

int main(void) {
    printf("=== uprintf security tests ===\n\n");

    printf("[%%n rejection - narrow]\n");
    test_percent_n_rejection();
    printf("\n[%%n rejection - wide]\n");
    test_percent_n_rejection_wide();
    printf("\n[NULL format handling]\n");
    test_null_format();
    printf("\n[%%n scanner unit tests]\n");
    test_percent_n_scanner();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
