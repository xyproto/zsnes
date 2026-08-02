/* Differential test: Mode7Process in video/mode716.mac against the C port in
 * video/c_mode716proc.c.
 *
 * Both macros - Mode7Process and its big-step sibling Mode7ProcessB - in all
 * ten pixel writers and all three
 * shapes mode7set selects (repeating map, no repetition, no repetition with
 * tile repeat). The A/B on real games only reaches the first of those, which
 * is the whole reason this exists.
 *
 * Compared: the two screen buffers and the EXTBG priority plane, every scratch
 * position, and the six registers the macro's tail hands on to domosaicng16b -
 * plus which of the two exits it took.
 *
 * The oracle (_m7proc.o, built by mkm7proc.sh from the pre-port revision) is
 * driven through asm_m7proc<n>, one per writer. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;

#define M7_BUF 75036u
#define WINDOW 512u
#define VSZ (M7_BUF * 8u + WINDOW)

u4 mtemp;
u4 mmode7xpos, mmode7ypos, mmode7xrpos, mmode7yrpos;
u4 mmode7xadder, mmode7yadder, mmode7xadd2, mmode7yadd2;
u4 mmode7ptr;
u4 mm7xaddof, mm7xaddof2, mm7yaddof, mm7yaddof2;
u4 m7xaddof, m7xaddof2, m7yaddof, m7yaddof2;
u4 mode7xpos, mode7ypos, mode7xrpos, mode7yrpos, mode7xadder, mode7yadder;
u4 ngwleft, ngwleftb, pixelsleft, switchtorep3;
/* The window run list the ngwin cluster walks; a big terminator stops it. */
#define WINLIST 600
static u4 winlist[WINLIST];
u4* ngcwinptr;
u4 mmode7xinc, mmode7xincc, mmode7yinc; /* dword slots, read a byte at a time */
u1 mode7set;
u1 mode7tab[65536];
/* edi is a tile pointer (base + byte*128) and is then indexed by ebx, so the
   walk reads past 64K by design; give both buffers the slack the emulator's
   larger allocation provides, or the test faults where the emulator does not. */
#define VRAM_SLACK 0x18000u
u1 vrama[65536 + VRAM_SLACK];
static u1 vrambuf[65536 + VRAM_SLACK];
u1* vram;
u1 curmosaicsz;
u4 UnusedBit[2], UnusedBitXor[2];

extern u4 M7PAX, M7PBX, M7PCX, M7PDX, M7PSI, M7PDI, M7PBP, M7PWriter;
extern u4 asm_mosaic; /* _m7proc.o */

#define ENT(n) void asm_m7proc##n(void);
ENT(0)
ENT(1) ENT(2) ENT(3) ENT(4) ENT(5) ENT(6) ENT(7) ENT(8) ENT(9)
    ENT(10) ENT(11) ENT(12) ENT(13) ENT(14) ENT(15) ENT(16) ENT(17) ENT(18) ENT(19)
        ENT(20) ENT(21) ENT(22) ENT(23) ENT(24) ENT(25) ENT(26) ENT(27) ENT(28) ENT(29)
            ENT(30) ENT(31) ENT(32) ENT(33) ENT(34) ENT(35) ENT(36) ENT(37) ENT(38) ENT(39) void c_Mode7Process(void);
void c_Mode7ProcessB(void);
void c_Mode7Processngw16b(void);
void c_Mode7Processngw216b(void);
extern u4 M7PWriter2;

/* 0-9 Mode7Process, 10-19 Mode7ProcessB, 20-29 Mode7Processngw16b - the same
   ten writers each; 30-39 Mode7Processngw216b, which takes a pair. */
static void (*const asm_entry[40])(void) = { asm_m7proc0, asm_m7proc1,
    asm_m7proc2, asm_m7proc3, asm_m7proc4, asm_m7proc5, asm_m7proc6,
    asm_m7proc7, asm_m7proc8, asm_m7proc9, asm_m7proc10, asm_m7proc11,
    asm_m7proc12, asm_m7proc13, asm_m7proc14, asm_m7proc15, asm_m7proc16,
    asm_m7proc17, asm_m7proc18, asm_m7proc19, asm_m7proc20, asm_m7proc21,
    asm_m7proc22, asm_m7proc23, asm_m7proc24, asm_m7proc25, asm_m7proc26,
    asm_m7proc27, asm_m7proc28, asm_m7proc29, asm_m7proc30, asm_m7proc31,
    asm_m7proc32, asm_m7proc33, asm_m7proc34, asm_m7proc35, asm_m7proc36,
    asm_m7proc37, asm_m7proc38, asm_m7proc39 };

