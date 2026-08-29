/*
 * The offset-per-tile helpers of video/vidmacro.mac. Modes 2 and 4 give every
 * tile column its own scroll offset from the BG3 map: initoffsetmode finds
 * where that map starts, procoffsetmode steps one column and returns the map
 * pointer for the next tile, offsetmcachechk faults a tile into the 4-bit
 * cache before it draws.
 *
 * video/c_makev16b.c has its own static copies for the non-transparency
 * drawers; they are *not* interchangeable with these.
 *
 * Most of the arithmetic is deliberately 16-bit on the low half of a 32-bit
 * register - the assembly used ax/bx/dx and the high halves carry through
 * untouched, which a u2 local would drop.
 */
#ifndef C_MV16TOFFS_H
#define C_MV16TOFFS_H

#include <stdint.h>

#include "../types.h"
#include "newgfx.h"

/* Declared here rather than pulled in from vcache.h, which declares
   fulladdtab one entry shorter than the dword load in c_mv16tt.h needs. */
extern u4 OMBGTestVal, ngptrdat2, ofshvaladd, ofsmcptr2, ofsmtptrs;
extern void c_cachesingle4bng(u4 ecx);
extern u1* vram;

extern u2 bg1objptr[4], bg1ptr[4], bg1scrolx[4], bg1scroly[4];
extern u4 bg1ptrx[4], bg1ptry[4];
extern u2 bg3ptr, bg3scrolx, bg3scroly;
extern u2 curypos;
extern u1 bg1ptr_b[10], bg1scrolx_b[10];
extern u2 vidmemch4[2048];
extern u4 yadder, yrevadder;

/* Sets up the whole offset-mode block. Preserves every register the assembly
   is called with, so it needs no seam of its own. */
static void offs_init(u4 const ebp, u1 const* const edi)
{
    u4 eax, ebx, ecx, edx;

    ebx = 0x2000u << (ebp & 31u);
    OMBGTestVal = ebx;
    ecx = bg1scroly[ebp] + ebx;

    edx = bg3scroly;
    if ((u2)edx != 0xFFFFu) {
        edx &= 0x01FFu;
    }
    edx = (edx >> 3) << 6;
    eax = bg3ptr;
    eax = (u2)(eax + edx);
    edx = bg3scrolx & 0xF8u;
    /* `mov ebx,[curypos]` was a dword read of a u2 scanline counter, so it
       carried whatever the linker happened to put after it. Only the low nine
       bits of ofsmcyps are ever looked at - the users add it to a value under
       0x400, test bit 8 and mask to 0xFF - and curypos never exceeds 261, so
       the rest was never observable. Read the counter itself. */
    ebx = curypos;
    ofsmcyps = ebx;
    edx = (edx >> 3) << 1;
    eax = (u2)(eax + edx);
    if (bg3scroly > 0xFFF7u) {
        eax = (u2)(eax + 0x0780u);
    }
    eax += 0x40u;
    ofsmcptr = vram + (eax & 0xFFFFFFC0u);
    ofsmcptr2 = eax & 0x3Fu;
    ofsmady = bg1ptry[ebp];
    ofsmadx = bg1ptrx[ebp];
    /* A dword read of a word array: the next layer's pointer rides in the top
       half and reaches ofsmtptr with it. bg1ptr_b names those bytes plus the
       two that follow the last layer, so the read stays in one object. */
    memcpy(&eax, bg1ptr_b + ebp * 2u, 4);
    ofsmtptr = eax;
    ofsmtptrs = eax;
    if (ecx & 0x0100u) {
        eax += bg1ptry[ebp];
    }
    ecx = (ecx * 8u) & 0x07C0u;
    eax += ecx;
    yposngom = yadder;
    flipyposngom = yrevadder;
    memcpy(&ecx, bg1scrolx_b + ebp * 2u, 4);
    edx = bg1ptrx[ebp];
    if (ecx & 0x0100u) {
        eax += edx;
        ofsmtptr += edx;
        edx = (edx & 0xFFFF0000u) | (u2)(0u - edx); /* neg dx */
    }
    edx = (edx & 0xFFFF0000u) | (u2)(edx - 64u);
    ecx &= 0xF8u;
    eax &= 0xFFFFu;
    ecx >>= 2;
    bgtxadd = edx;
    eax += ecx;
    ofsmtptr += ecx;
    ofsmmptr = eax; /* dead: overwritten from edi below */
    ngptrdat2 = (u4)bg1objptr[ebp] >> 5;
    ofsmmptr = (u4)(uintptr_t)edi - (u4)(uintptr_t)vram;
    ofshvaladd = 0;
}

