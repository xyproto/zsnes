/*
 * Mode 7 16-bit renderer unit tests (video/mode716b.c).
 *
 * Renders a synthetic tile map through c_drawmode716b with an identity
 * matrix and checks pixels, transparency, windowing, the mosaic path,
 * and domosaic16b's block expansion.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

void c_drawmode716b(u4 ypos, u4 xpos);
void domosaic16b(void);
extern u1 tileleft16b;

/* globals mode716b.c renders from; the test owns them here */
u1 vrama[65536];
u1* vram;
u1 mode7tab[65536];
u4 pal16b[256];
u2 xtravbuf[288];
u1* curvidoffset;
u1* cwinptr;
u1* winptrref;
u1 curmosaicsz;
u1 winon;
u1 scaddset;
u1 scrndis;
u1 mode7set;
u2 mode7A, mode7B, mode7C, mode7D, mode7X0, mode7Y0;
u2 m7starty;

static u4 dcolor_calls;
void drawmode7dcolor(void)
{
    ++dcolor_calls;
}

static u2 destbuf[512];
static u1 winbuf[512];

static void reset(void)
{
    vram = vrama;
    for (u4 i = 0; i != 65536; ++i) /* offset of pixel (x, y) in a tile */
        mode7tab[i] = (u1)(((i & 0x07) << 4) + ((i >> 8 & 0x07) << 1) + 1);
    for (u4 i = 0; i != 256; ++i)
        pal16b[i] = 0x8000u | i;
    pal16b[0] = 0;

    /* map: tile column t holds tile t+1; tile pixel (x, y) = tile*8+x+y */
    memset(vrama, 0, sizeof vrama);
    for (u4 t = 0; t != 32; ++t)
        vrama[t * 2] = (u1)(t + 1);
    for (u4 tile = 1; tile != 33; ++tile)
        for (u4 y = 0; y != 8; ++y)
            for (u4 x = 0; x != 8; ++x)
                vrama[tile * 128 + y * 16 + x * 2 + 1]
                    = (u1)(tile * 8 + x + y);

    for (u4 i = 0; i != 512; ++i)
        destbuf[i] = 0xCCCC;
    memset(winbuf, 0, sizeof winbuf);
    curvidoffset = (u1*)destbuf;
    cwinptr = winbuf;
    winptrref = NULL;
    curmosaicsz = 1;
    winon = 0;
    scaddset = 0;
    scrndis = 0;
    mode7set = 0;
    mode7A = 0x100; /* identity */
    mode7B = 0;
    mode7C = 0;
    mode7D = 0x100;
    mode7X0 = 0;
    mode7Y0 = 0;
    m7starty = 0;
    dcolor_calls = 0;
}

static void test_earlyouts(void)
{
    ZT_SECTION("disabled layer and color-add delegation");

    reset();
    scrndis = 1;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[0], 0xCCCC);

    reset();
    scaddset = 1;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(dcolor_calls, 1);
    ZT_CHECK_INT(destbuf[0], 0xCCCC);
}

static void test_identity_render(void)
{
    ZT_SECTION("identity matrix renders map row 0");

    reset();
    c_drawmode716b(0, 0);
    /* pixel p reads tile p/8+1 at x = p%8, y = 0 */
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[1], 0x8000 | 9);
    ZT_CHECK_INT(destbuf[7], 0x8000 | 15);
    ZT_CHECK_INT(destbuf[8], 0x8000 | 16);
    ZT_CHECK_INT(destbuf[255], 0x8000 | ((32 * 8 + 7) & 0xFF));
    ZT_CHECK_INT(destbuf[256], 0xCCCC); /* exactly 256 pixels drawn */

    /* transparent color 0 leaves the buffer untouched */
    reset();
    vrama[1 * 128 + 0 * 16 + 3 * 2 + 1] = 0;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[3], 0xCCCC);
    ZT_CHECK_INT(destbuf[4], 0x8000 | 12);
}

static void test_window(void)
{
    ZT_SECTION("window bytes mask pixels");

    reset();
    winon = 1;
    winbuf[2] = 1;
    winbuf[5] = 1;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[1], 0x8000 | 9);
    ZT_CHECK_INT(destbuf[2], 0xCCCC);
    ZT_CHECK_INT(destbuf[5], 0xCCCC);
    ZT_CHECK_INT(destbuf[6], 0x8000 | 14);
}

static void test_mosaic(void)
{
    ZT_SECTION("mosaic renders via xtravbuf and expands blocks");

    reset();
    curmosaicsz = 2;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(xtravbuf[16], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[1], 0x8000 | 8); /* block repeats pixel 0 */
    ZT_CHECK_INT(destbuf[2], 0x8000 | 10);
    ZT_CHECK_INT(destbuf[3], 0x8000 | 10);
}

static void test_domosaic(void)
{
    ZT_SECTION("domosaic16b block expansion");

    reset();
    curmosaicsz = 2;
    winptrref = winbuf;
    memset(xtravbuf, 0, sizeof xtravbuf);
    xtravbuf[16] = 5;
    xtravbuf[18] = 0; /* transparent block */
    xtravbuf[20] = 7;
    domosaic16b();
    ZT_CHECK_INT(destbuf[0], 5);
    ZT_CHECK_INT(destbuf[1], 5);
    ZT_CHECK_INT(destbuf[2], 0xCCCC);
    ZT_CHECK_INT(destbuf[3], 0xCCCC);
    ZT_CHECK_INT(destbuf[4], 7);

    /* window blocks single pixels */
    reset();
    curmosaicsz = 2;
    winon = 1;
    winptrref = winbuf;
    winbuf[1] = 1;
    memset(xtravbuf, 0, sizeof xtravbuf);
    xtravbuf[16] = 9;
    domosaic16b();
    ZT_CHECK_INT(destbuf[0], 9);
    ZT_CHECK_INT(destbuf[1], 0xCCCC);
}

int main(void)
{
    tileleft16b = 0;

    test_earlyouts();
    test_identity_render();
    test_window();
    test_mosaic();
    test_domosaic();

    printf("mode 7 16-bit renderer port tests\n");
    ZT_RESULTS();
}
