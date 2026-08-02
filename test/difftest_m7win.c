/* Differential test: the ProcessMode7ngwin*16b cluster in video/mode716.mac
 * against the C port in video/c_mode716win.c.
 *
 * Five entry points that tail-jump into each other, so every run is driven
 * from one of the five and compared on everything they can touch: the five
 * registers the renderer keeps, the window list cursor and its two counters,
 * the four position accumulators and the tile pointer.
 *
 * The parts that are easy to get wrong are which off-tile handler retries its
 * own test (B's do not, E's do), the second off-tile step inside B's handlers,
 * and the tile-repeat tail in E - which ends by writing mode7yadder into
 * mode7xrpos, an oddity of the original that the port has to keep.
 *
 * The oracle (_m7win.o, built by mkm7win.sh from the pre-port revision) is
 * driven through asm_m7win<n>, which sets the registers up from the same seam
 * block the ported side reads. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int32_t s4;

u4 *ngcwinptr, ngwleft, ngwleftb, pixelsleft;
u4 mode7xpos, mode7ypos, mode7xrpos, mode7yrpos;
u4 mode7xadder, mode7yadder;
u4 m7xaddof, m7yaddof;
u4 m7xaddof2, m7yaddof2; /* dword slots the assembly reads a byte out of */
u4 mmode7ptr, mmode7xadd2, mmode7yadd2;
u4 mmode7xinc, mmode7xincc, mmode7yinc, switchtorep3;
u1 mode7set;
u1 vrama[65536];
static u1 vrambuf[65536];
u1* vram;

/* The seam block lives in video/c_mode716win.c; the oracle uses the same
   names. */
extern u4 M7WinAX, M7WinBX, M7WinCX, M7WinSI, M7WinDI;
extern u4 asm_winedx; /* _m7win.o: edx after the call, which must survive */

void asm_m7wina(void), asm_m7winb(void), asm_m7winc(void);
void asm_m7wind(void), asm_m7wine(void);
void c_ProcessMode7ngwin16b(void), c_ProcessMode7ngwinB16b(void);
void c_ProcessMode7ngwinC16b(void), c_ProcessMode7ngwinD16b(void);
void c_ProcessMode7ngwinE16b(void);

/* The window list. The routines step forwards through it and stop on a run
   that covers the rest of the line, so a short list with a big terminator can
   never be walked off the end. */
#define WINLIST 32
static u4 winlist[WINLIST];

typedef struct {
    u4 ax, bx, cx, dx, si, di;
    u4 cursor, left, leftb, pixels;
    u4 xpos, ypos, xrpos, yrpos, ptr;
} snapshot;

static void run(int const asm_side, int const which, u4 const bx, u4 const si,
    u4 const di, snapshot* const out)
{
    ngcwinptr = winlist;

    M7WinAX = 0xA0000000u;
    M7WinBX = bx;
    M7WinCX = 0xC0000000u;
    M7WinSI = si;
    M7WinDI = di;

    asm_winedx = 0xD1D10000u;
    if (asm_side) {
        switch (which) {
        case 0: asm_m7wina(); break;
        case 1: asm_m7winb(); break;
        case 2: asm_m7winc(); break;
        case 3: asm_m7wind(); break;
        default: asm_m7wine(); break;
        }
    } else {
        switch (which) {
        case 0: c_ProcessMode7ngwin16b(); break;
        case 1: c_ProcessMode7ngwinB16b(); break;
        case 2: c_ProcessMode7ngwinC16b(); break;
        case 3: c_ProcessMode7ngwinD16b(); break;
        default: c_ProcessMode7ngwinE16b(); break;
        }
    }

    out->ax = M7WinAX;
    out->bx = M7WinBX;
    out->cx = M7WinCX;
    out->dx = asm_winedx;
    out->si = M7WinSI;
    out->di = M7WinDI;
    out->cursor = (u4)(ngcwinptr - winlist);
    out->left = ngwleft;
    out->leftb = ngwleftb;
    out->pixels = pixelsleft;
    out->xpos = mode7xpos;
    out->ypos = mode7ypos;
    out->xrpos = mode7xrpos;
    out->yrpos = mode7yrpos;
    out->ptr = mmode7ptr;
}

/* Everything the routines read, captured so both sides start identically. */
typedef struct {
    u4 list[WINLIST], left, xpos, ypos, xrpos, yrpos, xadder, yadder;
    u4 xaddof, xaddof2, yaddof, yaddof2, ptr, xadd2, yadd2;
    u4 xinc, xincc, yinc;
    u1 set;
} inputs;

