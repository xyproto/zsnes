/* Colour-maths variants from ProcessTransparencies (video/newgfx16.asm).
 *
 * That routine combines a finished main and sub screen pixel by pixel, in one
 * of five ways chosen by two bits of scadtng. `perf` puts it at about 9% of a
 * run - the hottest code in the emulator - so unlike most of the video files it
 * really is reachable and a pixel A/B can verify it. It is being moved one
 * variant at a time, because a first attempt at all five diverged on Super
 * Metroid and there was no way to tell which variant was at fault.
 *
 * The register-level detail matters here. HalfTrans is 0xF7DEF7DE, so its upper
 * half is set too: `and edx,HalfTrans` does not clear the top of the register,
 * and the `shr` after it can walk bit 16 down into bit 15 of the stored pixel.
 * Hence the pushad seam and u4 everywhere. In this variant both eax and edx are
 * zeroed on entry, so nothing from the caller leaks in - that is not true of
 * every variant.
 */
#include <stdint.h>

#include "../types.h"

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u2 fulladdtab[65536];
extern u4 UnusedBit[2], HalfTrans[2];

/* The sub screen, in u2 units from the main one. */
#define SUBOFF 75036u

/* .fulltransp / .nextfa - full add, both screens' pixels through the table. */
void c_transp_fulladd(u4* const r)
{
    u2* p = (u2*)(uintptr_t)r[R_ESI];
    u4 ecx = 256, ebp = HalfTrans[0], edx = 0, eax = 0;
    u4 const ebx = (r[R_EBX] & 0xFFFF0000u) | (u2)UnusedBit[0];

    for (; ecx; ecx--, p++) {
        eax = (eax & 0xFFFF0000u) | *p;
        if (!((u2)eax & (u2)ebx))
            continue;
        edx = (edx & 0xFFFF0000u) | p[SUBOFF];
        eax &= ebp;
        edx &= ebp;
        edx += eax;
        edx >>= 1;
        edx = (edx & 0xFFFF0000u) | fulladdtab[edx];
        *p = (u2)edx;
    }

    /* esi and ebx are restored by the push/pop the caller still has. */
    r[R_EAX] = eax;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EBP] = ebp;
}
