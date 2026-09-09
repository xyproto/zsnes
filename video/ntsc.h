/* NTSC video filter */

#ifndef NTSC_H
#define NTSC_H

#include "../types.h"
#include "snes_ntsc/snes_ntsc.h"

extern u1 NTSCPresetVar; /* 0 to 3 */

/* What the filter makes from one 256-pixel line. It consumes three input
   pixels for every seven it writes, so a wider request reads past the end of
   the line: at 640 it runs 15 pixels into the right border, and past about 700
   it leaves the line altogether. Ask for this width and scale the result. */
enum { NTSC_OUT_WIDTH = SNES_NTSC_OUT_WIDTH(256) };

/* (Re)initialize filter with new NTSC settings above */
void NTSCFilterInit(void);

/* Draw current image to specified output pixels */
void NTSCFilterDraw(int out_width, int out_height, int out_pitch, unsigned char* rgb16_out);

#endif
