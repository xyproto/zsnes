/* Differential test: the shared draw*ms prologue in video/mv16tms.asm against
 * the C port in video/c_mv16tms.c.
 *
 * Straight-line setup, but it decides three things the eleven draw routines
 * downstream all depend on: the biased video and window pointers, whether the
 * mosaic scratch line is used, and which of the three tile caches the tile
 * pointer falls in. It also leaves ecx zeroed on the mosaic path, which the
 * caller sees. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

u1 temp, bshifter, curmosaicsz;
/* The 16x16 prologue in the same file wants these; only the 8x8 one runs
   here, so plain globals are enough - see difftest_mvall.c for the layout
   that one depends on. */
u1 a16x16xinc, a16x16yinc, drawn, curypos;
u2 yadd, yflipadd;
u4 yadder, yrevadder, tempcach, temptile, bgsubby, bgofwptr;
u1 *cwinptr, *winptrref, *curvidoffset;
u1 *vcache2b, *vcache4b, *vcache8b;
u1 xtravbuf[576];

/* The three caches are one allocation in the emulator, 2-bit then 4-bit then
   8-bit; the prologue's compare chain only makes sense against that layout. */
#define C2SZ 262144u
#define C4SZ 131072u
#define C8SZ 65536u
static u1 cache[C2SZ + C4SZ + C8SZ];
static u1 vidbuf[4096], winbuf[4096];

extern u4 MVAX, MVBX, MVCX, MVDX, MVSI; /* video/c_mv16tms.c */
extern u4 asm_mvdi, asm_mvbp; /* _mv16.o */
void asm_mv16(void);
void c_draw16tms_setup(void);

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
    u4 yadd, yrev, cach, tile, subby, ofw, wptr;
    u1 tmp, shift;
    u1 scratch[576];
} snapshot;

static void run(int const asm_side, u4 const ax, u4 const bx, u4 const cx,
    u4 const dx, u4 const si, snapshot* const out)
{
    temp = bshifter = 0xEE;
    yadder = yrevadder = tempcach = temptile = 0xEEEEEEEEu;
    bgsubby = bgofwptr = 0xEEEEEEEEu;
    winptrref = 0;
    memset(xtravbuf, 0x5A, sizeof xtravbuf);

    MVAX = ax;
    MVBX = bx;
    MVCX = cx;
    MVDX = dx;
    MVSI = si;
    asm_mvdi = asm_mvbp = 0;

    if (asm_side) {
        asm_mv16();
    } else {
        c_draw16tms_setup();
        asm_mvdi = 0xD1D1D1D1u;
        asm_mvbp = 0xB9B9B9B9u;
    }

    out->ax = MVAX;
    out->bx = MVBX;
    out->cx = MVCX;
    out->dx = MVDX;
    out->si = MVSI;
    out->di = asm_mvdi;
    out->bp = asm_mvbp;
    out->yadd = yadder;
    out->yrev = yrevadder;
    out->cach = tempcach;
    out->tile = temptile;
    out->subby = bgsubby;
    out->ofw = bgofwptr;
    out->wptr = (u4)(uintptr_t)winptrref;
    out->tmp = temp;
    out->shift = bshifter;
    memcpy(out->scratch, xtravbuf, sizeof out->scratch);
}

int main(void)
{
    cwinptr = winbuf + 2048;
    curvidoffset = vidbuf + 2048;
    vcache2b = cache;
    vcache4b = cache + C2SZ;
    vcache8b = cache + C2SZ + C4SZ;

    DT_MAIN(20260802, 200000)
    {
        u4 const ax = dt_u32();
        u4 const cx = dt_u32();
        u4 const dx = dt_u32();
        /* A small horizontal offset, as the renderer uses. */
        u4 const si = dt_u32() & 0xFFu;
        u4 bx;
        snapshot x, y;

        /* The cache test is a chain of unsigned compares against three fixed
           bases; land the pointer in each region often, not by chance. */
        switch (dt_mod(5)) {
        case 0:
            bx = (u4)(uintptr_t)cache + dt_mod(C2SZ);
            break;
        case 1:
            bx = (u4)(uintptr_t)cache + C2SZ + dt_mod(C4SZ);
            break;
        case 2:
            bx = (u4)(uintptr_t)cache + C2SZ + C4SZ + dt_mod(C8SZ);
            break;
        /* Straddle each boundary exactly, so the >= compares are pinned. */
        case 3:
            bx = (u4)(uintptr_t)cache + C2SZ - 2u + dt_mod(4);
            break;
        default:
            bx = (u4)(uintptr_t)cache + C2SZ + C4SZ - 2u + dt_mod(4);
            break;
        }
        curmosaicsz = (u1)(dt_mod(2) ? 1 : dt_mod(16));

        run(1, ax, bx, cx, dx, si, &x);
        run(0, ax, bx, cx, dx, si, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi (untouched)", x.di, y.di);
        DT_EQ("ebp (untouched)", x.bp, y.bp);
        DT_EQ("temp", x.tmp, y.tmp);
        DT_EQ("bshifter", x.shift, y.shift);
        DT_EQ("yadder", x.yadd, y.yadd);
        DT_EQ("yrevadder", x.yrev, y.yrev);
        DT_EQ("tempcach", x.cach, y.cach);
        DT_EQ("temptile", x.tile, y.tile);
        DT_EQ("bgsubby", x.subby, y.subby);
        DT_EQ("bgofwptr", x.ofw, y.ofw);
        DT_EQ("winptrref", x.wptr, y.wptr);
        DT_MEM("xtravbuf", x.scratch, y.scratch, sizeof x.scratch);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ eax=%08x ebx=%08x ecx=%08x hofs=%02x mosaic=%u\n",
                ax, bx, cx, si, curmosaicsz);
        }
    }
    DT_DONE("mv16tms shared prologue");
}
