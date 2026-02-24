/*
 * test_color.c — Tests for uprintf_color.h
 * Tests color conversions (RGB, Hex, HSL, OKLCH, CSS) and escape generation.
 */

#define UPRINTF_HEADER_ONLY
#include "uprintf.h"
#include "uprintf_color.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>

static int g_pass = 0;
static int g_fail = 0;

static void check_str(const char *test_name, const char *got, const char *expected) {
    printf("  [TEST] %s... ", test_name);
    if (strcmp(got, expected) == 0) { printf("OK\n"); g_pass++; }
    else { printf("FAIL: got \"%s\", expected \"%s\"\n", got, expected); g_fail++; }
}

static void check_true(const char *test_name, int cond) {
    printf("  [TEST] %s... ", test_name);
    if (cond) { printf("OK\n"); g_pass++; }
    else { printf("FAIL\n"); g_fail++; }
}

static void check_rgb(const char *test_name, const char *buf, int r, int g_val, int b) {
    char expected[UC_SEQ_MAX];
    snprintf(expected, sizeof(expected), "\033[38;2;%d;%d;%dm", r, g_val, b);
    check_str(test_name, buf, expected);
}

static void check_bg_rgb(const char *test_name, const char *buf, int r, int g_val, int b) {
    char expected[UC_SEQ_MAX];
    snprintf(expected, sizeof(expected), "\033[48;2;%d;%d;%dm", r, g_val, b);
    check_str(test_name, buf, expected);
}

/* ========================================================================== */
/*  RGB tests                                                                 */
/* ========================================================================== */

static void test_rgb(void) {
    char buf[UC_SEQ_MAX];

    uc_fg_rgb(buf, 255, 0, 0);
    check_str("fg red", buf, "\033[38;2;255;0;0m");

    uc_fg_rgb(buf, 0, 255, 0);
    check_str("fg green", buf, "\033[38;2;0;255;0m");

    uc_fg_rgb(buf, 0, 0, 255);
    check_str("fg blue", buf, "\033[38;2;0;0;255m");

    uc_bg_rgb(buf, 255, 255, 255);
    check_str("bg white", buf, "\033[48;2;255;255;255m");

    uc_fg_rgb(buf, 0, 0, 0);
    check_str("fg black", buf, "\033[38;2;0;0;0m");

    /* Clamping */
    uc_fg_rgb(buf, 300, -10, 128);
    check_str("clamped values", buf, "\033[38;2;255;0;128m");
}

/* ========================================================================== */
/*  Hex tests                                                                 */
/* ========================================================================== */

static void test_hex(void) {
    char buf[UC_SEQ_MAX];

    uc_fg_hex(buf, "#ff0000");
    check_rgb("hex #ff0000", buf, 255, 0, 0);

    uc_fg_hex(buf, "00ff00");
    check_rgb("hex 00ff00 (no #)", buf, 0, 255, 0);

    uc_fg_hex(buf, "#0000FF");
    check_rgb("hex #0000FF (uppercase)", buf, 0, 0, 255);

    uc_bg_hex(buf, "#ffffff");
    check_bg_rgb("bg hex #ffffff", buf, 255, 255, 255);

    uc_fg_hex(buf, "#000000");
    check_rgb("hex #000000", buf, 0, 0, 0);

    uc_fg_hex(buf, "#ff6347");
    check_rgb("hex tomato #ff6347", buf, 255, 99, 71);

    /* Edge cases */
    uc_fg_hex(buf, NULL);
    check_rgb("hex NULL", buf, 0, 0, 0);

    uc_fg_hex(buf, "#fff");
    check_rgb("hex too short", buf, 0, 0, 0);
}

/* ========================================================================== */
/*  HSL tests                                                                 */
/* ========================================================================== */

