/*
 * The eight mode 7 scanline gates, from video/makev16t.asm. Each spilled eax,
 * ebx, edx and ebp into the M7T seam, picked a renderer, reloaded them and
 * tail-jumped in, so the renderer's `ret` reached the gate's own caller. ecx,
 * esi and edi survive across a gate.
 *
 * Here the choice comes back as a tail id and video/c_mv16tline.c dispatches
 * on it, which keeps the register hand-off out of this file.
 */

#include "c_m716gate.h"

extern zreg M7TAX, M7TBX, M7TDX, M7TBP;
extern u4 M7TTail;
extern zreg SPRAX, SPRBX, SPRCX, SPRBP, SPRDX, SPRTail;
extern zreg BGAX, BGBX, BGCX, BGDX, BGSI, BGDI, BGBP, BGTail;
extern zreg SPTAX, SPTBX, SPTCX, SPTDX, SPTSI, SPTDI, SPTBP;
extern zreg SPBAX, SPBBX, SPBCX, SPBDX, SPBSI, SPBDI, SPBBP;
extern zreg SPPAX, SPPBX, SPPCX, SPPDX, SPPSI, SPPDI, SPPBP;
extern u1 sprprifix;
extern zreg TXAX, TXBX, TXCX, TXDX, TXSI, TXDI, TXBP, TXTail;
extern zreg T16AX, T16BX, T16CX, T16DX, T16SI, T16DI, T16BP, T16Tail;
extern u1 bgfixer;

void c_procmode716tsub(void);
void c_procmode716tsubextbg(void);
void c_procmode716tsubextbgb(void);
void c_procmode716tsubextbg2(void);
void c_procmode716tmain(void);
void c_procmode716tmainextbg(void);
void c_procmode716tmainextbgb(void);
void c_procmode716tmainextbg2(void);
void c_procspritessub16t(void);
void c_procspritesmain16t(void);
void c_procspritessub16tfix(void);
void c_procspritesmain16tfix(void);
void c_drawbackgrndsub16t(void);
void c_drawbackgrndmain16t(void);
void c_drawbackgrndsub16tfix(void);
void c_drawbackgrndmain16tfix(void);
void c_bg_mark_drawn(void);
void c_drawsprites16t(void);
void c_drawsprites16bt(void);
void c_drawsprites16tprio(void);
void c_draw16x1616t(void);
void c_draw16x1616bt(void);

/* ecx, esi and edi are absent on purpose: the gate leaves the caller's own. */
static u4 gate(void (*const body)(void), m7regs* const r)
{
    M7TAX = r->ax;
    M7TBX = r->bx;
    M7TDX = r->dx;
    M7TBP = r->bp;

    body();

    r->ax = M7TAX;
    r->bx = M7TBX;
    r->dx = M7TDX;
    r->bp = M7TBP;
    return M7TTail;
}

