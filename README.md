# uprintf

**Universal printf for C** — a single API that dispatches between `printf` (char\*) and `wprintf` (wchar\_t\*) automatically.

Portable across Linux, macOS, Windows (MSVC, MinGW), and BSD. Zero dependencies. Zero malloc. Header-only.

## Why?

C has two incompatible printf families:

| Family | Functions | String type |
|--------|-----------|-------------|
| Narrow | `printf`, `fprintf`, `snprintf` | `char*` |
| Wide | `wprintf`, `fwprintf`, `swprintf` | `wchar_t*` |

Windows offers `_tprintf` via `<tchar.h>`, but it's Microsoft-only. **uprintf** solves this portably using C11 `_Generic` with a C99 fallback.

## Quick start

Copy `include/uprintf.h`, `include/uprintf_config.h`, and optionally `include/uprintf_color.h` into your project.

```c
#define UPRINTF_HEADER_ONLY
#include "uprintf.h"

int main(void) {
    uprintf_init();

    // Narrow (char*) — dispatches to printf
    uprintf("Hello %s, you are %d\n", "world", 42);

    // Wide (wchar_t*) — dispatches to wprintf
    uprintf(L"Hello %ls, you are %d\n", L"world", 42);

    // Buffer variant
    char buf[256];
    usnprintf(buf, sizeof(buf), "value=%d", 42);

    return 0;
}
```

Compile:

```bash
gcc -std=c11 -Iinclude -o example example.c
```

## API

### Output functions

| Function | Description |
|----------|-------------|
| `uprintf(fmt, ...)` | Print to stdout |
| `ufprintf(stream, fmt, ...)` | Print to FILE* |
| `usnprintf(buf, n, fmt, ...)` | Print to buffer (bounded) |
| `usprintf(buf, fmt, ...)` | Print to buffer (legacy, unbounded) |

Each function exists in two variants: `*_narrow` (char\*) and `*_wide` (wchar\_t\*). The macros above auto-dispatch via `_Generic` in C11, or statically via `UPRINTF_UNICODE` in C99.

### TCHAR compatibility

```c
#define UPRINTF_UNICODE  // optional: forces wide mode
#include "uprintf.h"

TCHAR name[] = _T("world");
uprintf(_T("Hello %s\n"), name);
```

Works like Windows `<tchar.h>` but is portable. Does not conflict with `<tchar.h>` if both are included.

### Initialization

```c
uprintf_init();
```

Call once at program start. On Unix, sets the locale (`setlocale`). On Windows, configures the console for UTF-16 (`_setmode`).

## Format specifiers

All standard C format specifiers are supported without modification. uprintf delegates directly to the platform's native printf/wprintf.

Full syntax: `%[flags][width][.precision][length]specifier`

**Flags:** `-` `+` ` ` `0` `#`

**Width:** fixed (`%10d`) or dynamic (`%*d`)

**Precision:** fixed (`%.2f`) or dynamic (`%.*f`)

**Length modifiers:** `hh` `h` `l` `ll` `L` `j` `z` `t`

**Specifiers:** `d` `i` `u` `o` `x` `X` `f` `F` `e` `E` `g` `G` `a` `A` `c` `s` `p` `n` `%`

Compatible with `<inttypes.h>` macros (`PRId32`, `PRIu64`, etc.).

## Color support

Include `uprintf_color.h` for true-color (24-bit) terminal output with multiple color spaces.

```c
#include "uprintf_color.h"

int main(void) {
    uc_init();  // enable virtual terminal on Windows
    char fg[UC_SEQ_MAX], bg[UC_SEQ_MAX];

    // Compile-time macros
    printf(UC_FG(255,0,0) "Red text" UC_RESET "\n");
    printf(UC_BOLD UC_FG(0,255,0) "Bold green" UC_RESET "\n");

    // Runtime — RGB
    uc_fg_rgb(fg, 255, 165, 0);
    printf("%sOrange%s\n", fg, UC_RESET);

    // Runtime — Hex
    uc_fg_hex(fg, "#ff6347");
    printf("%sTomato%s\n", fg, UC_RESET);

    // Runtime — HSL (hue 0-360, saturation 0-1, lightness 0-1)
    uc_fg_hsl(fg, 280.0, 0.8, 0.6);
    printf("%sPurple%s\n", fg, UC_RESET);

    // Runtime — OKLCH (lightness 0-1, chroma 0-0.4, hue 0-360)
    uc_fg_oklch(fg, 0.7, 0.15, 150.0);
    printf("%sGreen%s\n", fg, UC_RESET);

    // Runtime — CSS named colors (148 colors)
    uc_fg_css(fg, "cornflowerblue");
    uc_bg_css(bg, "darkslateblue");
    printf("%s%sStyled%s\n", fg, bg, UC_RESET);

    return 0;
}
```

