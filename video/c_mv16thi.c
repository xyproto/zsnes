/*
 * The twelve 16x8 colour-maths tile drawers of video/makev16t.asm - the hi-res
 * form. A 16x8 tile is 16 pixels wide but only eight land on a 256-pixel line,
 * so each group takes every *second* byte (`res512switch` picks which half)
 * and the eight screen pixels come from two tiles 64 bytes apart.
 *
 * Twelve entry points from two macros, so one body with `mode`, `winon` and
 * `half`. The writers are the shared ones in video/c_mv16tt.h except half add,
 * whose two forms here swap which register carries the colour and which the
 * transparency, so those are written out below.
 *
 * Reached with al = the starting column, ah = the palette shifter, ebx = the
 * tile cache pointer, ecx = the y adder, edx = the map pointer to reload at
 * the column wrap, esi = the horizontal offset and edi = the map pointer.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16tt.h"
#include "makevid.h"

zreg THAX;
zreg THBX;
zreg THCX;
zreg THDX;
zreg THSI;
zreg THDI;
zreg THBP;
zreg THTail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b, scaddtype, coadder16, res512switch;
extern u2 scrnon, curypos;
extern u1 hirestiledat[256];
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* draw8x816ta2 and draw8x816tawinon2. Same average as the 8x8 half-add writer,
   but the plain form keeps the colour in ecx and the transparency in ebx while
   the windowed one has them the other way round, and neither clears eax. */
static void hi_half(tt_regs* const r, u1 const* const tile,
    u1 const* const win, u1 const adder, u1* const esi, u1 const* const ebp,
    u4 const k, u4 const n, u4 const w)
{
    u4 eax = tile[k];
    u4 col, t;

    r->ax = eax;
    if (eax == 0) {
        return;
    }
    if (win != 0 && win[w] != 0) {
        return;
    }
    eax = (u1)(eax + adder);
    r->ax = eax;
    memcpy(&t, ebp + n * 2, 4);
    col = pal16b[eax];
    /* A transparent pixel underneath means there is nothing to average with,
       so the colour goes down unchanged. */
    if ((u2)t != 0) {
        t &= TT_CLBIT;
        col = ((col & TT_CLBIT) + t) >> 1;
    }
    *(u2*)(esi + n * 2) = (u2)col;
    if (win != 0) {
        r->cx = t;
        r->bx = col;
    } else {
        r->bx = t;
        r->cx = col;
    }
}

/* Eight screen pixels out of two tiles. `half` picks which byte of each pair
   the line gets; the flipped form takes the other one and runs the screen
   backwards, but the window mask keeps counting forwards - so a flipped tile
   reads it reversed, exactly as the assembly leaves it. */
static void hi_row(tt_regs* const r, u1 const* const tile,
    u1 const* const win, u1 const adder, u1* const esi, u1 const* const ebp,
    int const flip, int const half, int const mode)
{
    for (u4 i = 0; i < 8; i++) {
        u1 const* const t = tile + (i >= 4 ? 64u : 0u);
        u4 const k = 2u * (i & 3u) + (u4)(flip ? 1 - half : half);
        u4 const n = flip ? 7u - i : i;

        if (mode == T_HALF) {
            hi_half(r, t, win, adder, esi, ebp, k, n, i);
        } else {
            tt_px(r, t, win, adder, esi, ebp, k, n, i, mode);
        }
    }
}

