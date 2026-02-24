/*
 * uprintf_color.h — Terminal color support for uprintf
 *
 * Supports: RGB, Hex (#rrggbb), HSL, OKLCH, CSS named colors (148)
 * Uses ANSI true-color escape sequences (24-bit: \033[38;2;r;g;bm)
 *
 * Usage:
 *   #include "uprintf_color.h"
 *
 *   // Compile-time macros (literal values only)
 *   uprintf(UC_FG(255,0,0) "Red text" UC_RESET "\n");
 *   uprintf(UC_BG(0,0,255) UC_FG(255,255,255) "White on blue" UC_RESET "\n");
 *   uprintf(UC_BOLD UC_FG(0,255,0) "Bold green" UC_RESET "\n");
 *
 *   // Runtime functions (any color format)
 *   char fg[UC_SEQ_MAX];
 *   uc_fg_hex(fg, "#ff6347");
 *   uprintf("%sTomato%s\n", fg, UC_RESET);
 *
 *   uc_fg_css(fg, "cornflowerblue");
 *   uprintf("%sCornflower%s\n", fg, UC_RESET);
 *
 *   uc_fg_hsl(fg, 0.0, 1.0, 0.5);   // red
 *   uc_fg_oklch(fg, 0.63, 0.26, 29); // red-ish
 *
 * Zero malloc. All buffers are caller-provided.
 * Requires: <math.h> — link with -lm on Unix.
 */

#ifndef UPRINTF_COLOR_H
#define UPRINTF_COLOR_H

#include "uprintf_config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>

/* Max size of an ANSI escape sequence buffer: "\033[48;2;255;255;255m\0" = 22 chars */
#define UC_SEQ_MAX 28

/* ========================================================================== */
/*  Compile-time macros (string literal concatenation)                        */
/* ========================================================================== */

/* Foreground RGB */
#define UC_FG(r,g,b) "\033[38;2;" #r ";" #g ";" #b "m"

/* Background RGB */
#define UC_BG(r,g,b) "\033[48;2;" #r ";" #g ";" #b "m"

/* Reset */
#define UC_RESET      "\033[0m"

/* Styles */
#define UC_BOLD       "\033[1m"
#define UC_DIM        "\033[2m"
#define UC_ITALIC     "\033[3m"
#define UC_UNDERLINE  "\033[4m"
#define UC_BLINK      "\033[5m"
#define UC_INVERSE    "\033[7m"
#define UC_HIDDEN     "\033[8m"
#define UC_STRIKE     "\033[9m"

/* Wide equivalents */
#define UC_WRESET     L"\033[0m"
#define UC_WBOLD      L"\033[1m"
#define UC_WDIM       L"\033[2m"
#define UC_WITALIC    L"\033[3m"
#define UC_WUNDERLINE L"\033[4m"
#define UC_WBLINK     L"\033[5m"
#define UC_WINVERSE   L"\033[7m"
#define UC_WHIDDEN    L"\033[8m"
#define UC_WSTRIKE    L"\033[9m"

/* TCHAR variants */
#ifdef UPRINTF_UNICODE
    #define UC_TRESET     UC_WRESET
    #define UC_TBOLD      UC_WBOLD
    #define UC_TDIM       UC_WDIM
    #define UC_TITALIC    UC_WITALIC
    #define UC_TUNDERLINE UC_WUNDERLINE
#else
    #define UC_TRESET     UC_RESET
    #define UC_TBOLD      UC_BOLD
    #define UC_TDIM       UC_DIM
    #define UC_TITALIC    UC_ITALIC
    #define UC_TUNDERLINE UC_UNDERLINE
#endif

/* ========================================================================== */
/*  Internal: RGB clamping                                                    */
/* ========================================================================== */