/* One column along. Returns the map pointer the next tile is read through;
   the assembly leaves it in edi and preserves ebx and edx. */
static u1* offs_proc(void)
{
    u4 eax, edx;
    zreg ebx; /* holds ofsmcptr + an offset: a host address */
    u1* edi;

    ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)(ofsmmptr + 2u);
    ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)(ofsmtptr + 2u);
    ebx = yposngom;
    eax = flipyposngom;
    yadder = ebx;
    yrevadder = eax;
    eax = (eax & 0xFFFF0000u) | (u2)ofsmmptr;
    if ((eax & 0x3Fu) == 0) {
        ebx = (ebx & 0xFFFF0000u) | (u2)bgtxadd;
        eax = (eax & 0xFFFF0000u) | (u2)(eax + ebx);
        ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)(ofsmmptr + ebx);
        ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)(ofsmtptr + ebx);
    }
    edi = vram + eax;

    /* The vertical offset for this column. */
    ebx = (uintptr_t)ofsmcptr + ofsmcptr2;
    eax = OMBGTestVal;
    if (*(u4 const*)(uintptr_t)ebx & eax) {
        ebx = *(u4 const*)(uintptr_t)ebx;
        eax = (eax & 0xFFFF0000u) | (u2)ofsmtptr;
        ebx &= 0x03FFu;
        ebx += ofsmcyps;
        if (ebx & 0x0100u) {
            eax = (eax & 0xFFFF0000u) | (u2)(eax + ofsmady);
        }
        ebx &= 0xFFu;
        edx = (ebx & 0x07u) << 3;
        ebx = (ebx >> 3) << 6;
        eax = (eax & 0xFFFF0000u) | (u2)(eax + ebx);
        yadder = edx;
        yrevadder = edx ^ 0x38u;
        edi = vram + eax;
    }

    /* And the horizontal one, which lives 0x40 bytes earlier in the same map
       and is read before ofsmcptr2 steps on. */
    ebx = (uintptr_t)ofsmcptr + ofsmcptr2;
    ofshvaladd += 8;
    eax = OMBGTestVal;
    ofsmcptr2 = (ofsmcptr2 + 2u) & 0x3Fu;
    if (*(u4 const*)(uintptr_t)(ebx - 0x40u) & eax) {
        eax = (u4)(uintptr_t)edi - (u4)(uintptr_t)vram;
        ebx = *(u4 const*)(uintptr_t)(ebx - 0x40u);
        eax = (eax & 0xFFFF0000u) | (u2)(eax - ofsmtptr);
        eax = (eax & 0xFFFF0000u) | (u2)(eax + ofsmtptrs);
        ebx += ofshvaladd;
        if (ebx & 0x0100u) {
            eax = (eax & 0xFFFF0000u) | (u2)(eax + ofsmadx);
        }
        ebx = (ebx & 0xF8u) >> 2;
        eax = (eax & 0xFFFF0000u) | (u2)(eax + ebx);
        edi = vram + eax;
    }
    return edi;
}

/* Fault the tile into the 4-bit cache if it is not there yet. */
static void offs_cachechk(u4 const eax)
{
    u4 const ecx = (eax + ngptrdat2) & 2047u;

    if (vidmemch4[ecx] != 0) {
        c_cachesingle4bng(ecx);
    }
}

#endif