static void install(inputs const* const in)
{
    memcpy(winlist, in->list, sizeof winlist);
    ngwleft = in->left;
    ngwleftb = 0xEEEEEEEEu;
    pixelsleft = 0xEEEEEEEEu;
    mode7xpos = in->xpos;
    mode7ypos = in->ypos;
    mode7xrpos = in->xrpos;
    mode7yrpos = in->yrpos;
    mode7xadder = in->xadder;
    mode7yadder = in->yadder;
    m7xaddof = in->xaddof;
    m7xaddof2 = in->xaddof2;
    m7yaddof = in->yaddof;
    m7yaddof2 = in->yaddof2;
    mmode7ptr = in->ptr;
    mmode7xadd2 = in->xadd2;
    mmode7yadd2 = in->yadd2;
    mmode7xinc = in->xinc;
    mmode7xincc = in->xincc;
    mmode7yinc = in->yinc;
    mode7set = in->set;
    switchtorep3 = 0;
}

int main(void)
{
    /* Distinct contents, so a path that reaches the buffer by the wrong one
       of the two names is visible (see the vram/vrama trap in $2118/$2119). */
    vram = vrambuf;
    for (u4 i = 0; i < sizeof vrama; i++) {
        vrama[i] = (u1)(i * 7u + (i >> 8));
        vrambuf[i] = (u1)(i * 13u + 5u);
    }

    DT_MAIN(20260802, 200000)
    {
        inputs in;
        snapshot x, y;
        int const which = (int)dt_mod(5);
        u4 bx, si, di;

        /* Short runs, so a line crosses several list entries; the last entry
           is large enough to always terminate the walk. */
        in.left = 1 + dt_mod(24);
        for (int i = 0; i < WINLIST; i++) {
            in.list[i] = dt_mod(4) ? dt_mod(6) : 0;
        }
        in.list[WINLIST - 2] = 1000;
        in.list[WINLIST - 1] = 1000;

        /* Byte 1 of each position decides the off-tile path; bias it so both
           the in-tile and the crossed-tile branches come up often. */
        in.xrpos = (dt_mod(2) ? (dt_u32() & 0xFFFF00FFu) : dt_u32());
        in.yrpos = (dt_mod(2) ? (dt_u32() & 0xFFFF00FFu) : dt_u32());
        in.xpos = dt_u32();
        in.ypos = dt_u32();
        in.xadder = dt_u32();
        in.yadder = dt_u32();
        in.xaddof = dt_u32();
        in.yaddof = dt_u32();
        in.xaddof2 = dt_u32();
        in.yaddof2 = dt_u32();
        in.xadd2 = dt_mod(2) ? 0x800u : (u4)-0x800;
        in.yadd2 = dt_mod(2) ? 0x800u : (u4)-0x800;
        /* mmode7ptr indexes vrama, and the assembly does so without masking on
           the x path - keep it inside the buffer. */
        in.ptr = dt_u32() & 0x7FFFu;
        /* Mode7Startup16b only ever produces these, and the values matter:
           a zero increment never reaches the exit test, so a random one hangs
           the oracle as readily as the port. */
        in.xinc = dt_mod(2) ? 2 : (u4)-2;
        in.xincc = dt_mod(2) ? 0 : 0xFE;
        in.yinc = dt_mod(2) ? 1 : (u4)-1;
        in.set = (u1)dt_u32(); /* bit 6 is the tile-repeat flag */

        /* Steer into E's wrap exit - and so the tile-repeat tail - far more
           often than chance: park the pointer byte one increment short of the
           value that ends the walk. Must come after xinc/xincc are chosen. */
        if (dt_mod(2)) {
            in.ptr = (in.ptr & ~0xFFu) | ((in.xincc - in.xinc) & 0xFFu);
        }

        bx = dt_u32() & 0xFFFFu;
        si = 0x51000000u | (dt_u32() & 0xFFFFu);
        di = (u4)(uintptr_t)vrama;

        install(&in);
        run(1, which, bx, si, di, &x);
        install(&in);
        run(0, which, bx, si, di, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx (must be preserved)", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ngcwinptr", x.cursor, y.cursor);
        DT_EQ("ngwleft", x.left, y.left);
        DT_EQ("ngwleftb", x.leftb, y.leftb);
        DT_EQ("pixelsleft", x.pixels, y.pixels);
        DT_EQ("mode7xpos", x.xpos, y.xpos);
        DT_EQ("mode7ypos", x.ypos, y.ypos);
        DT_EQ("mode7xrpos", x.xrpos, y.xrpos);
        DT_EQ("mode7yrpos", x.yrpos, y.yrpos);
        DT_EQ("mmode7ptr", x.ptr, y.ptr);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ entry=%d left=%u xrpos=%08x yrpos=%08x ptr=%08x set=%02x\n",
                which, in.left, in.xrpos, in.yrpos, in.ptr, in.set);
        }
    }
    DT_DONE("mode 7 window-skip cluster");
}