/* Both sides render into the same buffer - esi comes back as a raw pointer -
   so the three windows it can touch are refreshed between runs instead. */
static u1 vbuf[VSZ];
static u2 pal[512];

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp, mosaic;
    u4 temp, xpos, ypos, xrpos, yrpos, ptr;
    u4 xaddof, xaddof2, yaddof, yaddof2;
    u4 nxpos, nypos, nxrpos, nyrpos, wleft, wleftb, cursor, rep3;
    u1 main[WINDOW], sub[WINDOW], prio[WINDOW];
} snapshot;

static void run(int const asm_side, u4 const e, u1 const* const painted,
    u1 const* const prio, snapshot* const out)
{
    memcpy(vbuf, painted, WINDOW);
    memcpy(vbuf + M7_BUF * 2, painted, WINDOW);
    memcpy(vbuf + M7_BUF * 8, prio, WINDOW);

    M7PAX = 0;
    M7PBX = 0;
    M7PCX = 0;
    M7PDX = 0;
    M7PSI = (u4)(uintptr_t)vbuf;
    M7PDI = (u4)(uintptr_t)vram;
    M7PBP = (u4)(uintptr_t)pal;
    M7PWriter = e % 10u;
    /* The pairs mkm7proc.sh's PROC_ENTRY2 table uses: each group of five
       wraps within itself, so it is not simply the next index. */
    static u4 const pair[10] = { 1, 2, 3, 4, 0, 6, 7, 8, 9, 5 };
    M7PWriter2 = e >= 30u ? pair[e % 10u] : 0u;
    asm_mosaic = 0;
    ngcwinptr = winlist;
    mm7xaddof = mm7yaddof = 0xEEEEEEEEu;
    mm7xaddof2 = mm7yaddof2 = 0xEEEEEEEEu;

    if (asm_side) {
        asm_entry[e]();
    } else {
        if (e < 10u) {
            c_Mode7Process();
        } else if (e < 20u) {
            c_Mode7ProcessB();
        } else if (e < 30u) {
            c_Mode7Processngw16b();
        } else {
            c_Mode7Processngw216b();
        }
        /* The macro's tail, which stayed in assembly. */
        M7PAX = 0;
        M7PDX = (M7PDX & ~0xFF00u) | ((u4)curmosaicsz << 8);
        asm_mosaic = curmosaicsz != 1;
    }

    out->ax = M7PAX;
    out->bx = M7PBX;
    out->cx = M7PCX;
    out->dx = M7PDX;
    out->si = M7PSI;
    out->di = M7PDI;
    out->bp = M7PBP;
    out->mosaic = asm_mosaic;
    out->temp = mtemp;
    out->xpos = mmode7xpos;
    out->ypos = mmode7ypos;
    out->xrpos = mmode7xrpos;
    out->yrpos = mmode7yrpos;
    out->ptr = mmode7ptr;
    out->xaddof = mm7xaddof;
    out->xaddof2 = mm7xaddof2;
    out->yaddof = mm7yaddof;
    out->yaddof2 = mm7yaddof2;
    out->nxpos = mode7xpos;
    out->nypos = mode7ypos;
    out->nxrpos = mode7xrpos;
    out->nyrpos = mode7yrpos;
    out->wleft = ngwleft;
    out->wleftb = ngwleftb;
    out->cursor = (u4)(ngcwinptr - winlist);
    out->rep3 = switchtorep3;
    memcpy(out->main, vbuf, WINDOW);
    memcpy(out->sub, vbuf + M7_BUF * 2, WINDOW);
    memcpy(out->prio, vbuf + M7_BUF * 8, WINDOW);
}

typedef struct {
    u4 xpos, ypos, xadder, yadder, xadd2, yadd2, ptr, xinc, xincc, yinc;
    u1 set, mosaic;
} inputs;