static void test_hsl(void) {
    char buf[UC_SEQ_MAX];

    /* Red: H=0, S=1, L=0.5 -> RGB(255, 0, 0) */
    uc_fg_hsl(buf, 0.0, 1.0, 0.5);
    check_rgb("hsl red (0,1,0.5)", buf, 255, 0, 0);

    /* Green: H=120, S=1, L=0.5 -> RGB(0, 255, 0) */
    uc_fg_hsl(buf, 120.0, 1.0, 0.5);
    check_rgb("hsl green (120,1,0.5)", buf, 0, 255, 0);

    /* Blue: H=240, S=1, L=0.5 -> RGB(0, 0, 255) */
    uc_fg_hsl(buf, 240.0, 1.0, 0.5);
    check_rgb("hsl blue (240,1,0.5)", buf, 0, 0, 255);

    /* White: H=0, S=0, L=1 -> RGB(255, 255, 255) */
    uc_fg_hsl(buf, 0.0, 0.0, 1.0);
    check_rgb("hsl white (0,0,1)", buf, 255, 255, 255);

    /* Black: H=0, S=0, L=0 -> RGB(0, 0, 0) */
    uc_fg_hsl(buf, 0.0, 0.0, 0.0);
    check_rgb("hsl black (0,0,0)", buf, 0, 0, 0);

    /* Yellow: H=60, S=1, L=0.5 -> RGB(255, 255, 0) */
    uc_fg_hsl(buf, 60.0, 1.0, 0.5);
    check_rgb("hsl yellow (60,1,0.5)", buf, 255, 255, 0);

    /* Cyan: H=180, S=1, L=0.5 -> RGB(0, 255, 255) */
    uc_fg_hsl(buf, 180.0, 1.0, 0.5);
    check_rgb("hsl cyan (180,1,0.5)", buf, 0, 255, 255);

    /* Background */
    uc_bg_hsl(buf, 0.0, 1.0, 0.5);
    check_bg_rgb("bg hsl red", buf, 255, 0, 0);
}

/* ========================================================================== */
/*  OKLCH tests                                                               */
/* ========================================================================== */

static void test_oklch(void) {
    char buf[UC_SEQ_MAX];
    int r, g, b;

    /* Black: L=0, C=0, H=0 -> RGB(0, 0, 0) */
    uc__oklch_to_rgb(0.0, 0.0, 0.0, &r, &g, &b);
    check_true("oklch black (0,0,0)", r == 0 && g == 0 && b == 0);

    /* White: L=1, C=0, H=0 -> RGB(255, 255, 255) */
    uc__oklch_to_rgb(1.0, 0.0, 0.0, &r, &g, &b);
    check_true("oklch white (1,0,0)", r == 255 && g == 255 && b == 255);

    /* Gray: L=0.5, C=0, H=0 -> should be gray */
    uc__oklch_to_rgb(0.5, 0.0, 0.0, &r, &g, &b);
    check_true("oklch gray (0.5,0,0) r==g==b", r == g && g == b);

    /* Red-ish: high chroma, low hue */
    uc__oklch_to_rgb(0.63, 0.26, 29.0, &r, &g, &b);
    check_true("oklch red-ish: r > g and r > b", r > g && r > b);

    /* Green-ish: hue ~142 */
    uc__oklch_to_rgb(0.52, 0.17, 142.0, &r, &g, &b);
    check_true("oklch green-ish: g > r and g > b", g > r && g > b);

    /* Blue-ish: hue ~265 */
    uc__oklch_to_rgb(0.45, 0.31, 265.0, &r, &g, &b);
    check_true("oklch blue-ish: b > r and b > g", b > r && b > g);

    /* Ensure escape sequence is generated */
    uc_fg_oklch(buf, 0.63, 0.26, 29.0);
    check_true("oklch fg generates escape", strncmp(buf, "\033[38;2;", 7) == 0);

    uc_bg_oklch(buf, 0.63, 0.26, 29.0);
    check_true("oklch bg generates escape", strncmp(buf, "\033[48;2;", 7) == 0);
}

/* ========================================================================== */
/*  CSS named colors tests                                                    */
/* ========================================================================== */

static void test_css(void) {
    char buf[UC_SEQ_MAX];

    uc_fg_css(buf, "red");
    check_rgb("css red", buf, 255, 0, 0);

    uc_fg_css(buf, "blue");
    check_rgb("css blue", buf, 0, 0, 255);

    uc_fg_css(buf, "green");
    check_rgb("css green", buf, 0, 128, 0);

    uc_fg_css(buf, "white");
    check_rgb("css white", buf, 255, 255, 255);

    uc_fg_css(buf, "black");
    check_rgb("css black", buf, 0, 0, 0);

    uc_fg_css(buf, "tomato");
    check_rgb("css tomato", buf, 255, 99, 71);

    uc_fg_css(buf, "cornflowerblue");
    check_rgb("css cornflowerblue", buf, 100, 149, 237);

    uc_fg_css(buf, "rebeccapurple");
    check_rgb("css rebeccapurple", buf, 102, 51, 153);

    uc_bg_css(buf, "navy");
    check_bg_rgb("css bg navy", buf, 0, 0, 128);

    /* Case insensitive */
    uc_fg_css(buf, "DarkSlateGray");
    check_rgb("css case-insensitive", buf, 47, 79, 79);

    uc_fg_css(buf, "GOLD");
    check_rgb("css uppercase GOLD", buf, 255, 215, 0);

    /* Unknown color -> black */
    uc_fg_css(buf, "notacolor");
    check_rgb("css unknown -> black", buf, 0, 0, 0);
}

