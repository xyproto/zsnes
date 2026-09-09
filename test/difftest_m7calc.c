/* Differential test: CalculateNewValues in video/mode716.asm against the C
 * port in video/c_mode716calc.c.
 *
 * A leaf over globals, but the caller keeps three registers it leaves behind,
 * so the test compares those as well as the four matrix words and m7starty.
 * The oracle (_m7calc.o, built by mkm7calc.sh from the pre-port revision) is
 * driven through asm_m7call, which sets the registers up from the same seam
 * block the ported side reads. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../video/c_mode716gate.h"
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

/* Both index one and two scanlines past bx without a bounds check, so give
   every table the slack endmem.c's contiguous layout provides. */
#define SLACK 4
u4 mode7ab[256 + SLACK], mode7cd[256 + SLACK];
u2 BG1SXl[256 + SLACK], BG1SYl[256 + SLACK];
u1 BGMA[256 + SLACK];
u2 mode7A, mode7B, mode7C, mode7D;
u1 mode7set;
u2 m7starty;

/* The seam block itself lives in video/c_mode716calc.c. */
extern u4 M7SeamA, M7SeamB, M7SeamC, M7SeamD;

void asm_m7call(void); /* _m7calc.o */
void asm_m7hires(void); /* _m7calc.o */
void c_CalculateNewValues(void);
void c_processmode7hires16b(void);

extern u4 M7SeamSI, M7SeamDI, M7SeamBP;

/* The renderer the hi-res pass calls is stubbed in the oracle assembly, where
   it can log the registers it was actually reached with - the C side gets
   there through the M7CallDraw trampoline, so both see the same ones. It
   clobbers everything on the way out, modelling a real renderer, which is what
   makes a wrapper that forgets to write a register back visible. */
u1* curvidoffset;
u4 M7HROn;
extern u4 DrawHits, DrawRegs[7], DrawVid, DrawHRon; /* _m7calc.o */

/* The C side's renderer, recording into the same block the oracle's does. */
void drawmode7win16b(m7regs* const r)
{
    DrawRegs[0] = (u4)r->ax;
    DrawRegs[1] = (u4)r->bx;
    DrawRegs[2] = (u4)r->cx;
    DrawRegs[3] = (u4)r->dx;
    DrawRegs[4] = (u4)r->si;
    DrawRegs[5] = (u4)r->di;
    DrawRegs[6] = (u4)r->bp;
    DrawVid = (u4)(uintptr_t)curvidoffset;
    DrawHRon = M7HROn;
    DrawHits++;
    r->ax = 0xA5A50001u;
    r->bx = 0xA5A50002u;
    r->cx = 0xA5A50003u;
    r->dx = 0xA5A50004u;
    r->si = 0xA5A50005u;
    r->di = 0xA5A50006u;
    r->bp = 0xA5A50007u;
}

typedef struct {
    u4 a, b, c, d;
    u4 ab[256 + SLACK], cd[256 + SLACK];
    u2 sx[256 + SLACK], sy[256 + SLACK];
    u1 ma[256 + SLACK];
    u2 A, B, C, D, starty;
    u1 set;
    u4 si, di, bp, vid, hron;
    u4 dhits, dregs[7], dvid, dhron;
} snapshot;

static u4 ab_init[256 + SLACK], cd_init[256 + SLACK];
static u2 sx_init[256 + SLACK], sy_init[256 + SLACK];
static u1 ma_init[256 + SLACK];

/* The assembly divides a 64-bit square by a 16-bit span with idiv, which
   faults outright when the quotient will not fit in 32 bits. Real matrices
   never get there, and the C port truncates instead of trapping, so keep the
   oracle away from inputs it cannot survive: |v1 - v0| <= 46340 bounds the
   square below INT32_MAX even at a span of one. */
#define IDIV_SAFE 46340

static s4 word_at(u4 const* const tab, u4 const half, u4 const i)
{
    return *(s2 const*)((u1 const*)tab + i * 4 + half);
}

static void clamp_pred(u4* const tab, u4 const half, u4 const bx)
{
    s4 const v0 = word_at(tab, half, bx);
    s4 const v1 = word_at(tab, half, bx + 1);
    s4 const d = v1 - v0;

    if (d > IDIV_SAFE || d < -IDIV_SAFE) {
        *(s2*)((u1*)tab + (bx + 1) * 4 + half) = (s2)v0;
    }
}

