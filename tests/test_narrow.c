/*
 * test_narrow.c — Tests for uprintf with narrow (char*) format strings
 */

#define UPRINTF_HEADER_ONLY
#define UPRINTF_AUTO_LOCALE
#include "uprintf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <float.h>
#include <limits.h>

static int g_pass = 0;
static int g_fail = 0;

static void check_str(const char *test_name, const char *got, const char *expected) {
    printf("  [TEST] %s... ", test_name);
    if (strcmp(got, expected) == 0) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: got \"%s\", expected \"%s\"\n", got, expected); g_fail++; }
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

static void test_integers(void) {
    char buf[256];

    usnprintf_narrow(buf, sizeof(buf), "%d", 42);
    check_str("%d positive", buf, "42");

    usnprintf_narrow(buf, sizeof(buf), "%d", -7);
    check_str("%d negative", buf, "-7");

    usnprintf_narrow(buf, sizeof(buf), "%i", 123);
    check_str("%i", buf, "123");

    usnprintf_narrow(buf, sizeof(buf), "%u", 42u);
    check_str("%u", buf, "42");

    usnprintf_narrow(buf, sizeof(buf), "%o", 255);
    check_str("%o", buf, "377");

    usnprintf_narrow(buf, sizeof(buf), "%x", 255);
    check_str("%x", buf, "ff");

    usnprintf_narrow(buf, sizeof(buf), "%X", 255);
    check_str("%X", buf, "FF");

    usnprintf_narrow(buf, sizeof(buf), "%#x", 255);
    check_str("%#x", buf, "0xff");

    usnprintf_narrow(buf, sizeof(buf), "%#o", 255);
    check_str("%#o", buf, "0377");

    usnprintf_narrow(buf, sizeof(buf), "%ld", 123456789L);
    check_str("%ld", buf, "123456789");

    usnprintf_narrow(buf, sizeof(buf), "%lld", 9223372036854775807LL);
    check_str("%lld", buf, "9223372036854775807");

    usnprintf_narrow(buf, sizeof(buf), "%hd", (short)32767);
    check_str("%hd", buf, "32767");

    usnprintf_narrow(buf, sizeof(buf), "%hhd", (signed char)-1);
    check_str("%hhd", buf, "-1");

    usnprintf_narrow(buf, sizeof(buf), "%zu", (size_t)1024);
    check_str("%zu", buf, "1024");

    usnprintf_narrow(buf, sizeof(buf), "%" PRId32, (int32_t)42);
    check_str("PRId32", buf, "42");

    usnprintf_narrow(buf, sizeof(buf), "%" PRIu64, (uint64_t)123456789ULL);
    check_str("PRIu64", buf, "123456789");
}

static void test_floats(void) {
    char buf[256];

    usnprintf_narrow(buf, sizeof(buf), "%f", 3.14);
    check_str("%f", buf, "3.140000");

    usnprintf_narrow(buf, sizeof(buf), "%.2f", 3.14159);
    check_str("%.2f", buf, "3.14");

    usnprintf_narrow(buf, sizeof(buf), "%.2e", 3.14);
    check_str("%.2e", buf, "3.14e+00");

    usnprintf_narrow(buf, sizeof(buf), "%.2E", 3.14);
    check_str("%.2E", buf, "3.14E+00");

    usnprintf_narrow(buf, sizeof(buf), "%g", 3.14);
    check_str("%g", buf, "3.14");

    usnprintf_narrow(buf, sizeof(buf), "%g", 0.00001);
    check_str("%g small", buf, "1e-05");

    usnprintf_narrow(buf, sizeof(buf), "%.2Lf", (long double)3.14);
    check_str("%Lf", buf, "3.14");

    usnprintf_narrow(buf, sizeof(buf), "%.0f", 3.7);
    check_str("%.0f", buf, "4");

    usnprintf_narrow(buf, sizeof(buf), "%+.1f", 3.14);
    check_str("%+.1f", buf, "+3.1");
}

static void test_strings(void) {
    char buf[256];
    int x;
    int ret;

    usnprintf_narrow(buf, sizeof(buf), "%s", "hello");
    check_str("%s", buf, "hello");

    usnprintf_narrow(buf, sizeof(buf), "%.3s", "hello");
    check_str("%.3s truncation", buf, "hel");

    usnprintf_narrow(buf, sizeof(buf), "%c", 'A');
    check_str("%c", buf, "A");

    usnprintf_narrow(buf, sizeof(buf), "100%%");
    check_str("%%", buf, "100%");

    x = 42;
    ret = usnprintf_narrow(buf, sizeof(buf), "%p", (void*)&x);
    check_true("%p not null", ret > 0);
}

static void test_flags_width_precision(void) {
    char buf[256];

    usnprintf_narrow(buf, sizeof(buf), "%-10d|", 42);
    check_str("left align %-10d", buf, "42        |");

    usnprintf_narrow(buf, sizeof(buf), "%010d", 42);
    check_str("zero pad %010d", buf, "0000000042");

    usnprintf_narrow(buf, sizeof(buf), "%10d", 42);
    check_str("width %10d", buf, "        42");

    usnprintf_narrow(buf, sizeof(buf), "%*d", 10, 42);
    check_str("dynamic width %*d", buf, "        42");

    usnprintf_narrow(buf, sizeof(buf), "%.5d", 42);
    check_str("precision %.5d", buf, "00042");

    usnprintf_narrow(buf, sizeof(buf), "%.*f", 4, 3.14159);
    check_str("dynamic precision %.*f", buf, "3.1416");

    usnprintf_narrow(buf, sizeof(buf), "%+010.2f", 3.14);
    check_str("combined %+010.2f", buf, "+000003.14");

    usnprintf_narrow(buf, sizeof(buf), "%#012.5x", 255);
    check_str("combined %#012.5x", buf, "     0x000ff");

    usnprintf_narrow(buf, sizeof(buf), "% d", 42);
    check_str("space flag", buf, " 42");

    usnprintf_narrow(buf, sizeof(buf), "%s is %d years old", "Alice", 30);
    check_str("multiple args", buf, "Alice is 30 years old");
}

static void test_return_value(void) {
    char buf[256];
    int ret;

    ret = usnprintf_narrow(buf, sizeof(buf), "hello");
    check_ret("return value matches length", ret, 5);

    ret = usnprintf_narrow(buf, sizeof(buf), "%d", 42);
    check_ret("return value with args", ret, 2);
}

static void test_utf8(void) {
    char buf[256];

    usnprintf_narrow(buf, sizeof(buf), "%s", "caf\xc3\xa9");
    check_str("UTF-8 accents", buf, "caf\xc3\xa9");

    usnprintf_narrow(buf, sizeof(buf), "%s", "\xe6\xbc\xa2\xe5\xad\x97");
    check_str("UTF-8 CJK", buf, "\xe6\xbc\xa2\xe5\xad\x97");
}

int main(void) {
    printf("=== uprintf narrow tests ===\n\n");

    printf("[Integers]\n");
    test_integers();
    printf("\n[Floats]\n");
    test_floats();
    printf("\n[Strings]\n");
    test_strings();
    printf("\n[Flags/Width/Precision]\n");
    test_flags_width_precision();
    printf("\n[Return values]\n");
    test_return_value();
    printf("\n[UTF-8]\n");
    test_utf8();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
