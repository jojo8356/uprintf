# Cahier Des Charges — uprintf

## Bibliothèque C portable de formatted output universel

---

## 1. Contexte et problématique

### 1.1 Constat

Le langage C propose deux familles de fonctions d'affichage formaté, incompatibles entre elles :

| Famille    | Fonctions                  | Type de chaîne |
|------------|----------------------------|----------------|
| Narrow     | `printf`, `fprintf`, `snprintf` | `char*`        |
| Wide       | `wprintf`, `fwprintf`, `swprintf` | `wchar_t*`     |

Windows propose `_uprintf` via `<tchar.h>`, qui bascule entre les deux selon la macro `_UNICODE`. Cette solution est **non-portable** (Microsoft only).

C11 `_Generic` permet du dispatch par type mais est limité à un seul argument et ne supporte pas les format strings variadiques.

### 1.2 Besoin

Une bibliothèque **header-only** (ou header + un fichier .c) fournissant une macro/fonction `uprintf` qui :

- Accepte des format strings `char*` ou `wchar_t*`
- Fonctionne sur **Linux, macOS, Windows (MSVC, MinGW), BSD**
- Ne nécessite aucune dépendance externe (pas de GLib, ICU, etc.)
- S'intègre facilement dans un projet existant (1-2 fichiers à inclure)

---

## 2. Objectifs

