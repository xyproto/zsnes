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
/* SRC_LINE comes from video/c_hqx.c, included below. */
#define DSTW 512
#define PITCH (DSTW * 2)
#define DSTW3 768
#define PITCH3 (DSTW3 * 2)
#define DSTW4 1024
#define PITCH4 (DSTW4 * 2)

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

/* Included, not linked: the interpolation helpers below are static, and they
   are where the filter's arithmetic lives. */
#include "../video/c_hqx.c"

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
    /* Large enough for the 3x surface at 32bpp too, plus a guard band. */
    dst = calloc(1, (size_t)DSTW4 * 4 * H * 4 + 4096);
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

/* A pattern with plenty of edges, so the rule set really engages rather than
   falling through to the flat-field case. */
static void fill_edges(void)
{
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x]
                = (u2)(((x * 7 + y * 13) & 3) == 0 ? 0x001F
                        : ((x ^ y) & 4)            ? 0xF800
                                                   : 0x07E0);
}

/* The 16- and 32-bit entry points share one rule set, so the 32-bit output
   must be the 16-bit output put through the same 16->32bit table. Catches a
   32-bit path that quietly replicates pixels instead of filtering. */
static void check_32_matches_16(char const* what, void (*f16)(void),
    void (*f32)(void), u4 const outw, u4 const outh)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;
    size_t const bytes16 = (size_t)outw * 2 * outh;
    u2* snap = malloc(bytes16);

    ZT_SECTION(what);
    setup();
    NumBytesPerLine = outw * 2;
    fill_edges();
    f16();
    memcpy(snap, dst, bytes16);

    memset(dst, 0, (size_t)outw * 4 * outh);
    NumBytesPerLine = outw * 4;
    f32();

    int same = 1, filtered = 0;
    for (u4 y = 0; y < outh && same; y++)
        for (u4 x = 0; x < outw; x++) {
            u2 const p16 = snap[(size_t)y * outw + x];
            u4 const p32 = ((u4 const*)(dst + (size_t)y * outw * 4))[x];

            if (p32 != conv[p16]) {
                same = 0;
                break;
            }
        }
    /* Guard against a vacuous pass: if the source really was filtered, some
       output pixel differs from the source pixel it came from. */
    for (u4 y = 0; y < outh && !filtered; y++)
        for (u4 x = 0; x < outw; x++)
            if (snap[(size_t)y * outw + x]
                != src_pixels[(y / (outh / H)) * SRC_LINE + x / (outw / W)]) {
                filtered = 1;
                break;
            }
    ZT_CHECK(same);
    ZT_CHECK(filtered);
    free(snap);
}

static void test_32b_matches_16b(void)
{
    check_32_matches_16("hq2x_32b runs the same rules as hq2x_16b", hq2x_16b,
        hq2x_32b, DSTW, H * 2);
}

static void test3_32b_matches_16b(void)
{
    check_32_matches_16("hq3x_32b runs the same rules as hq3x_16b", hq3x_16b,
        hq3x_32b, DSTW3, H * 3);
}

/* Both 32-bit entry points must honour the same two guards the 16-bit ones do. */
static void test_32b_guards(void)
{
    u4 const* const conv = (u4 const*)BitConv32Ptr;

    ZT_SECTION("hq2x_32b: curblank 0x40 writes nothing");
    setup();
    NumBytesPerLine = DSTW * 4;
    fill(0x1234);
    memset(dst, 0xAB, (size_t)DSTW * 4 * H * 2);
    curblank = 0x40;
    hq2x_32b();
    int clean = 1;
    for (size_t i = 0; i < (size_t)DSTW * 4 * H * 2; i++)
        if (dst[i] != 0xAB)
            clean = 0;
    ZT_CHECK(clean);

    ZT_SECTION("hq2x_32b: filter off replicates each pixel 2x2");
    setup();
    NumBytesPerLine = DSTW * 4;
    fill_edges();
    hqFilter = 0;
    hq2x_32b();
    int doubled = 1;
    for (u4 y = 0; y < H && doubled; y++)
        for (u4 x = 0; x < W; x++) {
            u4 const want = conv[src_pixels[y * SRC_LINE + x]];
            u4 const* const r0 = (u4 const*)(dst + (size_t)(y * 2) * DSTW * 4);
            u4 const* const r1 = (u4 const*)(dst + (size_t)(y * 2 + 1) * DSTW * 4);

            if (r0[x * 2] != want || r0[x * 2 + 1] != want || r1[x * 2] != want
                || r1[x * 2 + 1] != want) {
                doubled = 0;
                break;
            }
        }
    ZT_CHECK(doubled);
}

