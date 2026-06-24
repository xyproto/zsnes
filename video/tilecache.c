// tilecache.c, SNES background tile decoding into the linear pixel cache.
// Ported from the cachetile* routines in vcache.asm.

#include <stdbool.h>

#include "../endmem.h"
#include "../gblvars.h"
#include "../types.h"
#include "../ui.h"
#include "../vcache.h"
#include "makevid.h"

#if defined(__APPLE__) || defined(__MINGW32__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

// Decode one SNES planar 8x8 tile into the linear pixel cache.
// src points at the tile in VRAM, dst at the 64-byte cache slot, bpp = 2, 4 or 8.
static void decode_tile(u1 const* const src, u1* const dst, int const bpp)
{
    for (int row = 0; row != 8; ++row) {
        for (int col = 0; col != 8; ++col) {
            int const b = 7 - col;
            u1 px = 0;
            for (int p = 0; p != bpp; ++p) {
                int const off = (p >> 1) * 16 + (p & 1) + row * 2;
                px |= (u1)(((src[off] >> b) & 1) << p);
            }
            dst[row * 8 + col] = px;
        }
    }
}

// Cache background tiles referenced by a 32-entry tilemap row (two rows when the
// second screen differs).  eax selects the tilemap base, bpp the colour depth,
// tile16 whether the tilemap holds 16x16 (2x2) tiles.  Faithful port of the
// cachetile* routines from vcache.asm.
static void cache_tiles(u4 const eax, int const bpp, bool const tile16)
{
    int const preshift = (bpp == 2) ? 0 : (bpp == 4) ? 1
                                                     : 2;
    u1* const vidmemch = (bpp == 2) ? vidmemch2 : (bpp == 4) ? (u1*)vidmemch4
                                                             : vidmemch8;
    u1* const vcache = (bpp == 2) ? vcache2b : (bpp == 4) ? vcache4b
                                                          : vcache8b;
    u4 const nbg = (u2)(curtileptr >> (4 + preshift));

    u4 const ptrword = (eax & 0x20) ? (bgptrc & 0xFFFF) : (bgptr & 0xFFFF);
    u4 const idx = (eax & 0x20) ? ((eax & 0x1F) << 6) : (eax << 6);
    u4 addr = (idx + ptrword) & 0xFFFF;

    static u4 const suboff[4] = { 0, 1, 16, 17 };
    int const subn = tile16 ? 4 : 1;
    int count = 32;
    int nextar = 1;

    for (;;) {
        u2 const entry = *(u2 const*)(vram + addr);
        u4 const esi = ((entry & 0x3FF) + nbg) & 0xFFFF;
        u4 const m0 = (esi << preshift) & 0xFFF;

        for (int s = 0; s != subn; ++s) {
            u4 const eb = (m0 + (suboff[s] << preshift)) & 0xFFF;
            bool dirty;
            if (bpp == 2)
                dirty = (vidmemch[eb] & 0x01) != 0;
            else if (bpp == 4)
                dirty = (*(u2*)(vidmemch + eb) & 0x0101) != 0;
            else
                dirty = (*(u4*)(vidmemch + eb) & 0x01010101) != 0;
            if (!dirty)
                continue;
            if (bpp == 2)
                vidmemch[eb] = 0;
            else if (bpp == 4)
                *(u2*)(vidmemch + eb) = 0;
            else
                *(u4*)(vidmemch + eb) = 0;
            decode_tile(vram + (eb << 4), vcache + (eb << (6 - preshift)), bpp);
        }

        addr += 2;
        if (--count != 0)
            continue;
        if (nextar && (u2)bgptrd != (u2)bgptrc) {
            addr += 2048 - 64;
            count = 32;
            nextar = 0;
            continue;
        }
        break;
    }
}

void cachetile2b(u4 const eax) { cache_tiles(eax, 2, false); }
void cachetile4b(u4 const eax) { cache_tiles(eax, 4, false); }
void cachetile8b(u4 const eax) { cache_tiles(eax, 8, false); }
void cachetile2b16x16(u4 const eax) { cache_tiles(eax, 2, true); }
void cachetile4b16x16(u4 const eax) { cache_tiles(eax, 4, true); }
void cachetile8b16x16(u4 const eax) { cache_tiles(eax, 8, true); }

// Per-tile opacity flags, defined in endmem.asm.
extern u1 tltype2b[4096];
extern u1 tltype4b[2048];
extern u1 tltype8b[1024];

// Decode a single tile (index ecx) into the cache and record its opacity type.
// 3 = unset, 1 = fully opaque, 2 = fully transparent, 0 = mixed.  Faithful port
// of the cachesingle*bng routines from vcache.asm.
static void cachesingle_ng(u4 const ecx, int const bpp)
{
    u1* const vcache = (bpp == 2) ? vcache2b : (bpp == 4) ? vcache4b
                                                          : vcache8b;
    u1* const tltype = (bpp == 2) ? tltype2b : (bpp == 4) ? tltype4b
                                                          : tltype8b;

    if (bpp == 2)
        vidmemch2[ecx] = 0;
    else if (bpp == 4)
        vidmemch4[ecx] = 0;
    else
        *(u4*)(vidmemch8 + ecx * 4) = 0;

    u1* const dst = vcache + (ecx << 6);
    decode_tile(vram + ecx * (u4)(bpp * 8), dst, bpp);

    u1 type = 3;
    for (int i = 0; i != 64; ++i)
        type &= dst[i] ? 1 : 2;
    tltype[ecx] = type;
}

void c_cachesingle2bng(u4 const ecx) { cachesingle_ng(ecx, 2); }
void c_cachesingle4bng(u4 const ecx) { cachesingle_ng(ecx, 4); }
void c_cachesingle8bng(u4 const ecx) { cachesingle_ng(ecx, 8); }

// Register-preserving trampolines for the asm render inner loops, which call
// cachesingle*bng with the tile index in ecx and expect all registers intact.
// cdecl only preserves ebx/esi/edi/ebp, so save the caller-saved eax/ecx/edx.
__asm__(
    ".text\n"
    ".globl " CSYM(cachesingle2bng) "\n" CSYM(cachesingle2bng) ":\n"
                                                               "    push %eax\n    push %ecx\n    push %edx\n"
                                                               "    push %ecx\n    call " CSYM(c_cachesingle2bng) "\n    add $4, %esp\n"
                                                                                                                  "    pop %edx\n    pop %ecx\n    pop %eax\n    ret\n"
                                                                                                                  ".globl " CSYM(cachesingle4bng) "\n" CSYM(cachesingle4bng) ":\n"
                                                                                                                                                                             "    push %eax\n    push %ecx\n    push %edx\n"
                                                                                                                                                                             "    push %ecx\n    call " CSYM(c_cachesingle4bng) "\n    add $4, %esp\n"
                                                                                                                                                                                                                                "    pop %edx\n    pop %ecx\n    pop %eax\n    ret\n"
                                                                                                                                                                                                                                ".globl " CSYM(cachesingle8bng) "\n" CSYM(cachesingle8bng) ":\n"
                                                                                                                                                                                                                                                                                           "    push %eax\n    push %ecx\n    push %edx\n"
                                                                                                                                                                                                                                                                                           "    push %ecx\n    call " CSYM(c_cachesingle8bng) "\n    add $4, %esp\n"
                                                                                                                                                                                                                                                                                                                                              "    pop %edx\n    pop %ecx\n    pop %eax\n    ret\n");