static void install(inputs const* const in)
{
    mmode7xpos = in->xpos;
    mmode7ypos = in->ypos;
    mmode7xrpos = 0xEEEEEEEEu;
    mmode7yrpos = 0xEEEEEEEEu;
    mmode7xadder = in->xadder;
    mmode7yadder = in->yadder;
    mmode7xadd2 = in->xadd2;
    mmode7yadd2 = in->yadd2;
    mmode7ptr = in->ptr;
    mmode7xinc = in->xinc;
    mmode7xincc = in->xincc;
    mmode7yinc = in->yinc;
    mode7set = in->set;
    curmosaicsz = in->mosaic;
    mtemp = 0xEEEEEEEEu;
    /* The windowed variant walks these instead of the mmode7* copies. */
    mode7xpos = in->xpos;
    mode7ypos = in->ypos;
    mode7xrpos = 0xEEEEEEEEu;
    mode7yrpos = 0xEEEEEEEEu;
    mode7xadder = in->xadder;
    mode7yadder = in->yadder;
    m7xaddof = m7yaddof = 0xEEEEEEEEu;
    m7xaddof2 = m7yaddof2 = 0xEEEEEEEEu;
    ngwleft = ngwleftb = 0xEEEEEEEEu;
    switchtorep3 = 0xEE;
}

