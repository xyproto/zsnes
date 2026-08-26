/* Differential test: drawmode7win16b / drawmode7ngextbg16b, as video/mode716.asm
 * had them, against the C port in video/c_mode716draw.c and the seam around it
 * in video/c_mode716gate.c.
 *
 * The top of the Mode 7 renderer: the transparency test, the writer-selection
 * tree over the main/sub window tables, the four Mode7*Sub wrappers, and the
 * choice of scanline walk. The A/B on real games only reaches a few arms of
 * that tree, which is why this exists.
 *
 * The oracle is the whole pre-port file (_m7draw.o, built by mkm7draw.sh), so
 * it runs the original scanline walks rather than the ported ones - an
 * end-to-end comparison, not just of the dispatch.
 *
 * BuildWindow is stubbed and ignores its arguments on purpose: the port
 * deliberately passes different ones (see difftest_m7bw.c, which pins that),
 * and letting it through would swamp everything else.
 *
 * domosaicng16b is stubbed as a recorder on both sides: the oracle tail-jumps
 * to it, and the ported seam in video/c_mode716gate.c calls it. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../video/c_mode716gate.h"
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;

#define M7_BUF 75036u
#define WINDOW 512u
#define VRAM_SLACK 0x18000u
#define VSZ (M7_BUF * 8u + WINDOW)

/* The Mode 7 scratch block. */
u4 mtemp;
/* The emulator's data block puts a four-byte spacer after each of these,
   because video/c_mode716proc.c reads a dword one byte in and that has to stay
   inside one object. Mirror the shape here so the same names line up. */
u4 mmode7xpos8[2], mmode7ypos8[2], mode7xpos8[2], mode7ypos8[2];
extern u4 mmode7xpos __attribute__((alias("mmode7xpos8")));
extern u4 mmode7ypos __attribute__((alias("mmode7ypos8")));
extern u4 mode7xpos __attribute__((alias("mode7xpos8")));
extern u4 mode7ypos __attribute__((alias("mode7ypos8")));
u4 mmode7xrpos, mmode7yrpos;
u4 mmode7xadder, mmode7yadder, mmode7xadd2, mmode7yadd2;
u4 mmode7ptr, mmode7xinc, mmode7xincc, mmode7yinc;
u4 mm7xaddof, mm7xaddof2, mm7yaddof, mm7yaddof2;
u4 m7xaddof, m7xaddof2, m7yaddof, m7yaddof2;
u4 mode7xrpos, mode7yrpos, mode7xadder, mode7yadder;
u4 ngwleft, ngwleftb, pixelsleft, switchtorep3, M7HROn;
u2 mcxloc, mcyloc, m7starty;
u2 mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0;
u1 mode7set, mode7hr[256], curmosaicsz;
u1 mode7tab[65536];
u1 vrama[65536 + VRAM_SLACK];
static u1 vrambuf[65536 + VRAM_SLACK];
u1* vram;
u1 xtravbuf[576];
u1* pesimpng;
u4 UnusedBit[2], UnusedBitXor[2];

/* Window and screen state. */
u1 scrndis;
u1 BGMS1[2048], FillSubScr[256], scadtng[256];
static u1 mainwin[256], subwin[256];
u1 *CMainWinScr = mainwin, *CSubWinScr = subwin;
u1 winbg1enval[1024], winlogicaval[1024];
u4 nglogicval, ngwinen;
u4 ngwintable[64];
u4* ngcwinptr;
u1 *cwinptr, *winptrref;
u1* curvidoffset;

#define WINLIST 600
static u4 winlist[WINLIST];

/* Stubbed: see the header comment. Wrapped in the same register-preserving
   shim the real one has (video/c_makev16b.c), because ProcessBuildWindow's
   caller keeps the Mode 7 x coordinate in edx across the call. */
