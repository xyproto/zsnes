/* Minimal test framework for ZSNES2 headless tests */
#pragma once

#include <stdio.h>
#include <string.h>

static int zt_passes = 0;
static int zt_failures = 0;
static int zt_section_fails = 0;

#define ZT_SECTION(name)                \
    do {                                \
        printf("  %s\n", name);         \
        zt_section_fails = zt_failures; \
    } while (0)

#define ZT_CHECK(expr)                                                            \
    do {                                                                          \
        if (expr) {                                                               \
            zt_passes++;                                                          \
        } else {                                                                  \
            fprintf(stderr, "    FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #expr); \
            zt_failures++;                                                        \
        }                                                                         \
    } while (0)

#define ZT_CHECK_STR(got, expected)                                            \
    do {                                                                       \
        const char *_g = (got), *_e = (expected);                              \
        if (strcmp(_g, _e) == 0) {                                             \
            zt_passes++;                                                       \
        } else {                                                               \
            fprintf(stderr, "    FAIL [%s:%d]: got \"%s\", expected \"%s\"\n", \
                __FILE__, __LINE__, _g, _e);                                   \
            zt_failures++;                                                     \
        }                                                                      \
    } while (0)

#define ZT_CHECK_INT(got, expected)                                    \
    do {                                                               \
        int _g = (int)(got), _e = (int)(expected);                     \
        if (_g == _e) {                                                \
            zt_passes++;                                               \
        } else {                                                       \
            fprintf(stderr, "    FAIL [%s:%d]: got %d, expected %d\n", \
                __FILE__, __LINE__, _g, _e);                           \
            zt_failures++;                                             \
        }                                                              \
    } while (0)

#define ZT_RESULTS()                                                \
    do {                                                            \
        printf("\n%d passed, %d failed\n", zt_passes, zt_failures); \
        return zt_failures ? 1 : 0;                                 \
    } while (0)
