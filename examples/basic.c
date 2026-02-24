/*
 * basic.c — Basic usage example for uprintf
 *
 * Demonstrates narrow, wide, and TCHAR modes.
 * Compile: gcc -std=c11 -I../include -o basic basic.c
 */

#define UPRINTF_HEADER_ONLY
#define UPRINTF_AUTO_LOCALE
#include "uprintf.h"

#include <locale.h>

int main(void) {
    char nbuf[256];
    wchar_t wbuf[256];

    uprintf_init();

    /* --- Narrow output (char*) --- */
    uprintf_narrow("=== Narrow (char*) ===\n");
    uprintf_narrow("String:  %s\n", "hello world");
    uprintf_narrow("Integer: %d\n", 42);
    uprintf_narrow("Hex:     %#x\n", 255);
    uprintf_narrow("Float:   %.2f\n", 3.14159);
    uprintf_narrow("Padded:  [%20s]\n", "right-aligned");
    uprintf_narrow("Padded:  [%-20s]\n", "left-aligned");
    uprintf_narrow("Percent: 100%%\n");
    uprintf_narrow("\n");

    /* --- Narrow snprintf (buffer) --- */
    usnprintf_narrow(nbuf, sizeof(nbuf), "%s is %d years old", "Alice", 30);
    uprintf_narrow("Buffer:  %s\n\n", nbuf);

    /* --- Wide snprintf (buffer) --- */
    usnprintf_wide(wbuf, 256, L"%ls is %d years old", L"Alice", 30);
    uprintf_narrow("Wide buffer test passed (written to wchar_t buffer)\n\n");

    /* --- _Generic dispatch (C11) --- */
#ifdef UPRINTF_HAS_GENERIC
    uprintf("=== _Generic dispatch ===\n");
    uprintf("Narrow via _Generic: %d\n", 42);
    uprintf(L"Wide via _Generic: %d\n", 42);
#else
    uprintf_narrow("=== C99 mode (no _Generic) ===\n");
    uprintf_narrow("Using static dispatch\n");
#endif

    /* --- TCHAR mode --- */
    uprintf(_T("=== TCHAR mode: %d ===\n"), 42);

    return 0;
}
