/*
 * The register seams left in video/mode716.asm once its bodies moved to C. The
 * M7Seam pair is a plain spill, call, reload; the M7DRAW pair also carries the
 * mosaic tail, which the assembly reached by a jump, so it runs on the
 * registers the renderer ended on - eax cleared, dh holding curmosaicsz - and
 * its answer is the caller's.
 */
#include "c_mode716gate.h"

extern u1 curmosaicsz; /* cpu/regs.inc */

/* video/c_mode716draw.c */
extern zreg M7DrawAX, M7DrawDX, M7DrawBX, M7DrawSI, M7DrawDI, M7DrawBP;
extern u4 M7DrawMosaic;
void c_drawmode7win16b(void);
void c_drawmode7ngextbg16b(void);

/* video/c_mode716proc.c: where a renderer leaves its registers. */
extern zreg M7PBX, M7PCX, M7PDX, M7PSI, M7PDI;

/* video/c_mode716calc.c */
extern zreg M7SeamA, M7SeamB, M7SeamC, M7SeamD, M7SeamSI, M7SeamDI, M7SeamBP;
void c_processmode7hires16b(void);

/* video/c_mode716ext2.c */
void c_drawmode7ngextbg216b(void);

/* video/c_ngmosaic.c */
extern zreg MOSAX, MOSBX, MOSCX, MOSDX, MOSSI, MOSDI, MOSBP;
void c_domosaicng16b(void);

static void mosaic_tail(m7regs* const r)
{
    MOSAX = r->ax;
    MOSBX = r->bx;
    MOSCX = r->cx;
    MOSDX = r->dx;
    MOSSI = r->si;
    MOSDI = r->di;
    MOSBP = r->bp;
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
    M7DrawAX = r->ax;
    M7DrawDX = r->dx;
    M7DrawBX = r->bx;
    M7DrawSI = r->si;
    M7DrawDI = r->di;
    M7DrawBP = r->bp;
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
    M7SeamA = r->ax;
    M7SeamB = r->bx;
    M7SeamC = r->cx;
    M7SeamD = r->dx;
    M7SeamSI = r->si;
    M7SeamBP = r->bp;
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
    M7SeamA = r->ax;
    M7SeamB = r->bx;
    M7SeamC = r->cx;
    M7SeamD = r->dx;
    M7SeamSI = r->si;
    M7SeamDI = r->di;
    M7SeamBP = r->bp;
    c_processmode7hires16b();
    r->ax = M7SeamA;
    r->bx = M7SeamB;
    r->cx = M7SeamC;
    r->dx = M7SeamD;
    r->si = M7SeamSI;
    r->di = M7SeamDI;
    r->bp = M7SeamBP;
}
