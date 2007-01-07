/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* NTSC video filter */

#ifndef NTSC_H
#define NTSC_H

#include "snes_ntsc/snes_ntsc.h"

/* Parameters vary from -100 to +100 */
extern signed char NTSCHue, NTSCSat, NTSCCont, NTSCWarp, NTSCBright, NTSCSharp, NTSCGamma, NTSCRes, NTSCArt, NTSCFringe, NTSCBleed;
extern unsigned char NTSCBlend; /* 0 or 1 */
extern unsigned char NTSCPresetVar; /* 0 to 3 */

/* (Re)initialize filter with new NTSC settings above */
void NTSCFilterInit();

/* Draw current image to specified output pixels */
void NTSCFilterDraw( int out_width, int out_height, int out_pitch, unsigned char* rgb16_out );

#endif