| ID   | Objectif                                          | Priorité |
|------|---------------------------------------------------|----------|
| O-01 | Printf universel char*/wchar_t* via une seule API | Must     |
| O-02 | Portabilité Linux / macOS / Windows               | Must     |
| O-03 | Zero dépendance externe                           | Must     |
| O-04 | Support variadic (nombre d'arguments variable)    | Must     |
| O-05 | Variantes fprintf, snprintf, sprintf              | Should   |
| O-06 | Header-only possible (mode single-header)         | Should   |
| O-07 | Thread-safe                                       | Should   |
| O-08 | Support C99 minimum, C11 pour _Generic            | Must     |
| O-09 | Performance comparable au printf natif             | Should   |
| O-10 | Pas de malloc dans le chemin critique              | Could    |

---

## 3. Spécifications fonctionnelles

### 3.1 API publique

```c
#include "uprintf.h"

// Macro principale — dispatch automatique selon le type du format string
uprintf(fmt, ...)          // → stdout
ufprintf(stream, fmt, ...) // → FILE*
usnprintf(buf, n, fmt, ...) // → buffer (char* ou wchar_t* selon fmt)
usprintf(buf, fmt, ...)     // → buffer sans limite (dangereux, compat legacy)
```

### 3.2 Comportement attendu

```c
// Avec char* → délègue à printf/fprintf/snprintf
uprintf("Hello %s, you are %d\n", "world", 42);

// Avec wchar_t* → délègue à wprintf/fwprintf/swprintf
uprintf(L"Hello %ls, you are %d\n", L"world", 42);

// Le type du PREMIER argument (format string) détermine la famille utilisée
```

### 3.3 Macro TCHAR et littéral _T()

Pour la compatibilité avec le style Windows `<tchar.h>` :

```c
#include "uprintf.h"

// TCHAR est char ou wchar_t selon UPRINTF_UNICODE
TCHAR name[] = _T("world");
uprintf(_T("Hello %s\n"), name);
```

| Macro              | Définie         | Effet                          |
|--------------------|-----------------|--------------------------------|
| `UPRINTF_UNICODE`  | Oui             | `TCHAR = wchar_t`, `_T() = L""` |
| `UPRINTF_UNICODE`  | Non (défaut)    | `TCHAR = char`, `_T() = ""`    |

### 3.4 Spécificateurs de format

Tous les spécificateurs du C standard sont supportés **sans exception**. La lib ne filtre, ne modifie et n'invente aucun format — elle délègue intégralement au printf/wprintf natif de la plateforme.

#### 3.4.1 Syntaxe complète d'un spécificateur

```
%[flags][width][.precision][length]specifier
```

Chaque composant est optionnel sauf `%` et le spécificateur final.

#### 3.4.2 Flags

| Flag  | Effet                                                        |
|-------|--------------------------------------------------------------|
| `-`   | Aligne à gauche dans le champ (défaut : à droite)            |
| `+`   | Force l'affichage du signe `+` pour les nombres positifs     |
| ` `   | Espace avant les nombres positifs (ignoré si `+` présent)    |
| `0`   | Padding avec des `0` au lieu d'espaces (ignoré si `-`)       |
| `#`   | Forme alternative : `0x` pour hex, `0` pour octal, `.` forcé pour flottants |

Les flags sont combinables dans n'importe quel ordre :

```c
uprintf(_T("%+010.2f"), 3.14);   // "+000003.14"
uprintf(_T("%-20s|"), _T("left")); // "left                |"
uprintf(_T("%#x"), 255);          // "0xff"
```

#### 3.4.3 Width (largeur de champ)

| Syntaxe | Effet                                                  |
|---------|--------------------------------------------------------|
| `N`     | Largeur minimale fixe (ex: `%10d`)                     |
| `*`     | Largeur passée comme argument `int` (ex: `%*d`, 10, x) |

```c
uprintf(_T("%10d"), 42);         // "        42"
uprintf(_T("%*d"), 10, 42);     // "        42" (identique, largeur dynamique)
```

#### 3.4.4 Precision

| Syntaxe  | Effet selon le type                                          |
|----------|--------------------------------------------------------------|
| `.N`     | Précision fixe                                                |
| `.*`     | Précision passée comme argument `int`                        |
| `.0`     | Entiers : précision minimale 0. Flottants : pas de décimales |

Effet par famille de spécificateurs :

| Type          | Effet de `.N`                                              |
|---------------|-------------------------------------------------------------|
| `%d`, `%i`, `%u`, `%o`, `%x`, `%X` | Nombre minimum de chiffres (padding avec `0` à gauche) |
| `%f`, `%F`    | Nombre de chiffres après la virgule (défaut : 6)           |
| `%e`, `%E`    | Nombre de chiffres après la virgule (défaut : 6)           |
| `%g`, `%G`    | Nombre maximum de chiffres significatifs (défaut : 6)      |
| `%a`, `%A`    | Nombre de chiffres hex après la virgule                     |
| `%s`          | Nombre maximum de caractères à afficher (troncature)        |

```c
uprintf(_T("%.5d"), 42);         // "00042"
uprintf(_T("%.2f"), 3.14159);    // "3.14"
uprintf(_T("%.3s"), _T("hello"));// "hel"
uprintf(_T("%.*f"), 4, 3.14159); // "3.1416" (précision dynamique)
```

#### 3.4.5 Length modifiers (modificateurs de longueur)

| Modifier | `d`/`i`          | `u`/`o`/`x`/`X`   | `f`/`e`/`g`/`a` | `c`        | `s`          | `n`          |
|----------|-------------------|--------------------|------------------|------------|--------------|--------------|
| *(none)* | `int`             | `unsigned int`     | `double`         | `int`      | `char*`      | `int*`       |
| `hh`     | `signed char`     | `unsigned char`    | —                | —          | —            | `signed char*` |
| `h`      | `short`           | `unsigned short`   | —                | —          | —            | `short*`     |
| `l`      | `long`            | `unsigned long`    | —                | `wint_t`   | `wchar_t*`   | `long*`      |
| `ll`     | `long long`       | `unsigned long long` | —              | —          | —            | `long long*` |
| `L`      | —                 | —                  | `long double`    | —          | —            | —            |
| `j`      | `intmax_t`        | `uintmax_t`        | —                | —          | —            | `intmax_t*`  |
| `z`      | `size_t` (signé)  | `size_t`           | —                | —          | —            | `size_t*`    |
| `t`      | `ptrdiff_t`       | `ptrdiff_t` (non-signé) | —           | —          | —            | `ptrdiff_t*` |

```c
uprintf(_T("%hhd"), (signed char)-1);   // "-1"
uprintf(_T("%hu"), (unsigned short)65535); // "65535"
uprintf(_T("%ld"), 123456789L);         // "123456789"
uprintf(_T("%lld"), 9223372036854775807LL); // max int64
uprintf(_T("%zu"), sizeof(int));        // taille portable
uprintf(_T("%td"), ptr2 - ptr1);        // ptrdiff_t
uprintf(_T("%jd"), (intmax_t)val);      // intmax_t
uprintf(_T("%Lf"), 3.14L);             // long double
uprintf(_T("%lc"), L'é');              // wide char dans narrow printf
uprintf(_T("%ls"), L"wide");           // wide string dans narrow printf
```

#### 3.4.6 Spécificateurs de conversion — table complète

**Entiers :**

| Spec  | Description                          | Exemple          | Résultat      |
|-------|--------------------------------------|------------------|---------------|
| `%d`  | Entier signé décimal                 | `%d`, 42         | `42`          |
| `%i`  | Entier signé décimal (= `%d`)       | `%i`, -7         | `-7`          |
| `%u`  | Entier non-signé décimal             | `%u`, 42u        | `42`          |
| `%o`  | Entier non-signé octal               | `%o`, 255        | `377`         |
| `%x`  | Entier non-signé hex (minuscule)     | `%x`, 255        | `ff`          |
| `%X`  | Entier non-signé hex (majuscule)     | `%X`, 255        | `FF`          |

**Flottants :**

| Spec  | Description                                    | Exemple          | Résultat         |
|-------|------------------------------------------------|------------------|------------------|
| `%f`  | Décimal à virgule fixe (minuscule)             | `%f`, 3.14       | `3.140000`       |
| `%F`  | Décimal à virgule fixe (majuscule INF/NAN)     | `%F`, INFINITY   | `INF`            |
| `%e`  | Notation scientifique (minuscule)              | `%e`, 3.14       | `3.140000e+00`   |
| `%E`  | Notation scientifique (majuscule)              | `%E`, 3.14       | `3.140000E+00`   |
| `%g`  | Shortest entre `%f` et `%e`                    | `%g`, 3.14       | `3.14`           |
| `%G`  | Shortest entre `%F` et `%E`                    | `%G`, 0.00001    | `1E-05`          |
| `%a`  | Hex flottant (minuscule, C99)                  | `%a`, 3.14       | `0x1.91eb86...p+1` |
| `%A`  | Hex flottant (majuscule, C99)                  | `%A`, 3.14       | `0X1.91EB86...P+1` |

**Caractères et chaînes :**

| Spec  | Narrow (`char*` fmt)            | Wide (`wchar_t*` fmt)           |
|-------|---------------------------------|---------------------------------|
| `%c`  | `int` → `char`                  | `wint_t` → `wchar_t`           |
| `%lc` | `wint_t` → converti en MB      | `wint_t` → `wchar_t`           |
| `%s`  | `char*`                         | `wchar_t*`                      |
| `%ls` | `wchar_t*` → converti en MB    | `wchar_t*`                      |
| `%hs` | `char*`                         | `char*` → converti en wide      |

**Autres :**

| Spec  | Description                                    | Type attendu    |
|-------|------------------------------------------------|-----------------|
| `%p`  | Adresse pointeur (format impl-defined)         | `void*`         |
| `%n`  | Écrit le nb de chars écrits dans un pointeur   | `int*`          |
| `%%`  | Littéral `%`                                   | *(aucun)*       |

#### 3.4.7 Support `%n`

`%n` est supporté car c'est du C standard, mais :

- **MSVC désactive `%n` par défaut** (sécurité). Il faut appeler `_set_printf_count_output(1)` pour le réactiver.
- La lib fournit une macro `UPRINTF_ENABLE_N` qui active `%n` sur MSVC via cet appel dans `uprintf_init()`.
- Sur GCC/Clang, `%n` fonctionne nativement.

#### 3.4.8 Spécificateurs étendus Microsoft (MSVC)

Ces spécificateurs sont **non-standard** mais courants sur Windows. La lib les **documente sans les garantir** sur les autres plateformes :

| Spec    | Description                         | Portable ? |
|---------|-------------------------------------|------------|
| `%I32d` | `__int32` (= `int`)                | Non        |
| `%I64d` | `__int64` (= `long long`)          | Non        |
| `%Id`   | `ptrdiff_t` / `size_t` (MSVC)      | Non        |
| `%Iu`   | `size_t` non-signé (MSVC)          | Non        |
| `%S`    | Inverse de `%s` (narrow↔wide swap) | Non        |
| `%C`    | Inverse de `%c`                     | Non        |

Pour la portabilité, préférer `%lld`, `%zu`, `%td` (C99 standard) aux extensions Microsoft.

#### 3.4.9 Macros de format C99 `<inttypes.h>`

La lib est compatible avec les macros de `<inttypes.h>` pour les types à taille fixe :

```c
#include <inttypes.h>

int32_t  val32  = 42;
uint64_t val64  = 123456789ULL;
intptr_t valptr = (intptr_t)&val32;

uprintf("int32:  %" PRId32 "\n", val32);
uprintf("uint64: %" PRIu64 "\n", val64);
uprintf("intptr: %" PRIxPTR "\n", valptr);
```

| Famille   | Exemples                          | Types couverts                    |
|-----------|-----------------------------------|-----------------------------------|
| `PRId*`   | `PRId8`, `PRId16`, `PRId32`, `PRId64` | `int8_t` → `int64_t` signé   |
| `PRIu*`   | `PRIu8`, `PRIu16`, `PRIu32`, `PRIu64` | `uint8_t` → `uint64_t`      |
| `PRIx*`   | `PRIx8`, `PRIx16`, `PRIx32`, `PRIx64` | hex minuscule                |
| `PRIX*`   | `PRIX8`, `PRIX16`, `PRIX32`, `PRIX64` | hex majuscule                |
| `PRIo*`   | `PRIo8`, `PRIo16`, `PRIo32`, `PRIo64` | octal                        |
| `*PTR`    | `PRIdPTR`, `PRIuPTR`, `PRIxPTR`  | `intptr_t` / `uintptr_t`         |
| `*MAX`    | `PRIdMAX`, `PRIuMAX`             | `intmax_t` / `uintmax_t`         |

**Note** : en mode wide, ces macros s'expandent en littéraux `char*` (ex: `"d"`), donc elles ne sont **pas directement utilisables** dans un format `wchar_t*`. La lib fournira des équivalents wide si nécessaire :

```c
// Problème : PRId32 = "d" (narrow), incompatible avec L"..."
// Solution fournie par la lib :
uprintf(_T("val: %" UPRI32 "\n"), val32); // UPRI32 = PRId32 ou L"d"
```

#### 3.4.10 Exemples combinés complets

```c
// Flags + width + precision + length + specifier
uprintf(_T("%-+20.10lld"),  123456789LL);   // left-align, sign, 20 wide, 10 digits min, long long
uprintf(_T("%#012.5x"),     255);            // "0x" prefix, 12 wide, 0-padded, 5 hex digits min
uprintf(_T("%0*.*f"),       15, 3, 3.14159); // dynamic width=15, precision=3, 0-padded
uprintf(_T("%-30.25s"),     _T("troncature si > 25 chars"));
uprintf(_T("[%10c]"),       _T('A'));        // char dans champ de 10
uprintf(_T("%+.0e"),        100.0);          // "+1e+02" (0 décimales, signe forcé)
```

### 3.5 Valeur de retour

Identique au comportement standard :
- `uprintf` / `ufprintf` : nombre de caractères écrits, ou négatif en cas d'erreur
- `usnprintf` : nombre de caractères qui auraient été écrits (hors `\0`), ou négatif en cas d'erreur

---

## 4. Spécifications techniques

### 4.1 Mécanisme de dispatch

**Approche C11 (_Generic)** — méthode recommandée :

```c
#define uprintf(fmt, ...) _Generic((fmt),       \
    char*:          printf(fmt, ##__VA_ARGS__),  \
    const char*:    printf(fmt, ##__VA_ARGS__),  \
    wchar_t*:       wprintf(fmt, ##__VA_ARGS__), \
    const wchar_t*: wprintf(fmt, ##__VA_ARGS__)  \
)
```

**Approche C99 fallback** — quand `_Generic` n'est pas disponible :

Le dispatch se fait via `UPRINTF_UNICODE` au préprocesseur (mode statique, comme `<tchar.h>` mais portable).

### 4.2 Détection de plateforme

```c
// uprintf_platform.h
#if defined(_WIN32) || defined(_WIN64)
    #define UPRINTF_WINDOWS 1
#elif defined(__APPLE__)
    #define UPRINTF_MACOS 1
#elif defined(__linux__)
    #define UPRINTF_LINUX 1
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #define UPRINTF_BSD 1
#else
    #define UPRINTF_UNKNOWN 1
#endif

// Détection C11
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define UPRINTF_HAS_GENERIC 1
#endif
```

### 4.3 Gestion du mode console Windows

Sur Windows, `wprintf` nécessite `_setmode(_fileno(stdout), _O_U16TEXT)` pour fonctionner correctement. La lib doit :

1. Détecter si stdout est un terminal Windows
2. Appeler `_setmode` automatiquement si nécessaire (une seule fois)
3. Ou bien utiliser `WriteConsoleW` directement en alternative

```c
#ifdef UPRINTF_WINDOWS
    #include <io.h>
    #include <fcntl.h>
    // Init automatique via constructeur ou appel explicite
    void uprintf_init(void);
#endif
```

### 4.4 Gestion de la locale (Unix/POSIX)

Sur Linux/macOS, `wprintf` nécessite que la locale soit initialisée :

```c
// La lib doit s'assurer que setlocale a été appelé
// Option 1 : appel automatique une fois
// Option 2 : documentation + macro UPRINTF_AUTO_LOCALE
#include <locale.h>
setlocale(LC_ALL, "");
```

### 4.5 Structure des fichiers

```
uprintf/
├── include/
│   ├── uprintf.h          # Header principal (API publique)
│   ├── uprintf_config.h   # Détection plateforme + options
│   └── uprintf_impl.h     # Implémentation (si header-only)
├── src/
│   └── uprintf.c          # Implémentation (si mode compilé)
├── tests/
│   ├── test_narrow.c      # Tests printf char*
│   ├── test_wide.c        # Tests wprintf wchar_t*
│   ├── test_tchar.c       # Tests mode TCHAR/_T()
│   ├── test_snprintf.c    # Tests buffer variants
│   └── test_platform.c    # Tests spécifiques plateforme
├── examples/
│   ├── basic.c
│   ├── unicode.c
│   └── cross_platform.c
├── CMakeLists.txt
├── Makefile               # Fallback sans CMake
├── LICENSE
└── README.md
```

### 4.6 Options de compilation

| Macro                   | Effet                                               |
|-------------------------|------------------------------------------------------|
| `UPRINTF_UNICODE`       | Force le mode wide (TCHAR = wchar_t)                |
| `UPRINTF_HEADER_ONLY`   | Active le mode header-only (inline tout)             |
| `UPRINTF_AUTO_LOCALE`   | Initialise la locale automatiquement (Unix)          |
| `UPRINTF_AUTO_CONSOLE`  | Configure la console automatiquement (Windows)       |
| `UPRINTF_NO_GENERIC`    | Force le mode C99 même si C11 est disponible         |

### 4.7 Règles de sécurité — Protection overflow, malloc et mémoire

#### 4.7.1 Politique zéro malloc

La bibliothèque **n'alloue jamais de mémoire dynamiquement** dans aucun chemin d'exécution. Aucun appel à :

- `malloc`, `calloc`, `realloc`, `free`
- `strdup`, `wcsdup`, `asprintf`
- `mmap` (sauf si fait par la libc en interne, hors contrôle)

Si un buffer temporaire est nécessaire en interne, il est **toujours sur la stack** avec une taille maximale définie :

```c
#define UPRINTF_STACK_BUF_MAX 4096  // Taille max d'un buffer interne stack
```

Cela élimine entièrement les classes de bugs suivantes :
- Use-after-free
- Double free
- Memory leaks
- Heap overflow
- NULL pointer dereference sur malloc échoué

#### 4.7.2 Protection contre les buffer overflows

**Règle 1 — Toujours utiliser les variantes "n" (bornées)**

En interne, la lib utilise **exclusivement** les fonctions bornées :

| Interdit            | Obligatoire                          |
|---------------------|--------------------------------------|
| `sprintf`           | `snprintf`                           |
| `swprintf` (MSVC)   | `_snwprintf` / `swprintf` avec taille |
| `vsprintf`          | `vsnprintf`                          |
| `strcpy`            | `strncpy` ou `memcpy` avec taille    |
| `strcat`            | `strncat` avec calcul de reste       |
| `wcscpy`            | `wcsncpy` ou `wmemcpy` avec taille   |
| `gets`              | *(jamais utilisé)*                   |

**Règle 2 — Vérification systématique des tailles**

Toute opération sur buffer vérifie **avant** l'écriture :

```c
// Pattern obligatoire dans tout le code interne
if (count >= buf_size) {
    // Troncature contrôlée, jamais d'écriture hors limites
    count = buf_size - 1;
}
buf[count] = '\0';  // Null-termination garantie
```

**Règle 3 — Null-termination garantie**

`usnprintf` garantit **toujours** que le buffer est null-terminé, même en cas de troncature. Ceci corrige le comportement de `snprintf` sur certaines implémentations (MSVC `_snprintf` ne null-termine pas si le buffer est plein).

```c
// Wrapper interne pour MSVC
#ifdef _MSC_VER
static inline int uprintf_safe_vsnprintf(char *buf, size_t n, const char *fmt, va_list ap) {
    if (n == 0) return 0;
    int ret = _vsnprintf(buf, n, fmt, ap);
    buf[n - 1] = '\0';  // Toujours null-terminé
    if (ret < 0 || (size_t)ret >= n)
        ret = (int)(n - 1);  // Signale la troncature
    return ret;
}
#endif
```

**Règle 4 — Pas de buffer de taille 0**

Les fonctions refusent les appels avec `n == 0` ou `buf == NULL` :

```c
if (buf == NULL || n == 0) {
    return -1;  // Erreur, pas d'UB
}
```

#### 4.7.3 Protection contre les integer overflows

**Règle 5 — Vérification arithmétique sur les tailles**

Toute opération arithmétique sur des `size_t` vérifie le débordement :

```c
// Avant toute addition de tailles
static inline int uprintf_size_add(size_t a, size_t b, size_t *result) {
    if (a > SIZE_MAX - b) return -1;  // Overflow détecté
    *result = a + b;
    return 0;
}

// Avant toute multiplication de tailles
static inline int uprintf_size_mul(size_t a, size_t b, size_t *result) {
    if (b != 0 && a > SIZE_MAX / b) return -1;  // Overflow détecté
    *result = a * b;
    return 0;
}
```

**Règle 6 — Cast explicites et vérifiés**

Tout cast entre types signés et non-signés est explicite et vérifié :

```c
// Interdit : cast implicite silencieux
size_t len = (size_t)some_int;  // DANGER si some_int < 0

// Obligatoire : vérification avant cast
if (some_int < 0) return -1;
size_t len = (size_t)some_int;
```

**Règle 7 — Largeur et précision bornées**

Les valeurs dynamiques de width (`%*d`) et precision (`%.*d`) sont bornées pour éviter des allocations implicites géantes par la libc :

```c
#define UPRINTF_MAX_WIDTH     1048576  // 1 MB max de largeur de champ
#define UPRINTF_MAX_PRECISION 1048576  // 1 MB max de précision

// Validation avant dispatch au printf natif
if (width < 0) width = 0;
if (width > UPRINTF_MAX_WIDTH) width = UPRINTF_MAX_WIDTH;
if (precision > UPRINTF_MAX_PRECISION) precision = UPRINTF_MAX_PRECISION;
```

#### 4.7.4 Protection des format strings

**Règle 8 — Format string non-NULL**

```c
if (fmt == NULL) {
    return -1;  // Pas de crash, retour erreur propre
}
```

**Règle 9 — Attribut format (GCC/Clang)**

Les fonctions internes portent l'attribut `__attribute__((format))` pour que le compilateur vérifie la correspondance format/arguments au compile-time :

```c
#if defined(__GNUC__) || defined(__clang__)
    #define UPRINTF_FORMAT_CHECK(fmt_idx, args_idx) \
        __attribute__((format(printf, fmt_idx, args_idx)))
    #define UPRINTF_WFORMAT_CHECK  // Pas d'équivalent wide standard
#else
    #define UPRINTF_FORMAT_CHECK(fmt_idx, args_idx)
#endif
```

Sur MSVC, l'équivalent est activé via SAL annotations :

```c
#ifdef _MSC_VER
    #include <sal.h>
    // _Printf_format_string_ sur le paramètre fmt
#endif
```

**Règle 10 — Détection de `%n` en mode sécurisé**

Par défaut, `%n` est **désactivé**. Il faut explicitement définir `UPRINTF_ENABLE_N` pour l'autoriser. Sans cette macro, si un `%n` est détecté dans le format string, la fonction retourne `-1` sans rien écrire.

```c
#ifndef UPRINTF_ENABLE_N
// Scan rapide du format string pour rejeter %n
static inline int uprintf_has_percent_n(const char *fmt) {
    for (const char *p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            // Skip flags, width, precision, length
            while (*p == '-' || *p == '+' || *p == ' ' || *p == '0' || *p == '#') p++;
            while (*p >= '0' && *p <= '9') p++;
            if (*p == '.') { p++; while (*p >= '0' && *p <= '9') p++; }
            while (*p == 'h' || *p == 'l' || *p == 'j' || *p == 'z' || *p == 't' || *p == 'L') p++;
            if (*p == 'n') return 1;  // %n détecté → refus
        }
    }
    return 0;
}
#endif
```

#### 4.7.5 Flags de compilation obligatoires

**GCC / Clang :**

```makefile
# Warnings maximum
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Werror                    # Warnings = erreurs
CFLAGS += -Wshadow                   # Variable masquée
CFLAGS += -Wconversion               # Conversions implicites dangereuses
CFLAGS += -Wsign-conversion          # Signed/unsigned implicite
CFLAGS += -Wformat=2                 # Vérification format strings renforcée
CFLAGS += -Wformat-overflow=2        # Détection overflow dans sprintf/snprintf
CFLAGS += -Wformat-truncation=2      # Détection troncature dans snprintf
CFLAGS += -Wformat-security          # Format strings non-littérales
CFLAGS += -Wformat-signedness        # Mismatch signed/unsigned dans formats
CFLAGS += -Wstringop-overflow=4      # Overflow sur opérations string (niveau max)
CFLAGS += -Wstringop-truncation      # Troncature sur strncpy/strncat
CFLAGS += -Warray-bounds=2           # Accès hors limites tableau (niveau max)
CFLAGS += -Wshift-overflow=2         # Overflow sur shift
CFLAGS += -Wvla                      # Interdit les VLA (taille variable sur stack)
CFLAGS += -Walloca                   # Interdit alloca (allocation stack non bornée)
CFLAGS += -Wstack-protector          # Avertit si stack protector impossible
CFLAGS += -Wnull-dereference         # Déréférence de NULL
CFLAGS += -Wdouble-promotion         # Float promu en double implicitement
CFLAGS += -Wcast-align               # Alignement incorrect sur cast
CFLAGS += -Wcast-qual                # Suppression de const/volatile via cast
CFLAGS += -Wpointer-arith            # Arithmétique sur void*/function*
CFLAGS += -Wswitch-enum              # Switch incomplet sur enum
CFLAGS += -Wundef                    # Macro non définie utilisée dans #if
CFLAGS += -Wunused                   # Toute entité non utilisée
CFLAGS += -Winit-self                # Variable initialisée avec elle-même
CFLAGS += -Wwrite-strings            # String literals en const char*

# Hardening (protection runtime)
CFLAGS += -fstack-protector-strong   # Canary sur la stack (fonctions à risque)
CFLAGS += -fstack-clash-protection   # Protection contre stack clash
CFLAGS += -fcf-protection            # Control-Flow Integrity (x86)
CFLAGS += -D_FORTIFY_SOURCE=3        # Vérification runtime des fonctions libc (niveau max)
CFLAGS += -ftrapv                    # Trap sur signed integer overflow
CFLAGS += -fno-strict-aliasing       # Pas d'optimisation aliasing dangereuse

# Linker hardening
LDFLAGS += -Wl,-z,relro              # Relocation read-only
LDFLAGS += -Wl,-z,now                # Résolution symboles immédiate (full RELRO)
LDFLAGS += -Wl,-z,noexecstack        # Stack non-exécutable
LDFLAGS += -Wl,-z,separate-code      # Sépare code et données en mémoire

# Sanitizers (mode debug/test uniquement)
CFLAGS_DEBUG += -fsanitize=address          # Heap overflow, use-after-free, leaks
CFLAGS_DEBUG += -fsanitize=undefined        # Integer overflow, shift, null deref
CFLAGS_DEBUG += -fsanitize=bounds           # Array bounds checking
CFLAGS_DEBUG += -fsanitize=integer          # Tous les integer bugs (Clang only)
CFLAGS_DEBUG += -fsanitize=nullability      # Null checks (Clang only)
CFLAGS_DEBUG += -fno-omit-frame-pointer     # Stack traces lisibles
CFLAGS_DEBUG += -fno-sanitize-recover=all   # Crash immédiat, pas de recovery
```

**MSVC :**

```
# Warnings maximum
cl /W4 /WX                # Warning level 4 + warnings as errors
cl /sdl                    # Security Development Lifecycle checks
cl /GS                     # Buffer Security Check (stack canary)
cl /guard:cf               # Control Flow Guard
cl /Qspectre               # Spectre mitigation
cl /analyze                # Static analysis intégrée
cl /analyze:stacksize 4096 # Limite taille stack analysée
cl /DYNAMICBASE            # ASLR activé
cl /NXCOMPAT               # DEP (stack non-exécutable)
cl /wd4996                 # Désactive warning deprecated (on gère nous-mêmes)

# Defines de sécurité
/D_CRT_SECURE_NO_WARNINGS=0   # Garder les warnings CRT actifs
/D_CRT_NONSTDC_NO_DEPRECATE   # Mais ignorer les deprecations POSIX
```

#### 4.7.6 Règles de code statiques

Vérifications appliquées dans le code de la lib et vérifiables par analyse statique :

| ID    | Règle                                                          | Outil de vérification       |
|-------|----------------------------------------------------------------|-----------------------------|
| S-01  | Aucun appel à `malloc`/`calloc`/`realloc`/`free`              | grep + cppcheck             |
| S-02  | Aucun appel à `sprintf`/`vsprintf` (non borné)                | grep + `-Wformat-overflow`  |
| S-03  | Aucun appel à `strcpy`/`strcat`/`wcscpy`/`wcscat`             | grep + cppcheck             |
| S-04  | Aucun appel à `gets`                                           | grep                        |
| S-05  | Aucun VLA (`int arr[n]` avec n variable)                       | `-Wvla`                     |
| S-06  | Aucun `alloca`                                                 | `-Walloca`                  |
| S-07  | Tout buffer stack a une taille `<= UPRINTF_STACK_BUF_MAX`     | review + cppcheck           |
| S-08  | Tout `snprintf` vérifie sa valeur de retour                   | `-Wunused-result` + review  |
| S-09  | Tout cast `int → size_t` vérifie que `int >= 0`               | `-Wsign-conversion`         |
| S-10  | Tout pointeur est vérifié non-NULL avant déréférence           | `-Wnull-dereference` + ASAN |
| S-11  | Pas de signed integer overflow (UB en C)                       | `-ftrapv` + UBSAN           |
| S-12  | Pas d'accès hors limites tableau                               | `-Warray-bounds` + ASAN     |
| S-13  | Format strings toujours littérales ou contrôlées               | `-Wformat-security`         |
| S-14  | `%n` rejeté sauf si `UPRINTF_ENABLE_N` défini                 | Runtime check + tests       |
| S-15  | Pas de recursion (risque stack overflow)                       | Review + cppcheck            |
| S-16  | Pas de boucle sans borne de sortie garantie                    | Review                       |

#### 4.7.7 Macro de vérification d'assertion interne

En mode debug, des assertions internes vérifient les invariants :

```c
#ifdef UPRINTF_DEBUG
    #include <assert.h>
    #define UPRINTF_ASSERT(cond, msg) assert((cond) && (msg))
#else
    #define UPRINTF_ASSERT(cond, msg) ((void)0)
#endif

// Usage interne
UPRINTF_ASSERT(buf != NULL, "usnprintf: buf is NULL");
UPRINTF_ASSERT(n > 0, "usnprintf: buffer size is 0");
UPRINTF_ASSERT(fmt != NULL, "uprintf: format string is NULL");
```

#### 4.7.8 Résumé des protections par classe de vulnérabilité

| Vulnérabilité              | Protections appliquées                                                     |
|----------------------------|----------------------------------------------------------------------------|
| **Heap overflow**          | Zéro malloc → impossible                                                   |
| **Stack overflow (buffer)**| `snprintf` only + `-Wformat-overflow` + `-fstack-protector-strong` + ASAN  |
| **Stack overflow (depth)** | Pas de recursion + pas de VLA + pas d'alloca + `-fstack-clash-protection`  |
| **Integer overflow**       | `-ftrapv` + UBSAN + `uprintf_size_add/mul` + `-Wconversion`               |
| **Format string attack**   | `-Wformat=2` + `-Wformat-security` + `%n` désactivé + `format` attribute  |
| **Use-after-free**         | Zéro malloc → impossible                                                   |
| **NULL dereference**       | Vérification explicite + `-Wnull-dereference` + ASAN                      |
| **Signed/unsigned mismatch** | `-Wsign-conversion` + `-Wconversion` + casts explicites                  |
| **Out-of-bounds read/write** | `-Warray-bounds=2` + `-Wstringop-overflow=4` + ASAN + bounds sanitizer   |
| **Uninitialized memory**   | `-Wall` (inclut `-Wuninitialized`) + MSAN (si Clang)                      |
| **Control flow hijack**    | `-fcf-protection` + `/guard:cf` + RELRO + noexecstack                     |

---

## 5. Contraintes

### 5.1 Contraintes techniques

| ID   | Contrainte                                                  |
|------|-------------------------------------------------------------|
| C-01 | C99 minimum. C11 recommandé pour `_Generic`                 |
| C-02 | Aucune dépendance externe (libc uniquement)                 |
| C-03 | Aucune allocation dynamique (zéro malloc/calloc/realloc)    |
| C-04 | Compatible MSVC 2015+, GCC 4.9+, Clang 3.5+               |
| C-05 | Pas de conflit avec `<tchar.h>` si inclus en même temps     |
| C-06 | Pas de pollution du namespace global (préfixe `uprintf_`)   |
| C-07 | Aucun VLA ni alloca                                         |
| C-08 | Pas de recursion                                             |
| C-09 | Compilation sans warning avec `-Wall -Wextra -Wpedantic -Werror` |
| C-10 | Zéro finding ASAN/UBSAN/MSAN sur la suite de tests          |

### 5.2 Contraintes de portabilité

| Plateforme       | Compilateurs cibles          | Notes                              |
|------------------|------------------------------|------------------------------------|
| Windows          | MSVC, MinGW-w64, Clang-cl   | `wchar_t` = UTF-16 (2 octets)     |
| Linux            | GCC, Clang                   | `wchar_t` = UTF-32 (4 octets)     |
| macOS            | Clang (Apple), GCC           | `wchar_t` = UTF-32 (4 octets)     |
| FreeBSD/OpenBSD  | Clang, GCC                   | `wchar_t` = UTF-32 (4 octets)     |

**Point critique** : `wchar_t` n'a pas la même taille sur Windows vs Unix. La lib ne doit **pas** tenter de convertir entre les deux — elle délègue au printf natif de chaque plateforme.

---

## 6. Tests et validation

### 6.1 Matrice de tests

| Test                              | Narrow | Wide | TCHAR |
|-----------------------------------|--------|------|-------|
| String simple                     | x      | x    | x     |
| Entiers (%d, %i, %u, %x)         | x      | x    | x     |
| Flottants (%f, %e, %g)           | x      | x    | x     |
| Pointeurs (%p)                    | x      | x    | x     |
| Chaîne dans chaîne (%s, %ls)     | x      | x    | x     |
| Caractères spéciaux (é, ñ, 漢字)  | x      | x    | x     |
| Emoji (🎮, 🚀)                    | x      | x    | x     |
| Buffer overflow (snprintf)        | x      | x    | x     |
| NULL format string                | x      | x    | —     |
| Retour valeur correcte            | x      | x    | x     |

### 6.2 CI/CD

- **GitHub Actions** avec matrix build :
  - Linux : GCC 9/12/14, Clang 14/18
  - macOS : Apple Clang (latest)
  - Windows : MSVC 2019/2022, MinGW-w64
- Build en mode C99 et C11
- Build avec et sans `UPRINTF_UNICODE`
- Valgrind sur Linux (leak check)
- ASAN/UBSAN activés sur tous les builds

---

## 7. Limites connues et hors scope

### 7.1 Hors scope

- **Conversion automatique char* ↔ wchar_t*** : la lib ne convertit pas les arguments. Si le format est `wchar_t*`, les `%s` attendent des `wchar_t*`.
- **Nouveau système de format** : pas d'invention de `%S` magique ou de spécificateurs custom.
- **Support C89** : trop ancien, pas de `__VA_ARGS__`.
- **Buffering custom** : on utilise le buffering de la libc.
- **Interception/redirection** : pas de callback ou hook sur la sortie.

### 7.2 Limites

- Le dispatch C11 `_Generic` opère au **compile-time**. On ne peut pas choisir dynamiquement à runtime entre narrow et wide avec la même variable.
- En mode C99 (sans `_Generic`), le choix narrow/wide est fixé à la compilation via `UPRINTF_UNICODE`.
- Sur Windows, mélanger `printf` et `wprintf` sur le même stream est un comportement indéfini en C. La lib ne protège pas contre ça.

---

## 8. Livrables

| Livrable                | Format          | Description                              |
|-------------------------|-----------------|------------------------------------------|
| Code source             | C99/C11         | Headers + source optionnel               |
| Tests unitaires         | C + CMake/Make  | Suite complète multi-plateforme          |
| Exemples                | C               | 3 exemples commentés                     |
| README                  | Markdown        | Quick start + API reference              |
| CI config               | GitHub Actions  | Pipeline multi-plateforme                |
| LICENSE                 | MIT             | Licence permissive                       |

---

## 9. Résumé de l'API

```c
// === Mode automatique (C11 _Generic) ===
uprintf("narrow %s\n", "hello");       // → printf
uprintf(L"wide %ls\n", L"hello");      // → wprintf

// === Mode TCHAR (C99 compatible) ===
#define UPRINTF_UNICODE  // optionnel
#include "uprintf.h"
uprintf(_T("universal %s\n"), _T("hello"));

// === Variantes ===
ufprintf(stderr, _T("error: %s\n"), _T("oops"));
usnprintf(buf, sizeof(buf)/sizeof(TCHAR), _T("val=%d"), 42);

// === Init plateforme (optionnel, recommandé) ===
uprintf_init(); // configure console Windows / locale Unix
```
