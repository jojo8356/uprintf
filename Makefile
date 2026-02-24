# uprintf — Makefile
# Universal printf library

CC      ?= gcc
STD     ?= c11
SRCDIR   = src
INCDIR   = include
TESTDIR  = tests
BUILDDIR = build
EXDIR    = examples

# Base flags
CFLAGS_BASE = -std=$(STD) -I$(INCDIR)

# Warning flags (hardened)
CFLAGS_WARN = -Wall -Wextra -Wpedantic \
              -Wshadow \
              -Wformat=2 \
              -Wformat-security \
              -Wvla \
              -Wnull-dereference \
              -Wdouble-promotion \
              -Wcast-align \
              -Wcast-qual \
              -Wpointer-arith \
              -Wswitch-enum \
              -Wundef \
              -Wunused \
              -Wwrite-strings

# GCC-specific warnings (not supported by all Clang versions)
CFLAGS_GCC_EXTRA =
ifeq ($(CC),gcc)
    CFLAGS_GCC_EXTRA = -Wformat-overflow=2 \
                       -Wformat-truncation=2 \
                       -Wformat-signedness \
                       -Wstringop-overflow=4 \
                       -Wstringop-truncation \
                       -Warray-bounds=2 \
                       -Wshift-overflow=2 \
                       -Wstack-protector \
                       -Winit-self
endif

# Hardening flags
CFLAGS_HARDEN = -fstack-protector-strong \
                -D_FORTIFY_SOURCE=2 \
                -fno-strict-aliasing

# Linker hardening (Linux only)
LDFLAGS_HARDEN =
ifeq ($(shell uname -s),Linux)
    LDFLAGS_HARDEN = -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
endif

# Sanitizer flags (debug/test only)
CFLAGS_ASAN = -fsanitize=address,undefined \
              -fno-omit-frame-pointer \
              -fno-sanitize-recover=all
LDFLAGS_ASAN = -fsanitize=address,undefined

# Combine flags
CFLAGS  = $(CFLAGS_BASE) $(CFLAGS_WARN) $(CFLAGS_GCC_EXTRA) $(CFLAGS_HARDEN)
LDFLAGS = $(LDFLAGS_HARDEN)

# Unicode mode
ifdef UPRINTF_UNICODE
    CFLAGS += -DUPRINTF_UNICODE
endif

# Test executables
TESTS = $(BUILDDIR)/test_narrow \
        $(BUILDDIR)/test_wide \
        $(BUILDDIR)/test_snprintf \
        $(BUILDDIR)/test_security \
        $(BUILDDIR)/test_color

TESTS_ASAN = $(BUILDDIR)/test_narrow_asan \
             $(BUILDDIR)/test_wide_asan \
             $(BUILDDIR)/test_snprintf_asan \
             $(BUILDDIR)/test_security_asan \
             $(BUILDDIR)/test_color_asan

HEADERS = $(INCDIR)/uprintf.h $(INCDIR)/uprintf_config.h $(INCDIR)/uprintf_color.h

# Examples
EXAMPLES = $(BUILDDIR)/basic

# ============================================================================
# Targets
# ============================================================================

.PHONY: all test test-asan test-c99 test-unicode examples clean dirs

all: dirs $(TESTS) examples

dirs:
	@mkdir -p $(BUILDDIR)

# --- Library (compiled mode, produces .o) ---
$(BUILDDIR)/uprintf.o: $(SRCDIR)/uprintf.c $(INCDIR)/uprintf.h $(INCDIR)/uprintf_config.h | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

# --- Tests (header-only mode) ---
$(BUILDDIR)/test_narrow: $(TESTDIR)/test_narrow.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/test_wide: $(TESTDIR)/test_wide.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/test_snprintf: $(TESTDIR)/test_snprintf.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/test_security: $(TESTDIR)/test_security.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BUILDDIR)/test_color: $(TESTDIR)/test_color.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $< -lm

# --- Tests with ASAN ---
$(BUILDDIR)/test_narrow_asan: $(TESTDIR)/test_narrow.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) $(CFLAGS_ASAN) -o $@ $< $(LDFLAGS_ASAN)

$(BUILDDIR)/test_wide_asan: $(TESTDIR)/test_wide.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) $(CFLAGS_ASAN) -o $@ $< $(LDFLAGS_ASAN)

$(BUILDDIR)/test_snprintf_asan: $(TESTDIR)/test_snprintf.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) $(CFLAGS_ASAN) -o $@ $< $(LDFLAGS_ASAN)

$(BUILDDIR)/test_security_asan: $(TESTDIR)/test_security.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) $(CFLAGS_ASAN) -o $@ $< $(LDFLAGS_ASAN)

$(BUILDDIR)/test_color_asan: $(TESTDIR)/test_color.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) $(CFLAGS_ASAN) -o $@ $< $(LDFLAGS_ASAN) -lm

# --- Examples ---
$(BUILDDIR)/basic: $(EXDIR)/basic.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -o $@ $<

examples: $(EXAMPLES)

# --- Run tests ---
test: $(TESTS)
	@echo ""
	@echo "==============================="
	@echo "  Running uprintf test suite"
	@echo "==============================="
	@echo ""
	@fail=0; \
	for t in $(TESTS); do \
		echo "--- $$t ---"; \
		./$$t || fail=1; \
		echo ""; \
	done; \
	if [ $$fail -eq 0 ]; then \
		echo "=== ALL TEST SUITES PASSED ==="; \
	else \
		echo "=== SOME TESTS FAILED ==="; \
		exit 1; \
	fi

test-asan: $(TESTS_ASAN)
	@echo ""
	@echo "==============================="
	@echo "  Running tests with ASAN/UBSAN"
	@echo "==============================="
	@echo ""
	@fail=0; \
	for t in $(TESTS_ASAN); do \
		echo "--- $$t ---"; \
		./$$t || fail=1; \
		echo ""; \
	done; \
	if [ $$fail -eq 0 ]; then \
		echo "=== ALL ASAN TESTS PASSED ==="; \
	else \
		echo "=== SOME ASAN TESTS FAILED ==="; \
		exit 1; \
	fi

test-c99: STD=c99
test-c99: clean test

test-unicode: CFLAGS += -DUPRINTF_UNICODE
test-unicode: clean test

# --- Clean ---
clean:
	rm -rf $(BUILDDIR)