### Color functions

All functions exist in foreground (`uc_fg_*`) and background (`uc_bg_*`) variants, plus wide equivalents (`uc_wfg_*`, `uc_wbg_*`).

| Function | Description |
|----------|-------------|
| `uc_fg_rgb(buf, r, g, b)` | RGB values (0-255, clamped) |
| `uc_fg_hex(buf, "#rrggbb")` | Hex string (with or without `#`) |
| `uc_fg_hsl(buf, h, s, l)` | HSL (h: 0-360, s/l: 0-1) |
| `uc_fg_oklch(buf, l, c, h)` | OKLCH perceptual color space |
| `uc_fg_css(buf, "name")` | CSS named color (case-insensitive) |

### Compile-time macros

| Macro | Description |
|-------|-------------|
| `UC_FG(r,g,b)` / `UC_BG(r,g,b)` | Foreground/background color literal |
| `UC_RESET` | Reset all attributes |
| `UC_BOLD` `UC_DIM` `UC_ITALIC` | Text styles |
| `UC_UNDERLINE` `UC_BLINK` `UC_INVERSE` | Text styles |
| `UC_HIDDEN` `UC_STRIKE` | Text styles |

Compile with `-lm` when using HSL or OKLCH (requires `<math.h>`).

## Configuration macros

Define before including `uprintf.h`:

| Macro | Effect |
|-------|--------|
| `UPRINTF_HEADER_ONLY` | Header-only mode (no .c file needed) |
| `UPRINTF_UNICODE` | Force wide mode (TCHAR = wchar\_t) |
| `UPRINTF_AUTO_LOCALE` | Auto-init locale on Unix |
| `UPRINTF_AUTO_CONSOLE` | Auto-init console on Windows |
| `UPRINTF_NO_GENERIC` | Force C99 mode (no \_Generic) |
| `UPRINTF_ENABLE_N` | Allow %n specifier (disabled by default) |
| `UPRINTF_DEBUG` | Enable internal assertions |

## Security

- **Zero malloc** — no dynamic allocation, ever. Eliminates use-after-free, double free, memory leaks, and heap overflow.
- **%n disabled by default** — format strings containing `%n` are rejected unless `UPRINTF_ENABLE_N` is defined.
- **NULL-safe** — NULL format strings or buffers return `-1` instead of crashing.
- **Null-termination guaranteed** — `usnprintf` always null-terminates, even on truncation (fixes MSVC `_snprintf` behavior).
- **Format checking** — `__attribute__((format(printf)))` on GCC/Clang, SAL annotations on MSVC.
- **Hardened build** — Makefile includes 30+ warning flags, stack protectors, FORTIFY\_SOURCE, and ASAN/UBSAN support.

## Building

```bash
make            # Build tests + examples
make test       # Run all tests (167 tests)
make test-asan  # Run with AddressSanitizer + UBSan
make STD=c99 test       # Test in C99 mode
make UPRINTF_UNICODE=1 test  # Test in wide mode
make clean      # Clean build artifacts
```

## Compiled mode

If you prefer not to use header-only mode, compile `src/uprintf.c` and link it:

```bash
gcc -std=c11 -Iinclude -c src/uprintf.c -o uprintf.o
gcc -std=c11 -Iinclude -o myapp myapp.c uprintf.o
```

## Platform support

| Platform | Compilers | wchar\_t |
|----------|-----------|----------|
| Linux | GCC, Clang | UTF-32 (4 bytes) |
| macOS | Apple Clang, GCC | UTF-32 (4 bytes) |
| Windows | MSVC, MinGW-w64, Clang-cl | UTF-16 (2 bytes) |
| FreeBSD/OpenBSD | Clang, GCC | UTF-32 (4 bytes) |

Requires **C99** minimum. **C11** recommended for `_Generic` auto-dispatch.

## Install

### clib

```bash
clib install jojo8356/uprintf
```

### vcpkg

```bash
vcpkg install uprintf
```

### Conan

```bash
conan install --requires=uprintf/1.0.0
```

### Manual

Copy `include/uprintf.h`, `include/uprintf_config.h`, and optionally `include/uprintf_color.h` into your project. That's it.

## License

MIT
