/*
 * The per-scanline dispatch gates of video/makev16t.asm: eight procmode716t*
 * entry points and four procsprites*16t ones.
 *
 * Each decides whether its Mode 7 layer draws, builds the window mask, snaps y
 * to the mosaic grid and hands the scroll values to one of six drawmode716*
 * renderers. They differ only in the enable test, whether the vertical flip
 * and m7starty handoff apply, and which renderer they end on, so one body
 * covers all eight and the caller does the tail-jump.
 *
 * The renderers take their registers through the M7T* seam: ax = bg1scroly_m7,
 * dx = bg1scrolx_m7, ebx as the mosaic divide left it, ebp = 0.
 */
#include <stdint.h>

#include "../cpu/regs.h" /* Layer, for makewindow */
#include "../types.h"
#include "c_makevid.h"
#include "makevid.h"

zreg M7TAX;
zreg M7TBX;
zreg M7TDX;
zreg M7TBP;

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
 * procsprites{sub,main}16t and their *fix twins: enable tests, then the sprite
 * line buffer's count byte decides whether anything draws. The non-fix pair
 * redirect to the fix one when bgfixer says so, which stays in the caller.
 *
 * drawsprites16b is called from here; the other two come back through SPRTail,
 * the shape the assembly's tail-jump had. ecx is undefined after any of them,
 * so the count byte is what gets handed back. */
extern u1 scrndis, winonsp, sprprifix; /* video/makevid.h, cpu/regs.h */
void drawsprites16b(u1 cl, u4 ebp); /* video/c_makev16b.h */

zreg SPRAX;
zreg SPRBX;
zreg SPRCX;
zreg SPRBP;
zreg SPRDX;

/* 0 = nothing more to do, 1 = drawsprites16t, 2 = drawsprites16bt. */
zreg SPRTail;

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
    SPRBX = (u1)curypos + (zreg)(uintptr_t)cursprloc;
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

/* --- the background gates ------------------------------------------------- *
 *
 * drawbackgrnd{sub,main}16t and their *fix twins, one per layer (ebp):
 * colour-mode check, screen enables, window, mosaic, then the tile renderer
 * for this layer's size and colour-maths mode.
 *
 * draw8x816b and draw16x1616b are called from here; the other six come back
 * through BGTail. The `drawn == 33` fully-drawn check happens after the
 * renderer returns.
 *
 * Every early return leaves the registers the assembly set on the way: esi is
 * colormodeofs, bl the colour-mode byte, al curbgnum or - once the window is
 * built - winbg1en[ebp]. */
/* alreadydrawn, curbgnum, curbgpr, drawn, bgcoloradder and the bg1*loc
   tables come from video/makevid.h; bgmode/bgtilesz/winen from cpu/regs.h.
   `winbg1en+ebp` in the assembly is winen[ebp] - they are the same address,
   the per-layer window enables. */
extern u1* colormodeofs; /* c_vcache.h */
void draw8x816b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 layer, u4 esi,
    u2 const* edi);
void draw16x1616b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 esi, u2 const* edi);

zreg BGAX;
zreg BGBX;
zreg BGCX;
zreg BGDX;
zreg BGSI;
zreg BGDI;
zreg BGBP;

/* 0 = done; otherwise the assembly renderer the caller must call, after which
   it does the drawn==33 bookkeeping. */
enum bgtail { B_NONE = 0,
    B_8T,
    B_16T,
    B_8BT,
    B_16BT,
    B_8TMS,
    B_16TMS };
zreg BGTail;

static void mark_drawn(void)
{
    if (drawn == 33) {
        BGAX = (BGAX & ~0xFFu) | curbgnum;
        alreadydrawn = (u1)(alreadydrawn | curbgnum);
    }
}

