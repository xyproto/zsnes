/*
 * 2xSaI line filter tests (video/2xsaiw.c).
 *
 * The three entry points - _2xSaILine, _2xSaISuper2xSaILine and
 * _2xSaISuperEagleLine - are Kreed's filters. Bit-identity with the original
 * assembly is the difftest's job (make 2xsai); these cover the shape of the
 * output and that the three are no longer the same function.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../types.h"
#include "zstest.h"

typedef void LineFilter(u2*, u1*, u4, u4, u1*, u4);

LineFilter _2xSaILine;
LineFilter _2xSaISuper2xSaILine;
LineFilter _2xSaISuperEagleLine;

#define WIDTH 16
#define MARGIN 4
#define STRIDE (WIDTH + 2 * MARGIN)
#define ROWS 6
#define SRC_PITCH (STRIDE * 2)
#define DST_PITCH (WIDTH * 2 * 2)

static u2 srcbuf[ROWS * STRIDE];
static u1 dst[DST_PITCH * 2 + 32];

static u2* line(void) { return srcbuf + STRIDE + MARGIN; }

static void run(LineFilter* const f)
{
    memset(dst, 0xC3, sizeof dst);
    f(line(), NULL, SRC_PITCH, WIDTH, dst, DST_PITCH);
}

static u2 out(u4 const x, u4 const row)
{
    return ((u2*)(dst + row * DST_PITCH))[x];
}

static void check_flat(LineFilter* const f, char const* const label)
{
    ZT_SECTION(label);

    for (u4 i = 0; i < ROWS * STRIDE; i++)
        srcbuf[i] = 0x5AC3;
    run(f);

    int ok = 1;
    for (u4 row = 0; row < 2; row++)
        for (u4 x = 0; x < WIDTH * 2; x++)
            if (out(x, row) != 0x5AC3)
                ok = 0;
    ZT_CHECK(ok);

    /* Nothing past the two output rows. */
    for (size_t i = DST_PITCH * 2; i < sizeof dst; i++)
        ZT_CHECK(dst[i] == 0xC3);
}

static void check_zero_width(LineFilter* const f, char const* const label)
{
    ZT_SECTION(label);
    for (u4 i = 0; i < ROWS * STRIDE; i++)
        srcbuf[i] = (u2)(i * 37u);
    memset(dst, 0x55, sizeof dst);
    f(line(), NULL, SRC_PITCH, 0, dst, DST_PITCH);
    int clean = 1;
    for (size_t i = 0; i < sizeof dst; i++)
        if (dst[i] != 0x55)
            clean = 0;
    ZT_CHECK(clean);
}

/* A checkerboard is the case every one of these filters was written for, and
   the three disagree about it - which is the whole point of having three. */
static void check_three_differ(void)
{
    ZT_SECTION("the three filters no longer produce identical output");

    u1 a[DST_PITCH * 2], b[DST_PITCH * 2], c[DST_PITCH * 2];

    for (u4 r = 0; r < ROWS; r++)
        for (u4 x = 0; x < STRIDE; x++)
            srcbuf[r * STRIDE + x] = ((r + x) & 1) ? 0xFFFF : 0x0000;

    run(_2xSaILine);
    memcpy(a, dst, sizeof a);
    run(_2xSaISuper2xSaILine);
    memcpy(b, dst, sizeof b);
    run(_2xSaISuperEagleLine);
    memcpy(c, dst, sizeof c);

    ZT_CHECK(memcmp(a, b, sizeof a) != 0);
    ZT_CHECK(memcmp(a, c, sizeof a) != 0);
    ZT_CHECK(memcmp(b, c, sizeof b) != 0);
}

/* An edge has to be smoothed: output colours the input never had. */
static void check_interpolates(LineFilter* const f, char const* const label)
{
    ZT_SECTION(label);

    /* A checkerboard: every filter has a rule for it, and none of those
       rules is "copy a neighbour". */
    for (u4 r = 0; r < ROWS; r++)
        for (u4 x = 0; x < STRIDE; x++)
            srcbuf[r * STRIDE + x] = ((r + x) & 1) ? 0xFFFF : 0x0000;
    run(f);

    int blended = 0;
    for (u4 row = 0; row < 2; row++)
        for (u4 x = 0; x < WIDTH * 2; x++)
            if (out(x, row) != 0xFFFF && out(x, row) != 0x0000)
                blended = 1;
    ZT_CHECK(blended);
}

int main(void)
{
    printf("ZSNES2 2xSaI line filter tests\n");

    check_flat(_2xSaILine, "_2xSaILine: a flat field is unchanged");
    check_flat(_2xSaISuper2xSaILine, "Super2xSaI: a flat field is unchanged");
    check_flat(_2xSaISuperEagleLine, "SuperEagle: a flat field is unchanged");

    check_zero_width(_2xSaILine, "zero width leaves dst untouched");

    check_interpolates(_2xSaILine, "_2xSaILine smooths an edge");
    check_interpolates(_2xSaISuper2xSaILine, "Super2xSaI smooths an edge");
    check_interpolates(_2xSaISuperEagleLine, "SuperEagle smooths an edge");

    check_three_differ();

    ZT_RESULTS();
}
