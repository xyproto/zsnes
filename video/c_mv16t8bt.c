/*
 * draw8x816bt and draw8x816btwinon: the 8x8 tile drawer that produces the
 * transparency buffer; c_makev16b.c's draw8x816b is the same walk without the
 * second store. Entered with al = starting column, ah = palette shifter,
 * ebx = tile cache, ecx = y adder, edx = map pointer to reload at the column
 * wrap, esi = horizontal offset, edi = map pointer.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16tbt.h"
#include "makevid.h"

zreg T8AX;
zreg T8BX;
zreg T8CX;
zreg T8DX;
zreg T8SI;
zreg T8DI;
zreg T8BP;
zreg T8Tail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b;
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* One line of 33 tiles. `win` is the whole difference between the two entry
   points, except that the windowed one has no mosaic tail. */
static void tiles(bt_regs* const r, u4 const hofs, u1* esi, int const winon_)
{
    u2 const* edi = (u2 const*)(uintptr_t)r->di;
    u1* ebp = transpbuf + 32 - hofs * 2u;
    u1 const* win = winptrref;
    u4 edx = r->dx;

    tileleft16b = 33;
    drawn = 0;
    /* Only dl and dh are ever written, so the top half of edx stays whatever
       the caller passed in as the map pointer. */
    edx = (edx & 0xFFFFFF00u) | temp;
    do {
        u4 const ax = *edi++;
        u1 const dh = (u1)((ax >> 8) ^ curbgpr);

        edx = (edx & 0xFFFF00FFu) | (u4)dh << 8;
        /* `mov ax,[edi]` keeps the top half of eax, which is still the
           horizontal offset until the first tile draws. Never observable: the
           offset that fits the pointers it biases fits 16 bits. */
        r->ax = (r->ax & 0xFFFF0000u) | ax;
        if (!(dh & 0x20u)) {
            u4 const t = ax & 0x03FFu; /* filter out tile # */
            u1 const* tile = tempcach + t * 64u;
            u1 pal;

            drawn++;
            if (tile >= bgofwptr) {
                tile -= bgsubby;
            }
            tile += (dh & 0x80u) ? yrevadder : yadder;
            r->bx = (zreg)(uintptr_t)tile;
            if (!winon_) {
                r->cx = (r->cx & 0xFFFFFF00u) | bshifter;
            }
            /* process palette # (bits 10-12). `shl dh,cl` is an 8-bit shift
               by cl & 31, so a shifter of 8 or more leaves nothing. The mask
               could be 0x3C: bit 5 is the priority bit and is known clear
               here. Kept as the assembly writes it. */
            pal = (u1)((((u4)dh & 0x1Cu) << (bshifter & 31u)) + bgcoloradder);
            edx = (edx & 0xFFFF00FFu) | (u4)pal << 8;
            r->ax = 0;
            bt_row(r, tile, winon_ ? win : 0, pal, esi, ebp,
                (dh & 0x40u) != 0, 0);
        }
        esi += 16;
        ebp += 16;
        win += 8;
        edx = (edx & 0xFFFFFF00u) | (u1)(edx + 1);
        if ((u1)edx == 0x20) {
            edi = temptile;
        }
    } while (--tileleft16b != 0);
    r->dx = edx;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
    r->bp = (zreg)(uintptr_t)ebp;
    if (winon_) {
        r->cx = (zreg)(uintptr_t)win;
    }
}

void c_draw8x816bt(void)
{
    bt_regs r = { T8AX, T8BX, T8CX, T8DX, T8SI, T8DI, T8BP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;

    T8Tail = 0;
    temp = (u1)r.ax;
    bshifter = (u1)(r.ax >> 8);
    yadder = r.cx;
    tempcach = (u1*)(uintptr_t)r.bx;
    yrevadder = 56u - r.cx;
    r.ax = hofs;
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
    if (curmosaicsz == 1 && winon != 0) {
        tiles(&r, hofs, esi, 1);
    } else {
        tiles(&r, hofs, esi, 0);
        if (drawn != 0) {
            r.dx = (r.dx & 0xFFFF00FFu) | (u4)curmosaicsz << 8;
            T8Tail = curmosaicsz != 1;
        }
    }
    T8AX = r.ax;
    T8BX = r.bx;
    T8CX = r.cx;
    T8DX = r.dx;
    T8SI = r.si;
    T8DI = r.di;
    T8BP = r.bp;
}
