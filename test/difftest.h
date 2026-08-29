/*
 * Scaffolding for differential-testing an asm->C port: run both on the same
 * random inputs and compare every output. A port-time tool - it needs the
 * original assembly, so each difftest_<name>.c is transient.
 *
 *   1. Extract the routine into an object exposing `asm_<name>`:
 *        ./mkdifftest.sh ../cpu/dspproc.asm 1248 1277 NonEchoMono \
 *            "Voice0Volume Voice0EnvInc ..." > /dev/null
 *
 *   2. In difftest_<name>.c define the shared globals, include the C port,
 *      declare asm_<name>, then:
 *
 *        DT_MAIN(seed, iterations) {
 *            ... randomise inputs and save the state the routine mutates ...
 *            ... restore, run the asm, snapshot into A_* ...
 *            ... restore, run the C,   snapshot into C_* ...
 *            DT_EQ("esi", a_esi, c_esi);
 *            DT_MEM("DSPBuffer", A_dsp, C_dsp, sizeof A_dsp);
 *        }
 *        DT_DONE("NonEchoMono");
 *
 * The asm usually wants a register ABI, so call it through a small inline-asm
 * wrapper. Reset every piece of mutated state before *both* runs or the second
 * sees the first's leftovers.
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
/* DT_ITER overrides the count: a mutation sweep only needs enough iterations
   to trip, and the full run is far too slow to do 100 times. */
#define DT_MAIN(seed, iters)                         \
    srand(seed);                                     \
    dt_fails = 0;                                    \
    dt_iters = (long)(iters);                        \
    {                                                \
        const char* dt_e = getenv("DT_ITER");        \
        if (dt_e && *dt_e)                           \
            dt_iters = atol(dt_e);                   \
    }                                                \
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
