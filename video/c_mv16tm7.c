/*
 * video/c_mv16tm7.c - the per-scanline dispatch gates of video/makev16t.asm:
 * the eight procmode716t* entry points and the four procsprites*16t ones.
 *
 * Per scanline, each decides whether its Mode 7 layer is drawn at all, builds
 * the window mask if one is enabled, snaps the y position to the mosaic grid,
 * and hands the scroll values to one of six drawmode716* renderers. They
 * differ only in the enable test, whether the vertical flip and the m7starty
 * handoff apply, and which renderer they end on - so one body covers all
 * eight, and the caller does the tail-jump.
 *
 * The renderers stay in assembly, so the registers they expect are set up
 * here and handed back through the M7T* seam: ax = bg1scroly_m7,
 * dx = bg1scrolx_m7, ebx as the mosaic divide left it, ebp = 0 from the
 * makewindow call.
 */
#include <stdint.h>

#include "../cpu/regs.h" /* Layer, for makewindow */
#include "../types.h"
#include "c_makevid.h"
#include "makevid.h"

u4 M7TAX;
u4 M7TBX;
u4 M7TDX;
u4 M7TBP;

/* Which renderer the caller must tail-jump to; 0 means the layer is off. */
u4 M7TTail;

/* mosaicon, mosaicsz, scaddset, scaddtype, mode7set, scrnon and
   winenabm/winenabs come from cpu/regs.h; winon, curmosaicsz and extbgdone
   from video/makevid.h. */
extern u1 winbg1en; /* cpu/c_regsdata.c */
extern u2 m7starty; /* video/c_mode716data.c */
extern u2 bg1scrolx_m7, bg1scroly_m7; /* cpu/c_regswdata.c */
extern u2 curypos;

/* scrnon is one word: the low byte is the main screen's layer enables and the
   high byte the sub screen's. The assembly reaches the second through
   `word[scrnon+1]`, a word read one byte in, but only ever masks bit 0 or 1
   of it - so it is the high byte that matters. */
#define MAIN_ON(bit) ((scrnon & (bit)) != 0)
#define SUB_ON(bit) (((scrnon >> 8) & (bit)) != 0)

/* Likewise `word[winenabm]` spans winenabm and winenabs, which are adjacent
   bytes: bit 0 is the main screen's window logic, bit 8 the sub screen's. */
#define WIN_M (winenabm & 0x01u)
#define WIN_S (winenabs & 0x01u)

enum tail { T_NONE = 0,
    T_M716T,
    T_M716B,
    T_M716TB,
    T_M716EXTBG,
    T_M716TEXTBG,
    T_M716EXTBG2,
    T_M716TEXTBG2 };

/* The four ways the layer can be gated. */
enum gate { G_SUB1,
    G_SUB2,
    G_SUB1_FIRST,
    G_MAIN1,
    G_MAIN2,
    G_MAIN1_FIRST,
    G_DONE };

static int enabled(enum gate const g)
{
    switch (g) {
    case G_SUB1:
        return SUB_ON(0x01u) && !MAIN_ON(0x01u);
    case G_SUB2:
        return SUB_ON(0x02u) && !MAIN_ON(0x02u);
    case G_SUB1_FIRST:
        return extbgdone == 0 && SUB_ON(0x01u) && !MAIN_ON(0x01u);
    case G_MAIN1:
        return MAIN_ON(0x01u);
    case G_MAIN2:
        return MAIN_ON(0x02u);
    case G_MAIN1_FIRST:
        return extbgdone == 0 && MAIN_ON(0x01u);
    case G_DONE:
        return extbgdone != 0;
    }
    return 0;
}

/* Returns 0 when makewindow masked the whole line away. */
static int build_window(void)
{
    winon = 0;
    if (WIN_M && !WIN_S) {
        /* `mov al,[winbg1en]` before the call, and ccallv restores eax after
           it - so al stays winbg1en, including on the abandoned-line return. */
        M7TAX = (M7TAX & ~0xFFu) | winbg1en;
        M7TBP = 0; /* mov ebp,0 - only on this path, and it survives the call */
        makewindow(winbg1en, 0);
        if (winon == 0xFFu) {
            return 0;
        }
    }
    return 1;
}

/* dx:ax / bx then * bx, i.e. snap the scanline down to a mosaic row. Returns
   the snapped value; ebx is an output because the assembly leaves it there. */
static u2 mosaic_snap(u2 const y, u4* const ebx)
{
    u1 sz;

    curmosaicsz = 1;
    if (!(mosaicon & 1u)) {
        return y;
    }
    /* `mov bl,[mosaicsz]` lands before the zero test, so bl keeps the size
       even when the test sends it straight past the divide. */
    sz = mosaicsz;
    *ebx = (*ebx & ~0xFFu) | sz;
    if (sz == 0) {
        return y;
    }
    sz = (u1)(sz + 1u);
    curmosaicsz = sz;
    /* bl then bh, so only the low word of ebx is touched. A zero divisor
       would fault the original; mosaicsz never reaches 0xFF. */
    *ebx = (*ebx & ~0xFFFFu) | sz;
    return (u2)((u2)(y / sz) * sz);
}

static void run(enum gate const g, int const flip, int const m7start,
    enum tail const plain, enum tail const transp, int const setdone)
{
    u4 ebx = M7TBX;
    u2 y;

