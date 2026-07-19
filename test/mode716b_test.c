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
void c_drawmode7dcolor(u4 ypos, u4 xpos);
void c_drawmode716t(u4 ypos, u4 xpos);
void c_drawmode716tb(u4 ypos, u4 xpos);
void c_drawmode716textbg(u4 ypos, u4 xpos);
void c_drawmode716textbg2(u4 craw);
void c_drawmode716extbg(u4 ypos, u4 xpos);
void c_drawmode716extbg2(u4 craw);
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
u2 dcolortab[2][256];
u1 vidbright;
u1 prevbrightdc;
u4 pal16bcl[256];
u4 pal16bxcl[256];
u2 fulladdtab[65536];
u4 vesa2_clbit;
u1 transpbuf[576 + 16 + 288 * 2];
u1 DoTransp;
u1 scaddtype;
u2 scrnon;
u1 coladdr, coladdg, coladdb, colnull;

static u4 gen_calls;
void Gendcolortable(void)
{
    ++gen_calls;
    for (u4 i = 0; i != 256; ++i)
        dcolortab[0][i] = (u2)(0x4000 | vidbright << 8 | i);
}

static u2 destbuf[1024]; /* extbg stashes bytes at +576..+1087 */
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

    for (u4 i = 0; i != 1024; ++i)
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
    for (u4 i = 0; i != 256; ++i)
        dcolortab[0][i] = (u2)(0x2000 | i);
    vidbright = 15;
    prevbrightdc = 15;
    gen_calls = 0;

    /* transparency state: even table values stay clean under the mask */
    for (u4 i = 0; i != 256; ++i) {
        pal16bcl[i] = i << 4;
        pal16bxcl[i] = 0xF000 - (i << 4);
    }
    for (u4 i = 0; i != 65536; ++i)
        fulladdtab[i] = (u2)(0x4000 ^ i);
    vesa2_clbit = 0xFFFE;
    memset(transpbuf, 0, sizeof transpbuf);
    DoTransp = 0;
    scaddtype = 0;
    scrnon = 0x0100;
    coladdr = coladdg = coladdb = colnull = 0;
}

static void test_earlyouts(void)
{
    ZT_SECTION("disabled layer and color-add delegation");

    reset();
    scrndis = 1;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[0], 0xCCCC);

    /* scaddset routes to the direct color renderer */
    reset();
    scaddset = 1;
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x2000 | 8);
}

static void test_dcolor(void)
{
    ZT_SECTION("direct color renderer and table regeneration");

    reset();
    c_drawmode7dcolor(0, 0);
    ZT_CHECK_INT(gen_calls, 0); /* brightness unchanged */
    ZT_CHECK_INT(destbuf[0], 0x2000 | 8);
    ZT_CHECK_INT(destbuf[255], 0x2000 | ((32 * 8 + 7) & 0xFF));

    /* brightness change regenerates dcolortab first */
    reset();
    vidbright = 7;
    c_drawmode7dcolor(0, 0);
    ZT_CHECK_INT(gen_calls, 1);
    ZT_CHECK_INT(prevbrightdc, 7);
    ZT_CHECK_INT(destbuf[0], 0x4000 | 7 << 8 | 8);

    /* the next mode716b render uses pal16b again */
    reset();
    c_drawmode716b(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);
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

static void test_transp(void)
{
    ZT_SECTION("transparency renderer color math and dispatch");

    /* half add: pixel 0 sees transp 0, pixel 1 averages with 0x200 */
    reset();
    scaddtype = 0x40;
    u2* const tw = (u2*)(transpbuf + 32);
    tw[1] = 0x200;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x80); /* pal16bcl[8] as-is */
    ZT_CHECK_INT(destbuf[1], (0x90 + 0x200) >> 1);

    /* full add when the sub screen high byte is off */
    reset();
    scaddtype = 0x40;
    scrnon = 0;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x4000 ^ (0x80 >> 1));

    /* full add when a color constant is added */
    reset();
    scaddtype = 0x40;
    coladdg = 1;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x4000 ^ (0x80 >> 1));

    /* full add by default, blending with the transp pixel */
    reset();
    tw[1] = 0x200;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x4000 ^ (0x80 >> 1));
    ZT_CHECK_INT(destbuf[1], 0x4000 ^ ((0x90 + 0x200) >> 1));

    /* full subtract */
    reset();
    scaddtype = 0x80;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], (0x4000 ^ (0xEF80 >> 1)) ^ 0xFFFF);

    /* DoTransp == 1 falls back to the plain renderer */
    reset();
    DoTransp = 1;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);

    /* scaddset routes to direct color, scrndis disables */
    reset();
    scaddset = 1;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x2000 | 8);
    reset();
    scrndis = 1;
    c_drawmode716t(0, 0);
    ZT_CHECK_INT(destbuf[0], 0xCCCC);
}