/* kind 0 = CalculateNewValues, 1 = processmode7hires16b. */
static void run(int const asm_side, int const kind, u4 const a, u4 const b,
    u4 const d, u1 const set, snapshot* const out)
{
    memcpy(mode7ab, ab_init, sizeof mode7ab);
    memcpy(mode7cd, cd_init, sizeof mode7cd);
    memcpy(BG1SXl, sx_init, sizeof BG1SXl);
    memcpy(BG1SYl, sy_init, sizeof BG1SYl);
    memcpy(BGMA, ma_init, sizeof BGMA);
    mode7A = mode7B = mode7C = mode7D = 0;
    m7starty = 0;
    mode7set = set;
    M7SeamA = a;
    M7SeamB = b;
    M7SeamC = 0;
    M7SeamD = d;
    /* Distinct, so a register that is dropped or swapped is visible. */
    M7SeamSI = 0x51000000u | b;
    M7SeamDI = 0xD1000000u | b;
    M7SeamBP = 0xB9000000u | b;
    curvidoffset = 0;
    M7HROn = 0;
    DrawHits = DrawVid = DrawHRon = 0;
    memset(DrawRegs, 0, sizeof DrawRegs);

    if (kind == 0) {
        if (asm_side) {
            asm_m7call();
        } else {
            c_CalculateNewValues();
        }
    } else {
        if (asm_side) {
            asm_m7hires();
        } else {
            c_processmode7hires16b();
        }
    }

    out->a = M7SeamA;
    out->b = M7SeamB;
    out->c = M7SeamC;
    out->d = M7SeamD;
    memcpy(out->ab, mode7ab, sizeof out->ab);
    memcpy(out->cd, mode7cd, sizeof out->cd);
    memcpy(out->sx, BG1SXl, sizeof out->sx);
    memcpy(out->sy, BG1SYl, sizeof out->sy);
    memcpy(out->ma, BGMA, sizeof out->ma);
    out->A = mode7A;
    out->B = mode7B;
    out->C = mode7C;
    out->D = mode7D;
    out->starty = m7starty;
    out->set = mode7set;
    out->si = M7SeamSI;
    out->di = M7SeamDI;
    out->bp = M7SeamBP;
    out->vid = (u4)(uintptr_t)curvidoffset;
    out->hron = M7HROn;
    out->dhits = DrawHits;
    out->dvid = DrawVid;
    out->dhron = DrawHRon;
    memcpy(out->dregs, DrawRegs, sizeof out->dregs);
}

int main(void)
{
    DT_MAIN(20260730, 300000)
    {
        snapshot x, y;
        int const kind = (int)dt_mod(2);
        u4 const b = dt_mod(254); /* bx + 2 has to stay inside the slack */
        u4 a, d;
        u1 set;

        /* The matrix tables decide which branch each prediction takes: equal
           endpoints, or a quadratic step whose span can be either sign and as
           small as one. Fill them mostly with near neighbours so the divide is
           exercised at small spans, where truncation shows. */
        for (int i = 0; i < 256 + SLACK; i++) {
            s4 const base = (s4)(dt_u32() & 0xFFFF) - 0x8000;
            s4 const step = dt_mod(2) ? (s4)dt_mod(8) - 4 : (s4)dt_u32();

            ab_init[i] = (u4)((base & 0xFFFF) | ((base + step) & 0xFFFF) << 16);
            cd_init[i] = dt_u32();
            sx_init[i] = (u2)dt_u32();
            sy_init[i] = (u2)dt_u32();
            /* Only 7 takes the quadratic branch; hit it about half the time. */
            ma_init[i] = dt_mod(2) ? 7 : (u1)dt_mod(8);
        }
        /* Force a run of equal endpoints now and then, so the cheap branch and
           the boundary between the two are both covered. */
        if (dt_mod(4) == 0) {
            ab_init[b + 2] = ab_init[b];
        }
        if (dt_mod(4) == 0) {
            cd_init[b + 2] = cd_init[b];
        }

        clamp_pred(ab_init, 0, b);
        clamp_pred(ab_init, 2, b);
        clamp_pred(cd_init, 0, b);
        clamp_pred(cd_init, 2, b);

        a = dt_u32();
        d = dt_u32();
        set = (u1)dt_u32();

        /* The hi-res pass only runs when the next scanline is mode 7, so
           make both answers common. */
        ma_init[b + 1] = dt_mod(2) ? 7 : (u1)dt_mod(8);

        run(1, kind, a, b, d, set, &x);
        run(0, kind, a, b, d, set, &y);

        DT_EQ("eax", x.a, y.a);
        DT_EQ("ebx", x.b, y.b);
        DT_EQ("ecx", x.c, y.c);
        DT_EQ("edx", x.d, y.d);
        DT_EQ("mode7A", x.A, y.A);
        DT_EQ("mode7B", x.B, y.B);
        DT_EQ("mode7C", x.C, y.C);
        DT_EQ("mode7D", x.D, y.D);
        DT_EQ("m7starty", x.starty, y.starty);
        DT_EQ("mode7set", x.set, y.set);
        DT_MEM("mode7ab", x.ab, y.ab, sizeof x.ab);
        DT_MEM("mode7cd", x.cd, y.cd, sizeof x.cd);
        DT_MEM("BG1SXl", x.sx, y.sx, sizeof x.sx);
        DT_MEM("BG1SYl", x.sy, y.sy, sizeof x.sy);
        DT_MEM("BGMA", x.ma, y.ma, sizeof x.ma);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("curvidoffset", x.vid, y.vid);
        DT_EQ("M7HROn", x.hron, y.hron);
        DT_EQ("renderer calls", x.dhits, y.dhits);
        DT_MEM("renderer registers", x.dregs, y.dregs, sizeof x.dregs);
        DT_EQ("renderer curvidoffset", x.dvid, y.dvid);
        DT_EQ("renderer M7HROn", x.dhron, y.dhron);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ kind=%d bx=%x eax=%x edx=%x set=%x\n", kind, b, a, d, set);
        }
    }
    DT_DONE("mode 7 scanline setup");
}
