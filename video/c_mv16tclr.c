/*
 * clearback16t and clearback16ts: the first thing each scanline does, filling
 * the line with the backdrop colour or - when colour maths applies to the back
 * area - blending it with the transparency buffer. Five routes, picked by
 * scaddtype and whether anything is on the sub screen:
 *
 *   no maths          copy the backdrop across, two pixels per store
 *   subtractive       clearback16ts, the fulladd loop with both ends inverted
 *   main + sub on     average each transparency pixel with the backdrop
 *   backdrop is zero  copy the transparency buffer straight over
 *   otherwise         run the pair through fulladdtab
 *
 * The averaging loop reads the buffer a dword at a time for two pixels; the
 * fulladd ones read a dword but step two bytes, so each read overlaps the
 * last. Reproduced, not fixed.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "../vcache.h"

zreg CBAX;
zreg CBBX;
zreg CBCX;
zreg CBDX;
zreg CBSI;
zreg CBDI;
zreg CBBP;

extern u1 scaddtype;
extern u2 scrnon;
extern u4 pal16b[256];
extern u2 fulladdtab[65537]; /* the dword load below reads the last entry */
extern u1 transpbuf[];
extern u1* curvidoffset;

/* The shared tail of clearback16t's .fulladd and the whole of clearback16ts:
   256 pixels through fulladdtab. `eax` arrives already masked and halved -
   .fulladd is jumped into with it in that state - and `sub` inverts the
   result coming out. */
static void fulladd(u4 const eax, int const sub)
{
    u1 const* ebp = transpbuf + 32;
    u1* esi = curvidoffset;

    for (u4 n = 256; n != 0; n--) {
        u4 ebx = *(u4 const*)ebp;

        ebx &= vesa2_clbit;
        ebx >>= 1;
        ebx += eax;
        ebp += 2; /* two bytes, though four were read */
        /* A dword load from a table of words: the neighbouring entry comes in
           as the top half and only the low half is stored. */
        memcpy(&ebx, fulladdtab + ebx, 4);
        if (sub) {
            ebx ^= 0xFFFFu;
        }
        *(u2*)esi = (u2)ebx;
        esi += 2;
        CBBX = ebx;
    }
    CBAX = 0;
    CBCX = 0;
    CBSI = (zreg)(uintptr_t)esi;
    CBBP = (zreg)(uintptr_t)ebp;
}

void c_clearback16ts(void)
{
    u4 eax = pal16b[0];

    eax ^= 0xFFFFu;
    eax &= vesa2_clbit;
    eax >>= 1;
    fulladd(eax, 1);
}

void c_clearback16t(void)
{
    u4 eax;
    u1 const* ebp;
    u1* esi;

    if (!(scaddtype & 0x20u)) {
        /* No maths on the back area: two pixels per dword store. edi walks,
           esi and ebp are never set on this path. */
        u2 const col = (u2)pal16b[0];
        u4* edi = (u4*)curvidoffset;

        for (u4 n = 128; n != 0; n--) {
            *edi++ = ((u4)col << 16) | col;
        }
        CBAX = 0;
        CBCX = 0;
        CBDI = (zreg)(uintptr_t)edi;
        return;
    }
    if (scaddtype & 0x80u) {
        c_clearback16ts();
        return;
    }

    eax = pal16b[0];
    esi = curvidoffset;
    ebp = transpbuf + 32;
    CBDX = (CBDX & ~0xFFFFu) | (u2)eax; /* mov dx,ax */
    eax &= vesa2_clbit;
    eax >>= 1;

    if ((scaddtype & 0x40u) && (scrnon >> 8) != 0) {
        /* Main and sub both on: average, but leave a transparent pixel as the
           backdrop rather than half of it. */
        for (u4 n = 128; n != 0; n--) {
            u4 ebx = *(u4 const*)ebp;

            /* The arms rewrite bx in place. Only the second is observable -
               the shift below throws the first's low half away - so ebx ends
               holding that pixel's result rather than the byte read. Kept on
               both, as the assembly has it. */
            if ((u2)ebx != 0) {
                ebx = (ebx & ~0xFFFFu)
                    | (u2)((u2)(((u2)ebx & (u2)vesa2_clbit) >> 1) + (u2)eax);
                *(u2*)esi = (u2)ebx;
            } else {
                *(u2*)esi = (u2)CBDX;
            }
            ebx >>= 16;
            if ((u2)ebx != 0) {
                ebx = (ebx & ~0xFFFFu)
                    | (u2)((u2)(((u2)ebx & (u2)vesa2_clbit) >> 1) + (u2)eax);
                *(u2*)(esi + 2) = (u2)ebx;
            } else {
                *(u2*)(esi + 2) = (u2)CBDX;
            }
            ebp += 4;
            esi += 4;
            CBBX = ebx;
        }
        CBAX = 0;
        CBCX = 0;
        CBSI = (zreg)(uintptr_t)esi;
        CBBP = (zreg)(uintptr_t)ebp;
        return;
    }

    if (eax == 0) {
        /* A black backdrop adds nothing: copy the transparency buffer over. */
        memcpy(esi, ebp, 512);
        CBAX = 0;
        CBBX = 0;
        CBCX = 0;
        CBSI = (zreg)(uintptr_t)(ebp + 512);
        CBDI = (zreg)(uintptr_t)(esi + 512);
        CBBP = (zreg)(uintptr_t)ebp;
        return;
    }

    fulladd(eax, 0);
}