static void bg(int const sub, int const fix)
{
    u4 const ebp = BGBP;
    u1 const bg = curbgnum;
    u1 sz;

    BGTail = B_NONE;

    /* mov esi,[colormodeofs] / mov bl,[esi+ebp] - both survive every return
       below. */
    BGSI = (zreg)(uintptr_t)colormodeofs;
    BGBX = (BGBX & ~0xFFu) | colormodeofs[ebp];
    if ((u1)BGBX == 0) {
        return;
    }
    BGAX = (BGAX & ~0xFFu) | bg;
    if (sub) {
        if (!((scrnon >> 8) & bg)) {
            return;
        }
        /* The fix form drops the "and not on the main screen" exclusion. */
        if (!fix && (scrnon & bg)) {
            return;
        }
    } else if (!(scrnon & bg)) {
        return;
    }
    /* drawbackgrndmain16tfix still tests alreadydrawn but its branch is
       commented out in the assembly, so the layer is drawn again. */
    if ((alreadydrawn & bg) && !(!sub && fix)) {
        return;
    }
    if (scrndis & bg) {
        return;
    }

    winon = 0;
    if ((sub ? winenabs : winenabm) & bg) {
        u1 const al = winen[ebp];

        BGAX = (BGAX & ~0xFFu) | al;
        makewindow(al, ebp);
        if (winon == 0xFFu) {
            return;
        }
    }

    /* Everything below writes bl only to have it overwritten by bg1cachloc a
       few lines down - kept because it is what the assembly does, but nothing
       can observe it, which is why two mutants here are unkillable. */
    BGBX = (BGBX & ~0xFFu) | bg;
    curmosaicsz = 1;
    if (mosaicon & bg) {
        sz = mosaicsz;
        BGBX = (BGBX & ~0xFFu) | sz;
        if (sz != 0) {
            curmosaicsz = (u1)(sz + 1u);
            BGBX = (BGBX & ~0xFFu) | (u1)(sz + 1u);
        }
    }

    /* Mode 0 gives each layer its own 32-entry palette slice. The two forms
       compute it differently - an 8-bit `mul bl` against a 32-bit `shl` - but
       only the low byte is kept, so they agree. */
    bgcoloradder = 0;
    if (bgmode == 0) {
        bgcoloradder = (u1)(ebp * 32u);
        if (sub) {
            BGBX = (BGBX & ~0xFFu) | 0x20u; /* mov bl,20h, left behind */
        }
    }

    BGSI = bg1vbufloc[ebp];
    BGDI = (zreg)(uintptr_t)bg1tdatloc[ebp];
    BGDX = (zreg)(uintptr_t)bg1tdabloc[ebp];
    BGBX = (zreg)(uintptr_t)bg1cachloc[ebp];
    BGAX = bg1xposloc[ebp];
    BGCX = (BGCX & ~0xFFu) | bg;

    if (!sub) {
        /* Main screen: colour maths can send this to a different writer, and
           a layer that is on both screens goes to the *bt pair - which the
           fix form does not have. */
        if (!fix && (scaddset & 0x02u) && ((scrnon >> 8) & bg)) {
            if (!(curbgpr & 0x20u) && (scaddtype & bg)) {
                BGCX = bg1yaddval[ebp];
                BGTail = (bgtilesz & bg) ? B_16TMS : B_8TMS;
                return;
            }
            BGCX = bg1yaddval[ebp];
            BGTail = (bgtilesz & bg) ? B_16BT : B_8BT;
            return;
        }
        if (scaddtype & bg) {
            BGCX = bg1yaddval[ebp];
            BGTail = (bgtilesz & bg) ? B_16T : B_8T;
            return;
        }
    }

    BGCX = bg1yaddval[ebp];
    if (bgtilesz & bg) {
        draw16x1616b(BGAX, BGCX, (u2*)(uintptr_t)BGDX, (u1*)(uintptr_t)BGBX,
            BGSI, (u2 const*)(uintptr_t)BGDI);
    } else {
        draw8x816b(BGAX, BGCX, (u2*)(uintptr_t)BGDX, (u1*)(uintptr_t)BGBX, ebp,
            BGSI, (u2 const*)(uintptr_t)BGDI);
    }
    mark_drawn();
}

void c_drawbackgrndsub16t(void) { bg(1, 0); }
void c_drawbackgrndmain16t(void) { bg(0, 0); }
void c_drawbackgrndsub16tfix(void) { bg(1, 1); }
void c_drawbackgrndmain16tfix(void) { bg(0, 1); }

/* Called by the thunk after an assembly renderer returns. */
void c_bg_mark_drawn(void) { mark_drawn(); }
