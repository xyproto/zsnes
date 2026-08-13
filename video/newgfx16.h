#ifndef NEWGFX16_H
#define NEWGFX16_H

#include "../types.h"

extern eop* mosjmptab16b[15];
extern eop* mosjmptab16bntms[15];
extern eop* mosjmptab16bt[15];
extern eop* mosjmptab16btms[15];
extern u4 BackAreaAdd;
extern u4 BackAreaFillCol;
extern u4 BackAreaUnFillCol;
extern u4 HalfTransB[2];
extern u4 HalfTransC[2];
extern u4 HalfTrans[2];
extern u4 UnusedBitXor[2];
extern u4 UnusedBit[2];
extern u4 cpalptrng;
extern u4 ngbposng;
extern u4 nggposng;
extern u4 ngmsdraw;
extern u4 ngrposng;
extern u4 palchanged;
extern u1 prevbrightdc;

#endif
