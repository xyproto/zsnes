#ifndef C_VCACHE_H
#define C_VCACHE_H

#include "types.h"

void cachevideo(void);
void genfulladdtab(void);
void ConvertToAFormat(void);

extern u1 SloMo;
extern u1 colormodedef[][4];
extern u1 curblank;
extern u1 curcolbg[4];
extern u1 hiresstuff;
extern u1 osm2dis;
extern u1* colormodeofs;
extern u2 curbgofs[4];
extern u4 CSprWinPtr;
extern u4 sramb4save;

#endif
