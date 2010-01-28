#ifndef MAKEVID_H
#define MAKEVID_H

#include "../types.h"

extern void makewindow();
extern void makewindowsp();
extern void procbackgrnd();

extern u1  alreadydrawn;
extern u1  bg3high2;
extern u1  bgcoloradder;
extern u1  curbgnum;
extern u1  curbgpr; // 00h = low priority, 20h = high priority
extern u1  curmosaicsz;
extern u1  cwinenabm;
extern u1  drawn;
extern u1  hirestiledat[256];
extern u1  maxbr;
extern u1  res512switch;
extern u1  winon;
extern u1  winonsp;
extern u1* currentobjptr;
extern u1* cursprloc;
extern u1* curvidoffset;
extern u4  bg1cachloc[4];
extern u4  bg1tdabloc[4];
extern u4  bg1tdatloc[4];
extern u4  bg1vbufloc[4];
extern u4  bg1xposloc[4];
extern u4  bg1yaddval[4];

#endif
