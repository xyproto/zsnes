/* The picture filters, in one place: which one is on, how large a picture it
 * makes, and draw it here. Each display path used to answer those for itself,
 * gated on the window size it happened to meet - which is why a filter could
 * behave differently per backend, be unreachable in a small window, or change
 * when you went fullscreen. A backend now only finds somewhere to put it.
 *
 * The settings behind this are unchanged (En2xSaI, hqFilter, hqFilterlevel,
 * NTSCFilter in cfg.psr), so saved configurations keep working. */

#ifndef ZSNES_VIDEO_FILTER_H
#define ZSNES_VIDEO_FILTER_H

#include "../types.h"
#include <stddef.h>

/* At most one of these is on at a time. */
typedef enum {
    VFILTER_NONE = 0,
    VFILTER_2XSAI,
    VFILTER_SUPEREAGLE,
    VFILTER_SUPER2XSAI,
    VFILTER_HQ2X,
    VFILTER_HQ3X,
    VFILTER_HQ4X,
    VFILTER_NTSC,
    VFILTER_COUNT
} VideoFilter;

/* The picture a filter makes from the frame the emulator just drew: `scale`
   output rows per source scanline, which is what a scanline pass steps by. */
typedef struct {
    int w;
    int h;
    int scale;
} VideoFilterPicture;

VideoFilter VideoFilterGet(void);

/* Turn one on, turning the others off. VideoFilterToggle is what a checkbox
   wants: clicking the filter that is already on turns it off. */
void VideoFilterSet(VideoFilter f);
void VideoFilterToggle(VideoFilter f);

/* Settle the settings again after something outside here has changed them - a
   loaded configuration, or a video mode that does not offer what was on. Same
   rule as VideoFilterSet without its side effects, so it is cheap enough to
   call while drawing. */
void VideoFilterNormalise(void);

char const* VideoFilterName(VideoFilter f);

VideoFilterPicture VideoFilterOutput(VideoFilter f);

/* Draw into `dst`: `pitch` bytes per row, `capacity` in total. A picture that
   does not fit is refused rather than written past the end, which is what the
   old size-matched paths did on an overscan frame. */
int VideoFilterDraw(VideoFilter f, void* dst, int pitch, size_t capacity);

/* The largest picture any filter makes, for sizing a buffer that has to hold
   all of them: hq4x over a 239-line overscan frame. */
enum { VFILTER_MAX_W = 1024,
    VFILTER_MAX_H = 239 * 4 };

#endif