static void test4_flat(void)
{
    ZT_SECTION("hq4x: a flat field survives the filter unchanged");
    setup();
    NumBytesPerLine = PITCH4;
    fill(0x4A69);
    hq4x_16b();
    int ok = 1;
    for (u4 y = 0; y < H * 4; y++)
        for (u4 x = 0; x < DSTW4; x++)
            if (((u2*)(dst + (size_t)y * PITCH4))[x] != 0x4A69)
                ok = 0;
    ZT_CHECK(ok);
}

static void test4_filter_off_quadruples(void)
{
    ZT_SECTION("hq4x: hqFilter 0 falls back to pixel quadrupling");
    setup();
    NumBytesPerLine = PITCH4;
    fill_edges();
    hqFilter = 0;
    hq4x_16b();
    int ok = 1;
    for (u4 y = 0; y < H && ok; y++)
        for (u4 x = 0; x < W; x++) {
            u2 const want = src_pixels[y * SRC_LINE + x];

            for (unsigned ry = 0; ry < 4 && ok; ry++)
                for (unsigned rx = 0; rx < 4; rx++)
                    if (((u2*)(dst + (size_t)(y * 4 + ry) * PITCH4))[x * 4 + rx]
                        != want) {
                        ok = 0;
                        break;
                    }
        }
    ZT_CHECK(ok);
}

/* The rule set must actually blend: on an edge, some output pixel is neither
   of the two source colours. A block scaler cannot produce that. */
static void test4_edge_interpolates(void)
{
    ZT_SECTION("hq4x: an edge is interpolated, within the two colours it spans");
    setup();
    NumBytesPerLine = PITCH4;
    u2 const a = 0x0000, b = 0xFFFF;
    /* Diagonal: hqx keeps an axis-aligned edge sharp by design and only
       interpolates across a slope, so a vertical edge proves nothing here. */
    for (u4 y = 0; y < H; y++)
        for (u4 x = 0; x < W; x++)
            src_pixels[y * SRC_LINE + x] = (x + y < 128) ? b : a;
    hq4x_16b();
    int blended = 0, inrange = 1;
    for (u4 y = 0; y < H * 4; y++)
        for (u4 x = 0; x < DSTW4; x++) {
            u2 const p = ((u2*)(dst + (size_t)y * PITCH4))[x];

            if (p != a && p != b)
                blended = 1;
            if (((p >> 11) & 0x1Fu) > 0x1Fu || ((p >> 5) & 0x3Fu) > 0x3Fu)
                inrange = 0;
        }
    ZT_CHECK(blended);
    ZT_CHECK(inrange);
}

static void test4_no_overrun(void)
{
    ZT_SECTION("hq4x: writes stay inside the 1024x896 picture");
    setup();
    NumBytesPerLine = PITCH4;
    size_t const used = (size_t)PITCH4 * H * 4;
    memset(dst + used, 0x5A, 4096);
    fill_edges();
    hq4x_16b();
    int clean = 1;
    for (size_t i = 0; i < 4096; i++)
        if (dst[used + i] != 0x5A)
            clean = 0;
    ZT_CHECK(clean);
}

static void test4_32b_matches_16b(void)
{
    check_32_matches_16("hq4x_32b runs the same rules as hq4x_16b", hq4x_16b,
        hq4x_32b, DSTW4, H * 4);
}

