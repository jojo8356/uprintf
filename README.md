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

Copy `include/uprintf.h` and `include/uprintf_config.h` into your project.

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
make test       # Run all tests (114 tests)
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

Copy `include/uprintf.h` and `include/uprintf_config.h` into your project. That's it.

## License

MIT