static u4 bw_fill0, bw_fill1, bw_hits;
void c_BuildWindow(u4 eax, u4 ebx)
{
    (void)eax;
    (void)ebx;
    bw_hits++;
    ngwintable[0] = bw_fill0;
    ngwintable[1] = bw_fill1;
    ngwinen = 1;
}

/* clang-format off */
__asm__(".globl BuildWindow\n"
        "BuildWindow:\n"
        "pushl %ecx\n"
        "pushl %edx\n"
        "pushl 16(%esp)\n"
        "pushl 16(%esp)\n"
        "call c_BuildWindow\n"
        "addl $8, %esp\n"
        "popl %edx\n"
        "popl %ecx\n"
        "ret\n");
void BuildWindow(u4, u4);
/* clang-format on */

extern u4 asm_mosaic, asm_regs[7]; /* _m7draw.o */
void asm_m7draw0(void), asm_m7draw1(void);

extern u4 M7PAX, M7PBX, M7PCX, M7PDX, M7PSI, M7PDI;

/* The mosaic recorder the seam reaches, and the M7Seam half of that file,
   which nothing here drives. */
u4 MOSAX, MOSBX, MOSCX, MOSDX, MOSSI, MOSDI, MOSBP;
u4 M7SeamA, M7SeamB, M7SeamC, M7SeamD, M7SeamSI, M7SeamDI, M7SeamBP;
void c_domosaicng16b(void) { asm_mosaic = 1; }
void c_processmode7hires16b(void) { }
void c_drawmode7ngextbg216b(void) { }
void c_drawmode7win16b(void);
void c_drawmode7ngextbg16b(void);

static u1 vbuf[VSZ];
static u2 pal[512];

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp, mosaic, hits;
    u4 winen, xpos, ypos, xrpos, yrpos, ptr, temp, wleft, wleftb, cursor;
    u4 nxpos, nypos, nxrpos, nyrpos, esim, logic, wptr;
    u1 main[WINDOW], sub[WINDOW], prio[WINDOW], scratch[576];
} snapshot;

typedef struct {
    u4 xpos, ypos, xadder, yadder, ptr;
    u2 A, B, C, D, X0, Y0, starty;
    u1 set, mosaic, dis, hr;
    u4 winen; /* carries over from the previous line */
} inputs;

static void install(inputs const* const in)
{
    mmode7xpos = in->xpos;
    mmode7ypos = in->ypos;
    mmode7xrpos = mmode7yrpos = 0xEEEEEEEEu;
    mmode7xadder = in->xadder;
    mmode7yadder = in->yadder;
    mmode7ptr = in->ptr;
    mmode7xadd2 = mmode7yadd2 = 0xEEEEEEEEu;
    mmode7xinc = mmode7xincc = mmode7yinc = 0xEEEEEEEEu;
    mm7xaddof = mm7yaddof = mm7xaddof2 = mm7yaddof2 = 0xEEEEEEEEu;
    m7xaddof = m7yaddof = m7xaddof2 = m7yaddof2 = 0xEEEEEEEEu;
    mode7xpos = mode7ypos = mode7xrpos = mode7yrpos = 0xEEEEEEEEu;
    mode7xadder = mode7yadder = 0xEEEEEEEEu;
    mtemp = ngwleft = ngwleftb = pixelsleft = 0xEEEEEEEEu;
    switchtorep3 = 0;
    mode7A = in->A;
    mode7B = in->B;
    mode7C = in->C;
    mode7D = in->D;
    mode7X0 = in->X0;
    mode7Y0 = in->Y0;
    m7starty = in->starty;
    mode7set = in->set;
    curmosaicsz = in->mosaic;
    scrndis = in->dis;
    M7HROn = 0;
    ngwinen = in->winen;
    nglogicval = 0;
    ngcwinptr = winlist;
    pesimpng = 0;
    memset(xtravbuf, 0x5A, sizeof xtravbuf);
    memset(ngwintable, 0x33, sizeof ngwintable);
    bw_hits = 0;
}

