#include "filter.h"

#include "2xsaiw.h"
#include "cfg.h"
#include "copyvwin.h"
#include "ntsc.h"
#include "procvidc.h"

extern u1* vidbuffer;
extern u2 resolutn;
extern u1 FilteredGUI;
extern u1 GUIOn2;

void hq2x_16b(void);
void hq3x_16b(void);
void hq4x_16b(void);

VideoFilter VideoFilterGet(void)
{
    if (NTSCFilter) {
        return VFILTER_NTSC;
    }
    if (hqFilter) {
        return hqFilterlevel >= 4 ? VFILTER_HQ4X
            : hqFilterlevel == 3  ? VFILTER_HQ3X
                                  : VFILTER_HQ2X;
    }
    switch (En2xSaI) {
    case 1:
        return VFILTER_2XSAI;
    case 2:
        return VFILTER_SUPEREAGLE;
    case 3:
        return VFILTER_SUPER2XSAI;
    default:
        return VFILTER_NONE;
    }
}

/* The one place the settings are written, so the rule that at most one filter
   is on lives here and nowhere else. */
static void apply(VideoFilter const f)
{
    En2xSaI = 0;
    hqFilter = 0;
    NTSCFilter = 0;
    switch (f) {
    case VFILTER_2XSAI:
        En2xSaI = 1;
        break;
    case VFILTER_SUPEREAGLE:
        En2xSaI = 2;
        break;
    case VFILTER_SUPER2XSAI:
        En2xSaI = 3;
        break;
    case VFILTER_HQ2X:
        hqFilter = 1;
        hqFilterlevel = 2;
        break;
    case VFILTER_HQ3X:
        hqFilter = 1;
        hqFilterlevel = 3;
        break;
    case VFILTER_HQ4X:
        hqFilter = 1;
        hqFilterlevel = 4;
        break;
    case VFILTER_NTSC:
        NTSCFilter = 1;
        break;
    default:
        return;
    }
    /* The smoothing and the software scanlines draw their own picture, so they
       cannot be combined with one of these. */
    antienab = 0;
    scanlines = 0;
}

void VideoFilterSet(VideoFilter const f)
{
    apply(f);
    if (f == VFILTER_NTSC) {
        NTSCFilterInit();
    }
    Clear2xSaIBuffer();
}

void VideoFilterNormalise(void)
{
    apply(VideoFilterGet());
}

void VideoFilterToggle(VideoFilter const f)
{
    VideoFilterSet(VideoFilterGet() == f ? VFILTER_NONE : f);
}

char const* VideoFilterName(VideoFilter const f)
{
    static char const* const names[VFILTER_COUNT] = { "None", "2xSaI",
        "Super Eagle", "Super 2xSaI", "hq2x", "hq3x", "hq4x", "NTSC" };

    return f < VFILTER_COUNT ? names[f] : names[VFILTER_NONE];
}

VideoFilterPicture VideoFilterOutput(VideoFilter const f)
{
    VideoFilterPicture p;

    p.scale = f == VFILTER_HQ4X ? 4 : f == VFILTER_HQ3X ? 3
                                                        : 2;
    p.w = f == VFILTER_NTSC ? NTSC_OUT_WIDTH : 256 * p.scale;
    p.h = (int)resolutn * p.scale;
    return p;
}

/* Kreed's filters take one line at a time and read a row above and two below,
   which the vidbuffer border already provides. */
static void draw_2xsai(LineFilter* const line, u1* const dst, int const pitch)
{
    u2* const base = (u2*)vidbuffer + VID_FIRST;
    unsigned y;

    if (!FilteredGUI && GUIOn2 == 1) {
        /* An unfiltered GUI is drawn doubled, as the hq entry points do it;
           hqFilter is off here, so this is their plain doubler. */
        hq2x_16b();
        return;
    }
    for (y = 0; y < resolutn; y++) {
        line(base + (size_t)y * VID_STRIDE, NULL, VID_STRIDE * 2, 256,
            dst + (size_t)y * 2 * (size_t)pitch, (u4)pitch);
    }
}

int VideoFilterDraw(VideoFilter const f, void* const dst, int const pitch,
    size_t const capacity)
{
    VideoFilterPicture const p = VideoFilterOutput(f);

    if (f == VFILTER_NONE || !dst || pitch < p.w * 2
        || capacity < (size_t)pitch * (size_t)p.h) {
        return 0;
    }
    if (f == VFILTER_NTSC) {
        NTSCFilterDraw(p.w, p.h, pitch, (u1*)dst);
        return 1;
    }

    AddEndBytes = (u4)(pitch - p.w * 2);
    NumBytesPerLine = (u4)pitch;
    WinVidMemStart = (u1*)dst;
    switch (f) {
    case VFILTER_HQ4X:
        hq4x_16b();
        break;
    case VFILTER_HQ3X:
        hq3x_16b();
        break;
    case VFILTER_HQ2X:
        hq2x_16b();
        break;
    case VFILTER_SUPEREAGLE:
        draw_2xsai(_2xSaISuperEagleLine, (u1*)dst, pitch);
        break;
    case VFILTER_SUPER2XSAI:
        draw_2xsai(_2xSaISuper2xSaILine, (u1*)dst, pitch);
        break;
    default:
        draw_2xsai(_2xSaILine, (u1*)dst, pitch);
        break;
    }
    return 1;
}
