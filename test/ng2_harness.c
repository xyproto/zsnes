/* State setup for the newg162 drawers. See ng2_harness.h. */
#include "ng2_harness.h"

typedef unsigned char u1;
typedef unsigned int u4;

unsigned int NG2_EAX, NG2_EBX, NG2_ECX, NG2_EDX, NG2_EDI, NG2_ESI, NG2_EBP;

/* the palette the tile cache reads through ebp */
u1 ng2_palette[512];

/* CPalPtrng holds a *pointer* to the converted palette; the 16x16 drawers load
   it into eax and index [eax+ebx*2], so the stub's zero faults. */
extern u1 CPalPtrng[];
static u1 palconv[1 << 17];

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
extern u1 ofsmtptr[], ofsmval[], ofsmvalh[], ofshvaladd[];
extern u4 ngcwinptr, ngcwinmode, ngcpixleft, ngwintable[];
extern u1 tleftn;
extern u4 tleftnb;
static u1 ofsbuf[1 << 18];

/* The mosaic pass is a separate routine the line drawers tail-jump to; the
   oracle only has it as a stub. A bare ret matches its real exit. */
__asm__(".text\n.globl domosaicng16b\ndomosaicng16b: ret\n");

void ng2_reset(void)
{
    *(u1**)ofsmcptr = ofsbuf + 4096;
    *(u4*)ofsmcptr2 = 0;
    *(u1**)ofsmmptr = ofsbuf + 4096;
    *(u1**)ofsmtptrs = ofsbuf + 4096;
    *(u4*)ofsmtptr = 0;
    *(u4*)ofsmval = 0;
    *(u4*)ofsmvalh = 0;
    *(u4*)ofshvaladd = 0;
    ngcwinptr = (unsigned)(unsigned long)ngwintable;
    ngcwinmode = 0;
    ngcpixleft = 0;
    tleftn = 0;
    tleftnb = 0;
}

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

    *(u1**)CPalPtrng = palconv;
    ng2_reset();

    for (u4 i = 0; i != 32; ++i) {
        ngpalcon4b[i] = (u4)(u1)(i >> 2 << 4) * 0x01010101U;
        ngpalcon2b[i] = (u4)(u1)(i >> 2 << 2) * 0x01010101U;
    }
}