#define GATE(name)                \
    u4 name(m7regs* const r)      \
    {                             \
        return gate(c_##name, r); \
    }

GATE(procmode716tsub)
GATE(procmode716tsubextbg)
GATE(procmode716tsubextbgb)
GATE(procmode716tsubextbg2)
GATE(procmode716tmain)
GATE(procmode716tmainextbg)
GATE(procmode716tmainextbgb)
GATE(procmode716tmainextbg2)

/* The sprite gates. Same shape, a different seam - ecx is spilled here rather
   than pushed, and esi and edi are the ones left alone - and the *fix twins
   were reached by a branch at the top rather than being called from outside.
   The tail is 0 for none, 1 for drawsprites16t, anything else 16bt. */
static u4 sprite_gate(void (*const body)(void), m7regs* const r)
{
    SPRAX = r->ax;
    SPRBX = r->bx;
    SPRCX = r->cx;
    SPRBP = r->bp;
    SPRDX = r->dx;

    body();

    r->ax = SPRAX;
    r->bx = SPRBX;
    r->cx = SPRCX;
    /* The bodies only read SPRBP, so this reads back what went in - as the
       assembly's `mov ebp,[SPRBP]` did. Kept so it stays right if one starts
       writing it. */
    r->bp = SPRBP;
    r->dx = SPRDX;
    return SPRTail;
}

u4 procspritesmain16t(m7regs* const r)
{
    return sprite_gate(
        bgfixer == 1 ? c_procspritesmain16tfix : c_procspritesmain16t, r);
}

u4 procspritessub16t(m7regs* const r)
{
    return sprite_gate(
        bgfixer == 1 ? c_procspritessub16tfix : c_procspritessub16t, r);
}

/* The *fix twins. The branch above reaches them, so the emulator never calls
   these - but the assembly exported them and test/difftest_m716t.c drives them
   as cases of their own, which is worth keeping. */
u4 procspritesmain16tfix(m7regs* const r)
{
    return sprite_gate(c_procspritesmain16tfix, r);
}

u4 procspritessub16tfix(m7regs* const r)
{
    return sprite_gate(c_procspritessub16tfix, r);
}

/* The background gates *call* their renderer and then do one more thing, so
   they split in two: the caller runs the first half, dispatches on the id, and
   runs the second. The whole register file crosses this seam; the assembly's
   push of ecx and edx around the bookkeeping call is unnecessary here, where
   they are struct fields nothing touches. */
static u4 bg_gate(void (*const body)(void), m7regs* const r)
{
    BGAX = r->ax;
    BGBX = r->bx;
    BGCX = r->cx;
    BGDX = r->dx;
    BGSI = r->si;
    BGDI = r->di;
    BGBP = r->bp;

    body();

    r->ax = BGAX;
    r->bx = BGBX;
    r->cx = BGCX;
    r->dx = BGDX;
    r->si = BGSI;
    r->di = BGDI;
    /* Read back like SPRBP: the bodies only read it, but the assembly reloaded
       it and this stays right if one starts writing it. */
    r->bp = BGBP;
    return BGTail;
}

u4 drawbackgrndmain16t(m7regs* const r)
{
    return bg_gate(
        bgfixer == 1 ? c_drawbackgrndmain16tfix : c_drawbackgrndmain16t, r);
}

u4 drawbackgrndsub16t(m7regs* const r)
{
    return bg_gate(
        bgfixer == 1 ? c_drawbackgrndsub16tfix : c_drawbackgrndsub16t, r);
}

u4 drawbackgrndmain16tfix(m7regs* const r)
{
    return bg_gate(c_drawbackgrndmain16tfix, r);
}

u4 drawbackgrndsub16tfix(m7regs* const r)
{
    return bg_gate(c_drawbackgrndsub16tfix, r);
}

/* The tail of a background gate: run once the renderer, if any, has been. */
void drawbackgrnd_mark(m7regs* const r)
{
    BGAX = r->ax;
    c_bg_mark_drawn();
    r->ax = BGAX;
}

/* The sprite renderers. Each spilled the whole register file into a seam of
   its own, called its body and reloaded - no tail, so nothing comes back but
   the registers. drawsprites16t branches to the prio form up front, the way
   the assembly did. */
#define SPRITE_DRAW(name, P, body) \
    void name(m7regs* const r)     \
    {                              \
        P##AX = r->ax;             \
        P##BX = r->bx;             \
        P##CX = r->cx;             \
        P##DX = r->dx;             \
        P##SI = r->si;             \
        P##DI = r->di;             \
        P##BP = r->bp;             \
                                   \
        body();                    \
                                   \
        r->ax = P##AX;             \
        r->bx = P##BX;             \
        r->cx = P##CX;             \
        r->dx = P##DX;             \
        r->si = P##SI;             \
        r->di = P##DI;             \
        r->bp = P##BP;             \
    }

SPRITE_DRAW(drawsprites16bt, SPB, c_drawsprites16bt)
SPRITE_DRAW(drawsprites16tprio, SPP, c_drawsprites16tprio)
SPRITE_DRAW(drawsprites16t_plain, SPT, c_drawsprites16t)

void drawsprites16t(m7regs* const r)
{
    if (sprprifix == 1) {
        drawsprites16tprio(r);
        return;
    }
    drawsprites16t_plain(r);
}

/* Two of the tile renderers a background gate calls: the same spill and
   reload, then the mosaic tail, which was a jump into domosaic16b and so
   returned to the gate's caller. Handed back as an id for that reason. */
#define TILE_DRAW(name, P, body) \
    u4 name(m7regs* const r)     \
    {                            \
        P##AX = r->ax;           \
        P##BX = r->bx;           \
        P##CX = r->cx;           \
        P##DX = r->dx;           \
        P##SI = r->si;           \
        P##DI = r->di;           \
        P##BP = r->bp;           \
                                 \
        body();                  \
                                 \
        r->ax = P##AX;           \
        r->bx = P##BX;           \
        r->cx = P##CX;           \
        r->dx = P##DX;           \
        r->si = P##SI;           \
        r->di = P##DI;           \
        r->bp = P##BP;           \
        return P##Tail;          \
    }

TILE_DRAW(draw16x1616t, TX, c_draw16x1616t)
TILE_DRAW(draw16x1616bt, T16, c_draw16x1616bt)
