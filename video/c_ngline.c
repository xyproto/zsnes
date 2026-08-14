/*
 * video/c_ngline.c - the line-state half of newengine16b in
 * video/newgfx16.asm.
 *
 * Once per scanline the new graphics engine records everything the renderer
 * will need for that line - scroll positions, tile and map pointers, the mode,
 * the mosaic and interlace settings, the palette - into a set of 256-entry
 * tables, and sets a change flag whenever a value differs from the line above.
 * StartDrawNewGfx16b reads those flags to decide whether a run of eight lines
 * can be drawn as whole tiles.
 *
 * This is the first two thirds of newengine16b, up to the windowing section;
 * the window and sprite-window construction that follows is still assembly and
 * runs straight after this returns. Nothing here is passed in registers, so it
 * needs no seam.
 *
 * Two things to keep in mind reading it: several of these tables are written
 * *wider* than one entry - a dword store into a byte array clobbers the next
 * three lines, which the next line then rewrites - and the comparison against
 * the previous line is often narrower than the store.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"

extern u2 curypos;
extern u1 bgwinchange[256];
extern u1 bgallchange[256], bg1change[256], bg2change[256], bg3change[256];
extern u1 bg4change[256];
extern u4 palchanged, startlinet, endlinet, scfbl, bgcmsung;
extern u1 bg3highst, BG3PRI[256], BGMA[256], BGFB[256], modeused[8];
extern u1 FillSubScr[256], clinemainsub, ngmsdraw, ngextbg;
extern u1 scaddtype, scaddset, scadtng[256], scadsng[256];
extern u1 bgmode, forceblnk, interlval, intrlng[256];
extern u1 mosaicon, mosaicsz, mosenng[256], mosszng[256];
extern u1 BGMS1[], mode7st[256], t16x161[256], t16x162[256];
extern u1 t16x163[256], t16x164[256];
extern u1 BG116x16t, BG216x16t, BG316x16t, BG416x16t;
extern u2 scrnon, cgram[];
extern u2 bg1scrolx[4], bg1scroly[4], bg1objptr[4], bg1ptr[4];
extern u4 bg1ptrx[4], bg1ptry[4];
extern u2 BG1SXl[256], BG2SXl[256], BG3SXl[256], BG4SXl[256];
extern u2 BG1SYl[256], BG2SYl[256], BG3SYl[256], BG4SYl[256];
extern u2 BGOPT1[256], BGOPT2[256], BGOPT3[256], BGOPT4[256];
extern u2 BGPT1[256], BGPT2[256], BGPT3[256], BGPT4[256];
extern u2 BGPT1X[256], BGPT2X[256], BGPT3X[256], BGPT4X[256];
extern u2 BGPT1Y[256], BGPT2Y[256], BGPT3Y[256], BGPT4Y[256];
extern u4 mode7A, mode7C, mode7X0;
extern u1 mode7set;
extern u4 mode7ab[256], mode7cd[256], mode7xy[256];
extern u4 cpalptrng, cpalval[256];
extern u1* vbufdptr;
extern u1 vidmemch2s[];
extern void setpalette16bng(void);
extern u1 winbg1en[6], winenabm, winenabs, disableeffects;
extern u1 winbg1enval[], winbg1envalm[], winbg1envals[];
extern u1 winbg2enval[], winbg3enval[], winbg4enval[];
extern u1 winlogica, winl1;
extern u2 winlogicaval[256];
extern u4 winboundary[256];

/* Store a word and flag the layer if the line above held something else. */
static void recw(u2* const tab, u4 const y, u2 const v, u1* const chg)
{
    tab[y] = v;
    if (tab[y - 1] != v) {
        chg[y] = 1;
    }
}

/* The pointer tables are written a dword at a time but compared as a word, so
   the store spills into the next line's entry and the compare ignores it. */
static void recd(u2* const tab, u4 const y, u4 const v, u1* const chg)
{
    memcpy(&tab[y], &v, 4);
    if (tab[y - 1] != (u2)v) {
        chg[y] = 1;
    }
}

static void recb(u1* const tab, u4 const y, u1 const v, u1* const chg)
{
    tab[y] = v;
    if (tab[y - 1] != v) {
        chg[y] = 1;
    }
}

/* Several of these globals are a byte or a word wide but are *read* as a
   dword, so whatever sits after them in memory rides along into the table. */
static u4 dwr(void const* const p)
{
    u4 v;

    memcpy(&v, p, 4);
    return v;
}