    M7TTail = T_NONE;
    if (!enabled(g)) {
        return;
    }
    if (!build_window()) {
        return;
    }

    /* xor eax,eax / xor edx,edx: the top halves are cleared here, not just
       the words the moves below write. */
    y = curypos;
    if (flip && (mode7set & 0x02u)) {
        y = (u2)(-(int)y + 255);
    }
    y = mosaic_snap(y, &ebx);
    M7TBX = ebx;

    if (!m7start) {
        M7TAX = y;
        M7TDX = 0;
        M7TTail = plain;
        return;
    }
    m7starty = y;
    M7TAX = bg1scroly_m7;
    M7TDX = bg1scrolx_m7;

    if (setdone) {
        extbgdone = 1;
    }
    if (transp == T_NONE) {
        M7TTail = plain;
        return;
    }
    M7TTail = (scaddtype & 0x01u) ? plain : transp;
}

void c_procmode716tsub(void)
{
    run(G_SUB1, 1, 1, T_M716T, T_NONE, 0);
}

void c_procmode716tsubextbg(void)
{
    run(G_SUB2, 1, 1, T_M716EXTBG, T_NONE, 1);
}

void c_procmode716tsubextbgb(void)
{
    run(G_SUB1_FIRST, 1, 1, T_M716TEXTBG, T_NONE, 1);
}

void c_procmode716tsubextbg2(void)
{
    /* No vertical flip and no m7starty handoff: this one runs after the pass
       that already set them. */
    run(G_DONE, 0, 0, T_M716EXTBG2, T_NONE, 0);
}

/* The main-screen forms pick between the transparent and opaque renderers on
   scaddtype, and procmode716tmain has a third route when colour maths is on
   for a sub-screen layer that is also enabled. */
void c_procmode716tmain(void)
{
    run(G_MAIN1, 1, 1, T_M716T, T_M716B, 0);
    if (M7TTail != T_NONE && (scaddset & 0x02u) && SUB_ON(0x01u)) {
        M7TTail = T_M716TB;
    }
}

void c_procmode716tmainextbg(void)
{
    run(G_MAIN2, 1, 1, T_M716TEXTBG, T_M716EXTBG, 1);
}

void c_procmode716tmainextbgb(void)
{
    run(G_MAIN1_FIRST, 1, 1, T_M716TEXTBG, T_M716EXTBG, 1);
}

void c_procmode716tmainextbg2(void)
{
    run(G_DONE, 1, 1, T_M716TEXTBG2, T_M716EXTBG2, 0);
}

/* --- the sprite gates ----------------------------------------------------- *
 *
 * procsprites{sub,main}16t and their *fix twins. Same shape as the Mode 7
 * gates: a few enable tests, then the sprite line buffer's count byte for this
 * scanline decides whether anything is drawn. The non-fix pair start by
 * redirecting to the fix one when bgfixer says so, which stays in the caller.
 *
 * drawsprites16b is already C (video/c_makev16b.c) so it is called from here;
 * drawsprites16t and drawsprites16bt are still assembly and come back through
 * SPRTail. ecx after any of them is undefined - cdecl lets a callee clobber
 * it - so what is handed back is the count byte, the value the assembly had
 * there going in.
 */
extern u1 scrndis, winonsp, sprprifix; /* video/makevid.h, cpu/regs.h */
void drawsprites16b(u1 cl, u4 ebp); /* video/c_makev16b.h */

u4 SPRAX;
u4 SPRBX;
u4 SPRCX;
u4 SPRBP;
u4 SPRDX;

/* 0 = nothing more to do, 1 = drawsprites16t, 2 = drawsprites16bt. */
u4 SPRTail;

static void sprites(int const sub, int const fix)
{
    u1 const* at;
    u1 count;

    SPRTail = 0;
    if (scrndis & 0x10u) {
        return;
    }
    if (sub) {
        if (!SUB_ON(0x10u)) {
            return;
        }
        /* The fix form drops the main-screen test the other one has. */
        if (!fix && MAIN_ON(0x10u)) {
            return;
        }
    } else if (!MAIN_ON(0x10u)) {
        return;
    }
    if (winonsp == 0xFFu) {
        return;
    }

    /* xor ebx,ebx / mov bl,[curypos]: the low byte of the scanline only. */
    SPRBX = (u1)curypos + (u4)(uintptr_t)cursprloc;
    at = (u1 const*)(uintptr_t)SPRBX;
    count = *at;
    SPRCX = (SPRCX & ~0xFFu) | count;
    if (sprprifix == 0) {
        cursprloc += 256;
    }
    if (count == 0) {
        return;
    }

    if (!sub && !fix && SUB_ON(0x10u)) {
        SPRTail = 2;
        return;
    }
    if (sub) {
        drawsprites16b(count, SPRBP);
        return;
    }
    if (scaddtype & 0x10u) {
        SPRTail = 1;
        return;
    }
    drawsprites16b(count, SPRBP);
    SPRAX = 0;
}

void c_procspritessub16t(void) { sprites(1, 0); }
void c_procspritesmain16t(void) { sprites(0, 0); }
void c_procspritessub16tfix(void) { sprites(1, 1); }
void c_procspritesmain16tfix(void) { sprites(0, 1); }
