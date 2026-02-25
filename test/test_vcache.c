/*
 * test_vcache.c — Tests for tile caching functions (c_vcache_asm.c)
 *
 * Verifies bitplane decoding for 2bpp, 4bpp, and 8bpp tile formats
 * by feeding known VRAM data and checking the decoded cache output.
 */
#include "zstest.h"
#include "../types.h"
#include <string.h>
#include <stdlib.h>

/* Externs from c_vcache_asm.c (data section) */
extern u1  vidmemch2[4096];
extern u2  vidmemch4[2048];
extern u1  vidmemch8[4096];

/* Tile cache functions (C versions) */
extern void cachetile2b(u4 area);
extern void cachetile4b(u4 area);
extern void cachetile8b(u4 area);

/* We need real buffers for VRAM and cache */
static u1 test_vram[65536];
static u1 test_cache2b[4096 * 64];
static u1 test_cache4b[2048 * 64];
static u1 test_cache8b[1024 * 64];

/* Provide the globals that c_vcache_asm.c references as extern.
 * Only define symbols NOT already in c_endmem_data.o or init_data.o.
 * Already provided by c_endmem_data.o: vidmemch2/4/8, sprleft*, sprcnt,
 *   sprstart, sprtilecnt, sprend, sprendx, tltype2b/4b/8b
 * Already provided by c_vcache_asm.o: sprprifix, OMBGTestVal, etc. */
u1* vram;
u1* vcache2b;
u1* vcache4b;
u1* vcache8b;
u4  bgptr;
u4  bgptrc;
u4  bgptrd;
u2  curtileptr;

/* Sprite-related globals (needed for link, not used in tile tests) */
u1  oamram[544];
u2  objptr;
u2  objptrn;
u1  objsize1;
u1  objsize2;
u2  objadds1;
u2  objadds2;
u1  objmovs1;
u1  objmovs2;
u1  objhipr;
u2  resolutn;
u1  curypos;
u1  interlval;
u4  offsetmshl;
u1* spritetablea;

static void setup(void)
{
    memset(test_vram, 0, sizeof(test_vram));
    memset(test_cache2b, 0, sizeof(test_cache2b));
    memset(test_cache4b, 0, sizeof(test_cache4b));
    memset(test_cache8b, 0, sizeof(test_cache8b));
    memset(vidmemch2, 0, sizeof(vidmemch2));
    memset(vidmemch4, 0, sizeof(vidmemch4));
    memset(vidmemch8, 0, sizeof(vidmemch8));

    vram = test_vram;
    vcache2b = test_cache2b;
    vcache4b = test_cache4b;
    vcache8b = test_cache8b;
    bgptr = 0;
    bgptrc = 0;
    bgptrd = 0;
    curtileptr = 0;
}

/*
 * 2bpp tile format (16 bytes per tile):
 *   For each row (8 rows): byte0 = plane0, byte1 = plane1
 *   Pixel[x] = (plane1_bit << 1) | plane0_bit
 *   Bit 7 = leftmost pixel (pixel 0)
 */