int main(void)
{
    /* Distinct contents: the walk reaches the buffer by the vram pointer in
       one place and the vrama symbol in another, and they are not
       interchangeable. */
    vram = vrambuf;
    for (u4 i = 0; i < 65536 + VRAM_SLACK; i++) {
        vrama[i] = (u1)(i * 7u + (i >> 8));
        vrambuf[i] = (u1)(i * 13u + 5u);
    }
    for (u4 i = 0; i < 65536; i++) {
        mode7tab[i] = (u1)(((i & 0x07u) << 4) + ((i >> 8 & 0x07u) << 1) + 1u);
    }

    DT_MAIN(20260802, 20000)
    {
        inputs in;
        snapshot x, y;
        u4 const e = dt_mod(40); /* writer, and which of the four macros */
        u1 painted[WINDOW], prio[WINDOW];

        /* mode7set bit 7 picks the shape and bit 6 the tile repeat; drive the
           three combinations evenly rather than 1-in-4. */
        u4 const shape = dt_mod(3);
        in.set = (u1)((dt_u32() & 0x3Fu)
            | (shape == 0 ? 0u : (shape == 1 ? 0x80u : 0xC0u)));
        /* Positions land on the map about half the time; byte 2 over 3 is off
           it, and the whole no-repetition path turns on that. */
        in.xpos = dt_mod(2) ? (dt_u32() & 0x03FFFFFFu) : dt_u32();
        in.ypos = dt_mod(2) ? (dt_u32() & 0x03FFFFFFu) : dt_u32();
        /* Small steps, so a 256-pixel line crosses tiles many times. */
        /* Bounded to what a Mode 7 matrix can produce. The windowed variant
           hands the y adder to the ngwin cluster, which uses it as a vrama
           index - an arbitrary 32-bit value reads far out of the buffer, which
           the emulator's larger allocation absorbs and this test cannot.
           0x80000000 is the one adder whose negation stays negative, and the
           only input separating ProcessB's signed step compare from an
           unsigned one, so inject it where it cannot reach the cluster. */
        in.xadder = dt_mod(2) ? (dt_u32() & 0x3FFFu) : (dt_u32() & 0x1FFFFu);
        in.yadder = dt_mod(2) ? (dt_u32() & 0x3FFFu) : (dt_u32() & 0x1FFFFu);
        if (e < 20u || e >= 30u) {
            /* Negative adders and the 0x80000000 corner only where they cannot
               reach the cluster as an index. */
            if (dt_mod(2)) {
                in.xadder = (u4)(s4)(s2)dt_u32();
            }
            if (dt_mod(2)) {
                in.yadder = (u4)(s4)(s2)dt_u32();
            }
            if (dt_mod(64) == 0) {
                in.xadder = 0x80000000u;
            }
            if (dt_mod(64) == 0) {
                in.yadder = 0x80000000u;
            }
        }
        /* A zero increment never reaches the wrap test, which hangs the
           original as readily as the port. */
        in.xinc = dt_mod(2) ? 2u : (u4)-2;
        in.xincc = dt_mod(2) ? 0u : 0xFEu;
        in.yinc = dt_mod(2) ? 1u : (u4)-1;
        in.xadd2 = dt_mod(2) ? 0x800u : (u4)-0x800;
        in.yadd2 = dt_mod(2) ? 0x800u : (u4)-0x800;
        in.ptr = dt_u32() & 0x7FFFu;
        /* Park the tile pointer one increment short of the value that ends the
           on-map walk, so the wrap - and the tile-repeat tail behind it -
           happens early enough that pixels are left to exercise it. */
        if (dt_mod(2)) {
            in.ptr = (in.ptr & ~0xFFu) | ((in.xincc - in.xinc) & 0xFFu);
        }
        in.mosaic = (u1)(dt_mod(2) ? 1 : dt_mod(16));

        UnusedBit[0] = dt_u32();
        UnusedBitXor[0] = dt_u32();
        for (u4 i = 0; i < 512; i++) {
            pal[i] = (u2)dt_u32();
        }
        dt_fill(painted, WINDOW);
        dt_fill(prio, WINDOW);

        /* Short runs throughout, so a 256-pixel line crosses many of them -
           ngw216b switches writers at every boundary, and a list that settles
           on a big terminator early stops exercising that. The last two are
           still huge, as a bound for the ngwin cluster's walk. */
        for (int i = 0; i < WINLIST; i++) {
            /* Zero-length runs are legal but poisonous for coverage: --ngwleft
               wraps to 4 billion (as the assembly does), so no further run
               boundary fires on that line. Keep them rare. */
            winlist[i] = dt_mod(16) ? (1u + dt_mod(8)) : 0u;
        }
        winlist[WINLIST - 2] = 1000;
        winlist[WINLIST - 1] = 1000;

        install(&in);
        run(1, e, painted, prio, &x);
        install(&in);
        run(0, e, painted, prio, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("mosaic exit", x.mosaic, y.mosaic);
        DT_EQ("mtemp", x.temp, y.temp);
        DT_EQ("mmode7xpos", x.xpos, y.xpos);
        DT_EQ("mmode7ypos", x.ypos, y.ypos);
        DT_EQ("mmode7xrpos", x.xrpos, y.xrpos);
        DT_EQ("mmode7yrpos", x.yrpos, y.yrpos);
        DT_EQ("mmode7ptr", x.ptr, y.ptr);
        DT_EQ("mm7xaddof", x.xaddof, y.xaddof);
        DT_EQ("mm7xaddof2", x.xaddof2, y.xaddof2);
        DT_EQ("mm7yaddof", x.yaddof, y.yaddof);
        DT_EQ("mm7yaddof2", x.yaddof2, y.yaddof2);
        DT_EQ("mode7xpos", x.nxpos, y.nxpos);
        DT_EQ("mode7ypos", x.nypos, y.nypos);
        DT_EQ("mode7xrpos", x.nxrpos, y.nxrpos);
        DT_EQ("mode7yrpos", x.nyrpos, y.nyrpos);
        DT_EQ("ngwleft", x.wleft, y.wleft);
        DT_EQ("ngwleftb", x.wleftb, y.wleftb);
        DT_EQ("ngcwinptr", x.cursor, y.cursor);
        DT_EQ("switchtorep3", x.rep3, y.rep3);
        DT_MEM("main screen", x.main, y.main, WINDOW);
        DT_MEM("sub screen", x.sub, y.sub, WINDOW);
        DT_MEM("priority plane", x.prio, y.prio, WINDOW);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s writer=%u set=%02x xpos=%08x ypos=%08x ptr=%04x mosaic=%u\n",
                e < 10u ? "Process"
                        : (e < 20u ? "ProcessB" : (e < 30u ? "ngw16b" : "ngw216b")),
                e % 10u, in.set, in.xpos,
                in.ypos, in.ptr, in.mosaic);
        }
    }
    DT_DONE("mode 7 scanline walk");
}
