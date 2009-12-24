#ifndef INTRF_H
#define INTRF_H

#include "types.h"

extern u1 GUIWFVID[];
extern u4 NumVideoModes;

#ifndef __MSDOS__
extern u4          CurKeyReadPos;
extern u4          KeyBuffer[16];
extern u4          converta;
extern u4 volatile CurKeyPos;
#endif

#endif
