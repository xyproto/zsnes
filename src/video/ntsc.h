/* NTSC video filter */

#ifndef NTSC_H
#define NTSC_H

#include "snes_ntsc/snes_ntsc.h"

/* Parameters vary from -100 to +100 */
extern signed char NTSCHue, NTSCSat, NTSCCont, NTSCBright, NTSCSharp, NTSCWarp;
extern unsigned char NTSCBlend; /* 0 or 1 */

/* (Re)initialize filter with new NTSC settings above */
void NTSCFilterInit();

/* Draw current image to specified output pixels */
void NTSCFilterDraw( int out_width, int out_height, int out_pitch, unsigned char* rgb16_out );

#endif
