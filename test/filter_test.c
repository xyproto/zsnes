/*
 * The shared filter front end (video/filter.c).
 *
 * What every display path relies on: the picture VideoFilterOutput promises is
 * the picture VideoFilterDraw fills, and nothing outside it is touched. The
 * old size-matched blitters wrote past the end of the surface whenever a game
 * turned on overscan and the picture grew from 224 lines to 239, so both
 * heights are covered here.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "../video/filter.h"
#include "cfg.h"
#include "zstest.h"

/* Everything the filters reach for; the emulator owns these normally. */
u1* vidbuffer;
u1* vidbufferofsb;
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
u1 FilteredGUI;
u1 newengen;
u1 res512switch;
u2 PrevResoln;

void Clear2xSaIBuffer(void) { }

/* The configuration the filters read. cfg.h declares them; the emulator's
   generated cfg.o defines them, and this links without it. */
unsigned char antienab;
unsigned char NTSCFilter;
unsigned char NTSCBlend;
unsigned char NTSCRef;
signed char NTSCHue;
signed char NTSCSat;
signed char NTSCCont;
signed char NTSCBright;
signed char NTSCSharp;
signed char NTSCGamma;
signed char NTSCRes;
signed char NTSCArt;
signed char NTSCFringe;
signed char NTSCBleed;
signed char NTSCWarp;
unsigned char En2xSaI;
unsigned char hqFilter;
unsigned char hqFilterlevel;
unsigned char scanlines;

#define SRC_STRIDE 288
#define GUARD 0xC3

static u1* dst;
static size_t dst_bytes;

static void build_tables(void)
{
    BitConv32Ptr = malloc(65536u * 4u + 4096u);
    RGBtoYUVPtr = malloc(65536u * 4u + 4096u);
    for (u4 i = 0; i < 65536u; i++) {
        ((u4*)BitConv32Ptr)[i] = ((i & 0xF800u) << 8) + ((i & 0x07E0u) << 5)
            + ((i & 0x001Fu) << 3) + 0xFF000000u;
        ((u4*)RGBtoYUVPtr)[i] = i * 2654435761u;
    }
    HalfTrans[0] = 0xF7DEF7DEu;
    HalfTrans[1] = 0xF7DEF7DEu;
}

static void setup(u2 const lines)
{
    u4 i;

    /* Room for both fields and the priority tail the mode 7 writers use. */
    vidbuffer = calloc(1, 512u * 296u * 4u + 4096u + 512u * 296u + 75036u * 4u);
    vidbufferofsb = calloc(1, 288u * 2u * 256u + 4096u);
    for (i = 0; i < 512u * 296u * 2u; i++) {
        ((u2*)vidbuffer)[i] = (u2)(i * 2654435761u >> 16);
    }
    resolutn = lines;
    curblank = 0;
    FilteredGUI = 1;
    GUIOn = 0;
    GUIOn2 = 0;
    newengen = 1;
    memset(hirestiledat, 0, sizeof hirestiledat);
    memset(SpecialLine, 0, sizeof SpecialLine);
}

/* One filter into a buffer that is exactly the size it asked for, followed by
   a guard band. */
static void run_one(VideoFilter const f, u2 const lines, char const* const label)
{
    VideoFilterPicture pic;
    size_t used, i;
    int ok;

    ZT_SECTION(label);
    setup(lines);
    VideoFilterSet(f);
    ZT_CHECK(VideoFilterGet() == f);

    pic = VideoFilterOutput(f);
    ZT_CHECK(pic.w > 0 && pic.w <= VFILTER_MAX_W);
    ZT_CHECK(pic.h > 0 && pic.h <= VFILTER_MAX_H);
    ZT_CHECK(pic.h == lines * pic.scale);

    used = (size_t)pic.w * 2u * (size_t)pic.h;
    dst_bytes = used + 4096u;
    dst = malloc(dst_bytes);
    memset(dst, GUARD, dst_bytes);

    ZT_CHECK(VideoFilterDraw(f, dst, pic.w * 2, used) == 1);

    ok = 1;
    for (i = used; i < dst_bytes; i++) {
        if (dst[i] != GUARD) {
            ok = 0;
            break;
        }
    }
    if (!ok) {
        printf("    wrote %zu bytes past a %zu byte picture\n", i - used + 1, used);
    }
    ZT_CHECK(ok);

    /* And it drew something: a picture left entirely at the guard value means
       the filter quietly did nothing. */
    ok = 0;
    for (i = 0; i < used; i++) {
        if (dst[i] != GUARD) {
            ok = 1;
            break;
        }
    }
    ZT_CHECK(ok);

    free(dst);
    free(vidbuffer);
    free(vidbufferofsb);
}

static void test_sizes(void)
{
    ZT_SECTION("a picture that does not fit is refused, not written");
    setup(239);
    VideoFilterSet(VFILTER_HQ4X);
    {
        VideoFilterPicture const pic = VideoFilterOutput(VFILTER_HQ4X);
        size_t const used = (size_t)pic.w * 2u * (size_t)pic.h;
        u1* const small = malloc(used);

        memset(small, GUARD, used);
        /* One byte short, and one row too narrow. */
        ZT_CHECK(VideoFilterDraw(VFILTER_HQ4X, small, pic.w * 2, used - 1) == 0);
        ZT_CHECK(VideoFilterDraw(VFILTER_HQ4X, small, pic.w * 2 - 2, used) == 0);
        ZT_CHECK(small[0] == GUARD);
        free(small);
    }
    ZT_CHECK(VideoFilterDraw(VFILTER_NONE, NULL, 0, 0) == 0);
    free(vidbuffer);
    free(vidbufferofsb);
}

static void test_exclusive(void)
{
    ZT_SECTION("only one filter is on at a time");
    setup(224);
    antienab = 1;
    scanlines = 3;
    VideoFilterSet(VFILTER_HQ3X);
    ZT_CHECK(hqFilter == 1 && hqFilterlevel == 3);
    ZT_CHECK(En2xSaI == 0 && NTSCFilter == 0);
    ZT_CHECK(antienab == 0 && scanlines == 0);

    VideoFilterSet(VFILTER_SUPER2XSAI);
    ZT_CHECK(En2xSaI == 3 && hqFilter == 0 && NTSCFilter == 0);

    VideoFilterToggle(VFILTER_SUPER2XSAI);
    ZT_CHECK(VideoFilterGet() == VFILTER_NONE);
    ZT_CHECK(En2xSaI == 0 && hqFilter == 0 && NTSCFilter == 0);
    free(vidbuffer);
    free(vidbufferofsb);
}

int main(void)
{
    static VideoFilter const all[] = { VFILTER_2XSAI, VFILTER_SUPEREAGLE,
        VFILTER_SUPER2XSAI, VFILTER_HQ2X, VFILTER_HQ3X, VFILTER_HQ4X,
        VFILTER_NTSC };
    static u2 const heights[] = { 224, 239 };
    unsigned i, j;
    char label[64];

    printf("ZSNES2 video filter tests\n");
    build_tables();
    for (j = 0; j < sizeof heights / sizeof *heights; j++) {
        for (i = 0; i < sizeof all / sizeof *all; i++) {
            snprintf(label, sizeof label, "%s stays inside its picture at %u lines",
                VideoFilterName(all[i]), (unsigned)heights[j]);
            run_one(all[i], heights[j], label);
        }
    }
    test_sizes();
    test_exclusive();
    ZT_RESULTS();
}
