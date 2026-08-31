/*
 * The six 16x16 colour-maths tile drawers: the column walk of
 * video/c_mv16t16bt.c with the writers of video/c_mv16tt.h. Entered with
 * ebx = tile cache, ecx = y adder, edx = map pointer to reload at the column
 * wrap, esi = horizontal offset, edi = map pointer. eax is four bytes of
 * input: `mov [temp],eax` stores over temp, bshifter, a16x16xinc, a16x16yinc.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "c_mv16tt.h"
#include "makevid.h"

zreg TXAX;
zreg TXBX;
zreg TXCX;
zreg TXDX;
zreg TXSI;
zreg TXDI;
zreg TXBP;
zreg TXTail; /* 1 = tail-jump to domosaic16b */

extern u1 tileleft16b, scaddtype, coadder16;
extern u2 scrnon;
extern u1 transpbuf[];
extern u1 xtravbuf[576];
extern u2 yadd, yflipadd;
extern u2 curypos;
extern u1 *vcache2b, *vcache4b, *vcache8b;

/* One line of 33 tile columns. */
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
    /* `mov dl,[temp]` is in the plain loops only, but edx is the window
       pointer in the windowed ones and overwritten below either way. */
    edx = (edx & 0xFFFFFF00u) | temp;
    do {
        u1 attr;

        eax = (eax & 0xFFFF0000u) | *edi;
        /* The attribute is taken before the increment below, so it is the one
           the word was read with even when adding 1 carries into it - and the
           x flip is tested before the priority xor. */
        attr = (u1)(eax >> 8);
        a16x16xinc ^= 1;
        if (!(attr & 0x40u)) {
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
        attr = (u1)(attr ^ curbgpr);
        if (winon_) {
            ecx = (ecx & 0xFFFFFF00u) | attr;
        } else {
            edx = (edx & 0xFFFF00FFu) | (u4)attr << 8;
        }
        if (!(attr & 0x20u)) {
            u1 const* tile;
            u1 adder;

            drawn++;
            /* The row within the tile: a flipped tile takes the other half. */
            eax = (eax & 0xFFFF0000u)
                | (u2)(eax + ((attr & 0x80u) ? yflipadd : yadd));
            /* filter out tile #. The mask and the shift are both 16-bit, so
               the top half of eax rides along into the address below. */
            eax = (eax & 0xFFFF0000u) | (eax & 0x03FFu);
            eax = (eax & 0xFFFF0000u) | (u2)(eax << 6);
            tile = tempcach + eax;
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
        /* The column counter follows the same toggle as the map pointer, and
           is reset at the wrap. The windowed forms count it in memory. */
        if (!(a16x16xinc & 1u)) {
            if (winon_) {
                temp = (u1)(temp + 1);
            } else {
                edx = (edx & 0xFFFFFF00u) | (u1)(edx + 1);
            }
        }
        if (winon_) {
            if (temp == 0x20) {
                temp = 0;
                edi = temptile;
            }
        } else if ((u1)edx == 0x20) {
            edx &= 0xFFFFFF00u;
            edi = temptile;
        }
    } while (--tileleft16b != 0);
    r->ax = eax;
    r->cx = ecx;
    r->dx = winon_ ? (zreg)(uintptr_t)win : edx;
    r->si = (zreg)(uintptr_t)esi;
    r->di = (zreg)(uintptr_t)edi;
    r->bp = (zreg)(uintptr_t)ebp;
}

void c_draw16x1616t(void)
{
    tt_regs r = { TXAX, TXBX, TXCX, TXDX, TXSI, TXDI, TXBP };
    u4 const hofs = r.si; /* esi on entry - the horizontal offset in pixels */
    u1* esi;
    int mode, win;

    TXTail = 0;
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
    /* All three plain forms carry the mosaic tail, and none tests drawn. It
       overwrites dh unconditionally, which is what makes every write to edx's
       high byte in the loop above unobservable - they are kept because the
       assembly makes them. */
    if (!win) {
        r.dx = (r.dx & 0xFFFF00FFu) | (u4)curmosaicsz << 8;
        TXTail = curmosaicsz != 1;
    }
    TXAX = r.ax;
    TXBX = r.bx;
    TXCX = r.cx;
    TXDX = r.dx;
    TXSI = r.si;
    TXDI = r.di;
    TXBP = r.bp;
}
