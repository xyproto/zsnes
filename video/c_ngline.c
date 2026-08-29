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
 * the window and sprite-window construction runs straight after this returns.
 * Nothing here is passed in registers, so it needs no seam.
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
extern u1 FillSubScr[256], clinemainsub;
extern u4 ngmsdraw, ngextbg; /* dwords where they are defined */
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
/* Read a dword at a time: the pairs A+B, C+D and X0+Y0 are adjacent words
   and the tables below are named for holding both. */
extern u1 mode7A_dw[4], mode7C_dw[4], mode7X0_dw[4];
extern u1 mode7set;
extern u4 mode7ab[256], mode7cd[256], mode7xy[256];
extern u4 cpalptrng;
extern zreg cpalval[256];
extern u1* vbufdptr;
extern u1 vidmemch2s[];
extern void setpalette16bng(void);
extern u1 winbg1en[6], winenabm, winenabs, disableeffects;
extern u1 winbg1enval[], winbg1envalm[], winbg1envals[];
extern u1 winbg2enval[], winbg3enval[], winbg4enval[];
extern u1 winlogica, winl1, winlogicb;
extern u4 nglogicval; /* a dword where it is defined (video/newgfx.c) */
extern u4 objwlrpos[256], objclineptr[256], ngwinen;
extern u1* ngwinptr;
extern u4 ngwintable[32], CSprWinPtr;
extern u2 objwen[256];
extern void BuildWindow2(u4 y, u4 idx);
extern u2 winlogicaval[256];
extern u4 winboundary[256];

/* The four bytes starting at each of these, under a name that covers all
   four - see cpu/c_regsdata.c. The dword reads below are deliberate: the
   assembly loaded a register from the byte and got its neighbours with it,
   and the wide stores below pass them on. */
extern u1 bg4ptr_dw[4], bg4objptr_dw[4];
extern u1 scrnon_dw[4], forceblnk_dw[4], mode7set_dw[4], BG116x16t_dw[4], BG216x16t_dw[4], BG316x16t_dw[4], BG416x16t_dw[4], mosaicon_dw[4], mosaicsz_dw[4], interlval_dw[4], winl1_dw[4], winlogica_dw[4];

/* Store a word and flag the layer if the line above held something else.

   The "line above" index is signed on purpose: on line 0 the assembly read
   the dword before the table and compared against that, which a u4 y - 1
   only reproduces while the slot is 32 bits wide. */
static void recw(u2* const tab, u4 const y, u2 const v, u1* const chg)
{
    tab[y] = v;
    if (tab[(s4)y - 1] != v) {
        chg[y] = 1;
    }
}

/* The pointer tables are written a dword at a time but compared as a word, so
   the store spills into the next line's entry and the compare ignores it. */
static void recd(u2* const tab, u4 const y, u4 const v, u1* const chg)
{
    memcpy(&tab[y], &v, 4);
    if (tab[(s4)y - 1] != (u2)v) {
        chg[y] = 1;
    }
}