/* The interpolation helpers carry the filter's arithmetic: which of the two or
   three colours a blended pixel leans towards, and by how much. Nothing else
   in this file pins them - the structural tests pass with any weights, and
   tools/hqxport.py only checks which helper each rule calls. So blend black
   with white and check where the result lands, which fixes each weight and
   catches a swapped pair.

   Weights, from the assembly video/hq*.asm: interp1 (3a+b)/4, interp2
   (2a+b+c)/4, interp3 (7a+b)/8, interp5 (a+b)/2, interp6 (5a+2b+c)/8,
   interp7 (6a+b+c)/8, interp8 (5a+3b)/8, interp9 (2a+3b+3c)/8 and
   interp10 (14a+b+c)/16. */
static void check_blend(char const* what, u2 got, double b_share)
{
    /* Red is the top 5 bits, so a share of white lands at share * 31. */
    unsigned const r = (got >> 11) & 0x1Fu;
    double const want = b_share * 31.0;

    if (r + 1.0 < want || r > want + 1.0) {
        fprintf(stderr, "    FAIL [%s]: red=%u, expected about %.1f\n", what, r, want);
        zt_failures++;
    } else {
        zt_passes++;
    }
}

static void test_interp_weights(void)
{
    u2 const a = 0x0000, b = 0xFFFF; /* all-black and all-white */

    ZT_SECTION("interpolation helpers weight their inputs as the asm did");
    setup();
    /* Two arguments: the second's share of the result. */
    check_blend("interp1 (3a+b)/4", interp1(a, b), 1.0 / 4.0);
    check_blend("interp3 (7a+b)/8", interp3(a, b), 1.0 / 8.0);
    check_blend("interp5 (a+b)/2", interp5(a, b), 1.0 / 2.0);
    check_blend("interp8 (5a+3b)/8", interp8(a, b), 3.0 / 8.0);
    /* Three arguments, with the last two the same colour. */
    check_blend("interp2 (2a+b+c)/4", interp2(a, b, b), 2.0 / 4.0);
    check_blend("interp6 (5a+2b+c)/8", interp6(a, b, b), 3.0 / 8.0);
    check_blend("interp7 (6a+b+c)/8", interp7(a, b, b), 2.0 / 8.0);
    check_blend("interp9 (2a+3b+3c)/8", interp9(a, b, b), 6.0 / 8.0);
    check_blend("interp10 (14a+b+c)/16", interp10(a, b, b), 2.0 / 16.0);

    ZT_SECTION("an interpolation of one colour with itself is that colour");
    ZT_CHECK(interp1(0x4A69, 0x4A69) == 0x4A69);
    ZT_CHECK(interp5(0x4A69, 0x4A69) == 0x4A69);
    ZT_CHECK(interp2(0x4A69, 0x4A69, 0x4A69) == 0x4A69);
}

/* The flat-neighbourhood shortcut in the filters assumes every rule maps
   nine equal pixels back to that colour. True for all of them, all levels. */
static void test_flat_shortcut_exact(void)
{
    int bad = 0;

    ZT_SECTION("every colour survives a flat 3x3 at every level");
    for (u4 c = 0; c < 65536u; c++) {
        u2 w[10], q4[16], q3[9], q2[4];

        for (int i = 1; i < 10; i++)
            w[i] = (u2)c;
        hq4x_sixteen(w, q4);
        hq3x_nine(w, q3);
        hq2x_quad(w, q2);
        for (int i = 0; i < 16; i++)
            bad += q4[i] != c;
        for (int i = 0; i < 9; i++)
            bad += q3[i] != c;
        for (int i = 0; i < 4; i++)
            bad += q2[i] != c;
    }
    ZT_CHECK_INT(bad, 0);
}

int main(void)
{
    printf("ZSNES2 hqx filter tests\n");
    build_tables();

    test_blank();
    test_flat();
    test_flat_shortcut_exact();
    test_filter_off_doubles();
    test_hires_line_doubles();
    test_edge_interpolates();
    test_threshold();
    test_no_overrun();

    test3_flat();
    test3_filter_off_triples();
    test3_edge_interpolates();
    test3_no_overrun();

    test4_flat();
    test4_filter_off_quadruples();
    test4_edge_interpolates();
    test4_no_overrun();

    test_32b_matches_16b();
    test3_32b_matches_16b();
    test4_32b_matches_16b();
    test_interp_weights();
    test_32b_guards();

    ZT_RESULTS();
}
