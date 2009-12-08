#ifndef INTRF_H
#define INTRF_H

#include "types.h"

extern void GUIDeInit();
extern void GUIInit();
extern void Get_MousePositionDisplacement();
extern void JoyRead();
extern void vidpastecopyscr();

extern u4 Init_Mouse(void);
extern u1 GUIWFVID[];
extern u4 NumVideoModes;

#ifndef __MSDOS__
extern u4 converta;
#endif

#endif