/* A dword store into a byte table: three lines ahead are clobbered and then
   rewritten when their turn comes. */
static void wide(u1* const tab, u4 const y, u4 const v)
{
    memcpy(tab + y, &v, 4);
}

void newengine16b_lines(void);

void newengine16b_lines(void)
{
    u4 const y = curypos & 0xFFu;
    u4 ebx;

    if (y == 1) {
        /* First line of the frame: drop the whole tile cache. */
        for (u4 i = 0; i < 1024u + 512u + 256u; i++) {
            ((u4*)vidmemch2s)[i] = 0xFFFFFFFFu;
        }
        startlinet = 0;
        endlinet = 255;
    }

    bgallchange[y] = 0;
    bgallchange[y + 1] = 0xFF;
    bg1change[y] = 0;
    bg2change[y] = 0;
    bg3change[y] = 0;
    bg4change[y] = 0;
    palchanged = 0;

    recb(BG3PRI, y, bg3highst, bgallchange);

    /* Which screens this line is on. The sub screen counts as filled unless
       colour maths has something to add to it. */
    ebx = dwr(&scrnon);
    {
        u1 const cl = (u1)((((scrnon & 0x1Fu) | 0x20u) & scaddtype) & 0x3Fu);

        FillSubScr[y] = 1;
        if (cl == 0) {
            ebx &= 0xFFFF00FFu;
            FillSubScr[y] = 0;
        }
    }
    if (!(scaddset & 2u)) {
        ebx &= 0xFFFF00FFu;
    }
    bgcmsung |= ebx;
    memcpy(&BGMS1[y * 2], &ebx, 4);
    if (*(u2*)&BGMS1[y * 2 - 2] != (u2)ebx) {
        bgallchange[y] = 1;
    }

    /* If the back area is black and colour maths only adds to it, the two
       screens can be drawn as one. */
    clinemainsub = 0;
    if (cgram[0] == 0 && !(scaddset & 2u)
        && (u1)((((scrnon & 0x1Fu) | 0xE0u) & scaddtype)) == 0x20u) {
        ngmsdraw = 1;
        FillSubScr[y] = 0;
        clinemainsub = 1;
    }

    recw(BG1SXl, y, bg1scrolx[0], bg1change);
    recw(BG2SXl, y, bg1scrolx[1], bg2change);
    recw(BG3SXl, y, bg1scrolx[2], bg3change);
    recw(BG4SXl, y, bg1scrolx[3], bg4change);
    recw(BG1SYl, y, bg1scroly[0], bg1change);
    recw(BG2SYl, y, bg1scroly[1], bg2change);
    recw(BG3SYl, y, bg1scroly[2], bg3change);
    recw(BG4SYl, y, bg1scroly[3], bg4change);

    recb(BGMA, y, (u1)(bgmode & 7u), bgallchange);
    modeused[bgmode & 7u] = 1;

    recd(BGOPT1, y, *(u4*)&bg1objptr[0], bg1change);
    recd(BGOPT2, y, *(u4*)&bg1objptr[1], bg2change);
    recd(BGOPT3, y, *(u4*)&bg1objptr[2], bg3change);
    recd(BGOPT4, y, *(u4*)&bg1objptr[3], bg4change);

    recd(BGPT1, y, *(u4*)&bg1ptr[0], bg1change);
    recd(BGPT2, y, *(u4*)&bg1ptr[1], bg2change);
    recd(BGPT3, y, *(u4*)&bg1ptr[2], bg3change);
    recd(BGPT4, y, *(u4*)&bg1ptr[3], bg4change);

    recd(BGPT1X, y, bg1ptrx[0], bg1change);
    recd(BGPT2X, y, bg1ptrx[1], bg2change);
    recd(BGPT3X, y, bg1ptrx[2], bg3change);
    recd(BGPT4X, y, bg1ptrx[3], bg4change);

    recd(BGPT1Y, y, bg1ptry[0], bg1change);
    recd(BGPT2Y, y, bg1ptry[1], bg2change);
    recd(BGPT3Y, y, bg1ptry[2], bg3change);
    recd(BGPT4Y, y, bg1ptry[3], bg4change);

    ebx = dwr(&forceblnk);
    if ((u1)ebx == 0) {
        scfbl = 0;
    }
    recb(BGFB, y, (u1)ebx, bgallchange);

    if (interlval & 0x40u) {
        bgallchange[y] = 1;
    }

    mode7ab[y] = mode7A;
    mode7cd[y] = mode7C;
    mode7xy[y] = mode7X0;
    wide(mode7st, y, dwr(&mode7set));

    wide(t16x161, y, dwr(&BG116x16t));
    if (t16x161[y - 1] != (u1)BG116x16t) {
        bg1change[y] = 1;
    }
    wide(t16x162, y, dwr(&BG216x16t));
    if (t16x162[y - 1] != (u1)BG216x16t) {
        bg2change[y] = 1;
    }
    wide(t16x163, y, dwr(&BG316x16t));
    if (t16x163[y - 1] != (u1)BG316x16t) {
        bg3change[y] = 1;
    }
    wide(t16x164, y, dwr(&BG416x16t));
    if (t16x164[y - 1] != (u1)BG416x16t) {
        bg4change[y] = 1;
    }

    if (bgmode == 7 && (interlval & 0x40u)) {
        ngextbg = 1;
    }

    wide(mosenng, y, dwr(&mosaicon));
    wide(mosszng, y, dwr(&mosaicsz));
    wide(intrlng, y, dwr(&interlval));

    setpalette16bng();

    /* The first and last line the palette changed on bound the window in which
       a tile row may not be reused. */
    if (palchanged == 1) {
        if (y < 112u) {
            startlinet = y;
        } else if (endlinet >= 255u) {
            endlinet = y;
        }
    }

    cpalval[y] = cpalptrng + (u4)(uintptr_t)vbufdptr;

    recb(scadtng, y, scaddtype, bgallchange);
    recb(scadsng, y, scaddset, bgallchange);
}