UPRINTF_INLINE int uc__clamp(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

UPRINTF_INLINE double uc__clampf(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* ========================================================================== */
/*  Narrow: RGB                                                               */
/* ========================================================================== */

UPRINTF_INLINE void uc_fg_rgb(char *buf, int r, int g, int b) {
    snprintf(buf, UC_SEQ_MAX, "\033[38;2;%d;%d;%dm",
             uc__clamp(r), uc__clamp(g), uc__clamp(b));
}

UPRINTF_INLINE void uc_bg_rgb(char *buf, int r, int g, int b) {
    snprintf(buf, UC_SEQ_MAX, "\033[48;2;%d;%d;%dm",
             uc__clamp(r), uc__clamp(g), uc__clamp(b));
}

/* ========================================================================== */
/*  Wide: RGB                                                                 */
/* ========================================================================== */

UPRINTF_INLINE void uc_wfg_rgb(wchar_t *buf, int r, int g, int b) {
    swprintf(buf, UC_SEQ_MAX, L"\033[38;2;%d;%d;%dm",
             uc__clamp(r), uc__clamp(g), uc__clamp(b));
}

UPRINTF_INLINE void uc_wbg_rgb(wchar_t *buf, int r, int g, int b) {
    swprintf(buf, UC_SEQ_MAX, L"\033[48;2;%d;%d;%dm",
             uc__clamp(r), uc__clamp(g), uc__clamp(b));
}

/* ========================================================================== */
/*  Narrow: Hex (#rrggbb or rrggbb)                                          */
/* ========================================================================== */

UPRINTF_INLINE int uc__hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return 0;
}

UPRINTF_INLINE void uc__parse_hex(const char *hex, int *r, int *g, int *b) {
    const char *p = hex;
    if (p == NULL) { *r = *g = *b = 0; return; }
    if (*p == '#') p++;
    if (strlen(p) < 6) { *r = *g = *b = 0; return; }
    *r = uc__hex_digit(p[0]) * 16 + uc__hex_digit(p[1]);
    *g = uc__hex_digit(p[2]) * 16 + uc__hex_digit(p[3]);
    *b = uc__hex_digit(p[4]) * 16 + uc__hex_digit(p[5]);
}

UPRINTF_INLINE void uc_fg_hex(char *buf, const char *hex) {
    int r, g, b;
    uc__parse_hex(hex, &r, &g, &b);
    uc_fg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_bg_hex(char *buf, const char *hex) {
    int r, g, b;
    uc__parse_hex(hex, &r, &g, &b);
    uc_bg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wfg_hex(wchar_t *buf, const char *hex) {
    int r, g, b;
    uc__parse_hex(hex, &r, &g, &b);
    uc_wfg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wbg_hex(wchar_t *buf, const char *hex) {
    int r, g, b;
    uc__parse_hex(hex, &r, &g, &b);
    uc_wbg_rgb(buf, r, g, b);
}

/* ========================================================================== */
/*  Narrow: HSL                                                               */
/* ========================================================================== */

UPRINTF_INLINE void uc__hsl_to_rgb(double h, double s, double l, int *r, int *g, int *b) {
    double c, x, m, rf, gf, bf;
    h = fmod(h, 360.0);
    if (h < 0.0) h += 360.0;
    s = uc__clampf(s, 0.0, 1.0);
    l = uc__clampf(l, 0.0, 1.0);

    c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    m = l - c / 2.0;

    if      (h < 60.0)  { rf = c; gf = x; bf = 0; }
    else if (h < 120.0) { rf = x; gf = c; bf = 0; }
    else if (h < 180.0) { rf = 0; gf = c; bf = x; }
    else if (h < 240.0) { rf = 0; gf = x; bf = c; }
    else if (h < 300.0) { rf = x; gf = 0; bf = c; }
    else                 { rf = c; gf = 0; bf = x; }

    *r = (int)((rf + m) * 255.0 + 0.5);
    *g = (int)((gf + m) * 255.0 + 0.5);
    *b = (int)((bf + m) * 255.0 + 0.5);
}

UPRINTF_INLINE void uc_fg_hsl(char *buf, double h, double s, double l) {
    int r, g, b;
    uc__hsl_to_rgb(h, s, l, &r, &g, &b);
    uc_fg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_bg_hsl(char *buf, double h, double s, double l) {
    int r, g, b;
    uc__hsl_to_rgb(h, s, l, &r, &g, &b);
    uc_bg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wfg_hsl(wchar_t *buf, double h, double s, double l) {
    int r, g, b;
    uc__hsl_to_rgb(h, s, l, &r, &g, &b);
    uc_wfg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wbg_hsl(wchar_t *buf, double h, double s, double l) {
    int r, g, b;
    uc__hsl_to_rgb(h, s, l, &r, &g, &b);
    uc_wbg_rgb(buf, r, g, b);
}

/* ========================================================================== */
/*  Narrow: OKLCH                                                             */
/* ========================================================================== */

#ifndef UC_PI
#define UC_PI 3.14159265358979323846
#endif

UPRINTF_INLINE double uc__srgb_transfer(double c) {
    if (c <= 0.0031308) return 12.92 * c;
    return 1.055 * pow(c, 1.0 / 2.4) - 0.055;
}

UPRINTF_INLINE void uc__oklch_to_rgb(double L, double C, double H, int *r, int *g, int *b) {
    double a, ob, l_, m_, s_, l3, m3, s3, lr, lg, lb;
    double h_rad = H * UC_PI / 180.0;

    /* OKLCH -> OKLab */
    a  = C * cos(h_rad);
    ob = C * sin(h_rad);

    /* OKLab -> LMS (approximate) */
    l_ = L + 0.3963377774 * a + 0.2158037573 * ob;
    m_ = L - 0.1055613458 * a - 0.0638541728 * ob;
    s_ = L - 0.0894841775 * a - 1.2914855480 * ob;

    /* Cube */
    l3 = l_ * l_ * l_;
    m3 = m_ * m_ * m_;
    s3 = s_ * s_ * s_;

    /* LMS -> linear sRGB */
    lr =  4.0767416621 * l3 - 3.3077115913 * m3 + 0.2309699292 * s3;
    lg = -1.2684380046 * l3 + 2.6097574011 * m3 - 0.3413193965 * s3;
    lb = -0.0041960863 * l3 + 0.7341731176 * m3 + 0.2676544482 * s3;

    /* Gamma correction + clamp to [0, 255] */
    *r = (int)(uc__clampf(uc__srgb_transfer(lr), 0.0, 1.0) * 255.0 + 0.5);
    *g = (int)(uc__clampf(uc__srgb_transfer(lg), 0.0, 1.0) * 255.0 + 0.5);
    *b = (int)(uc__clampf(uc__srgb_transfer(lb), 0.0, 1.0) * 255.0 + 0.5);
}

UPRINTF_INLINE void uc_fg_oklch(char *buf, double L, double C, double H) {
    int r, g, b;
    uc__oklch_to_rgb(L, C, H, &r, &g, &b);
    uc_fg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_bg_oklch(char *buf, double L, double C, double H) {
    int r, g, b;
    uc__oklch_to_rgb(L, C, H, &r, &g, &b);
    uc_bg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wfg_oklch(wchar_t *buf, double L, double C, double H) {
    int r, g, b;
    uc__oklch_to_rgb(L, C, H, &r, &g, &b);
    uc_wfg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wbg_oklch(wchar_t *buf, double L, double C, double H) {
    int r, g, b;
    uc__oklch_to_rgb(L, C, H, &r, &g, &b);
    uc_wbg_rgb(buf, r, g, b);
}

/* ========================================================================== */
/*  CSS named colors (148 standard colors)                                    */
/* ========================================================================== */

typedef struct {
    const char *name;
    unsigned char r, g, b;
} uc__css_color;

/* Case-insensitive comparison */
UPRINTF_INLINE int uc__strcasecmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + 32);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static const uc__css_color uc__css_colors[] = {
    {"aliceblue",            240, 248, 255},
    {"antiquewhite",         250, 235, 215},
    {"aqua",                   0, 255, 255},
    {"aquamarine",           127, 255, 212},
    {"azure",                240, 255, 255},
    {"beige",                245, 245, 220},
    {"bisque",               255, 228, 196},
    {"black",                  0,   0,   0},
    {"blanchedalmond",       255, 235, 205},
    {"blue",                   0,   0, 255},
    {"blueviolet",           138,  43, 226},
    {"brown",                165,  42,  42},
    {"burlywood",            222, 184, 135},
    {"cadetblue",             95, 158, 160},
    {"chartreuse",           127, 255,   0},
    {"chocolate",            210, 105,  30},
    {"coral",                255, 127,  80},
    {"cornflowerblue",       100, 149, 237},
    {"cornsilk",             255, 248, 220},
    {"crimson",              220,  20,  60},
    {"cyan",                   0, 255, 255},
    {"darkblue",               0,   0, 139},
    {"darkcyan",               0, 139, 139},
    {"darkgoldenrod",        184, 134,  11},
    {"darkgray",             169, 169, 169},
    {"darkgreen",              0, 100,   0},
    {"darkgrey",             169, 169, 169},
    {"darkkhaki",            189, 183, 107},
    {"darkmagenta",          139,   0, 139},
    {"darkolivegreen",        85, 107,  47},
    {"darkorange",           255, 140,   0},
    {"darkorchid",           153,  50, 204},
    {"darkred",              139,   0,   0},
    {"darksalmon",           233, 150, 122},
    {"darkseagreen",         143, 188, 143},
    {"darkslateblue",         72,  61, 139},
    {"darkslategray",         47,  79,  79},
    {"darkslategrey",         47,  79,  79},
    {"darkturquoise",          0, 206, 209},
    {"darkviolet",           148,   0, 211},
    {"deeppink",             255,  20, 147},
    {"deepskyblue",            0, 191, 255},
    {"dimgray",              105, 105, 105},
    {"dimgrey",              105, 105, 105},
    {"dodgerblue",            30, 144, 255},
    {"firebrick",            178,  34,  34},
    {"floralwhite",          255, 250, 240},
    {"forestgreen",           34, 139,  34},
    {"fuchsia",              255,   0, 255},
    {"gainsboro",            220, 220, 220},
    {"ghostwhite",           248, 248, 255},
    {"gold",                 255, 215,   0},
    {"goldenrod",            218, 165,  32},
    {"gray",                 128, 128, 128},
    {"green",                  0, 128,   0},
    {"greenyellow",          173, 255,  47},
    {"grey",                 128, 128, 128},
    {"honeydew",             240, 255, 240},
    {"hotpink",              255, 105, 180},
    {"indianred",            205,  92,  92},
    {"indigo",                75,   0, 130},
    {"ivory",                255, 255, 240},
    {"khaki",                240, 230, 140},
    {"lavender",             230, 230, 250},
    {"lavenderblush",        255, 240, 245},
    {"lawngreen",            124, 252,   0},
    {"lemonchiffon",         255, 250, 205},
    {"lightblue",            173, 216, 230},
    {"lightcoral",           240, 128, 128},
    {"lightcyan",            224, 255, 255},
    {"lightgoldenrodyellow", 250, 250, 210},
    {"lightgray",            211, 211, 211},
    {"lightgreen",           144, 238, 144},
    {"lightgrey",            211, 211, 211},
    {"lightpink",            255, 182, 193},
    {"lightsalmon",          255, 160, 122},
    {"lightseagreen",         32, 178, 170},
    {"lightskyblue",         135, 206, 250},
    {"lightslategray",       119, 136, 153},
    {"lightslategrey",       119, 136, 153},
    {"lightsteelblue",       176, 196, 222},
    {"lightyellow",          255, 255, 224},
    {"lime",                   0, 255,   0},
    {"limegreen",             50, 205,  50},
    {"linen",                250, 240, 230},
    {"magenta",              255,   0, 255},
    {"maroon",               128,   0,   0},
    {"mediumaquamarine",     102, 205, 170},
    {"mediumblue",             0,   0, 205},
    {"mediumorchid",         186,  85, 211},
    {"mediumpurple",         147, 112, 219},
    {"mediumseagreen",        60, 179, 113},
    {"mediumslateblue",      123, 104, 238},
    {"mediumspringgreen",      0, 250, 154},
    {"mediumturquoise",       72, 209, 204},
    {"mediumvioletred",      199,  21, 133},
    {"midnightblue",          25,  25, 112},
    {"mintcream",            245, 255, 250},
    {"mistyrose",            255, 228, 225},
    {"moccasin",             255, 228, 181},
    {"navajowhite",          255, 222, 173},
    {"navy",                   0,   0, 128},
    {"oldlace",              253, 245, 230},
    {"olive",                128, 128,   0},
    {"olivedrab",            107, 142,  35},
    {"orange",               255, 165,   0},
    {"orangered",            255,  69,   0},
    {"orchid",               218, 112, 214},
    {"palegoldenrod",        238, 232, 170},
    {"palegreen",            152, 251, 152},
    {"paleturquoise",        175, 238, 238},
    {"palevioletred",        219, 112, 147},
    {"papayawhip",           255, 239, 213},
    {"peachpuff",            255, 218, 185},
    {"peru",                 205, 133,  63},
    {"pink",                 255, 192, 203},
    {"plum",                 221, 160, 221},
    {"powderblue",           176, 224, 230},
    {"purple",               128,   0, 128},
    {"rebeccapurple",        102,  51, 153},
    {"red",                  255,   0,   0},
    {"rosybrown",            188, 143, 143},
    {"royalblue",             65, 105, 225},
    {"saddlebrown",          139,  69,  19},
    {"salmon",               250, 128, 114},
    {"sandybrown",           244, 164,  96},
    {"seagreen",              46, 139,  87},
    {"seashell",             255, 245, 238},
    {"sienna",               160,  82,  45},
    {"silver",               192, 192, 192},
    {"skyblue",              135, 206, 235},
    {"slateblue",            106,  90, 205},
    {"slategray",            112, 128, 144},
    {"slategrey",            112, 128, 144},
    {"snow",                 255, 250, 250},
    {"springgreen",            0, 255, 127},
    {"steelblue",             70, 130, 180},
    {"tan",                  210, 180, 140},
    {"teal",                   0, 128, 128},
    {"thistle",              216, 191, 216},
    {"tomato",               255,  99,  71},
    {"turquoise",             64, 224, 208},
    {"violet",               238, 130, 238},
    {"wheat",                245, 222, 179},
    {"white",                255, 255, 255},
    {"whitesmoke",           245, 245, 245},
    {"yellow",               255, 255,   0},
    {"yellowgreen",          154, 205,  50}
};

#define UC__CSS_COLOR_COUNT (sizeof(uc__css_colors) / sizeof(uc__css_colors[0]))

UPRINTF_INLINE int uc__css_lookup(const char *name, int *r, int *g, int *b) {
    /* Binary search (table is sorted alphabetically) */
    int lo = 0;
    int hi = (int)UC__CSS_COLOR_COUNT - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = uc__strcasecmp(name, uc__css_colors[mid].name);
        if (cmp == 0) {
            *r = uc__css_colors[mid].r;
            *g = uc__css_colors[mid].g;
            *b = uc__css_colors[mid].b;
            return 0;
        }
        if (cmp < 0) hi = mid - 1;
        else lo = mid + 1;
    }
    *r = *g = *b = 0;
    return -1; /* Not found */
}

UPRINTF_INLINE void uc_fg_css(char *buf, const char *name) {
    int r, g, b;
    uc__css_lookup(name, &r, &g, &b);
    uc_fg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_bg_css(char *buf, const char *name) {
    int r, g, b;
    uc__css_lookup(name, &r, &g, &b);
    uc_bg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wfg_css(wchar_t *buf, const char *name) {
    int r, g, b;
    uc__css_lookup(name, &r, &g, &b);
    uc_wfg_rgb(buf, r, g, b);
}

UPRINTF_INLINE void uc_wbg_css(wchar_t *buf, const char *name) {
    int r, g, b;
    uc__css_lookup(name, &r, &g, &b);
    uc_wbg_rgb(buf, r, g, b);
}

/* ========================================================================== */
/*  Windows Virtual Terminal init                                             */
/* ========================================================================== */

#if defined(UPRINTF_WINDOWS)
#include <windows.h>
UPRINTF_INLINE void uc_init(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
    }
}
#else
UPRINTF_INLINE void uc_init(void) {
    /* ANSI escapes work natively on Unix terminals */
}
#endif

#endif /* UPRINTF_COLOR_H */