/* ========================================================================== */
/*  Wide variants tests                                                       */
/* ========================================================================== */

static void test_wide(void) {
    wchar_t wbuf[UC_SEQ_MAX];

    uc_wfg_rgb(wbuf, 255, 0, 0);
    check_true("wide fg rgb red", wcsncmp(wbuf, L"\033[38;2;255;0;0m", 15) == 0);

    uc_wbg_rgb(wbuf, 0, 0, 255);
    check_true("wide bg rgb blue", wcsncmp(wbuf, L"\033[48;2;0;0;255m", 15) == 0);

    uc_wfg_hex(wbuf, "#ff6347");
    check_true("wide fg hex tomato", wcsncmp(wbuf, L"\033[38;2;255;99;71m", 17) == 0);

    uc_wfg_css(wbuf, "gold");
    check_true("wide fg css gold", wcsncmp(wbuf, L"\033[38;2;255;215;0m", 17) == 0);
}

/* ========================================================================== */
/*  Compile-time macro tests                                                  */
/* ========================================================================== */

static void test_macros(void) {
    check_str("UC_FG macro", UC_FG(255,0,0), "\033[38;2;255;0;0m");
    check_str("UC_BG macro", UC_BG(0,0,255), "\033[48;2;0;0;255m");
    check_str("UC_RESET macro", UC_RESET, "\033[0m");
    check_str("UC_BOLD macro", UC_BOLD, "\033[1m");
    check_str("UC_ITALIC macro", UC_ITALIC, "\033[3m");
    check_str("UC_UNDERLINE macro", UC_UNDERLINE, "\033[4m");

    /* Concatenation test */
    {
        char buf[256];
        snprintf(buf, sizeof(buf), UC_FG(255,0,0) "red" UC_RESET);
        check_true("macro concatenation", strlen(buf) > 3);
    }
}

/* ========================================================================== */
/*  Visual demo (not validated, just prints colored text)                      */
/* ========================================================================== */

static void test_visual_demo(void) {
    char fg[UC_SEQ_MAX];
    char bg[UC_SEQ_MAX];

    printf("\n  --- Visual demo (check colors visually) ---\n");

    printf("  " UC_FG(255,0,0) "Red" UC_RESET " ");
    printf(UC_FG(0,255,0) "Green" UC_RESET " ");
    printf(UC_FG(0,0,255) "Blue" UC_RESET " ");
    printf(UC_BOLD "Bold" UC_RESET " ");
    printf(UC_ITALIC "Italic" UC_RESET " ");
    printf(UC_UNDERLINE "Underline" UC_RESET "\n");

    uc_fg_hex(fg, "#ff6347");
    printf("  %sTomato (hex)%s ", fg, UC_RESET);

    uc_fg_css(fg, "cornflowerblue");
    printf("%sCornflower (css)%s ", fg, UC_RESET);

    uc_fg_hsl(fg, 280.0, 0.8, 0.6);
    printf("%sPurple (hsl)%s ", fg, UC_RESET);

    uc_fg_oklch(fg, 0.7, 0.15, 150.0);
    printf("%sGreen (oklch)%s\n", fg, UC_RESET);

    uc_fg_css(fg, "white");
    uc_bg_css(bg, "darkslateblue");
    printf("  %s%s White on DarkSlateBlue %s\n", fg, bg, UC_RESET);

    printf("  --- End demo ---\n");
}

int main(void) {
    uc_init();

    printf("=== uprintf color tests ===\n\n");

    printf("[RGB]\n");
    test_rgb();
    printf("\n[Hex]\n");
    test_hex();
    printf("\n[HSL]\n");
    test_hsl();
    printf("\n[OKLCH]\n");
    test_oklch();
    printf("\n[CSS named colors]\n");
    test_css();
    printf("\n[Wide variants]\n");
    test_wide();
    printf("\n[Compile-time macros]\n");
    test_macros();

    test_visual_demo();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