/* One line of 33 tiles. */
static void tiles(tt_regs* const r, u4 const hofs, u1* esi, int const mode,
    int const winon_, int const half)
{
    u2 const* edi = (u2 const*)(uintptr_t)r->di;
    u1* ebp = transpbuf + 32 - hofs * 2u;
    u1 const* win = winptrref;
    u4 edx = r->dx;
    u4 ecx = r->cx;
    u4 eax = r->ax;

    tileleft16b = 33;
    drawn = 0;
    /* The windowed twin macro was copied from the plain one and kept its
       `mov dl,[temp]`, which in the windowed forms lands on the low byte of
       the window pointer edx has just been loaded with - so those forms read
       the mask from (winptrref & ~0xFF) | temp. Reproduced; it is what the
       assembly does. */
    edx = (edx & 0xFFFFFF00u) | temp;
    if (winon_)
        win = (u1 const*)(((uintptr_t)winptrref & ~(uintptr_t)0xFFu) | temp);
    do {
        u1 attr;

        eax = (eax & 0xFFFF0000u) | *edi++;
        attr = (u1)((eax >> 8) ^ curbgpr);
        if (winon_) {
            ecx = (ecx & 0xFFFFFF00u) | attr;
        } else {
            edx = (edx & 0xFFFF00FFu) | (u4)attr << 8;
        }
        if (!(attr & 0x20u)) {
            u1 const* tile;
            u1 adder;

            drawn++;
            eax &= 0x03FFu; /* filter out tile # */
            tile = tempcach + eax * 64u;
            if (tile >= bgofwptr) {
                tile -= bgsubby;
            }
            tile += (attr & 0x80u) ? yrevadder : yadder;
            /* process palette # (bits 10-12) */
            adder = (u1)((u1)((attr & 0x1Cu) << (bshifter & 31u))
                + bgcoloradder);
            if (winon_) {
                coadder16 = adder;
            } else {
                edx = (edx & 0xFFFF00FFu) | (u4)adder << 8;
            }
            r->ax = 0;
            r->cx = 0;
            hi_row(r, tile, winon_ ? win : 0, adder, esi, ebp,
                (attr & 0x40u) != 0, half, mode);
            eax = r->ax;
            ecx = r->cx;
        }
        esi += 16;
        ebp += 16;
        win += 8;
        /* The windowed forms count the column in memory, and neither form
           resets it at the wrap. */
        if (winon_) {
            temp = (u1)(temp + 1);
            if (temp == 0x20) {
                edi = temptile;
            }
        } else {
            edx = (edx & 0xFFFFFF00u) | (u1)(edx + 1);
            if ((u1)edx == 0x20) {
                edi = temptile;
            }
        }
    } while (--tileleft16b != 0);
    r->ax = eax;
    r->cx = ecx;
    r->dx = winon_ ? (zreg)(uintptr_t)win : edx;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
    r->bp = (zreg)(uintptr_t)ebp;
}

void c_draw16x816t(void)
{
    tt_regs r = { THAX, THBX, THCX, THDX, THSI, THDI, THBP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;
    int mode, win;

    THTail = 0;
    /* Mark the scanline as hi-res for the pass that copies it out. */
    hirestiledat[(u1)curypos] = 1;
    temp = (u1)r.ax;
    bshifter = (u1)(r.ax >> 8);
    r.ax = hofs;
    yadder = r.cx;
    tempcach = (u1*)(uintptr_t)r.bx;
    yrevadder = 56u - r.cx;
    r.bx = 56u - r.cx;

    winptrref = cwinptr - hofs;
    esi = curvidoffset - hofs * 2u;
    if (curmosaicsz != 1) {
        /* Mosaic draws into the scratch line, cleared a dword at a time - so
           ecx comes out zero, which the caller sees. */
        memset(xtravbuf + 32, 0, 512);
        r.cx = 0;
        esi = xtravbuf + 32 - hofs * 2u;
    }
    temptile = (u2*)(uintptr_t)r.dx;

    /* Which cache the tile pointer is in decides how far a clipped tile has to
       be pulled back. The tests are unsigned and run largest cache first. */
    bgsubby = 262144u;
    bgofwptr = vcache2b + 262144u;
    if (tempcach >= bgofwptr) {
        bgsubby = 131072u;
        bgofwptr = vcache4b + 131072u;
        if (tempcach >= bgofwptr) {
            bgofwptr = vcache8b + 65536u;
            bgsubby = 65536u;
        }
    }

    /* Half add wants colour maths on, something on the sub screen and not the
       subtractive mode; every other combination is the full add/sub path. */
    if (scaddtype & 0x80u) {
        mode = T_SUB;
    } else if (!(scaddtype & 0x40u) || (scrnon >> 8) == 0) {
        mode = T_ADD;
    } else {
        mode = T_HALF;
    }
    win = curmosaicsz == 1 && winon != 0;
    tiles(&r, hofs, esi, mode, win, res512switch != 0);
    /* Only the plain forms carry the mosaic tail. */
    if (!win && drawn != 0) {
        r.dx = (r.dx & 0xFFFF00FFu) | (u4)curmosaicsz << 8;
        THTail = curmosaicsz != 1;
    }
    THAX = r.ax;
    THBX = r.bx;
    THCX = r.cx;
    THDX = r.dx;
    THSI = r.si;
    THDI = r.di;
    THBP = r.bp;
}
