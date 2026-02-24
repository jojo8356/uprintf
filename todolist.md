# uprintf — Todolist

## Phase 1 : Fondations
- [ ] Créer la structure de dossiers (`include/`, `src/`, `tests/`, `examples/`)
- [ ] Écrire `include/uprintf_config.h` — détection plateforme + compilateur + C11
- [ ] Écrire `include/uprintf.h` — API publique, macros de dispatch `_Generic`
- [ ] Écrire le fallback C99 (dispatch via `UPRINTF_UNICODE` au préprocesseur)
- [ ] Implémenter les macros `TCHAR`, `_T()`, `_U()` portables
- [ ] Implémenter `uprintf_init()` (locale Unix + console Windows)

## Phase 2 : Variantes de fonctions
- [ ] `uprintf(fmt, ...)` — sortie stdout
- [ ] `ufprintf(stream, fmt, ...)` — sortie FILE*
- [ ] `usnprintf(buf, n, fmt, ...)` — écriture buffer avec limite
- [ ] `usprintf(buf, fmt, ...)` — écriture buffer legacy
- [ ] Vérifier que la valeur de retour est correcte pour chaque variante

## Phase 3 : Portabilité Windows
- [ ] Tester compilation MSVC 2019/2022
- [ ] Tester compilation MinGW-w64
- [ ] Tester compilation Clang-cl
- [ ] Gérer `_setmode` / `WriteConsoleW` pour la console Windows
- [ ] Vérifier absence de conflit avec `<tchar.h>` natif
- [ ] Gérer `%n` désactivé par défaut sur MSVC (`UPRINTF_ENABLE_N`)

## Phase 4 : Portabilité Unix
- [ ] Tester compilation GCC (9, 12, 14) sur Linux
- [ ] Tester compilation Clang (14, 18) sur Linux
- [ ] Tester compilation Apple Clang sur macOS
- [ ] Tester sur FreeBSD/OpenBSD (si possible)
- [ ] Valider `setlocale` automatique (`UPRINTF_AUTO_LOCALE`)

## Phase 5 : Tests unitaires
- [ ] `test_narrow.c` — tous les spécificateurs avec `char*` format
- [ ] `test_wide.c` — tous les spécificateurs avec `wchar_t*` format
- [ ] `test_tchar.c` — mode TCHAR/_T() avec et sans `UPRINTF_UNICODE`
- [ ] `test_snprintf.c` — buffer overflow, troncature, valeur de retour
- [ ] `test_platform.c` — comportements spécifiques plateforme
- [ ] Tester les flags (`-`, `+`, ` `, `0`, `#`) et combinaisons
- [ ] Tester width fixe et dynamique (`*`)
- [ ] Tester precision fixe et dynamique (`.*`) pour chaque famille
- [ ] Tester tous les length modifiers (`hh`, `h`, `l`, `ll`, `L`, `j`, `z`, `t`)
- [ ] Tester entiers : `%d`, `%i`, `%u`, `%o`, `%x`, `%X`
- [ ] Tester flottants : `%f`, `%F`, `%e`, `%E`, `%g`, `%G`, `%a`, `%A`
- [ ] Tester chars/strings : `%c`, `%lc`, `%s`, `%ls`, `%hs`
- [ ] Tester `%p`, `%n`, `%%`
- [ ] Tester caractères spéciaux (é, ñ, 漢字) et emoji
- [ ] Tester compatibilité `<inttypes.h>` (`PRId32`, `PRIu64`, etc.)
- [ ] Tester `UPRI32` et équivalents wide des macros inttypes

## Phase 6 : Sécurité et hardening
- [ ] Implémenter `uprintf_safe_vsnprintf` (wrapper MSVC null-termination garantie)
- [ ] Implémenter `uprintf_size_add` / `uprintf_size_mul` (integer overflow check)
- [ ] Implémenter scan `%n` et rejet si `UPRINTF_ENABLE_N` non défini
- [ ] Vérifier que NULL buf/fmt retournent `-1` sans crash
- [ ] Borner width/precision dynamiques (`UPRINTF_MAX_WIDTH/PRECISION`)
- [ ] Assertions internes (`UPRINTF_ASSERT`) en mode debug
- [ ] Vérifier zéro appels à `malloc`/`calloc`/`realloc`/`free` dans tout le code
- [ ] Vérifier zéro appels à `sprintf`/`vsprintf`/`strcpy`/`strcat`/`gets`
- [ ] Vérifier zéro VLA et zéro `alloca`
- [ ] Vérifier zéro recursion
- [ ] Ajouter `__attribute__((format))` sur les fonctions narrow (GCC/Clang)
- [ ] Ajouter SAL annotations `_Printf_format_string_` (MSVC)

## Phase 7 : Build system
- [ ] Écrire `CMakeLists.txt` avec options pour toutes les macros `UPRINTF_*`
- [ ] Écrire `Makefile` fallback
- [ ] Mode header-only (`UPRINTF_HEADER_ONLY`)
- [ ] Mode compilé (`uprintf.c`)
- [ ] Vérifier build en C99 strict (`-std=c99`)
- [ ] Vérifier build en C11 strict (`-std=c11`)
- [ ] Intégrer tous les CFLAGS hardening (GCC/Clang) dans le Makefile/CMake
- [ ] Intégrer tous les flags MSVC (`/W4 /WX /sdl /GS /guard:cf /Qspectre /analyze`)
- [ ] Intégrer les LDFLAGS hardening (RELRO, noexecstack, separate-code)
- [ ] Mode debug avec sanitizers (ASAN + UBSAN + bounds + integer)
- [ ] `-D_FORTIFY_SOURCE=3` activé par défaut en release

## Phase 8 : CI/CD
- [ ] GitHub Actions : matrix Linux (GCC 9/12/14, Clang 14/18)
- [ ] GitHub Actions : matrix macOS (Apple Clang latest)
- [ ] GitHub Actions : matrix Windows (MSVC 2019/2022, MinGW-w64)
- [ ] ASAN activé sur tous les builds de test
- [ ] UBSAN activé sur tous les builds de test
- [ ] Bounds sanitizer activé
- [ ] MSAN activé (Clang only, si possible)
- [ ] Valgrind sur Linux (leak check + memcheck)
- [ ] cppcheck en analyse statique
- [ ] Build avec et sans `UPRINTF_UNICODE`
- [ ] Vérifier grep zéro malloc/sprintf/strcpy dans le code source (CI check)

## Phase 9 : Exemples et documentation
- [ ] `examples/basic.c` — usage simple narrow + wide
- [ ] `examples/unicode.c` — caractères multi-octets, emoji
- [ ] `examples/cross_platform.c` — même code compilé partout
- [ ] README.md — quick start, API reference, options de compilation
- [ ] Documenter les flags de sécurité recommandés pour les utilisateurs
- [ ] LICENSE (MIT)

## Phase 10 : Finalisation
- [ ] Relecture complète du code
- [ ] Vérifier zero warnings avec `-Wall -Wextra -Wpedantic -Werror`
- [ ] Vérifier zero warnings avec MSVC `/W4 /WX /analyze`
- [ ] Vérifier zéro finding ASAN/UBSAN/MSAN
- [ ] Vérifier zéro finding Valgrind
- [ ] Vérifier zéro finding cppcheck
- [ ] Vérifier absence totale de malloc dans tout le code
- [ ] Vérifier thread-safety
- [ ] Tag v1.0.0
