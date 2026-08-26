/*
 * video/c_mv16t16bt.c - draw16x1616bt and draw16x1616btwinon of
 * video/makev16t.asm.
 *
 * The 16x16 background tile drawer that produces the transparency buffer. It
 * draws the same eight-pixel rows as the 8x8 form (video/c_mv16t8bt.c, whose
 * writers it shares) but a 16x16 tile spans two map columns, so a column
 * toggle decides whether the map pointer or the tile number advances - and the
 * x flip swaps which of the two the toggle picks.
 *
 * Reached with ebx = the tile cache pointer, ecx = the y adder, edx = the map
 * pointer to reload at the column wrap, esi = the horizontal offset and
 * edi = the map pointer. eax is *four* bytes of input: `mov [temp],eax` is a
 * dword store to a byte global, and temp, bshifter, a16x16xinc and a16x16yinc
 * are four consecutive bytes in video/makevid.c - so the caller's top two
 * bytes are the x and y increment flags this routine then reads back.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16tbt.h"
#include "makevid.h"

zreg T16AX;
zreg T16BX;
zreg T16CX;
zreg T16DX;
zreg T16SI;
zreg T16DI;
zreg T16BP;
zreg T16Tail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b;
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u2 yadd, yflipadd;
extern u2 curypos;
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* One line of 33 tile columns. */
static void tiles(bt_regs* const r, u4 const hofs, u1* esi, int const winon_)
{
    u2 const* edi = (u2 const*)(uintptr_t)r->di;
    u1* ebp = transpbuf + 32 - hofs * 2u;
    u1 const* win = winptrref;
    u4 edx = r->dx;
    u4 eax = r->ax;

    tileleft16b = 33;
    edx = (edx & 0xFFFFFF00u) | temp;
    do {
        u1 dh;

        eax = (eax & 0xFFFF0000u) | *edi;
        /* dh is taken before the increment below, so it is the attribute of
           the word as read even when adding 1 carries into it. */
        dh = (u1)(eax >> 8);
        a16x16xinc ^= 1;
        if (!(dh & 0x40u)) {
            /* Not flipped: the left half of the tile advances both. */
            if (!(a16x16xinc & 1u)) {
                eax = (eax & 0xFFFF0000u) | (u2)(eax + 1u);
                edi++;
            }
        } else if (!(a16x16xinc & 1u)) {
            edi++;
        } else {
            eax = (eax & 0xFFFF0000u) | (u2)(eax + 1u);
        }
        dh = (u1)(dh ^ curbgpr);
        edx = (edx & 0xFFFF00FFu) | (u4)dh << 8;
        if (!(dh & 0x20u)) {
            u1 const* tile;
            u1 pal;

            drawn++;
            /* The row within the tile: a flipped tile takes the other half. */
            eax = (eax & 0xFFFF0000u)
                | (u2)(eax + ((dh & 0x80u) ? yflipadd : yadd));
            /* filter out tile #. The mask and the shift are both 16-bit, so
               the top half of eax rides along into the address below. */
            eax = (eax & 0xFFFF0000u) | (eax & 0x03FFu);
            eax = (eax & 0xFFFF0000u) | (u2)(eax << 6);
            tile = tempcach + eax;
            if (tile >= bgofwptr) {
                tile -= bgsubby;
            }
            tile += (dh & 0x80u) ? yrevadder : yadder;
            r->bx = (zreg)(uintptr_t)tile;
            if (!winon_) {
                r->cx = (r->cx & 0xFFFFFF00u) | bshifter;
            }
            /* process palette # (bits 10-12) */
            pal = (u1)((((u4)dh & 0x1Cu) << (bshifter & 31u)) + bgcoloradder);
            edx = (edx & 0xFFFF00FFu) | (u4)pal << 8;
            r->ax = 0;
            bt_row(r, tile, winon_ ? win : 0, pal, esi, ebp,
                (dh & 0x40u) != 0, 1);
            eax = r->ax;
        }
        esi += 16;
        ebp += 16;
        win += 8;
        /* The column counter follows the same toggle, and here it is reset at
           the wrap rather than left to run past it as the 8x8 form does. */
        if (!(a16x16xinc & 1u)) {
            edx = (edx & 0xFFFFFF00u) | (u1)(edx + 1);
        }
        if ((u1)edx == 0x20) {
            edx &= 0xFFFFFF00u;
            edi = temptile;
        }
    } while (--tileleft16b != 0);
    r->ax = eax;
    r->dx = edx;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
    r->bp = (zreg)(uintptr_t)ebp;
    if (winon_) {
        r->cx = (zreg)(uintptr_t)win;
    }
}

void c_draw16x1616bt(void)
{
    bt_regs r = { T16AX, T16BX, T16CX, T16DX, T16SI, T16DI, T16BP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;

    T16Tail = 0;
    drawn = 0;
    /* One dword store over four adjacent byte globals. */
    temp = (u1)r.ax;
    bshifter = (u1)(r.ax >> 8);
    a16x16xinc = (u1)(r.ax >> 16);
    a16x16yinc = (u1)(r.ax >> 24);
    r.ax = hofs;
    yadder = r.cx;
    tempcach = (u1*)(uintptr_t)r.bx;
    yrevadder = 56u - r.cx;
    r.bx = curypos & 0xFFu;
    temptile = (u2*)(uintptr_t)r.dx;

    /* Which half of a vertically flipped tile a row comes from. */
    if (a16x16yinc & 1u) {
        yadd = 16;
        yflipadd = 0;
    } else {
        yadd = 0;
        yflipadd = 16;
    }

    winptrref = cwinptr - hofs;
    esi = curvidoffset - hofs * 2u;
    if (curmosaicsz != 1) {
        /* Mosaic draws into the scratch line, cleared a dword at a time - so
           ecx comes out zero, which the caller sees. */
        memset(xtravbuf + 32, 0, 512);
        r.cx = 0;
        esi = xtravbuf + 32 - hofs * 2u;
    }

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

    if (curmosaicsz == 1 && winon != 0) {
        tiles(&r, hofs, esi, 1);
    } else {
        tiles(&r, hofs, esi, 0);
        /* Unlike the 8x8 form this does not test drawn first. */
        r.dx = (r.dx & 0xFFFF00FFu) | (u4)curmosaicsz << 8;
        T16Tail = curmosaicsz != 1;
    }
    T16AX = r.ax;
    T16BX = r.bx;
    T16CX = r.cx;
    T16DX = r.dx;
    T16SI = r.si;
    T16DI = r.di;
    T16BP = r.bp;
}