static void recb(u1* const tab, u4 const y, u1 const v, u1* const chg)
{
    tab[y] = v;
    if (tab[(s4)y - 1] != v) {
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
    ebx = dwr(scrnon_dw);
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
    if ((u2)dwr(&BGMS1[(s4)(y * 2u) - 2]) != (u2)ebx) {
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

    recd(BGOPT1, y, dwr(&bg1objptr[0]), bg1change);
    recd(BGOPT2, y, dwr(&bg1objptr[1]), bg2change);
    recd(BGOPT3, y, dwr(&bg1objptr[2]), bg3change);
    recd(BGOPT4, y, dwr(bg4objptr_dw), bg4change);

    recd(BGPT1, y, dwr(&bg1ptr[0]), bg1change);
    recd(BGPT2, y, dwr(&bg1ptr[1]), bg2change);
    recd(BGPT3, y, dwr(&bg1ptr[2]), bg3change);
    recd(BGPT4, y, dwr(bg4ptr_dw), bg4change);

    recd(BGPT1X, y, bg1ptrx[0], bg1change);
    recd(BGPT2X, y, bg1ptrx[1], bg2change);
    recd(BGPT3X, y, bg1ptrx[2], bg3change);
    recd(BGPT4X, y, bg1ptrx[3], bg4change);

    recd(BGPT1Y, y, bg1ptry[0], bg1change);
    recd(BGPT2Y, y, bg1ptry[1], bg2change);
    recd(BGPT3Y, y, bg1ptry[2], bg3change);
    recd(BGPT4Y, y, bg1ptry[3], bg4change);

    ebx = dwr(forceblnk_dw);
    if ((u1)ebx == 0) {
        scfbl = 0;
    }
    recb(BGFB, y, (u1)ebx, bgallchange);

    if (interlval & 0x40u) {
        bgallchange[y] = 1;
    }

    mode7ab[y] = dwr(mode7A_dw);
    mode7cd[y] = dwr(mode7C_dw);
    mode7xy[y] = dwr(mode7X0_dw);
    wide(mode7st, y, dwr(mode7set_dw));

    wide(t16x161, y, dwr(BG116x16t_dw));
    if (t16x161[(s4)y - 1] != (u1)BG116x16t) {
        bg1change[y] = 1;
    }
    wide(t16x162, y, dwr(BG216x16t_dw));
    if (t16x162[(s4)y - 1] != (u1)BG216x16t) {
        bg2change[y] = 1;
    }
    wide(t16x163, y, dwr(BG316x16t_dw));
    if (t16x163[(s4)y - 1] != (u1)BG316x16t) {
        bg3change[y] = 1;
    }
    wide(t16x164, y, dwr(BG416x16t_dw));
    if (t16x164[(s4)y - 1] != (u1)BG416x16t) {
        bg4change[y] = 1;
    }

    if (bgmode == 7 && (interlval & 0x40u)) {
        ngextbg = 1;
    }

    wide(mosenng, y, dwr(mosaicon_dw));
    wide(mosszng, y, dwr(mosaicsz_dw));
    wide(intrlng, y, dwr(interlval_dw));

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

    cpalval[y] = cpalptrng + (zreg)(uintptr_t)vbufdptr;

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
        u4 edx = dwr(winl1_dw);
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

    ebx = dwr(winlogica_dw);
    memcpy(&winlogicaval[y], &ebx, 4);
    if (winlogicaval[(s4)y - 1] != (u2)ebx) {
        bgwinchange[y] = 1;
    }
    ebx = dwr(winl1_dw);
    winboundary[y] = ebx;
    if (winboundary[(s4)y - 1] != ebx) {
        bgwinchange[y] = 1;
    }
    if (winbg1enval[(s4)y - 1] != winbg1enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg2enval[(s4)y - 1] != winbg2enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg3enval[(s4)y - 1] != winbg3enval[y]) {
        bgwinchange[y] = 1;
    }
    if (winbg4enval[(s4)y - 1] != winbg4enval[y]) {
        bgwinchange[y] = 1;
    }
}

/* One alternating run of the sprite window mask.

   The writes are dword-wide and overshoot the end of the run; the correction
   afterwards backs both the pointer and the pixel count up by exactly the
   overshoot, so a run of length L advances ecx by L and consumes L pixels no
   matter how it lands. Both `sub`s are *unsigned* borrows, which is why edx
   goes on being used after it has gone negative - hence the u4 arithmetic and
   the signed cast where it becomes a pointer offset.

   Returns 1 when the 256 pixels are used up, at which point the last dword's
   overshoot is left in the buffer - that is what the assembly leaves too. */
static int win_run(u1** const pecx, u4* const peax, u4 edx, u4 const val)
{
    u1* ecx = *pecx;
    u4 eax = *peax;

    for (;;) {
        memcpy(ecx, &val, 4);
        ecx += 4;
        if (eax < 4u) { /* sub eax,4 / jc .done */
            *pecx = ecx;
            *peax = eax - 4u;
            return 1;
        }
        eax -= 4u;
        if (edx < 4u) { /* sub edx,4 / jnc .swloop */
            edx -= 4u;
            break;
        }
        edx -= 4u;
    }
    eax -= edx;
    ecx += (s4)edx;
    eax--;
    ecx++;
    *pecx = ecx;
    *peax = eax;
    return 0;
}

/* ngwintable holds alternating run lengths - covered, then not - and the mask
   is written from one byte *before* the line's buffer, which the first dword
   store then covers. Not static: test/difftest_sprwin.c compares it against a
   verbatim transcription of the assembly it replaced. */
void ng_build_sprite_window(u1* dest);

void ng_build_sprite_window(u1* const dest)
{
    u4 const* tab = ngwintable;
    u4 eax = 256;
    u1* ecx = dest - 1;

    for (;;) {
        u4 edx = *tab++;

        /* A zero-length run is skipped rather than written. */
        if (edx != 0 && win_run(&ecx, &eax, edx - 1u, 0x00000000u)) {
            return;
        }
        edx = *tab++ - 1u;
        if (win_run(&ecx, &eax, edx, 0x01010101u)) {
            return;
        }
    }
}

void newengine16b_sprwin(void);

void newengine16b_sprwin(void)
{
    u4 const y = curypos & 0xFFu;
    u4 const ebx = dwr(winl1_dw);
    u2 dx;

    if (winbg1enval[y + 4u * 256u] == 0) {
        objwlrpos[y] = 0xFFFFFFFFu;
        return;
    }
    dx = (u2)(winbg1enval[y + 4u * 256u] | (u4)(winlogicb & 3u) << 8);

    /* Nothing to rebuild if this line's window matches the one above, or the
       one already built for this line. */
    if (objwlrpos[(s4)y - 1] != 0xFFFFFFFFu) {
        if (objwlrpos[(s4)y - 1] == ebx && objwen[(s4)y - 1] == dx) {
            objwlrpos[y] = ebx;
            objwen[y] = dx;
            objclineptr[y] = objclineptr[(s4)y - 1];
            if (objclineptr[y] != 0xFFFFFFFFu) {
                return;
            }
            goto disable;
        }
        if (objwlrpos[y] == ebx && objwen[y] == dx
            && objclineptr[y] > CSprWinPtr) {
            /* Already built earlier this frame - point at it again. */
            if (objclineptr[y] == 0xFFFFFFFFu) {
                goto disable;
            }
            CSprWinPtr = objclineptr[y];
            return;
        }
    }

    objwlrpos[y] = ebx;
    objwen[y] = dx;
    nglogicval = (u1)(winlogicb & 3u);
    ngwinen = 0;
    BuildWindow2(y, 4u * 256u + y);
    if (ngwinen == 0) {
        goto disable;
    }
    CSprWinPtr += 260u;
    objclineptr[y] = CSprWinPtr;
    ng_build_sprite_window(ngwinptr + CSprWinPtr);
    return;

disable:
    objclineptr[y] = 0xFFFFFFFFu;
    winbg1enval[y + 4u * 256u] = 0;
    winbg1envals[y + 4u * 256u] = 0;
    winbg1envalm[y + 4u * 256u] = 0;
}

/* --- the rest of newengine16b -------------------------------------------- *
 *
 * What follows the two builders above: the colour-add cache, the back area,
 * the hi-res line duplication and the sprite-priority flag. Nothing here is
 * passed in registers either - the assembly ended `xor ebx,ebx / ret` and its
 * one caller declared every register clobbered - so the whole routine is a
 * plain C function now and video/newgfx16.asm has no entry point for it.
 */
extern u1 coladdr, coladdg, coladdb, vidbright;
extern u4 Prevcoladdr, ColResult;
extern u4 ngrposng, nggposng, ngbposng;
extern u1 winbgobjenval[];
extern u4 BackAreaAdd, BackAreaUnFillCol, BackAreaFillCol, UnusedBit[2];
extern u1 SpecialLine[256], Mode7HiRes16b, scanlines, hiresstuff, res640;
extern u4 sprleftpr[256];
extern u1* vidbuffer;
extern void BackAreaFill(u4 y);

/* One colour component scaled by the brightness and shifted into place. The
   assembly does this with a byte mul and a byte div, so the quotient has to
   fit in al - it does: the component is five bits and the brightness four. */
static u2 coladd_part(u4 const c, u4 const shift)
{
    u2 const q = (u2)((c * vidbright) / 15u);

    return (u2)((u4)q << (shift & 31u)); /* shl ax,cl - sixteen bits wide */
}

/* The fixed colour for this line, rebuilt only when a component or the
   brightness moved. The key is a dword read starting one byte before coladdr
   whose low byte is then replaced by the brightness. */
static void col_result(void)
{
    u4 const key = (u4)vidbright | (u4)coladdr << 8 | (u4)coladdg << 16
        | (u4)coladdb << 24;
    u2 bx;

    if (key == Prevcoladdr)
        return;
    Prevcoladdr = key;

    bx = coladd_part(coladdr, ngrposng);
    bx = (u2)(bx + coladd_part(coladdg, nggposng));
    bx = (u2)(bx + coladd_part(coladdb, ngbposng));
    ColResult = (u4)bx | (u4)bx << 16; /* two word stores, not one dword */
}

/* Which of the two colours the filled and unfilled halves of the back area
   get. `filled` hands both halves the second one. */
static void back_area_cols(u4 const ebx, u4 const edx, u1 const cl)
{
    if (cl & 0x10u) {
        BackAreaUnFillCol = edx;
        BackAreaFillCol = (cl & 0x20u) ? edx : ebx;
    } else {
        BackAreaUnFillCol = ebx;
        BackAreaFillCol = (cl & 0x20u) ? edx : ebx;
    }
}

static void back_area(u4 const y)
{
    u4 ebx, edx;
    u1 cl;

    ngwinen = 0;
    if (winbg1enval[y + 5u * 256u] != 0) {
        nglogicval = (u1)((winlogicb >> 2) & 3u);
        BuildWindow2(y, 5u * 256u + y);
    }
    BackAreaAdd = 0;

    if (clinemainsub == 1) {
        ebx = ColResult | UnusedBit[0];
        edx = UnusedBit[0];
        cl = scaddset;
    } else {
        u2 v;

        /* The back colour is the palette entry the line is pointing at,
           doubled into both halves of the dword. */
        memcpy(&v, vbufdptr + cpalptrng, 2);
        ebx = (u4)v | (u4)v << 16;
        edx = 0;
        cl = (u1)(scaddset >> 2); /* the main screen reads two bits higher */
        if (scaddtype & 0x20u) {
            ebx |= UnusedBit[0];
            edx = UnusedBit[0];
        }
    }
    back_area_cols(ebx, edx, cl);

    if (ngwinen == 0)
        BackAreaFillCol = BackAreaUnFillCol;
    if (forceblnk != 0) {
        BackAreaUnFillCol = 0;
        BackAreaFillCol = 0;
    }
    BackAreaFill(y);

    if (!(FillSubScr[y] & 1u))
        return;

    BackAreaAdd = 75036u * 2u;
    ebx = ColResult;
    edx = UnusedBit[0];
    if (scaddset & 2u)
        ebx |= UnusedBit[0];
    back_area_cols(ebx, edx, scaddset);
    if (ngwinen == 0)
        BackAreaFillCol = BackAreaUnFillCol;
    BackAreaFill(y);
}

/* A hi-res line is drawn once and copied to the second field. */
static void special_line(u4 const y)
{
    u1* base;

    SpecialLine[y] = 0;
    if (scanlines != 0)
        return;
    if (bgmode >= 7 && ((interlval & 0x40u) || Mode7HiRes16b != 1))
        goto interlace;
    if (res640 == 0 || bgmode < 5)
        goto interlace;

    SpecialLine[y] = (u1)(bgmode == 7 ? 3 : 2);
    hiresstuff = 1;
    base = vidbuffer + 16u * 2u + (y << 9) + (y << 6);
    memcpy(base + 75036u * 4u, base, 512u);
    if (FillSubScr[y] & 1u)
        memcpy(base + 75036u * 6u, base + 75036u * 2u, 512u);

interlace:
    if (interlval & 1u)
        SpecialLine[y] |= 4u;
}

void newengine16b(void);

void newengine16b(void)
{
    u4 y, p;

    newengine16b_lines();
    y = curypos & 0xFFu;

    bgwinchange[y] = 0;
    if (disableeffects == 1) {
        u4 q;

        winbg1enval[y] = 0;
        winbg2enval[y] = 0;
        winbg3enval[y] = 0;
        winbg4enval[y] = 0;
        winbgobjenval[y] = 0;
        for (q = 0; q < 5u; q++) {
            winbg1envalm[y + q * 256u] = 0;
            winbg1envals[y + q * 256u] = 0;
        }
    } else {
        newengine16b_windows();
        bgwinchange[y] = 1;
        newengine16b_sprwin();
    }

    col_result();
    /* A black fixed colour leaves the sub screen alone. */
    if ((u2)ColResult != 0 && FillSubScr[y] != 0)
        FillSubScr[y] |= 2u;

    back_area(y);
    special_line(y);

    /* One priority left on this line and nothing else: mark it so the sprite
       pass can take the short route. */
    p = sprleftpr[y];
    if (p == 1u || p == 0x100u || p == 0x10000u || p == 0x1000000u)
        sprleftpr[y] |= 0x80000000u;
}
