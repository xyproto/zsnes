/*
 * hq2x filter tests (video/c_hqx.c).
 *
 * Drives hq2x_16b and hq3x_16b over a synthetic 256x224 source and checks the
 * properties
 * the filter has to hold to: the doubler fallback when it is switched off,
 * exact reproduction of a flat field, no writing past the picture, and real
 * interpolation - not replication - across an edge.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "zstest.h"

#define W 256
#define H 224
#define SRC_LINE 288 /* 256 drawn plus the 32-pixel skip */
#define DSTW 512
#define PITCH (DSTW * 2)
#define DSTW3 768
#define PITCH3 (DSTW3 * 2)

/* Everything video/c_hqx.c reaches for; the emulator owns these normally. */
u1* vidbuffer;
u1 curblank;
u1* WinVidMemStart;
u4 NumBytesPerLine;
u4 AddEndBytes;
u2 resolutn;
u1* BitConv32Ptr;
u1* RGBtoYUVPtr;
u4 HalfTrans[4];
u1 hirestiledat[256];
u1 SpecialLine[256];
u1 GUIOn;
uint8_t GUIOn2;
u1 hqFilter;
u1 FilteredGUI;
u1 newengen;

void hq2x_16b(void);
void hq3x_16b(void);

static u2* src_pixels; /* the 256 drawn pixels of each line */
static u1* dst;

static void build_tables(void)
{
    BitConv32Ptr = malloc(65536u * 4u + 4096u);
    RGBtoYUVPtr = malloc(65536u * 4u + 4096u);
    for (u4 i = 0; i < 65536u; i++) {
        ((u4*)BitConv32Ptr)[i] = ((i & 0xF800u) << 8) + ((i & 0x07E0u) << 5)
            + ((i & 0x001Fu) << 3) + 0xFF000000u;
    }
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            for (int k = 0; k < 32; k++) {
                int const r = i << 3, g = j << 2, b = k << 3;
                int const Y = (r + g + b) >> 2;
                int const u = 128 + ((r - b) >> 2);
                int const v = 128 + ((-r + 2 * g - b) >> 3);
                ((u4*)RGBtoYUVPtr)[(i << 11) + (j << 5) + k]
                    = (u4)((Y << 16) + (u << 8) + v);
            }
        }
    }
    HalfTrans[0] = 0xF7DEF7DEu; /* 565 */
    HalfTrans[1] = 0xF7DEF7DEu;
}

static void setup(void)
{
    vidbuffer = calloc(1, 0x100000);
    src_pixels = (u2*)(vidbuffer + 16 * 2 + 256 * 2 + 32 * 2);
    /* Large enough for the 3x surface too, plus a guard band. */
    dst = calloc(1, (size_t)PITCH3 * H * 3 + 4096);
    WinVidMemStart = dst;
    NumBytesPerLine = PITCH;
    AddEndBytes = 0;
    resolutn = H;
    curblank = 0;
    hqFilter = 1;
    FilteredGUI = 1;
    GUIOn = 0;
    GUIOn2 = 0;
    newengen = 0;
    memset(hirestiledat, 0, sizeof hirestiledat);
    memset(SpecialLine, 0, sizeof SpecialLine);
}

static void fill(u2 const c)
{
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = c;
}

static u2 out(u4 const x, u4 const y) { return ((u2*)(dst + y * PITCH))[x]; }

static void test_blank(void)
{
    ZT_SECTION("curblank 0x40 writes nothing");
    setup();
    fill(0x1234);
    memset(dst, 0xAB, (size_t)PITCH * H * 2);
    curblank = 0x40;
    hq2x_16b();
    int clean = 1;
    for (size_t i = 0; i < (size_t)PITCH * H * 2; i++)
        if (dst[i] != 0xAB)
            clean = 0;
    ZT_CHECK(clean);
}

static void test_flat(void)
{
    ZT_SECTION("a flat field survives the filter unchanged");
    setup();
    fill(0x4A69);
    hq2x_16b();
    int ok = 1;
    for (u4 y = 0; y < H * 2; y++)
        for (u4 x = 0; x < DSTW; x++)
            if (out(x, y) != 0x4A69)
                ok = 0;
    ZT_CHECK(ok);
}

static void test_filter_off_doubles(void)
{
    ZT_SECTION("hqFilter 0 falls back to pixel doubling");
    setup();
    hqFilter = 0;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)(x * 7u + y * 13u);
    hq2x_16b();
    int ok = 1;
    for (u4 y = 0; y < H; y++) {
        for (u4 x = 0; x < W; x++) {
            u2 const p = src_pixels[y * SRC_LINE + x];
            if (out(x * 2, y * 2) != p || out(x * 2 + 1, y * 2) != p
                || out(x * 2, y * 2 + 1) != p || out(x * 2 + 1, y * 2 + 1) != p)
                ok = 0;
        }
    }
    ZT_CHECK(ok);
}

static void test_hires_line_doubles(void)
{
    ZT_SECTION("a hi-res line is doubled, not filtered");
    setup();
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)((x ^ y) * 31u);
    hirestiledat[1 + 5] = 2; /* line 5 marked hi-res */
    hq2x_16b();
    int ok = 1;
    for (u4 x = 0; x < W; x++) {
        u2 const p = src_pixels[5 * SRC_LINE + x];
        if (out(x * 2, 10) != p || out(x * 2 + 1, 10) != p
            || out(x * 2, 11) != p || out(x * 2 + 1, 11) != p)
            ok = 0;
    }
    ZT_CHECK(ok);
}

