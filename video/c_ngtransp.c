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
extern u4 UnusedBit[2], HalfTrans[4];

/* The sub screen, in u2 units from the main one. */
#define SUBOFF 75036u

/* .fulltransp / .nextfa - full add, both screens' pixels through the table. */
void c_transp_fulladd(zreg* const r)
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

/* .fullsubtract / .nextfs - the same shape with the main pixel complemented
   going in and the result complemented coming out. */
void c_transp_fullsub(zreg* const r)
{
    u2* p = (u2*)(uintptr_t)r[R_ESI];
    u4 ecx = 256, ebp = HalfTrans[0], edx = 0, eax = 0;
    u4 const ebx = (r[R_EBX] & 0xFFFF0000u) | (u2)UnusedBit[0];

    for (; ecx; ecx--, p++) {
        eax = (eax & 0xFFFF0000u) | *p;
        if (!((u2)eax & (u2)ebx))
            continue;
        edx = (edx & 0xFFFF0000u) | p[SUBOFF];
        eax = (eax & 0xFFFF0000u) | (u2) ~(u2)eax;
        edx &= ebp;
        eax &= ebp;
        edx += eax;
        edx >>= 1;
        edx = (edx & 0xFFFF0000u) | fulladdtab[edx];
        edx = (edx & 0xFFFF0000u) | (u2) ~(u2)edx;
        *p = (u2)edx;
    }

    r[R_EAX] = eax;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EBP] = ebp;
}

/*
 * .subtract / .nextfshs - half subtract. Unlike the two above, this one does
 * NOT clear eax on entry (the assembly only does `xor edx,edx`), so the
 * caller's eax upper half reaches the arithmetic; hence eax comes in from the
 * register block. It also re-reads the sub pixel to decide whether to halve.
 */
void c_transp_halfsub(zreg* const r)
{
    u2* p = (u2*)(uintptr_t)r[R_ESI];
    u4 ecx = 256, ebp = HalfTrans[0], edx = 0, eax = r[R_EAX];
    u4 const ebx = (r[R_EBX] & 0xFFFF0000u) | (u2)UnusedBit[0];

    for (; ecx; ecx--, p++) {
        eax = (eax & 0xFFFF0000u) | *p;
        if (!((u2)eax & (u2)ebx))
            continue;
        edx = (edx & 0xFFFF0000u) | p[SUBOFF];
        eax = (eax & 0xFFFF0000u) | (u2) ~(u2)eax;
        edx &= ebp;
        eax &= ebp;
        edx += eax;
        edx >>= 1;
        edx = (edx & 0xFFFF0000u) | fulladdtab[edx];
        edx = (edx & 0xFFFF0000u) | (u2) ~(u2)edx;
        if (!(p[SUBOFF] & (u2)ebx)) {
            edx &= ebp;
            edx >>= 1;
        }
        *p = (u2)edx;
    }

    r[R_EAX] = eax;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EBP] = ebp;
}

/*
 * .next2 - plain half add. Skips any pixel whose sub half already carries the
 * unused bit. This is the variant that clears eax but leaves edx alone, so the
 * caller's edx upper half survives into the arithmetic and the shr can walk
 * bit 16 down into the stored pixel.
 */
void c_transp_halfadd(zreg* const r)
{
    u2* p = (u2*)(uintptr_t)r[R_ESI];
    u4 ecx = 256, edi = HalfTrans[0], edx = r[R_EDX], eax = 0;
    u4 const ebx = UnusedBit[0];

    for (; ecx; ecx--, p++) {
        eax = (eax & 0xFFFF0000u) | *p;
        if (!((u2)eax & (u2)ebx))
            continue;
        edx = (edx & 0xFFFF0000u) | p[SUBOFF];
        if ((u2)edx & (u2)ebx)
            continue;
        eax &= edi;
        edx &= edi;
        eax += edx;
        eax >>= 1;
        *p = (u2)eax;
    }

    r[R_EAX] = eax;
    r[R_EBX] = ebx;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = edi;
}

/*
 * .next2c - half add where the sub screen is a fixed colour. Same arithmetic,
 * but a sub pixel that already carries the unused bit goes through the
 * full-add table instead of being skipped.
 */
void c_transp_halfaddfix(zreg* const r)
{
    u2* p = (u2*)(uintptr_t)r[R_ESI];
    u4 ecx = 256, edi = HalfTrans[0], edx = 0, eax = 0;
    u4 const ebx = UnusedBit[0];

    for (; ecx; ecx--, p++) {
        int viatable;
        eax = (eax & 0xFFFF0000u) | *p;
        if (!((u2)eax & (u2)ebx))
            continue;
        edx = (edx & 0xFFFF0000u) | p[SUBOFF];
        viatable = ((u2)edx & (u2)ebx) != 0;
        eax &= edi;
        edx &= edi;
        eax += edx;
        eax >>= 1;
        if (viatable)
            eax = (eax & 0xFFFF0000u) | fulladdtab[eax];
        *p = (u2)eax;
    }

    r[R_EAX] = eax;
    r[R_EBX] = ebx;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = edi;
}

/*
 * ProcessTransparencies itself: walk the lines, pick a variant per line, and
 * handle the hi-res second field.
 *
 * Each variant used to be wrapped in push/pop of esi and ebx, so their writes
 * to those two never escaped - hence esi is a local here and ebx is put back
 * after every call (the half-add variants leave UnusedBit in it). Everything
 * else they clobber is meant to escape, which is why they share the caller's
 * register block rather than getting a fresh one.
 */
extern u1* vidbuffer;
extern u1 FillSubScr[], scadtng[], SpecialLine[];
extern u2 resolutn;
extern u4 HiResDone, NGNoTransp;

void c_process_transparencies(zreg* const r)
{
    u2* esi;

    if (NGNoTransp != 0)
        return;

    esi = (u2*)(vidbuffer + 16 * 2 + 288 * 2);
    r[R_EBX] = 1;
    for (;;) {
        u4 const bx = r[R_EBX];

        if (FillSubScr[bx] & 1) {
            HiResDone = 0;
            for (;;) {
                u1 const sc = scadtng[bx];

                r[R_ESI] = (zreg)(uintptr_t)esi;
                if (!(sc & 0x40)) {
                    if (sc & 0x80)
                        c_transp_fullsub(r);
                    else
                        c_transp_fulladd(r);
                } else if (sc & 0x80) {
                    c_transp_halfsub(r);
                } else if (FillSubScr[bx] & 2) {
                    c_transp_halfaddfix(r);
                } else {
                    c_transp_halfadd(r);
                }
                r[R_EBX] = bx; /* the pop that followed each variant */

                if (!(SpecialLine[bx] & 3))
                    break;
                HiResDone ^= 1;
                if (HiResDone == 0) {
                    esi -= SUBOFF * 2;
                    break;
                }
                esi += SUBOFF * 2;
            }
        }

        r[R_EBX] = bx + 1;
        esi += 288;
        if (resolutn < (u2)(bx + 1))
            break;
    }
    r[R_ESI] = (zreg)(uintptr_t)esi;
}
