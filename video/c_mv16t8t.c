/*
 * The six 8x8 colour-maths tile drawers of video/makev16t.asm: draw8x816t,
 * draw8x8fulladd, draw8x816ts and their winon twins. One entry point; the rest
 * are jumped to once the shared prologue picks the mode. video/c_mv16t8bt.c
 * produces the transparency buffer, this one consumes it. plain keeps the
 * attribute in dh and the column in dl; winon uses cl, `temp` and coadder16.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16tt.h"
#include "makevid.h"

zreg TTAX;
zreg TTBX;
zreg TTCX;
zreg TTDX;
zreg TTSI;
zreg TTDI;
zreg TTBP;
zreg TTTail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b, scaddtype, coadder16;
extern u2 scrnon;
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* One line of 33 tiles. The half-add form keeps the tile pointer in ebx and
   the other two in edi, pushed around the group - but edi is the map pointer
   again by the time the iteration ends, so only ebx is visible either way. */
static void tiles(tt_regs* const r, u4 const hofs, u1* esi, int const mode,
    int const winon_)
{
    u2 const* edi = (u2 const*)(uintptr_t)r->di;
    u1* ebp = transpbuf + 32 - hofs * 2u;
    u1 const* win = winptrref;
    u4 edx = r->dx;
    u4 ecx = r->cx;
    u4 eax = r->ax;

    tileleft16b = 33;
    drawn = 0;
    /* `mov dl,[temp]` is in the plain loops only, but edx is the window
       pointer in the windowed ones and overwritten below either way. */
    edx = (edx & 0xFFFFFF00u) | temp;
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
            if (mode == T_HALF) {
                r->bx = (zreg)(uintptr_t)tile;
            }
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
            tt_row(r, tile, winon_ ? win : 0, adder, esi, ebp,
                (attr & 0x40u) != 0, mode);
            eax = r->ax;
            ecx = r->cx;
        }
        esi += 16;
        ebp += 16;
        win += 8;
        /* The windowed forms count the column in memory, so `temp` is an
           output there and untouched in the plain ones. */
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

void c_draw8x816t(void)
{
    tt_regs r = { TTAX, TTBX, TTCX, TTDX, TTSI, TTDI, TTBP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;
    int mode, win;

    TTTail = 0;
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

    /* tile value : bit 15 = flipy, bit 14 = flipx, bit 13 = priority value
       bit 10-12 = palette, 0-9 = tile# */
    win = curmosaicsz == 1 && winon != 0;
    /* Half add wants colour maths on, something on the sub screen and not the
       subtractive mode; every other combination is the full add/sub path. */
    if (scaddtype & 0x80u) {
        mode = T_SUB;
    } else if (!(scaddtype & 0x40u) || (scrnon >> 8) == 0) {
        mode = T_ADD;
    } else {
        mode = T_HALF;
    }
    tiles(&r, hofs, esi, mode, win);
    /* Only the plain half-add and full-add forms carry the mosaic tail. */
    if (!win && mode != T_SUB && drawn != 0) {
        r.dx = (r.dx & 0xFFFF00FFu) | (u4)curmosaicsz << 8;
        TTTail = curmosaicsz != 1;
    }
    TTAX = r.ax;
    TTBX = r.bx;
    TTCX = r.cx;
    TTDX = r.dx;
    TTSI = r.si;
    TTDI = r.di;
    TTBP = r.bp;
}
