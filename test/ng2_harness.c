/* State setup for the newg162 drawers. See ng2_harness.h. */
#include "ng2_harness.h"

typedef unsigned char u1;
typedef unsigned int u4;

unsigned int NG2_EAX, NG2_EBX, NG2_ECX, NG2_EDX, NG2_EDI, NG2_ESI, NG2_EBP;

/* the palette the tile cache reads through ebp */
u1 ng2_palette[512];

/* video/tilecache.o wants these; the newg162 stubs do not provide them. */
u1 ng2_vram[65536 * 2];
u1* vram = ng2_vram;
u1* curtileptr;
u1* bgptr;
u1* bgptrc;
u1* bgptrd;
static u1 tilebuf[65536];

/* The decoded-tile caches. vcache*s hold the base pointer, so the stubs'
   zeros would make preparet2batile produce a null. */
extern u1 vcache2b[], vcache4b[], vcache8b[];
extern u1 vcache2bs[], vcache4bs[], vcache8bs[];
/* preparet*tile does `shl ecx,8` on an index masked to 4095, so the cache is
   indexed up to 4095<<8 = 1MB exactly, and a tile is read past that. */
#define NG2_CACHE (8u << 20)
static u1 cache2[NG2_CACHE], cache4[NG2_CACHE], cache8[NG2_CACHE];

/* inittable() in cpu/c_table.c fills these; the two loops are replicated
   rather than linked, because inittable also builds the opcode tables. */
extern u4 ngpalcon2b[32], ngpalcon4b[32];

/* Offset-per-tile mode. ofsmcptr and ofsmmptr hold base *pointers*; ofsmcptr2
   is a small offset (the code masks it to 0x3F) and ofsmtptr is a 16-bit
   value, so those stay zero. The base is offset into the buffer because the
   code reads [ebx-0x40], i.e. behind it. */
extern u1 ofsmcptr[], ofsmcptr2[], ofsmmptr[], ofsmtptrs[];
static u1 ofsbuf[1 << 18];

void ng2_init(void)
{
    curtileptr = tilebuf;
    bgptr = bgptrc = bgptrd = ng2_vram;

    *(u1**)vcache2b = cache2;
    *(u1**)vcache4b = cache4;
    *(u1**)vcache8b = cache8;
    *(u1**)vcache2bs = cache2;
    *(u1**)vcache4bs = cache4;
    *(u1**)vcache8bs = cache8;

    *(u1**)ofsmcptr = ofsbuf + 4096;
    *(u4*)ofsmcptr2 = 0;
    *(u1**)ofsmmptr = ofsbuf + 4096;
    *(u1**)ofsmtptrs = ofsbuf + 4096;

    for (u4 i = 0; i != 32; ++i) {
        ngpalcon4b[i] = (u4)(u1)(i >> 2 << 4) * 0x01010101U;
        ngpalcon2b[i] = (u4)(u1)(i >> 2 << 2) * 0x01010101U;
    }
}
