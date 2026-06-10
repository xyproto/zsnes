/*
 * Scalar nearest-neighbor 2x line filters
 *
 * Ported from video/2xsaiw.asm.  All three entry points share an identical
 * scale2x implementation — they only differ in name to satisfy the
 * LineFilter typedef expected by callers that pick a filter by symbol.
 *
 * Signature (LineFilter, from video/2xsaiw.h):
 *   void f(u2 *src, u1 *unused2, u4 unused3, u4 width, u1 *dst, u4 dstPitch);
 *
 * For each of the `width` source pixels, write a 2x2 block of the same
 * 16-bit pixel into dst: two copies on the top row, two on the row at
 * dst + dstPitch.
 */

#include <stdint.h>

#include "../types.h"

static void scale2x_line(u2* src, u4 width, u1* dst, u4 dstPitch)
{
    u1* dst2 = dst + dstPitch;
    for (u4 i = 0; i < width; i++) {
        u2 p = src[i];
        ((u2*)dst)[0] = p;
        ((u2*)dst)[1] = p;
        ((u2*)dst2)[0] = p;
        ((u2*)dst2)[1] = p;
        dst += 4;
        dst2 += 4;
    }
}

void _2xSaISuper2xSaILine(u2* src, u1* u2_, u4 u3_, u4 width, u1* dst, u4 dstPitch)
{
    (void)u2_;
    (void)u3_;
    scale2x_line(src, width, dst, dstPitch);
}

void _2xSaISuperEagleLine(u2* src, u1* u2_, u4 u3_, u4 width, u1* dst, u4 dstPitch)
{
    (void)u2_;
    (void)u3_;
    scale2x_line(src, width, dst, dstPitch);
}

void _2xSaILine(u2* src, u1* u2_, u4 u3_, u4 width, u1* dst, u4 dstPitch)
{
    (void)u2_;
    (void)u3_;
    scale2x_line(src, width, dst, dstPitch);
}
