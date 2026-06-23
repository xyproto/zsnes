/*
 * Background tile cache unit tests
 *
 * Covers the cachetile* routines ported from vcache.asm into video/tilecache.c:
 *   cachetile2b/4b/8b and the 16x16 variants.
 * Verifies the SNES planar to linear pixel decode and the tilemap addressing,
 * dirty-tile checking and the second-screen pass.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- Globals consumed by video/tilecache.c -------------------------------- */

uint8_t vidmemch8[4096], vidmemch2[4096];
uint16_t vidmemch4[2048];
uint8_t *vcache2b, *vcache4b, *vcache8b;
uint8_t* vram;
uint16_t curtileptr;
uint32_t bgptr, bgptrc, bgptrd;

/* --- Functions under test ------------------------------------------------- */

void cachetile2b(uint32_t eax);
void cachetile4b(uint32_t eax);
void cachetile8b(uint32_t eax);
void cachetile2b16x16(uint32_t eax);
void cachetile4b16x16(uint32_t eax);
void cachetile8b16x16(uint32_t eax);
void c_cachesingle2bng(uint32_t ecx);
void c_cachesingle4bng(uint32_t ecx);
void c_cachesingle8bng(uint32_t ecx);

uint8_t tltype2b[4096];
uint8_t tltype4b[2048];
uint8_t tltype8b[1024];

/* --- Fixtures -------------------------------------------------------------- */

static uint8_t vram_buf[0x10000];
static uint8_t vc2[0x10000];
static uint8_t vc4[0x10000];
static uint8_t vc8[0x10000];

static void reset(void)
{
    memset(vidmemch2, 0, sizeof(vidmemch2));
    memset(vidmemch8, 0, sizeof(vidmemch8));
    memset(vidmemch4, 0, sizeof(vidmemch4));
    memset(vram_buf, 0, sizeof(vram_buf));
    memset(vc2, 0, sizeof(vc2));
    memset(vc4, 0, sizeof(vc4));
    memset(vc8, 0, sizeof(vc8));
    vram = vram_buf;
    vcache2b = vc2;
    vcache4b = vc4;
    vcache8b = vc8;
    curtileptr = 0;
    bgptr = 0;
    bgptrc = 0;
    bgptrd = 0; /* equal to bgptrc => single screen pass */
}

/* Fill all 32 first-pass tilemap entries with the same tile number. */
static void fill_map(uint16_t tilenum)
{
    for (int i = 0; i != 32; ++i) {
        vram_buf[i * 2] = (uint8_t)tilenum;
        vram_buf[i * 2 + 1] = (uint8_t)(tilenum >> 8);
    }
}

/* Write one byte plane into a tile of the given bpp at vram tile slot T. */
static void put_plane(uint16_t T, int bpp, int plane, int row, uint8_t val)
{
    int const off = (plane >> 1) * 16 + (plane & 1) + row * 2;
    vram_buf[T * bpp * 8 + off] = val;
}

/* --- Tests ----------------------------------------------------------------- */

static void test_2bpp_decode(void)
{
    ZT_SECTION("cachetile2b: planar to linear pixel decode");
    reset();
    uint16_t const T = 64; /* tile slot clear of the tilemap at vram[0..63] */
    fill_map(T);
    /* row 0: plane0 all set => every pixel value 1 */
    put_plane(T, 2, 0, 0, 0xFF);
    /* row 1: plane1 all set => every pixel value 2 */
    put_plane(T, 2, 1, 1, 0xFF);
    /* row 2: both planes set => every pixel value 3 */
    put_plane(T, 2, 0, 2, 0xFF);
    put_plane(T, 2, 1, 2, 0xFF);
    /* row 3: plane0 bit7 only => leftmost pixel 1, rest 0 */
    put_plane(T, 2, 0, 3, 0x80);
    vidmemch2[T] = 0x01; /* mark dirty */

    cachetile2b(0);

    uint8_t const* d = vc2 + (T << 6);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[c], 1);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[8 + c], 2);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[16 + c], 3);
    ZT_CHECK_INT(d[24], 1);
    for (int c = 1; c != 8; ++c)
        ZT_CHECK_INT(d[24 + c], 0);
    /* dirty flag must be cleared after caching */
    ZT_CHECK_INT(vidmemch2[T], 0);
}

static void test_2bpp_skip_clean(void)
{
    ZT_SECTION("cachetile2b: clean tiles are not decoded");
    reset();
    uint16_t const T = 64;
    fill_map(T);
    put_plane(T, 2, 0, 0, 0xFF);
    /* leave vidmemch2[T] == 0 (clean) */
    cachetile2b(0);
    uint8_t const* d = vc2 + (T << 6);
    for (int i = 0; i != 64; ++i)
        ZT_CHECK_INT(d[i], 0);
}

static void test_4bpp_decode(void)
{
    ZT_SECTION("cachetile4b: 4bpp planes combine into pixel values");
    reset();
    uint16_t const T = 64;
    fill_map(T);
    /* row 0: plane0 + plane2 set => pixel = 1 | (1<<2) = 5 */
    put_plane(T, 4, 0, 0, 0xFF);
    put_plane(T, 4, 2, 0, 0xFF);
    /* row 1: plane3 only (bit 3) => pixel = 8 */
    put_plane(T, 4, 3, 1, 0xFF);
    vidmemch4[T] = 0x0101; /* dirty, preshift=1 so slot index is T */

    cachetile4b(0);

    uint8_t const* d = vc4 + (T << 6);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[c], 5);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[8 + c], 8);
    ZT_CHECK_INT(vidmemch4[T], 0);
}

