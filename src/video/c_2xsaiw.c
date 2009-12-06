#include "2xsaiw.h"
#include "c_2xsaiw.h"


u4 Init_2xSaIMMX(u4 const format)
{
	u4 full;
	u4 color;
	switch (format) // PixelFormat
	{
		case 555: full = 0x7FFF7FFF; color = 0x7BDE7BDE; break;
		case 565: full = 0xFFFFFFFF; color = 0xF7DEF7DE; break;
		default:  return 1;
	}

	colorMask[0]       = color;
	colorMask[1]       = color;
	u4 const lowPixel  = color ^ full;
	lowPixelMask[0]    = lowPixel;
	lowPixelMask[1]    = lowPixel;
	u4 const qcolor    = color & (color << 1);
	qcolorMask[0]      = qcolor;
	qcolorMask[1]      = qcolor;
	u4 const qlowpixel = qcolor ^ full;
	qlowpixelMask[0]   = qlowpixel;
	qlowpixelMask[1]   = qlowpixel;
	return 0;
}
