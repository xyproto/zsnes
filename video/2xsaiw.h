#ifndef H2XSAIW_H
#define H2XSAIW_H

#include "../types.h"

typedef void LineFilter(u2*, u1*, u4, u4, u1*, u4);
extern LineFilter _2xSaISuperEagleLine;
extern LineFilter _2xSaISuper2xSaILine;
extern LineFilter _2xSaILine;

extern u4 colorMask[2];
extern u4 lowPixelMask[2];
extern u4 qcolorMask[2];
extern u4 qlowpixelMask[2];

#endif