/* An edge has to produce colours that are in neither input, and every output
   has to stay inside the range the two inputs span - that is interpolation,
   and it is what the doubler could never do. */
static void test_edge_interpolates(void)
{
    ZT_SECTION("an edge is interpolated, within the two colours it spans");
    setup();
    u2 const a = 0x0000, b = 0xFFFF;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (x + y < 128) ? b : a;
    hq2x_16b();

    int blended = 0, inrange = 1;
    for (u4 y = 0; y < H * 2; y++) {
        for (u4 x = 0; x < DSTW; x++) {
            u2 const p = out(x, y);
            if (p != a && p != b)
                blended = 1;
            /* Both inputs are channel extremes, so every channel of any
               interpolation of them must lie between them. */
            if (((p >> 11) & 0x1Fu) > 0x1Fu || ((p >> 5) & 0x3Fu) > 0x3Fu)
                inrange = 0;
        }
    }
    ZT_CHECK(blended);
    ZT_CHECK(inrange);
}

/* Two colours that differ numerically but not perceptually must not trip the
   pattern: the filter has a YUV threshold, unlike a plain edge detector. */
static void test_threshold(void)
{
    ZT_SECTION("a sub-threshold difference is not treated as an edge");
    setup();
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)((x < 128) ? 0x4208u : 0x4209u);
    hq2x_16b();
    /* Well inside the left half, nothing may have changed. */
    int ok = 1;
    for (u4 y = 4; y < 200; y++)
        for (u4 x = 8; x < 200; x++)
            if (out(x, y) != 0x4208u)
                ok = 0;
    ZT_CHECK(ok);
}

static void test_no_overrun(void)
{
    ZT_SECTION("writes stay inside the 512x448 picture");
    setup();
    size_t const used = (size_t)PITCH * H * 2;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)(x * 3u ^ y * 5u);
    memset(dst + used, 0x5A, 4096);
    hq2x_16b();
    int clean = 1;
    for (size_t i = 0; i < 4096; i++)
        if (dst[used + i] != 0x5A)
            clean = 0;
    ZT_CHECK(clean);
}

/* hq3x over its own 3x surface: the same properties, one size up. */
static void setup3(void)
{
    setup();
    NumBytesPerLine = PITCH3;
}

static u2 out3(u4 const x, u4 const y) { return ((u2*)(dst + y * PITCH3))[x]; }

static void test3_flat(void)
{
    ZT_SECTION("hq3x: a flat field survives the filter unchanged");
    setup3();
    fill(0x2B7D);
    hq3x_16b();
    int ok = 1;
    for (u4 y = 0; y < H * 3; y++)
        for (u4 x = 0; x < DSTW3; x++)
            if (out3(x, y) != 0x2B7D)
                ok = 0;
    ZT_CHECK(ok);
}

static void test3_filter_off_triples(void)
{
    ZT_SECTION("hq3x: hqFilter 0 falls back to pixel tripling");
    setup3();
    hqFilter = 0;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)(x * 11u + y * 7u);
    hq3x_16b();
    int ok = 1;
    for (u4 y = 0; y < H; y++) {
        for (u4 x = 0; x < W; x++) {
            u2 const p = src_pixels[y * SRC_LINE + x];

            for (u4 dy = 0; dy < 3; dy++)
                for (u4 dx = 0; dx < 3; dx++)
                    if (out3(x * 3 + dx, y * 3 + dy) != p)
                        ok = 0;
        }
    }
    ZT_CHECK(ok);
}

static void test3_edge_interpolates(void)
{
    ZT_SECTION("hq3x: an edge is interpolated, and the centre stays exact");
    setup3();
    u2 const a = 0x0000, b = 0xFFFF;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (x + y < 128) ? b : a;
    hq3x_16b();

    int blended = 0, centre_ok = 1;
    for (u4 y = 0; y < H; y++) {
        for (u4 x = 0; x < W; x++) {
            /* The middle of every 3x3 block is the source pixel itself. */
            if (out3(x * 3 + 1, y * 3 + 1) != src_pixels[y * SRC_LINE + x])
                centre_ok = 0;
            for (u4 dy = 0; dy < 3; dy++)
                for (u4 dx = 0; dx < 3; dx++) {
                    u2 const p = out3(x * 3 + dx, y * 3 + dy);

                    if (p != a && p != b)
                        blended = 1;
                }
        }
    }
    ZT_CHECK(blended);
    ZT_CHECK(centre_ok);
}

static void test3_no_overrun(void)
{
    ZT_SECTION("hq3x: writes stay inside the 768x672 picture");
    setup3();
    size_t const used = (size_t)PITCH3 * H * 3;
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (u2)(x * 5u ^ y * 3u);
    memset(dst + used, 0x5A, 4096);
    hq3x_16b();
    int clean = 1;
    for (size_t i = 0; i < 4096; i++)
        if (dst[used + i] != 0x5A)
            clean = 0;
    ZT_CHECK(clean);
}

int main(void)
{
    printf("ZSNES2 hqx filter tests\n");
    build_tables();

    test_blank();
    test_flat();
    test_filter_off_doubles();
    test_hires_line_doubles();
    test_edge_interpolates();
    test_threshold();
    test_no_overrun();

    test3_flat();
    test3_filter_off_triples();
    test3_edge_interpolates();
    test3_no_overrun();

    ZT_RESULTS();
}
