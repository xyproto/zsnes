/*
 * The two 8x8 tile dispatchers, ported from the tail of video/makev16t.asm.
 *
 * Each spills the register file into a seam, calls its C body and reloads,
 * then hands back the mosaic tail rather than taking it - see the mosaic tail note in git history: the
 * assembly reached domosaic16b by a jump, and test/difftest_t8t.c records the
 * register state there.
 *
 * draw8x816bt's mode 2 branch is a seven-argument call that carries on
 * afterwards, not a jump away like draw8x816t's. Reading the two alike caught
 * a first attempt.
 */

#include "c_mv16draw.h"

extern u1 bgmode, osm2dis;

extern u4 TTAX, TTBX, TTCX, TTDX, TTSI, TTDI, TTBP, TTTail;
extern u4 T8AX, T8BX, T8CX, T8DX, T8SI, T8DI, T8BP, T8Tail;

void c_draw8x816t(void);
void c_draw8x816bt(void);
/* Seven arguments, in the order the ccallv pushed them. */
void draw8x816boffset(u4 a, u4 c, u4 d, u4 b, u4 bp, u4 si, u4 di);

static TILE_LEAF(draw8x816t_plain, TT, c_draw8x816t) static TILE_LEAF(draw8x816bt_plain, T8, c_draw8x816bt)

    u4 draw8x816t(m7regs* const r)
{
    /* The offset form is mode 2 only, and only while offset-per-tile is
       enabled; mode 5 goes to the 16x8 drawer whichever way that went. */
    if (osm2dis != 1 && bgmode == 2)
        return draw8x816toffset(r);
    if (bgmode == 5)
        return draw16x816t(r);
    return draw8x816t_plain(r);
}

u4 draw8x816bt(m7regs* const r)
{
    if (bgmode == 2) {
        draw8x816boffset((u4)r->ax, (u4)r->cx, (u4)r->dx, (u4)r->bx,
            (u4)r->bp, (u4)r->si, (u4)r->di);
    }
    if (bgmode == 5)
        return draw16x816t(r);
    return draw8x816bt_plain(r);
}
