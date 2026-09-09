/* Mode7Startup16b (video/mode716.mac) against video/c_mode716start.c. A leaf
 * over the Mode 7 scratch block, but its callers keep eax, esi and edi, so
 * those are compared too - and ebx/ecx, which the assembly zeroes on the way
 * out.
 *
 * The two easy mistakes: the map coordinate lives one byte into each position
 * dword, so its 16-bit add must not carry into the top byte; and the M7HROn
 * branch is the same arithmetic with two extra halvings and a wider y clip. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int32_t s4;

/* The scratch block; the layout only matters to the renderer, not here, but
   mcxloc and mcyloc are words and the rest are dwords the assembly writes in
   full or a byte at a time. */
u4 M7HROn;
u2 mcxloc, mcyloc;
u4 mmode7xpos, mmode7ypos, mmode7xadder, mmode7yadder;
u4 mmode7xadd2, mmode7yadd2;
u4 mmode7xinc, mmode7xincc, mmode7yinc; /* dword slots, written a byte at a time */
u2 m7starty;
u2 mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0;
u1 mode7set;
u1 curmosaicsz;
u1* pesimpng;
u1 xtravbuf[576];

/* The seam block lives in video/c_mode716start.c; the oracle reaches it by the
   same names. */
extern u4 M7StartAX, M7StartDX, M7StartSI, M7StartDI;

void asm_m7start(void); /* _m7start.o */
extern u4 asm_bx, asm_cx, asm_dx; /* _m7start.o */
void c_Mode7Startup16b(void);

typedef struct {
    u4 ax, dx, si, di, bx, cx;
    u2 cx_loc, cy_loc;
    u4 xpos, ypos, xadder, yadder, xadd2, yadd2;
    u4 xinc, xincc, yinc;
    u4 esim;
    u1 buf[576];
} snapshot;

static void run(int const asm_side, u4 const ax, u4 const dx, u4 const si,
    snapshot* const out)
{
    /* Poison, so a field the port forgets to write shows up as a difference
       rather than as two matching leftovers. */
    mcxloc = mcyloc = 0x5A5A;
    mmode7xpos = mmode7ypos = 0xDEADBEEFu;
    mmode7xadder = mmode7yadder = 0xDEADBEEFu;
    mmode7xadd2 = mmode7yadd2 = 0xDEADBEEFu;
    mmode7xinc = mmode7xincc = mmode7yinc = 0xDEADBEEFu;
    pesimpng = 0;
    memset(xtravbuf, 0x5A, sizeof xtravbuf);

    M7StartAX = ax;
    M7StartDX = dx;
    M7StartSI = si;
    M7StartDI = 0xD1D1D1D1u;
    asm_bx = asm_cx = asm_dx = 0;

    if (asm_side) {
        asm_m7start();
    } else {
        c_Mode7Startup16b();
        /* The thunk in video/mode716.mac clears these; the C never sees
           them, so compare what the thunk will produce. */
        asm_bx = asm_cx = asm_dx = 0;
    }

    out->ax = M7StartAX;
    out->dx = asm_dx;
    out->si = M7StartSI;
    out->di = M7StartDI;
    out->bx = asm_bx;
    out->cx = asm_cx;
    out->cx_loc = mcxloc;
    out->cy_loc = mcyloc;
    out->xpos = mmode7xpos;
    out->ypos = mmode7ypos;
    out->xadder = mmode7xadder;
    out->yadder = mmode7yadder;
    out->xadd2 = mmode7xadd2;
    out->yadd2 = mmode7yadd2;
    out->xinc = mmode7xinc;
    out->xincc = mmode7xincc;
    out->yinc = mmode7yinc;
    out->esim = (u4)(uintptr_t)pesimpng;
    memcpy(out->buf, xtravbuf, sizeof out->buf);
}

/* Values the renderer really sees: 13-bit signed matrix entries and screen
   coordinates, with the odd full-range one to keep the sign extension honest. */
static u2 coord(void)
{
    return (u2)(dt_mod(4) ? (dt_u32() & 0x1FFFu) : dt_u32());
}

int main(void)
{
    DT_MAIN(20260801, 200000)
    {
        u4 const ax = dt_mod(4) ? (dt_u32() & 0x3FFFu) : dt_u32();
        u4 const dx = dt_mod(4) ? (dt_u32() & 0x3FFFu) : dt_u32();
        u4 const si = 0x51000000u | (dt_u32() & 0xFFFFu);
        snapshot x, y;

        /* The assembly tests the low byte of this dword slot, so drive the
           whole dword: 0/1 evenly for the two branches, plus values whose
           low byte and dword disagree. */
        M7HROn = dt_mod(2) ? dt_mod(2) : ((dt_u32() & 0xFFFF00u) | dt_mod(4));
        mode7A = coord();
        mode7B = coord();
        mode7C = coord();
        mode7D = coord();
        mode7X0 = coord();
        mode7Y0 = coord();
        mode7set = (u1)dt_u32(); /* bit 0 is the horizontal flip */
        m7starty = (u2)(dt_mod(2) ? dt_mod(256) : dt_u32());
        /* 1 is the common case; anything else takes the mosaic path. */
        curmosaicsz = (u1)(dt_mod(2) ? 1 : dt_mod(16));

        run(1, ax, dx, si, &x);
        run(0, ax, dx, si, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("mcxloc", x.cx_loc, y.cx_loc);
        DT_EQ("mcyloc", x.cy_loc, y.cy_loc);
        DT_EQ("mmode7xpos", x.xpos, y.xpos);
        DT_EQ("mmode7ypos", x.ypos, y.ypos);
        DT_EQ("mmode7xadder", x.xadder, y.xadder);
        DT_EQ("mmode7yadder", x.yadder, y.yadder);
        DT_EQ("mmode7xadd2", x.xadd2, y.xadd2);
        DT_EQ("mmode7yadd2", x.yadd2, y.yadd2);
        DT_EQ("mmode7xinc", x.xinc, y.xinc);
        DT_EQ("mmode7xincc", x.xincc, y.xincc);
        DT_EQ("mmode7yinc", x.yinc, y.yinc);
        DT_EQ("pesimpng", x.esim, y.esim);
        DT_MEM("xtravbuf", x.buf, y.buf, sizeof x.buf);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ hr=%u ax=%x dx=%x set=%02x mosaic=%u "
                   "A=%04x B=%04x C=%04x D=%04x X0=%04x Y0=%04x starty=%04x\n",
                M7HROn, ax, dx, mode7set, curmosaicsz, mode7A, mode7B, mode7C,
                mode7D, mode7X0, mode7Y0, m7starty);
        }
    }
    DT_DONE("mode 7 per-scanline startup");
}