/* WinBGCheck: which of the two windows apply to layer N on each screen. The
   assembly's window-bounds half is unreachable - a `jmp %%skip` sits in front
   of it - so this is the whole live macro. */
static void winbg(u4 const y, u4 const n)
{
    u1 bl = winbg1en[n];
    u1 bh = bl;
    u1 const bit = (u1)(1u << n);

    if (!(bl & 0x0Au)) {
        bl = 0;
        bh = 0;
    } else {
        int on = 1;

        if (scrnon & bit) {
            if (!((scrnon >> 8) & bit)) {
                bh = 0;
            }
        } else {
            bl = 0;
            if (!((scrnon >> 8) & bit)) {
                bh = 0;
                on = 0;
            }
        }
        if (on) {
            if (!(winenabs & bit)) {
                bh = 0;
            }
            if (!(winenabm & bit)) {
                bl = 0;
            }
        }
    }
    winbg1envalm[y + n * 256u] = bl;
    winbg1envals[y + n * 256u] = bh;
    winbg1enval[y + n * 256u] = (u1)(bl | bh);
}

/* WinBGCheck2: the same for the back area, where the bounds check *is* live -
   a window covering everything or nothing is dropped. */
static void winback(u4 const y, u4 const n)
{
    u1 bl = winbg1en[n];

    if (!(bl & 0x0Au)) {
        bl = 0;
    } else if ((bl & 0x0Au) != 0x0Au) {
        u4 edx = dwr(&winl1);
        u1 ch = bl;

        if ((bl & 0x0Au) != 0x02u) {
            ch = (u1)(ch >> 2);
            edx >>= 16;
        }
        if (ch & 0x01u) {
            /* Outside: only drop it if it covers the whole line. */
            if ((u1)edx == 0 && (u1)(edx >> 8) == 255u) {
                bl = 0;
            }
        } else if ((u1)edx > (u1)(edx >> 8)) {
            bl = 0;
        }
    }
    winbg1enval[y + n * 256u] = bl;
}

void newengine16b_windows(void);

void newengine16b_windows(void)
{
    u4 const y = curypos & 0xFFu;
    u4 ebx;

    for (u4 n = 0; n < 5; n++) {
        winbg(y, n);
    }
    winback(y, 5);

    ebx = dwr(&winlogica);
    memcpy(&winlogicaval[y], &ebx, 4);
    if (winlogicaval[y - 1] != (u2)ebx) {
        bgwinchange[y] = 1;
    }
    ebx = dwr(&winl1);
    winboundary[y] = ebx;
    if (winboundary[y - 1] != ebx) {
        bgwinchange[y] = 1;
    }
    if (winbg1enval[y - 1] != winbg1enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg2enval[y - 1] != winbg2enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg3enval[y - 1] != winbg3enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg4enval[y - 1] != winbg4enval[y]) {
        bgwinchange[y] = 1;
    }
}
