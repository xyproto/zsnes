/*
 * video/c_mode716gate.c - the register seams that were left in
 * video/mode716.asm once every body in it had moved to C.
 *
 * Two shapes. The M7Seam pair is a plain spill, call, reload. The M7DRAW pair
 * also carries the mosaic tail: the assembly reached domosaicng16b by a jump,
 * not a call, so it runs on the registers the renderer ended on - eax cleared
 * and dh holding curmosaicsz - and its answer is what the caller gets back.
 */
#include "c_mode716gate.h"

extern u1 curmosaicsz; /* cpu/regs.inc */

/* video/c_mode716draw.c */
extern u4 M7DrawAX, M7DrawDX, M7DrawBX, M7DrawSI, M7DrawDI, M7DrawBP;
extern u4 M7DrawMosaic;
void c_drawmode7win16b(void);
void c_drawmode7ngextbg16b(void);

/* video/c_mode716proc.c: where a renderer leaves its registers. */
extern u4 M7PBX, M7PCX, M7PDX, M7PSI, M7PDI;

/* video/c_mode716calc.c */
extern u4 M7SeamA, M7SeamB, M7SeamC, M7SeamD, M7SeamSI, M7SeamDI, M7SeamBP;
void c_processmode7hires16b(void);

/* video/c_mode716ext2.c */
void c_drawmode7ngextbg216b(void);

/* video/c_ngmosaic.c */
extern u4 MOSAX, MOSBX, MOSCX, MOSDX, MOSSI, MOSDI, MOSBP;
void c_domosaicng16b(void);

static void mosaic_tail(m7regs* const r)
{
    MOSAX = (u4)r->ax;
    MOSBX = (u4)r->bx;
    MOSCX = (u4)r->cx;
    MOSDX = (u4)r->dx;
    MOSSI = (u4)r->si;
    MOSDI = (u4)r->di;
    MOSBP = (u4)r->bp;
    c_domosaicng16b();
    r->ax = MOSAX;
    r->bx = MOSBX;
    r->cx = MOSCX;
    r->dx = MOSDX;
    r->si = MOSSI;
    r->di = MOSDI;
    r->bp = MOSBP;
}

static void m7draw(m7regs* const r, void (*const body)(void))
{
    M7DrawAX = (u4)r->ax;
    M7DrawDX = (u4)r->dx;
    M7DrawBX = (u4)r->bx;
    M7DrawSI = (u4)r->si;
    M7DrawDI = (u4)r->di;
    M7DrawBP = (u4)r->bp;
    body();
    /* ebp is not reloaded; the call preserved it. */
    r->ax = 0;
    r->bx = M7PBX;
    r->cx = M7PCX;
    r->dx = (M7PDX & ~(zreg)0xFF00u) | ((zreg)curmosaicsz << 8);
    r->si = M7PSI;
    r->di = M7PDI;
    if (M7DrawMosaic != 0) {
        mosaic_tail(r);
    }
}

void drawmode7win16b(m7regs* const r)
{
    m7draw(r, c_drawmode7win16b);
}

void drawmode7ngextbg16b(m7regs* const r)
{
    m7draw(r, c_drawmode7ngextbg16b);
}

/* edi is the one register this seam never carried, so it passes through. */
void drawmode7ngextbg216b(m7regs* const r)
{
    M7SeamA = (u4)r->ax;
    M7SeamB = (u4)r->bx;
    M7SeamC = (u4)r->cx;
    M7SeamD = (u4)r->dx;
    M7SeamSI = (u4)r->si;
    M7SeamBP = (u4)r->bp;
    c_drawmode7ngextbg216b();
    r->ax = M7SeamA;
    r->bx = M7SeamB;
    r->cx = M7SeamC;
    r->dx = M7SeamD;
    r->si = M7SeamSI;
    r->bp = M7SeamBP;
}

void processmode7hires16b(m7regs* const r)
{
    M7SeamA = (u4)r->ax;
    M7SeamB = (u4)r->bx;
    M7SeamC = (u4)r->cx;
    M7SeamD = (u4)r->dx;
    M7SeamSI = (u4)r->si;
    M7SeamDI = (u4)r->di;
    M7SeamBP = (u4)r->bp;
    c_processmode7hires16b();
    r->ax = M7SeamA;
    r->bx = M7SeamB;
    r->cx = M7SeamC;
    r->dx = M7SeamD;
    r->si = M7SeamSI;
    r->di = M7SeamDI;
    r->bp = M7SeamBP;
}
