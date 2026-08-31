/*
 * The shared prologue of video/mv16tms.asm, run by all eleven draw*ms entries:
 * stash the caller's registers, bias the video and window pointers by the
 * horizontal offset, clear any mosaic line, and work out which of the three
 * tile caches the tile pointer lands in. Entered with al = palette byte,
 * ah = shifter, ebx = tile cache, ecx = y adder, edx = tile, esi = h offset.
 */
#include <stdint.h>

#include "../types.h"

zreg MVAX;
zreg MVBX;
zreg MVCX;
zreg MVDX;
zreg MVSI;

extern u1 temp, bshifter; /* video/makevid.c */
extern u4 yadder, yrevadder;
extern u1* tempcach; /* video/makevid.c */
extern u2* temptile;
extern u4 bgsubby;
extern u1* bgofwptr;
extern u1 *cwinptr, *winptrref, *curvidoffset;
extern u1 *vcache2b, *vcache4b, *vcache8b;
extern u1 curmosaicsz;
extern u1 xtravbuf[576];

/* One video buffer's worth of 16-bit pixels, doubled: the assembly subtracts
   the horizontal offset twice, once per byte of each pixel. */
void c_draw16tms_setup(void)
{
    u4 const hofs = MVSI; /* esi on entry - the horizontal offset in pixels */
    u4 ecx = MVCX;
    u1* esi;

    temp = (u1)MVAX;
    bshifter = (u1)(MVAX >> 8);
    yadder = ecx;
    tempcach = (u1*)MVBX;
    yrevadder = 56u - ecx;

    winptrref = cwinptr - hofs;
    esi = curvidoffset - hofs - hofs;

    if (curmosaicsz != 1) {
        /* Mosaic draws into the scratch line, cleared a dword at a time - so
           ecx comes out zero, which the caller sees. */
        for (u4 i = 0; i < 128u; i++) {
            *(u4*)(xtravbuf + 32 + i * 4) = 0;
        }
        ecx = 0;
        esi = xtravbuf + 32 - hofs - hofs;
    }

    temptile = (u2*)MVDX;

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

    MVAX = hofs;
    MVBX = 56u - MVCX;
    MVCX = ecx;
    MVSI = (zreg)(uintptr_t)esi;
}

/* The 16x16 prologue, with three differences: the caller's eax is stored as a
   whole dword, landing on temp, bshifter, a16x16xinc and a16x16yinc at once
   (they are back to back in video/makevid.c) and the y adders are then picked
   on the byte it just wrote; ebx comes out holding curypos, not the reverse
   adder; and the cache walk keeps ecx, so the mosaic clear's zero survives. */
extern u1 a16x16xinc, a16x16yinc, drawn; /* video/makevid.c */
extern u2 curypos; /* u2 where it is defined; the asm reads the low byte */
extern u2 yadd, yflipadd; /* video/c_makev16tdata.c */

void c_draw16x16tms_setup(void)
{
    u4 const hofs = MVSI;
    u4 ecx = MVCX;
    u1* esi;

    drawn = 0;
    temp = (u1)MVAX;
    bshifter = (u1)(MVAX >> 8);
    a16x16xinc = (u1)(MVAX >> 16);
    a16x16yinc = (u1)(MVAX >> 24);
    yadder = ecx;
    tempcach = (u1*)MVBX;
    yrevadder = 56u - ecx;
    temptile = (u2*)MVDX;

    if (a16x16yinc & 1u) {
        yadd = 16;
        yflipadd = 0;
    } else {
        yadd = 0;
        yflipadd = 16;
    }

    winptrref = cwinptr - hofs;
    esi = curvidoffset - hofs - hofs;

    if (curmosaicsz != 1) {
        for (u4 i = 0; i < 128u; i++) {
            *(u4*)(xtravbuf + 32 + i * 4) = 0;
        }
        ecx = 0;
        esi = xtravbuf + 32 - hofs - hofs;
    }

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

    MVAX = hofs;
    MVBX = (u1)curypos;
    MVCX = ecx;
    MVSI = (zreg)(uintptr_t)esi;
}
