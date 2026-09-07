// Looks good
#ifndef COPYVWIN_H
#define COPYVWIN_H

#include "../types.h"

void copy640x480x16bwin(void);

/* vidbuffer geometry. A line is VID_STRIDE pixels: 256 of picture between a
   16-pixel border either side, which the filters read as context. Scanline 0
   is the SNES pre-render line and never holds picture, so output row N comes
   from source line N+1 and the first visible pixel is at VID_FIRST.

   These exist because the same address used to be spelled five different ways
   across the display paths - as a u2 index here, as a byte offset there - and
   two of them were a line short, putting a stale row at the top of the screen. */
enum {
    VID_STRIDE = 288, /* pixels per line, borders included */
    VID_SKIP = 16, /* left border, in pixels */
    VID_FIRST = VID_STRIDE + VID_SKIP /* first visible pixel, in pixels */
};

extern u1* WinVidMemStart;
extern u4 AddEndBytes; // Number of bytes between each line
extern u4 NumBytesPerLine; // Total number of bytes per line (1024+AddEndBytes)

#endif
