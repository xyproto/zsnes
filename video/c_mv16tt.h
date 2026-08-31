/*
 * The colour-maths pixel writers shared by the 16t tile drawers:
 * draw8x816t{a,b,c} with their winon/winonb twins, plus the group walkers
 * (the drawtilegrpfull pair differs only in which register holds the tile
 * pointer). The plain 8x8, offset-mode and 16x16 drawers all instantiate them.
 *
 * Where the *bt drawers (video/c_mv16tbt.h) *produce* the transparency buffer,
 * these consume it: every pixel is blended with what is underneath.
 */
#ifndef C_MV16TT_H
#define C_MV16TT_H

#include <stdint.h>
#include <string.h>

#include "../types.h"

extern u4 pal16b[256], pal16bcl[256], pal16bxcl[256];
extern u2 fulladdtab[65537]; /* the dword load below reads the last entry */

/* The assembly writes 1111011111011110b out in full here rather than reading
   vesa2_clbit: the low bit of each 5-6-5 channel, cleared so a sum cannot
   carry between them. */
#define TT_CLBIT 0xF7DEu

enum {
    T_HALF,
    T_ADD,
    T_SUB
};

typedef struct {
    zreg ax, bx, cx, dx, si, di, bp;
} tt_regs;

/* One pixel. `n` is its place on screen, `k` its place in the tile, `w` where
   the window mask is read: the 8x8 and 16x16 forms pass n, since their flipped
   writers index it with 7-k, and the 16x8 one passes its own counter. `adder`
   is the palette base, reaching the plain forms in dh and the windowed ones
   through coadder16.

   The half-add form works in eax and leaves it zero; the other two work in ebx
   and ecx and leave the palette index in eax. */
static void tt_px(tt_regs* const r, u1 const* const tile, u1 const* const win,
    u1 const adder, u1* const esi, u1 const* const ebp, u4 const k,
    u4 const n, u4 const w, int const mode)
{
    u4 eax = tile[k];

    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (win != 0 && win[w] != 0) {
        return;
    }
    eax = (u1)(eax + adder);
    r->ax = eax;
    if (mode == T_HALF) {
        u4 ecx;

        memcpy(&ecx, ebp + n * 2, 4);
        eax = pal16b[eax];
        /* A transparent pixel underneath means there is nothing to average
           with, so the colour goes down unchanged. */
        if ((u2)ecx != 0) {
            ecx &= TT_CLBIT;
            eax = ((eax & TT_CLBIT) + ecx) >> 1;
        }
        r->cx = ecx;
        *(u2*)(esi + n * 2) = (u2)eax;
        r->ax = 0;
    } else {
        /* pal16bcl and pal16bxcl are already clipped, so only the value from
           the transparency buffer needs masking; the pair then goes through
           fulladdtab, read as a dword from a table of words. */
        u4 const* const pal = mode == T_ADD ? pal16bcl : pal16bxcl;
        u4 ebx, ecx;

        memcpy(&ebx, ebp + n * 2, 4);
        ecx = pal[eax];
        ebx &= TT_CLBIT;
        ecx = (ecx + ebx) >> 1;
        memcpy(&ecx, fulladdtab + ecx, 4);
        if (mode == T_SUB) {
            ecx ^= 0xFFFFu;
        }
        r->bx = ebx;
        r->cx = ecx;
        *(u2*)(esi + n * 2) = (u2)ecx;
    }
}

/* Eight pixels in two halves, each skipped whole when its four tile bytes are
   zero. The flipped form walks the tile backwards, so it tests the second
   dword first - each half tests the one holding the pixels it is about to
   draw. */
static inline void tt_row(tt_regs* const r, u1 const* const tile, u1 const* const win,
    u1 const adder, u1* const esi, u1 const* const ebp, int const flip,
    int const mode)
{
    for (u4 g = 0; g < 8; g += 4) {
        u4 const k = flip ? 7 - g : g;

        if (*(u4 const*)(tile + (k & 4u)) == 0) {
            continue;
        }
        for (u4 n = g; n < g + 4; n++) {
            tt_px(r, tile, win, adder, esi, ebp, flip ? 7 - n : n, n, n,
                mode);
        }
    }
}

#endif
