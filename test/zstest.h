// Looks good
/* Minimal test framework for ZSNES2 headless tests */
#pragma once

#include <stdio.h>
#include <string.h>

static int zt_passes = 0;
static int zt_failures = 0;
static int zt_section_fails = 0; /* zt_failures when the open section began */
static int zt_sections = 0;
static int zt_sections_failed = 0;
static char const* zt_section_name = 0;

/* Close the running section. Only a section that failed says anything: the
   per-check FAIL lines already report what went wrong, this reports where, so
   a long run does not have to be read backwards to find the section. */
static inline void zt_end_section(void)
{
    if (!zt_section_name) {
        return;
    }
    zt_sections++;
    if (zt_failures > zt_section_fails) {
        zt_sections_failed++;
        fprintf(stderr, "    ^ %d check(s) failed in \"%s\"\n",
            zt_failures - zt_section_fails, zt_section_name);
    }
    zt_section_name = 0;
}

#define ZT_SECTION(name)                \
    do {                                \
        zt_end_section();               \
        printf("  %s\n", (name));       \
        zt_section_name = (name);       \
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

#define ZT_RESULTS()                                              \
    do {                                                          \
        zt_end_section();                                         \
        printf("\n%d passed, %d failed", zt_passes, zt_failures); \
        if (zt_sections_failed) {                                 \
            printf(" (%d of %d sections)", zt_sections_failed,    \
                zt_sections);                                     \
        }                                                         \
        printf("\n");                                             \
        return zt_failures ? 1 : 0;                               \
    } while (0)