static void test_mainsub(void)
{
    ZT_SECTION("main & sub renderer writes both buffers");

    reset();
    memset(transpbuf, 0xEE, sizeof transpbuf);
    u2* const tw = (u2*)(transpbuf + 32);
    vrama[1 * 128 + 0 * 16 + 3 * 2 + 1] = 0; /* transparent pixel 3 */
    c_drawmode716tb(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);
    ZT_CHECK_INT(tw[0], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[3], 0xCCCC);
    ZT_CHECK_INT(tw[3], 0xEEEE); /* untouched but cursor advanced */
    ZT_CHECK_INT(tw[4], 0x8000 | 12);

    /* window blocks the write, cursor still keeps step */
    reset();
    memset(transpbuf, 0xEE, sizeof transpbuf);
    winon = 1;
    winbuf[2] = 1;
    c_drawmode716tb(0, 0);
    ZT_CHECK_INT(destbuf[2], 0xCCCC);
    ZT_CHECK_INT(tw[2], 0xEEEE);
    ZT_CHECK_INT(tw[3], 0x8000 | 11);
}

static void test_extbg(void)
{
    ZT_SECTION("extbg pass 1: byte buffer and priority gate");

    /* pixel p is p+8, so slots 120..247 carry the priority bit */
    reset();
    c_drawmode716textbg(0, 0);
    ZT_CHECK_INT(((u1*)destbuf)[576], 8); /* raw byte stashed */
    ZT_CHECK_INT(((u1*)destbuf)[576 + 240], 128);
    ZT_CHECK_INT(destbuf[0], 0x4000 ^ (0x80 >> 1)); /* full add default */
    ZT_CHECK_INT(destbuf[120], 0xCCCC); /* high priority skipped */

    /* half add path blends with the sub screen */
    reset();
    scaddtype = 0x40;
    u2* const tw = (u2*)(transpbuf + 32);
    tw[1] = 0x200;
    c_drawmode716textbg(0, 0);
    ZT_CHECK_INT(destbuf[0], 0x80);
    ZT_CHECK_INT(destbuf[1], (0x90 + 0x200) >> 1);

    /* windowing masks pixels and advances the global cwinptr */
    reset();
    winon = 1;
    winbuf[2] = 1;
    c_drawmode716textbg(0, 0);
    ZT_CHECK_INT(destbuf[1], 0x4000 ^ (0x90 >> 1));
    ZT_CHECK_INT(destbuf[2], 0xCCCC);
    ZT_CHECK_INT(((u1*)destbuf)[576 + 4], 10); /* byte still stashed */
    ZT_CHECK_INT((int)(cwinptr - winbuf), 256);
    ZT_CHECK_INT(winptrref == winbuf, 1);
}

