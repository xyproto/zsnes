/*
 * video/c_mv16tbt.h - the pixel writers shared by the *16bt tile drawers of
 * video/makev16t.asm.
 *
 * draw8x816bta, draw8x816btawinon and draw8x816btawinonb, plus the
 * drawtilegrp / drawtilegrpf pair that walks them. The assembly instantiates
 * the same three macros from the 8x8 routine (video/c_mv16t8bt.c) and the
 * 16x16 one (video/c_mv16t16bt.c), so they live here rather than in either.
 */
#ifndef C_MV16TBT_H
#define C_MV16TBT_H

#include <stdint.h>

#include "../types.h"

extern u4 pal16b[256];

typedef struct {
    zreg ax, bx, cx, dx, si, di, bp;
} bt_regs;

/* One pixel. `n` is its place on screen, `k` its place in the tile and `w` the
   place the window mask is read at - which is not always either, see bt_row.

   eax is zero on entry to the group, and each form leaves it under 256, so the
   assembly's byte-wide `mov al` is exact here. The windowed forms put the
   colour through eax and clear it; the plain one puts it through ecx and
   leaves it there. */
static void bt_px(bt_regs* const r, u1 const* const tile, u1 const* const win,
    u1 const dh, u1* const esi, u1* const ebp, u4 const k, u4 const n,
    u4 const w)
{
    u4 eax = tile[k];

    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (win != 0 && win[w] != 0) {
        return;
    }
    eax = (u1)(eax + dh);
    r->ax = eax;
    if (win != 0) {
        u4 const col = pal16b[eax];

        *(u2*)(esi + n * 2) = (u2)col;
        *(u2*)(ebp + n * 2) = (u2)col;
        r->ax = 0;
    } else {
        r->cx = pal16b[eax];
        *(u2*)(esi + n * 2) = (u2)r->cx;
        *(u2*)(ebp + n * 2) = (u2)r->cx;
    }
}

/* Eight pixels in two halves, each skipped whole when its four tile bytes are
   zero. The flipped form walks the tile backwards, so it tests the second
   dword first - each half tests the one holding the pixels it is about to
   draw.

   `winbyk` picks which windowed writer the flipped form uses. The 8x8 routine
   instantiates draw8x816btawinonb, which undoes the flip and reads the mask at
   the screen position; the 16x16 one instantiates draw8x816btawinon, which
   does not, so its flipped tiles read the mask backwards. That asymmetry is in
   the original. */
static void bt_row(bt_regs* const r, u1 const* const tile, u1 const* const win,
    u1 const dh, u1* const esi, u1* const ebp, int const flip,
    int const winbyk)
{
    for (u4 g = 0; g < 8; g += 4) {
        u4 const k = flip ? 7 - g : g;

        if (*(u4 const*)(tile + (k & 4u)) == 0) {
            continue;
        }
        for (u4 n = g; n < g + 4; n++) {
            u4 const j = flip ? 7 - n : n;

            bt_px(r, tile, win, dh, esi, ebp, j, n, winbyk ? j : n);
        }
    }
}

#endif
