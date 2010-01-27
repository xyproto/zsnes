#ifndef MAKEVID_H
#define MAKEVID_H

#include "../types.h"

extern void makewindowsp();
extern void procbackgrnd();

extern u1  alreadydrawn;
extern u1  bg3high2;
extern u1  curbgnum;
extern u1  curbgpr; // 00h = low priority, 20h = high priority
extern u1  cwinenabm;
extern u1  hirestiledat[256];
extern u1  maxbr;
extern u1  res512switch;
extern u1* currentobjptr;
extern u1* cursprloc;
extern u1* curvidoffset;

#endif
