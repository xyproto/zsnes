/*
 * CalculateNewValues, from video/mode716.asm. Once per scanline from
 * processmode7hires16b, which reaches it with ebx = the scanline, eax = the Y
 * scroll accumulator and edx = the X one, and takes back eax, ecx and edx.
 * The M7Seam* block below is what the assembly spilled them into.
 */
#include "../types.h"
#include "c_mode716gate.h"

extern u4 mode7ab[256], mode7cd[256]; /* endmem.c: A|B and C|D per scanline */
extern u2 mode7A, mode7B, mode7C, mode7D; /* cpu/regs.inc */
extern u2 BG1SXl[256], BG1SYl[256]; /* endmem.c */
extern u1 BGMA[256]; /* endmem.c: BG mode per scanline */
extern u1 mode7set; /* cpu/regs.inc */
extern u2 m7starty; /* video/c_mode716data.c */

zreg M7SeamA;
zreg M7SeamB;
zreg M7SeamC;
zreg M7SeamD;
/* processmode7hires16b also has esi, edi and ebp live across the renderer it
   calls, and does not restore them. */
zreg M7SeamSI;
zreg M7SeamDI;
zreg M7SeamBP;

/* Predict this scanline's matrix entry. Two words per scanline, so `half`
   picks A or B (C or D), and it reads this scanline's, the next and the one
   after. Both look ahead unchecked, as the assembly does: endmem.c lays the
   tables back to back, so the last scanline reads its neighbour. */
static void m7_newvaluepred(u4 const* const tab, u4 const half, u2* const out,
    u4 const bx)
{
    s2 const* const p = (s2 const*)((u1 const*)tab + bx * 4 + half);
    s4 const v0 = p[0], v1 = p[2], v2 = p[4];

    if ((u2)v2 != (u2)v0 && BGMA[bx + 2] == 7) {
        /* Quadratic step: a 64-bit product over the two-scanline span. The
           one place this is not the assembly - idiv faults when the quotient
           will not fit in 32 bits, which needs |v1 - v0| > 46340 at a span of
           one. No real matrix reaches that, and truncating beats trapping. */
        *out = (u2)((s4)(((s8)(v1 - v0) * (v1 - v0)) / (v2 - v0)) + v0);
    } else {
        /* Arithmetic shift, matching sar. A logical one would agree here - the
           result is truncated to 16 bits and only bit 31 differs - but keep
           the signed shape the assembly has. */
        *out = (u2)((v1 + v0) >> 1);
    }
}

void c_CalculateNewValues(void)
{
    u4 const bx = M7SeamB;
    u4 const ax = M7SeamA;
    u4 start;

    m7_newvaluepred(mode7ab, 0, &mode7A, bx);
    m7_newvaluepred(mode7ab, 2, &mode7B, bx);
    m7_newvaluepred(mode7cd, 0, &mode7C, bx);
    m7_newvaluepred(mode7cd, 2, &mode7D, bx);

    M7SeamD = ((u4)BG1SXl[bx + 1] + M7SeamD) >> 1;

    /* Bit 1 of mode7set flips vertically, so the start line counts down. */
    start = (mode7set & 2) ? 255u - bx : bx + 1u;
    m7starty = (u2)start;
    /* Only ax is replaced, so whatever start left above it carries into the
       add - it is zero for every scanline the renderer walks, but reproduce
       the 32-bit shape rather than assume that. */
    M7SeamA = ((start & 0xFFFF0000u) | BG1SYl[bx + 1]) + ax;
    M7SeamC = ax;
}

/* --- processmode7hires16b ------------------------------------------------ *
 *
 * Re-predict the matrix, aim the renderer at the buffer's second field and run
 * it again with M7HROn set. Only when the *next* scanline is in mode 7. */
extern u1* curvidoffset; /* video/makevid.c */
extern u4 M7HROn; /* video/c_mode716data.c */
/* Hand the renderer the whole register file the seam is carrying; it does not
   restore any of it, and neither did the assembly. (The esi write-back is the
   one part nothing can currently observe, because the only caller puts its own
   esi back afterwards; keep it, so the next caller is not surprised.) */
static void M7CallDraw(void)
{
    m7regs r;

    r.ax = M7SeamA;
    r.bx = M7SeamB;
    r.cx = M7SeamC;
    r.dx = M7SeamD;
    r.si = M7SeamSI;
    r.di = M7SeamDI;
    r.bp = M7SeamBP;
    drawmode7win16b(&r);
    M7SeamA = r.ax;
    M7SeamB = r.bx;
    M7SeamC = r.cx;
    M7SeamD = r.dx;
    M7SeamSI = r.si;
    M7SeamDI = r.di;
    M7SeamBP = r.bp;
}

void c_processmode7hires16b(void)
{
    u4 const bx = M7SeamB;
    /* esi is a pointer into the video buffer, so it has to stay pointer-wide;
       narrowing it to 32 bits aimed the renderer at a truncated address. */
    zreg const si = M7SeamSI;

    if (BGMA[bx + 1] != 7) {
        return;
    }
    c_CalculateNewValues();
    /* The second field starts one full buffer on. */
    M7SeamSI = si + 75036u * 4u;
    curvidoffset = (u1*)(uintptr_t)M7SeamSI;
    M7HROn = 1;
    M7CallDraw();
    M7HROn = 0;
    /* ebx and esi are the caller's scanline and buffer pointer; the assembly
       pushes them around the renderer. Everything else stays as it left it. */
    M7SeamB = bx;
    M7SeamSI = si;
}
