/* The tube: scanlines, the vibrancy that puts back the light they take out,
 * and the bloom bright areas spill. Three passes over a 565 image and nothing
 * else, so any display path can run them over what it just composed. Settings
 * are the Retro tab's (sl_intensity, sl_vibrancy, BloomLevel in cfg.psr).
 *
 * `pitch` is in pixels: a surface's rows are not always `w` apart. */

#ifndef ZSNES_VIDEO_CRT_H
#define ZSNES_VIDEO_CRT_H

#include "../types.h"
#include "filter.h"

/* The largest picture these will touch: the same one the filters can make. */
enum { CRT_MAX_W = VFILTER_MAX_W,
    CRT_MAX_H = VFILTER_MAX_H };

/* Work out what each part of the picture spills, before the scanlines dim it:
   what spills is a property of the picture, not of which row of the beam
   pattern a pixel happened to land on. */
void CrtBloomBuild(u2 const* px, int w, int h, int pitch);

/* Scanlines and vibrancy. `vscale` is output rows per source scanline, which
   is what the beam pattern steps by; `scanlines` turns the beam off and leaves
   the brightness, for a picture that dims its own alternate rows. */
void CrtShade(u2* px, int w, int h, int pitch, int vscale, int scanlines);

/* Add the spill back, clipped into 565. */
void CrtBloomApply(u2* px, int w, int h, int pitch);

/* CrtShade and CrtBloomApply as one pass over the picture. */
void CrtShadeBloom(u2* px, int w, int h, int pitch, int vscale, int scanlines);

/* The spill at one pixel, for a caller with somewhere above white to put it. */
float CrtBloomAt(int x, int y, int w);

/* All of the above, in order, for a caller that has not. */
void CrtPass(u2* px, int w, int h, int pitch, int vscale, int scanlines);

#endif