static void run(int const asm_side, int const which, u4 const ax, u4 const dx,
    u4 const bx, u1 const* const painted, u1 const* const prio,
    snapshot* const out)
{
    memcpy(vbuf, painted, WINDOW);
    memcpy(vbuf + M7_BUF * 2, painted, WINDOW);
    memcpy(vbuf + M7_BUF * 8, prio, WINDOW);
    curvidoffset = vbuf;

    if (asm_side) {
        asm_regs[0] = ax;
        asm_regs[1] = bx;
        asm_regs[2] = 0xC0000000u;
        asm_regs[3] = dx;
        asm_regs[4] = (u4)(uintptr_t)vbuf;
        asm_regs[5] = 0xD1D1D1D1u;
        asm_regs[6] = (u4)(uintptr_t)pal;
        asm_mosaic = 0;
        which ? asm_m7draw1() : asm_m7draw0();
    } else {
        m7regs r;

        r.ax = ax;
        r.bx = bx;
        r.cx = 0xC0000000u;
        r.dx = dx;
        r.si = (u4)(uintptr_t)vbuf;
        r.di = 0xD1D1D1D1u;
        r.bp = (u4)(uintptr_t)pal;
        /* The seam's tail hands back M7P*, which a body that returns early
           never writes; start them at the registers it was entered with. */
        M7PAX = ax;
        M7PBX = bx;
        M7PCX = 0xC0000000u;
        M7PDX = dx;
        M7PSI = (u4)(uintptr_t)vbuf;
        M7PDI = 0xD1D1D1D1u;
        asm_mosaic = 0;
        if (which) {
            drawmode7ngextbg16b(&r);
        } else {
            drawmode7win16b(&r);
        }
        asm_regs[0] = (u4)r.ax;
        asm_regs[1] = (u4)r.bx;
        asm_regs[2] = (u4)r.cx;
        asm_regs[3] = (u4)r.dx;
        asm_regs[4] = (u4)r.si;
        asm_regs[5] = (u4)r.di;
        asm_regs[6] = (u4)r.bp;
        if (scrndis & 1u) {
            /* The assembly returns before the tail on this path. */
            asm_regs[0] = ax;
            asm_regs[1] = bx;
            asm_regs[2] = 0xC0000000u;
            asm_regs[3] = dx;
            asm_regs[4] = (u4)(uintptr_t)vbuf;
            asm_regs[5] = 0xD1D1D1D1u;
        }
    }

    out->ax = asm_regs[0];
    out->bx = asm_regs[1];
    out->cx = asm_regs[2];
    out->dx = asm_regs[3];
    out->si = asm_regs[4];
    out->di = asm_regs[5];
    out->bp = asm_regs[6];
    out->mosaic = asm_mosaic;
    out->hits = bw_hits;
    out->winen = ngwinen;
    out->xpos = mmode7xpos;
    out->ypos = mmode7ypos;
    out->xrpos = mmode7xrpos;
    out->yrpos = mmode7yrpos;
    out->ptr = mmode7ptr;
    out->temp = mtemp;
    out->wleft = ngwleft;
    out->wleftb = ngwleftb;
    out->cursor = (u4)(ngcwinptr - winlist);
    out->nxpos = mode7xpos;
    out->nypos = mode7ypos;
    out->nxrpos = mode7xrpos;
    out->nyrpos = mode7yrpos;
    out->esim = (u4)(uintptr_t)pesimpng;
    out->logic = nglogicval;
    out->wptr = (u4)(uintptr_t)winptrref;
    memcpy(out->main, vbuf, WINDOW);
    memcpy(out->sub, vbuf + M7_BUF * 2, WINDOW);
    memcpy(out->prio, vbuf + M7_BUF * 8, WINDOW);
    memcpy(out->scratch, xtravbuf, sizeof out->scratch);
}

