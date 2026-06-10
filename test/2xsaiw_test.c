/*
 * 2xSaI scalar line filter unit tests
 *
 * Covers the three identical entry points ported from video/2xsaiw.asm:
 *   _2xSaILine / _2xSaISuper2xSaILine / _2xSaISuperEagleLine
 *
 * Each expands one row of 16-bit source pixels into a 2x2 block on the
 * destination, writing the top row at dst and the bottom row at dst+dstPitch.
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

static void check_doubled_row(LineFilter* f, const char* label)
{
    ZT_SECTION(label);

    u2 src[4] = { 0x1234, 0x5678, 0x9ABC, 0xDEF0 };
    u1 dst[64];
    const u4 width = 4;
    const u4 dstPitch = 16; /* bytes per output row */

    memset(dst, 0xAA, sizeof(dst));
    f(src, NULL, 0, width, dst, dstPitch);

    /* Each source pixel produces a 2x2 block of identical 16-bit values. */
    u2* row0 = (u2*)dst;
    u2* row1 = (u2*)(dst + dstPitch);
    for (u4 i = 0; i < width; i++) {
        ZT_CHECK(row0[i * 2 + 0] == src[i]);
        ZT_CHECK(row0[i * 2 + 1] == src[i]);
        ZT_CHECK(row1[i * 2 + 0] == src[i]);
        ZT_CHECK(row1[i * 2 + 1] == src[i]);
    }

    /* Bytes beyond the second row must remain untouched. */
    for (size_t i = dstPitch * 2; i < sizeof(dst); i++) {
        ZT_CHECK(dst[i] == 0xAA);
    }
}

static void test_zero_width(void)
{
    ZT_SECTION("zero width: leaves dst untouched");

    u2 src[1] = { 0xCAFE };
    u1 dst[16];
    memset(dst, 0x55, sizeof(dst));
    _2xSaILine(src, NULL, 0, 0, dst, 8);
    for (size_t i = 0; i < sizeof(dst); i++) {
        ZT_CHECK(dst[i] == 0x55);
    }
}

static void test_nontrivial_pitch(void)
{
    ZT_SECTION("dstPitch separates the two output rows");

    u2 src[2] = { 0xAAAA, 0x5555 };
    u1 dst[64];
    const u4 dstPitch = 24; /* deliberately larger than row size */

    memset(dst, 0, sizeof(dst));
    _2xSaILine(src, NULL, 0, 2, dst, dstPitch);

    u2* row0 = (u2*)dst;
    u2* row1 = (u2*)(dst + dstPitch);
    ZT_CHECK(row0[0] == 0xAAAA && row0[1] == 0xAAAA);
    ZT_CHECK(row0[2] == 0x5555 && row0[3] == 0x5555);
    ZT_CHECK(row1[0] == 0xAAAA && row1[1] == 0xAAAA);
    ZT_CHECK(row1[2] == 0x5555 && row1[3] == 0x5555);

    /* Bytes between the two rows (gap from row-width to dstPitch) untouched. */
    for (u4 i = 8; i < dstPitch; i++) {
        ZT_CHECK(dst[i] == 0);
    }
}

int main(void)
{
    printf("ZSNES2 2xSaI line filter tests\n");

    check_doubled_row(_2xSaILine, "_2xSaILine: doubles each pixel into 2x2 block");
    check_doubled_row(_2xSaISuper2xSaILine, "_2xSaISuper2xSaILine: doubles each pixel into 2x2 block");
    check_doubled_row(_2xSaISuperEagleLine, "_2xSaISuperEagleLine: doubles each pixel into 2x2 block");
    test_zero_width();
    test_nontrivial_pitch();

    ZT_RESULTS();
}
