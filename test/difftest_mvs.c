/* Differential test: draw8x816tsms in video/mv16tms.asm against the C port in
 * video/c_mv16tsms.c.
 *
 * A row of 33 8x8 tiles with subtractive colour maths. What it has to get
 * right beyond the arithmetic: the priority bit skipping a tile without
 * advancing the output, the flip bits picking the reversed group order and the
 * reversed y adder, the tile-cache clip, the 8-bit palette shift, and the
 * column counter wrapping back to the saved tile map at 0x20. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

u1 tileleft16b, drawn, temp, bshifter, curbgpr, bgcoloradder, coadder16;
/* Read by the 'a' variant's mosaic tail, which this test does not drive. */
u1 curmosaicsz;
u4 pal16b[256];
static u1 winmask[512];
u1* winptrref = winmask;
u4 tempcach, temptile, bgofwptr, bgsubby, yadder, yrevadder;
/* The 16x16 writer in the same file wants these; nothing here reaches it. */
u1 a16x16xinc;
u2 yadd, yflipadd;
u4 pal16bxcl[256];
u4 pal16bcl[256];
/* One entry of slack: the writer's dword load reads past the last index. */
u2 fulladdtab[65537];

/* The tile cache, the tile map and the two output buffers. */
/* 1024 tiles x 64 bytes, plus room for the y adder and the 64-byte row. */
#define CACHE 0x10100u
#define OUTBUF 4096u
static u1 cache[CACHE];
static u1 tilemap[512];
static u1 vidbuf[OUTBUF], transp[OUTBUF];

extern u4 MVSAX, MVSBX, MVSCX, MVSDX, MVSSI, MVSDI, MVSBP;
void asm_mvs0(void), asm_mvs1(void);
void c_draw8x816tsms(void);
void c_draw8x816tswinonms(void);

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
    u1 left, drew, coadd, tmp;
    u1 vid[OUTBUF], tr[OUTBUF];
} snapshot;

static void run(int const asm_side, int const which, u4 const dx,
    u1 const* const vseed, u1 const* const tseed, snapshot* const out)
{
    memcpy(vidbuf, vseed, OUTBUF);
    memcpy(transp, tseed, OUTBUF);
    tileleft16b = 0xEE;
    drawn = 0xEE;
    coadder16 = 0xEE;
    winptrref = winmask;

    MVSAX = 0xA0000000u;
    MVSBX = 0xB0000000u;
    MVSCX = 0xC0000000u;
    MVSDX = dx;
    MVSSI = (u4)(uintptr_t)vidbuf;
    MVSDI = (u4)(uintptr_t)tilemap;
    MVSBP = (u4)(uintptr_t)transp;

    if (asm_side) {
        which ? asm_mvs1() : asm_mvs0();
    } else {
        which ? c_draw8x816tswinonms() : c_draw8x816tsms();
    }

    out->ax = MVSAX;
    out->bx = MVSBX;
    out->cx = MVSCX;
    out->dx = MVSDX;
    out->si = MVSSI;
    out->di = MVSDI;
    out->bp = MVSBP;
    out->left = tileleft16b;
    out->drew = drawn;
    out->coadd = coadder16;
    out->tmp = temp;
    memcpy(out->vid, vidbuf, OUTBUF);
    memcpy(out->tr, transp, OUTBUF);
}

int main(void)
{
    for (u4 i = 0; i < 65536; i++) {
        fulladdtab[i] = (u2)(i * 5u + (i >> 3));
    }
    for (u4 i = 0; i < 256; i++) {
        /* Real entries are 16-bit colours; anything wider would push the
           fulladdtab index past the table, which the emulator never does. */
        pal16bxcl[i] = (i * 0x0101u + 0x1234u) & 0xFFFFu;
    }

    DT_MAIN(20260802, 20000)
    {
        u1 vseed[OUTBUF], tseed[OUTBUF];
        snapshot x, y;
        int const which = (int)dt_mod(2);
        u4 const dx = dt_u32();

        /* The row is 33 tiles and the column counter wraps at 0x20, so start
           near the wrap often enough to cover it. */
        u1 const temp0 = (u1)(dt_mod(2) ? (0x1Cu + dt_mod(8)) : dt_u32());
        temp = temp0;
        /* Non-zero bytes drop a pixel, so keep zeros common. */
        for (u4 i = 0; i < sizeof winmask; i++) {
            winmask[i] = (u1)(dt_mod(3) ? 0 : dt_u32());
        }
        bshifter = (u1)(dt_mod(2) ? dt_mod(8) : dt_u32());
        curbgpr = (u1)(dt_mod(2) ? 0x00u : 0x20u);
        bgcoloradder = (u1)dt_u32();
        yadder = dt_mod(56);
        yrevadder = dt_mod(56);
        /* Keep every tile read inside the cache: 33 tiles x 64 bytes, plus the
           y adder, has to fit under CACHE. */
        tempcach = (u4)(uintptr_t)cache;
        /* Put the clip boundary inside the tile range so it actually fires,
           and keep the pull-back within the cache. */
        bgsubby = dt_mod(2) ? 0x8000u : 0x4000u;
        bgofwptr = tempcach + bgsubby + dt_mod(0x8000u);
        temptile = (u4)(uintptr_t)tilemap;

        for (u4 i = 0; i < sizeof cache; i++) {
            /* Zero dwords are the group-skip fast path; make them common. */
            cache[i] = (u1)(dt_mod(3) ? 0 : dt_u32());
        }
        for (u4 i = 0; i < sizeof tilemap; i += 2) {
            u2 e = (u2)dt_u32();
            /* Bound the tile number so tempcach + tile*64 stays in the cache. */
            e = (u2)((e & 0xFC00u) | dt_mod(1024));
            *(u2*)(tilemap + i) = e;
        }
        dt_fill(vseed, OUTBUF);
        dt_fill(tseed, OUTBUF);

        temp = temp0;
        run(1, which, dx, vseed, tseed, &x);
        temp = temp0;
        run(0, which, dx, vseed, tseed, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("tileleft16b", x.left, y.left);
        DT_EQ("drawn", x.drew, y.drew);
        DT_EQ("coadder16", x.coadd, y.coadd);
        DT_EQ("temp", x.tmp, y.tmp);
        DT_MEM("video buffer", x.vid, y.vid, OUTBUF);
        DT_MEM("transparency buffer", x.tr, y.tr, OUTBUF);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s temp=%02x shifter=%02x curbgpr=%02x adder=%02x "
                   "yadd=%u yrev=%u subby=%u\n",
                which ? "winon" : "plain", temp0, bshifter, curbgpr,
                bgcoloradder, yadder, yrevadder, bgsubby);
        }
    }
    DT_DONE("mv16tms 8x8 subtractive tile row");
}
