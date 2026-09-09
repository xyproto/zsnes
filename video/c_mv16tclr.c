/* Transparent scanline backdrop fill. */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "../unaligned.h"
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
extern u2 fulladdtab[65537];
extern u1 transpbuf[];
extern u1* curvidoffset;

static void fulladd(u4 const eax, int const sub)
{
    u1 const* ebp = transpbuf + 32;
    u1* esi = curvidoffset;

    for (u4 n = 256; n != 0; n--) {
        u4 ebx = ld32u(ebp);

        ebx &= vesa2_clbit;
        ebx >>= 1;
        ebx += eax;
        ebp += 2;
        ebx = ld32u(fulladdtab + ebx);
        if (sub) {
            ebx ^= 0xFFFFu;
        }
        st16u(esi, (u2)ebx);
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
        u2 const col = (u2)pal16b[0];
        u1* edi = curvidoffset;

        for (u4 n = 128; n != 0; n--) {
            st32u(edi, ((u4)col << 16) | col);
            edi += 4;
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
        for (u4 n = 128; n != 0; n--) {
            u4 ebx = ld32u(ebp);

            if ((u2)ebx != 0) {
                ebx = (ebx & ~0xFFFFu)
                    | (u2)((u2)(((u2)ebx & (u2)vesa2_clbit) >> 1) + (u2)eax);
                st16u(esi, (u2)ebx);
            } else {
                st16u(esi, (u2)CBDX);
            }
            ebx >>= 16;
            if ((u2)ebx != 0) {
                ebx = (ebx & ~0xFFFFu)
                    | (u2)((u2)(((u2)ebx & (u2)vesa2_clbit) >> 1) + (u2)eax);
                st16u(esi + 2, (u2)ebx);
            } else {
                st16u(esi + 2, (u2)CBDX);
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