void test_vcache(void)
{
    ZT_SECTION("vcache: 2bpp tile decode");
    {
        setup();

        /* Place a tilemap entry at VRAM[0] pointing to tile #1 */
        test_vram[0] = 0x01; /* tile 1, no flip */
        test_vram[1] = 0x00;

        /* Tile #1 data at VRAM offset 1*16 = 16 */
        /* Row 0: plane0=0xFF (all 1s), plane1=0x00 */
        test_vram[16] = 0xFF;
        test_vram[17] = 0x00;
        /* Row 1: plane0=0x00, plane1=0xFF */
        test_vram[18] = 0x00;
        test_vram[19] = 0xFF;
        /* Row 2: plane0=0xAA (10101010), plane1=0x55 (01010101) */
        test_vram[20] = 0xAA;
        test_vram[21] = 0x55;

        /* Mark tile 1 dirty */
        vidmemch2[1] = 1;

        cachetile2b(0);

        /* Check row 0: plane0=0xFF, plane1=0x00 → all pixels = 1 */
        u1* tile1 = test_cache2b + 1 * 64;
        int i;
        for (i = 0; i < 8; i++)
            ZT_CHECK_INT(tile1[i], 1);

        /* Check row 1: plane0=0x00, plane1=0xFF → all pixels = 2 */
        for (i = 0; i < 8; i++)
            ZT_CHECK_INT(tile1[8 + i], 2);

        /* Check row 2: plane0=0xAA, plane1=0x55 */
        /* bit7: p0=1,p1=0→1  bit6: p0=0,p1=1→2  bit5: p0=1,p1=0→1 ... */
        ZT_CHECK_INT(tile1[16 + 0], 1);
        ZT_CHECK_INT(tile1[16 + 1], 2);
        ZT_CHECK_INT(tile1[16 + 2], 1);
        ZT_CHECK_INT(tile1[16 + 3], 2);
        ZT_CHECK_INT(tile1[16 + 4], 1);
        ZT_CHECK_INT(tile1[16 + 5], 2);
        ZT_CHECK_INT(tile1[16 + 6], 1);
        ZT_CHECK_INT(tile1[16 + 7], 2);
    }

    ZT_SECTION("vcache: 2bpp all-3 tile");
    {
        setup();

        test_vram[0] = 0x02;  /* tile 2 */
        test_vram[1] = 0x00;

        /* Tile 2 at offset 32: both planes all 1s → pixel = 3 */
        int row;
        for (row = 0; row < 8; row++) {
            test_vram[32 + row * 2] = 0xFF;
            test_vram[32 + row * 2 + 1] = 0xFF;
        }
        vidmemch2[2] = 1;

        cachetile2b(0);

        u1* tile2 = test_cache2b + 2 * 64;
        int i;
        for (i = 0; i < 64; i++)
            ZT_CHECK_INT(tile2[i], 3);
    }

    ZT_SECTION("vcache: 4bpp tile decode");
    {
        setup();

        test_vram[0] = 0x03;  /* tile 3 */
        test_vram[1] = 0x00;

        /* Tile 3 in 4bpp: VRAM offset = (3*2)*16 = 96
         * 4bpp indexing: tile_base*2 gives vidmemch4 index
         * VRAM offset = idx * 16, cache offset = idx * 32 */
        u2 idx = 3 * 2;  /* = 6 */
        u4 vram_off = (u4)idx * 16;  /* = 96 */

        /* Row 0: all 4 planes = 0xFF → pixel = 0x0F */
        test_vram[vram_off + 0] = 0xFF;   /* plane 0 */
        test_vram[vram_off + 1] = 0xFF;   /* plane 1 */
        test_vram[vram_off + 16] = 0xFF;  /* plane 2 */
        test_vram[vram_off + 17] = 0xFF;  /* plane 3 */

        /* Row 1: only plane 2 = 0xFF → pixel = 4 */
        test_vram[vram_off + 2] = 0x00;
        test_vram[vram_off + 3] = 0x00;
        test_vram[vram_off + 18] = 0xFF;
        test_vram[vram_off + 19] = 0x00;

        /* Mark dirty */
        u1* ch = (u1*)vidmemch4;
        ch[idx] = 1;
        ch[idx + 1] = 1;

        cachetile4b(0);

        u1* tile3 = test_cache4b + (u4)idx * 32;
        int i;
        /* Row 0: all pixels should be 0x0F */
        for (i = 0; i < 8; i++)
            ZT_CHECK_INT(tile3[i], 0x0F);

        /* Row 1: all pixels should be 4 (only plane 2 set) */
        for (i = 0; i < 8; i++)
            ZT_CHECK_INT(tile3[8 + i], 4);
    }

    ZT_SECTION("vcache: 8bpp tile decode");
    {
        setup();

        test_vram[0] = 0x01;  /* tile 1 */
        test_vram[1] = 0x00;

        /* Tile 1 in 8bpp: idx = 1*4 = 4
         * VRAM offset = 4*16 = 64, cache offset = 4*16 = 64 */
        u2 idx = 1 * 4;
        u4 vram_off = (u4)idx * 16;  /* = 64 */

        /* Row 0: set all 8 planes to 0xFF → pixel = 0xFF */
        test_vram[vram_off + 0] = 0xFF;   /* plane 0 */
        test_vram[vram_off + 1] = 0xFF;   /* plane 1 */
        test_vram[vram_off + 16] = 0xFF;  /* plane 2 */
        test_vram[vram_off + 17] = 0xFF;  /* plane 3 */
        test_vram[vram_off + 32] = 0xFF;  /* plane 4 */
        test_vram[vram_off + 33] = 0xFF;  /* plane 5 */
        test_vram[vram_off + 48] = 0xFF;  /* plane 6 */
        test_vram[vram_off + 49] = 0xFF;  /* plane 7 */

        /* Row 1: only plane 7 = 0x80 (bit 7) → pixel 0 = 128, rest = 0 */
        test_vram[vram_off + 2] = 0x00;
        test_vram[vram_off + 3] = 0x00;
        test_vram[vram_off + 18] = 0x00;
        test_vram[vram_off + 19] = 0x00;
        test_vram[vram_off + 34] = 0x00;
        test_vram[vram_off + 35] = 0x00;
        test_vram[vram_off + 50] = 0x00;
        test_vram[vram_off + 51] = 0x80;  /* plane 7, bit 7 */

        /* Mark dirty */
        vidmemch8[idx] = 1;
        vidmemch8[idx + 1] = 1;
        vidmemch8[idx + 2] = 1;
        vidmemch8[idx + 3] = 1;

        cachetile8b(0);

        u1* tile1 = test_cache8b + (u4)idx * 16;
        int i;
        /* Row 0: all pixels = 0xFF */
        for (i = 0; i < 8; i++)
            ZT_CHECK_INT(tile1[i], 0xFF);

        /* Row 1: pixel 0 = 128, rest = 0 */
        ZT_CHECK_INT(tile1[8 + 0], 128);
        for (i = 1; i < 8; i++)
            ZT_CHECK_INT(tile1[8 + i], 0);
    }

    ZT_SECTION("vcache: dirty flag cleared after cache");
    {
        setup();

        test_vram[0] = 0x01;  /* tile 1 */
        test_vram[1] = 0x00;
        vidmemch2[1] = 1;

        cachetile2b(0);
        ZT_CHECK_INT(vidmemch2[1], 0);

        /* Calling again should not re-decode (flag is clear) */
        test_cache2b[1 * 64] = 0x42;  /* plant a sentinel */
        cachetile2b(0);
        ZT_CHECK_INT(test_cache2b[1 * 64], 0x42);  /* sentinel preserved */
    }
}
