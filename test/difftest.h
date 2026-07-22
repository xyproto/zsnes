/*
 * test/difftest.h - reusable scaffolding for differential-testing an asm->C
 * port.
 *
 * A differential test proves a C reimplementation is bit-identical to the
 * original assembly by running BOTH on the same random inputs and comparing
 * every output (globals, buffers, returned registers). It is a PORT-TIME tool:
 * it needs the original asm to still exist, so the per-port files
 * (difftest_<name>.c and the extracted object) are transient and get removed
 * once the asm is deleted. This header holds only the reusable bits.
 *
 * Typical workflow (see difftest_nem.c for a full worked example):
 *
 *   1. Extract the original routine into a standalone object that exposes
 *      `asm_<name>` and EXTERNs the shared globals:
 *          ./mkdifftest.sh ../cpu/dspproc.asm 1248 1277 NonEchoMono \
 *              "Voice0Volume Voice0EnvInc ..." > /dev/null
 *      (that writes _<name>.o and _<name>.inc into the current dir)
 *
 *   2. In difftest_<name>.c: define the shared globals, `#include` your C port,
 *      declare `extern void asm_<name>(void);`, then:
 *
 *          int main(void) {
 *              DT_MAIN(seed, iterations) {
 *                  ... randomize inputs and a saved copy of mutated state ...
 *                  ... restore state; run the asm; snapshot outputs into A_* ...
 *                  ... restore state; run the C;   snapshot outputs into C_* ...
 *                  DT_EQ("esi", a_esi, c_esi);
 *                  DT_MEM("DSPBuffer", A_dsp, C_dsp, sizeof A_dsp);
 *              }
 *              DT_DONE("NonEchoMono");
 *          }
 *
 * The asm typically uses a register ABI; call it via a small inline-asm wrapper
 * (again, see difftest_nem.c). Reset every piece of state the routine mutates
 * to the same value before BOTH runs, or the second run sees the first's
 * leftovers.
 */
#ifndef DIFFTEST_H
#define DIFFTEST_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Per-run bookkeeping. */
static int dt_fails; /* failing iterations so far */
static int dt_bad; /* did the current iteration mismatch?           */
static long dt_it; /* current iteration index                       */
static long dt_iters; /* total iterations                              */

/*
 * DT_MAIN(seed, iters) { body }
 *
 * Seeds the RNG and loops `iters` times, resetting the per-iteration flag each
 * time. Put the setup / run-asm / run-c / compare in the body.
 */
#define DT_MAIN(seed, iters)                         \
    srand(seed);                                     \
    dt_fails = 0;                                    \
    dt_iters = (long)(iters);                        \
    for (dt_it = 0; dt_it < dt_iters; dt_it++)       \
        for (int dt_once = (dt_bad = 0, 1); dt_once; \
            dt_once = 0, dt_fails += dt_bad)

/* Print the verdict and return from main(). */
#define DT_DONE(name)                                                      \
    do {                                                                   \
        if (dt_fails) {                                                    \
            printf("%s: FAIL (%d/%ld iterations mismatched)\n", (name),    \
                dt_fails, dt_iters);                                       \
            return 1;                                                      \
        }                                                                  \
        printf("%s: PASS (%ld iterations bit-identical to asm)\n", (name), \
            dt_iters);                                                     \
        return 0;                                                          \
    } while (0)

/* Only surface details for the first few failing iterations. */
#define DT_SHOW() (dt_fails < 4)

/* Random helpers (rand() only gives 15 usable bits, so spread it out). */
static inline uint32_t dt_u32(void)
{
    return ((uint32_t)rand() << 20) ^ ((uint32_t)rand() << 7) ^ (uint32_t)rand();
}
static inline uint32_t dt_mod(uint32_t m) { return dt_u32() % m; }
static inline void dt_fill(void* p, size_t n)
{
    uint8_t* b = (uint8_t*)p;
    for (size_t i = 0; i < n; i++)
        b[i] = (uint8_t)rand();
}

/* Compare a scalar; on mismatch, flag the iteration and (for the first few)
 * print the offending values. */
#define DT_EQ(label, a, c)                                               \
    do {                                                                 \
        if ((a) != (c)) {                                                \
            if (DT_SHOW())                                               \
                printf("  it=%ld %s: asm=%lld c=%lld\n", dt_it, (label), \
                    (long long)(a), (long long)(c));                     \
            dt_bad = 1;                                                  \
        }                                                                \
    } while (0)

static void dt_show_mem(const char* label, const uint8_t* a, const uint8_t* c, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (a[i] != c[i]) {
            printf("  it=%ld %s: first diff at byte %zu asm=%02x c=%02x\n", dt_it,
                label, i, a[i], c[i]);
            return;
        }
}

/* Compare a memory region. */
#define DT_MEM(label, a, c, n)                                                 \
    do {                                                                       \
        if (memcmp((a), (c), (n))) {                                           \
            if (DT_SHOW())                                                     \
                dt_show_mem((label), (const uint8_t*)(a), (const uint8_t*)(c), \
                    (n));                                                      \
            dt_bad = 1;                                                        \
        }                                                                      \
    } while (0)

#endif /* DIFFTEST_H */
