/*
 * The six offset-mode 8x8 colour-maths tile drawers: video/c_mv16t8t.c's
 * family with offset-per-tile scrolling. Modes 2 and 4 give every tile column
 * its own offset out of the BG3 map, so instead of stepping the map pointer by
 * two and wrapping at column 0x20, each iteration asks video/c_mv16toffs.h
 * where the next tile is. There is no column counter - dl keeps whatever
 * `temp` was seeded with for the whole line.
 *
 * Entered with al = the starting column, ah = the palette shifter, ebx = the
 * tile cache pointer, ecx = the y adder, edx = the map pointer to stash,
 * esi = the horizontal offset, edi = the map pointer and ebp = the layer.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16toffs.h"
#include "c_mv16tt.h"
#include "makevid.h"

zreg TOAX;
zreg TOBX;
zreg TOCX;
zreg TODX;
zreg TOSI;
zreg TODI;
zreg TOBP;
zreg TOTail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b, scaddtype, coadder16;
extern u2 scrnon;
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* One line of 33 tiles. */
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
            offs_cachechk(eax);
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
        /* The offset walk replaces both the map step and the column wrap. */
        edi = (u2 const*)offs_proc();
        esi += 16;
        ebp += 16;
        win += 8;
    } while (--tileleft16b != 0);
    /* Every one of the six clears eax before its tail. */
    r->ax = 0;
    r->cx = ecx;
    r->dx = winon_ ? (zreg)(uintptr_t)win : edx;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
    r->bp = (zreg)(uintptr_t)ebp;
}

void c_draw8x816toffset(void)
{
    tt_regs r = { TOAX, TOBX, TOCX, TODX, TOSI, TODI, TOBP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;
    int mode, win;

    TOTail = 0;
    temp = (u1)r.ax;
    bshifter = (u1)(r.ax >> 8);
    r.ax = hofs;
    yadder = r.cx;
    tempcach = (u1*)(uintptr_t)r.bx;
    yrevadder = 56u - r.cx;
    r.bx = 56u - r.cx;
    /* Reads yadder and yrevadder, so it has to follow them; preserves every
       register, so it needs no seam of its own. */
    offs_init(r.bp, (u1 const*)(uintptr_t)r.di);

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
        TOTail = curmosaicsz != 1;
    }
    TOAX = r.ax;
    TOBX = r.bx;
    TOCX = r.cx;
    TODX = r.dx;
    TOSI = r.si;
    TODI = r.di;
    TOBP = r.bp;
}