static void test_extbg2(void)
{
    ZT_SECTION("extbg pass 2: draws stashed high-priority pixels");

    reset();
    memset((u1*)destbuf + 576, 0, 512);
    ((u1*)destbuf)[576] = 0x85;
    ((u1*)destbuf)[576 + 2] = 0x05; /* no priority bit: skipped */
    c_drawmode716textbg2(0);
    ZT_CHECK_INT(destbuf[0], 0x4000 ^ (0x50 >> 1)); /* pal16bcl[5] full add */
    ZT_CHECK_INT(destbuf[1], 0xCCCC);

    /* half add here ignores scrnon, unlike pass 1 */
    reset();
    memset((u1*)destbuf + 576, 0, 512);
    ((u1*)destbuf)[576] = 0x85;
    scaddtype = 0x40;
    scrnon = 0;
    u2* const tw = (u2*)(transpbuf + 32);
    tw[0] = 0x200;
    c_drawmode716textbg2(0);
    ZT_CHECK_INT(destbuf[0], (0x50 + 0x200) >> 1);

    /* windowed: stray CL byte lands in pixel 0, cwinptr stays put */
    reset();
    memset((u1*)destbuf + 576, 0, 512);
    ((u1*)destbuf)[576 + 6] = 0x90;
    winon = 1;
    winbuf[3] = 1;
    c_drawmode716textbg2(0xAB);
    ZT_CHECK_INT(destbuf[0], 0xCCAB);
    ZT_CHECK_INT(destbuf[3], 0xCCCC); /* blocked by the window */
    ZT_CHECK_INT(cwinptr == winbuf, 1);
}

static void test_eextbg(void)
{
    ZT_SECTION("mode716e pass 1: plain palette, no transparency engine");

    /* pixel p is p+8, so slots 120..247 carry the priority bit */
    reset();
    c_drawmode716extbg(0, 0);
    ZT_CHECK_INT(((u1*)destbuf)[576], 8); /* raw byte stashed */
    ZT_CHECK_INT(((u1*)destbuf)[576 + 240], 128);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8); /* pal16b, no color math */
    ZT_CHECK_INT(destbuf[120], 0xCCCC); /* high priority skipped */
    ZT_CHECK_INT(winptrref == winbuf, 1);

    /* windowed variant keeps a local cursor and never touches cwinptr */
    reset();
    winon = 1;
    winbuf[2] = 1;
    c_drawmode716extbg(0, 0);
    ZT_CHECK_INT(destbuf[1], 0x8000 | 9);
    ZT_CHECK_INT(destbuf[2], 0xCCCC);
    ZT_CHECK_INT(((u1*)destbuf)[576 + 4], 10); /* byte still stashed */
    ZT_CHECK_INT(cwinptr == winbuf, 1);

    /* mosaic renders via xtravbuf and expands blocks */
    reset();
    curmosaicsz = 2;
    c_drawmode716extbg(0, 0);
    ZT_CHECK_INT(xtravbuf[16], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 8);
    ZT_CHECK_INT(destbuf[1], 0x8000 | 8);

    reset();
    scrndis = 1;
    c_drawmode716extbg(0, 0);
    ZT_CHECK_INT(destbuf[0], 0xCCCC);
}

static void test_eextbg2(void)
{
    ZT_SECTION("mode716e pass 2: stashed high-priority pixels via pal16b");

    reset();
    memset((u1*)destbuf + 576, 0, 512);
    ((u1*)destbuf)[576] = 0x85;
    ((u1*)destbuf)[576 + 2] = 0x05; /* no priority bit: skipped */
    c_drawmode716extbg2(0);
    ZT_CHECK_INT(destbuf[0], 0x8000 | 5); /* pal16b, no color math */
    ZT_CHECK_INT(destbuf[1], 0xCCCC);
    ZT_CHECK_INT(winptrref == winbuf, 1);

    /* windowed: stray CL byte lands in pixel 0, cwinptr stays put */
    reset();
    memset((u1*)destbuf + 576, 0, 512);
    ((u1*)destbuf)[576 + 6] = 0x90;
    ((u1*)destbuf)[576 + 8] = 0x91;
    winon = 1;
    winbuf[3] = 1;
    c_drawmode716extbg2(0xAB);
    ZT_CHECK_INT(destbuf[0], 0xCCAB);
    ZT_CHECK_INT(destbuf[3], 0xCCCC); /* blocked by the window */
    ZT_CHECK_INT(destbuf[4], 0x8000 | 0x11);
    ZT_CHECK_INT(cwinptr == winbuf, 1);
}

int main(void)
{
    tileleft16b = 0;

    test_earlyouts();
    test_dcolor();
    test_identity_render();
    test_window();
    test_mosaic();
    test_domosaic();
    test_transp();
    test_mainsub();
    test_extbg();
    test_extbg2();
    test_eextbg();
    test_eextbg2();

    printf("mode 7 16-bit renderer port tests\n");
    ZT_RESULTS();
}