int main(void)
{
    vram = vrambuf;
    cwinptr = vrama; /* only ever copied into winptrref */
    winptrref = 0;
    for (u4 i = 0; i < 65536 + VRAM_SLACK; i++) {
        vrama[i] = (u1)(i * 7u + (i >> 8));
        vrambuf[i] = (u1)(i * 13u + 5u);
    }
    for (u4 i = 0; i < 65536; i++) {
        mode7tab[i] = (u1)(((i & 0x07u) << 4) + ((i >> 8 & 0x07u) << 1) + 1u);
    }

    DT_MAIN(20260802, 4000)
    {
        inputs in;
        snapshot x, y;
        int const which = (int)dt_mod(2);
        u4 const bx = dt_mod(256);
        u4 ax, dx;
        u1 painted[WINDOW], prio[WINDOW];

        /* The dispatch tree turns on these four; drive each independently so
           every arm - including the paired main/sub walks - comes up. */
        BGMS1[bx * 2] = (u1)dt_u32();
        BGMS1[bx * 2 + 1] = (u1)dt_u32();
        FillSubScr[bx] = (u1)dt_u32();
        scadtng[bx] = (u1)dt_u32();
        mainwin[bx] = (u1)(dt_mod(2) ? 0 : dt_u32());
        subwin[bx] = (u1)(dt_mod(2) ? 0 : dt_u32());
        for (u4 i = 0; i < 1024; i++) {
            winbg1enval[i] = (u1)(dt_mod(2) ? (dt_u32() | 0x02u) : (dt_u32() & ~0x0Au));
            winlogicaval[i] = (u1)dt_u32();
        }
        in.winen = dt_mod(2) ? 0u : (dt_mod(2) ? 1u : (dt_u32() | 0x100u));
        in.dis = (u1)(dt_mod(16) == 0 ? 1 : 0);
        in.hr = (u1)dt_mod(2);
        mode7hr[bx] = in.hr;

        u4 const shape = dt_mod(3);
        in.set = (u1)((dt_u32() & 0x3Fu)
            | (shape == 0 ? 0u : (shape == 1 ? 0x80u : 0xC0u)));
        in.xpos = dt_mod(2) ? (dt_u32() & 0x03FFFFFFu) : dt_u32();
        in.ypos = dt_mod(2) ? (dt_u32() & 0x03FFFFFFu) : dt_u32();
        /* Bounded: the ngw walk hands the y adder to the window cluster, which
           uses it as a vrama index. Both sides of 0x7F0 so the plain and
           big-step walks are both chosen. */
        in.xadder = dt_mod(2) ? (dt_u32() & 0x7FFu) : (dt_u32() & 0x1FFFFu);
        in.yadder = dt_mod(2) ? (dt_u32() & 0x7FFu) : (dt_u32() & 0x1FFFFu);
        /* The walk is chosen on a signed compare against 0x7F0, so straddle it
           from both sides - including negative, which a mask never gives. */
        if (dt_mod(4) == 0) {
            in.xadder = (u4)(0x7E8 + (s4)dt_mod(32));
        }
        if (dt_mod(4) == 0) {
            in.yadder = (u4)(0x7E8 + (s4)dt_mod(32));
        }
        if (dt_mod(4) == 0) {
            in.xadder = (u4)(-(s4)(0x7E8 + (s4)dt_mod(32)));
        }
        if (dt_mod(4) == 0) {
            in.yadder = (u4)(-(s4)(0x7E8 + (s4)dt_mod(32)));
        }
        in.ptr = dt_u32() & 0x7FFFu;
        in.mosaic = (u1)(dt_mod(2) ? 1 : dt_mod(16));
        /* Mode7Startup16b derives mmode7xadder/yadder from A and C, and the
           walk is chosen on those against +/-0x7F0 - so straddle the bound
           here, not on the adders install() writes. */
        in.A = (u2)(dt_mod(4) ? (dt_u32() & 0x1FFFu) : dt_u32());
        if (dt_mod(3) == 0) {
            in.A = (u2)(0x7E8u + dt_mod(32));
        } else if (dt_mod(3) == 0) {
            in.A = (u2)(0x1FFFu & (0x2000u - (0x7E8u + dt_mod(32))));
        }
        in.B = (u2)(dt_mod(4) ? (dt_u32() & 0x1FFFu) : dt_u32());
        in.C = (u2)(dt_mod(4) ? (dt_u32() & 0x1FFFu) : dt_u32());
        if (dt_mod(3) == 0) {
            in.C = (u2)(0x7E8u + dt_mod(32));
        } else if (dt_mod(3) == 0) {
            in.C = (u2)(0x1FFFu & (0x2000u - (0x7E8u + dt_mod(32))));
        }
        in.D = (u2)(dt_mod(4) ? (dt_u32() & 0x1FFFu) : dt_u32());
        in.X0 = (u2)(dt_u32() & 0x1FFFu);
        in.Y0 = (u2)(dt_u32() & 0x1FFFu);
        in.starty = (u2)dt_mod(256);
        ax = dt_u32() & 0x3FFFu;
        dx = dt_u32() & 0x3FFFu;

        bw_fill0 = dt_mod(2) ? 0u : dt_u32();
        bw_fill1 = dt_u32();
        for (int i = 0; i < WINLIST; i++) {
            winlist[i] = dt_mod(16) ? (1u + dt_mod(8)) : 0u;
        }
        winlist[WINLIST - 2] = 1000;
        winlist[WINLIST - 1] = 1000;

        UnusedBit[0] = dt_u32();
        UnusedBitXor[0] = dt_u32();
        for (u4 i = 0; i < 512; i++) {
            pal[i] = (u2)dt_u32();
        }
        dt_fill(painted, WINDOW);
        dt_fill(prio, WINDOW);

        install(&in);
        run(1, which, ax, dx, bx, painted, prio, &x);
        install(&in);
        run(0, which, ax, dx, bx, painted, prio, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("mosaic tail taken", x.mosaic, y.mosaic);
        DT_EQ("BuildWindow calls", x.hits, y.hits);
        DT_EQ("ngwinen", x.winen, y.winen);
        DT_EQ("nglogicval", x.logic, y.logic);
        DT_EQ("winptrref", x.wptr, y.wptr);
        DT_EQ("mmode7xpos", x.xpos, y.xpos);
        DT_EQ("mmode7ypos", x.ypos, y.ypos);
        DT_EQ("mmode7xrpos", x.xrpos, y.xrpos);
        DT_EQ("mmode7yrpos", x.yrpos, y.yrpos);
        DT_EQ("mmode7ptr", x.ptr, y.ptr);
        DT_EQ("mtemp", x.temp, y.temp);
        DT_EQ("ngwleft", x.wleft, y.wleft);
        DT_EQ("ngwleftb", x.wleftb, y.wleftb);
        DT_EQ("ngcwinptr", x.cursor, y.cursor);
        DT_EQ("mode7xpos", x.nxpos, y.nxpos);
        DT_EQ("mode7ypos", x.nypos, y.nypos);
        DT_EQ("mode7xrpos", x.nxrpos, y.nxrpos);
        DT_EQ("mode7yrpos", x.nyrpos, y.nyrpos);
        DT_EQ("pesimpng", x.esim, y.esim);
        DT_MEM("main screen", x.main, y.main, WINDOW);
        DT_MEM("sub screen", x.sub, y.sub, WINDOW);
        DT_MEM("priority plane", x.prio, y.prio, WINDOW);
        DT_MEM("xtravbuf", x.scratch, y.scratch, sizeof x.scratch);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s bx=%x set=%02x dis=%u BGMS1=%02x%02x fill=%02x "
                   "scad=%02x mainwin=%02x subwin=%02x mosaic=%u\n",
                which ? "extbg" : "win", bx, in.set, in.dis, BGMS1[bx * 2 + 1],
                BGMS1[bx * 2], FillSubScr[bx], scadtng[bx], mainwin[bx],
                subwin[bx], in.mosaic);
        }
    }
    DT_DONE("mode 7 draw dispatch");
}
