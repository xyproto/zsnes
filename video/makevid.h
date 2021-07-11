#ifndef MAKEVID_H
#define MAKEVID_H

#include "../macros.h"
#include "../types.h"

typedef struct SpriteInfo {
    u2 x;
    u1* obj __attribute__((packed, aligned(2))); // XXX unaligned
    u1 pal;
    u1 status;
} SpriteInfo;
STATIC_ASSERT(sizeof(SpriteInfo) == 8);

extern void dualstartprocess();
extern void makedualwinsp();

extern SpriteInfo* currentobjptr;
extern u1 a16x16xinc;
extern u1 a16x16yinc;
extern u1 alreadydrawn;
extern u1 bg3high2;
extern u1 bgcoloradder;
extern u1 bshifter;
extern u1 csprbit;
extern u1 csprprlft;
extern u1 curbgnum;
extern u1 curbgpr; // 00h = low priority, 20h = high priority
extern u1 curmosaicsz;
extern u1 cwinenabm;
extern u1 drawn;
extern u1 dualwinbg;
extern u1 dualwinsp;
extern u1 extbgdone;
extern u1 hirestiledat[256];
extern u1 maxbr;
extern u1 pwinbgenab;
extern u1 pwinspenab;
extern u1 res512switch;
extern u1 temp;
extern u1 winon;
extern u1 winonbtype;
extern u1 winonsp;
extern u1 winonstype;
extern u1* bg1cachloc[4];
extern u1* bgofwptr;
extern u1* cursprloc;
extern u1* curvidoffset;
extern u1* cwinptr;
extern u1* dwinptrproc;
extern u1* tempcach; // points to cached memory
extern u1* winptrref;
extern u2 MosaicYAdder[16];
extern u2 curtileptr;
extern u2* bg1tdabloc[4];
extern u2* bg1tdatloc[4];
extern u2* temptile; // points to the secondary video pointer
extern u4 bg1vbufloc[4];
extern u4 bg1xposloc[4];
extern u4 bg1yaddval[4];
extern u4 bgptr;
extern u4 bgptrc;
extern u4 bgptrd;
extern u4 bgptrx1;
extern u4 bgptrx2;
extern u4 bgsubby;
extern u4 pwinbgtype;
extern u4 pwinsptype;
extern u4 yadder;
extern u4 yrevadder;

#endif