static void test_8bpp_decode(void)
{
    ZT_SECTION("cachetile8b: 8bpp planes combine into pixel values");
    reset();
    uint16_t const T = 64;
    fill_map(T);
    /* row 0: plane7 only (bit 7) => pixel = 0x80 */
    put_plane(T, 8, 7, 0, 0xFF);
    /* row 1: all planes set => pixel = 0xFF */
    for (int p = 0; p != 8; ++p)
        put_plane(T, 8, p, 1, 0xFF);
    /* preshift=2: slot index = T<<2 */
    uint32_t const slot = (uint32_t)T << 2;
    *(uint32_t*)(vidmemch8 + slot) = 0x01010101; /* dirty */

    cachetile8b(0);

    uint8_t const* d = vc8 + (slot << 4);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[c], 0x80);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[8 + c], 0xFF);
}

static void test_16x16_subtiles(void)
{
    ZT_SECTION("cachetile2b16x16: caches all four 8x8 sub-tiles");
    reset();
    uint16_t const T = 64; /* sub-tiles T, T+1, T+16, T+17 */
    fill_map(T);
    int const subs[4] = { 0, 1, 16, 17 };
    /* give each sub-tile a distinct flat colour in row 0 plane0 */
    for (int s = 0; s != 4; ++s) {
        put_plane((uint16_t)(T + subs[s]), 2, 0, 0, 0xFF);
        vidmemch2[T + subs[s]] = 0x01;
    }
    cachetile2b16x16(0);
    for (int s = 0; s != 4; ++s) {
        uint8_t const* d = vc2 + ((T + subs[s]) << 6);
        ZT_CHECK_INT(d[0], 1);
        ZT_CHECK_INT(vidmemch2[T + subs[s]], 0);
    }
}

static void test_highptr_addressing(void)
{
    ZT_SECTION("cachetile2b: eax bit 0x20 selects bgptrc base");
    reset();
    bgptr = 0x4000; /* must not be used when bit 0x20 set */
    bgptrc = 0; /* used base */
    uint16_t const T = 64;
    fill_map(T); /* tilemap at vram[0..] (bgptrc base) */
    put_plane(T, 2, 0, 0, 0xFF);
    vidmemch2[T] = 0x01;
    cachetile2b(0x20); /* (eax & 0x1F)=0 => addr = bgptrc + 0 = 0 */
    uint8_t const* d = vc2 + (T << 6);
    ZT_CHECK_INT(d[0], 1);
}

static void test_second_screen_pass(void)
{
    ZT_SECTION("cachetile2b: second pass when bgptrd != bgptrc");
    reset();
    bgptrc = 0;
    bgptrd = 0x1234; /* differs => trigger second pass */
    uint16_t const T = 64;
    fill_map(T);
    /* second pass starts at addr = 64 (32 entries) + 2048 - 64 = 2048 */
    uint16_t const T2 = 65;
    vram_buf[2048] = (uint8_t)T2;
    vram_buf[2049] = 0;
    put_plane(T, 2, 0, 0, 0xFF);
    put_plane(T2, 2, 1, 0, 0xFF); /* pixel value 2 */
    vidmemch2[T] = 0x01;
    vidmemch2[T2] = 0x01;
    cachetile2b(0);
    ZT_CHECK_INT(vc2[(T << 6)], 1);
    ZT_CHECK_INT(vc2[(T2 << 6)], 2); /* decoded only if second pass ran */
}

static void test_cachesingle_ng(void)
{
    ZT_SECTION("c_cachesingle4bng: single-tile decode + opacity type");
    reset();
    uint16_t const T = 5;
    /* mixed tile: row 0 plane0 set => some opaque, some transparent pixels */
    put_plane(T, 4, 0, 0, 0xFF); /* row 0: all pixels = 1 (opaque) */
    /* rows 1-7 remain 0 (transparent) => mixed => type 0 */
    vidmemch4[T] = 0x0101;
    c_cachesingle4bng(T);
    uint8_t const* d = vc4 + (T << 6);
    for (int c = 0; c != 8; ++c)
        ZT_CHECK_INT(d[c], 1);
    ZT_CHECK_INT(d[8], 0);
    ZT_CHECK_INT(tltype4b[T], 0); /* mixed */
    ZT_CHECK_INT(vidmemch4[T], 0); /* cleared */

    ZT_SECTION("c_cachesingle2bng: fully opaque tile => type 1");
    reset();
    for (int r = 0; r != 8; ++r) {
        put_plane(T, 2, 0, r, 0xFF);
        put_plane(T, 2, 1, r, 0xFF); /* every pixel = 3 */
    }
    vidmemch2[T] = 0x01;
    c_cachesingle2bng(T);
    ZT_CHECK_INT(tltype2b[T], 1);
    for (int i = 0; i != 64; ++i)
        ZT_CHECK_INT(vc2[(T << 6) + i], 3);

    ZT_SECTION("c_cachesingle8bng: fully transparent tile => type 2");
    reset();
    /* all planes zero => every pixel 0 */
    *(uint32_t*)(vidmemch8 + T * 4) = 0x01010101;
    c_cachesingle8bng(T);
    ZT_CHECK_INT(tltype8b[T], 2);
    for (int i = 0; i != 64; ++i)
        ZT_CHECK_INT(vc8[(T << 6) + i], 0);
    ZT_CHECK_INT(*(uint32_t*)(vidmemch8 + T * 4), 0);
}

int main(void)
{
    printf("vcache (tile cache) tests\n");
    test_2bpp_decode();
    test_2bpp_skip_clean();
    test_4bpp_decode();
    test_8bpp_decode();
    test_16x16_subtiles();
    test_highptr_addressing();
    test_second_screen_pass();
    test_cachesingle_ng();
    ZT_RESULTS();
}
