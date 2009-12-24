#ifndef LINK_H
#define LINK_H

#include "types.h"

#ifndef __MSDOS__
s4   GetMouseButton(void);
s4   GetMouseMoveX(void);
s4   GetMouseMoveY(void);
s4   GetMouseX(void);
s4   GetMouseY(void);
void CheckTimers(void);
void Start36HZ(void);
void Start60HZ(void);
void Stop36HZ(void);
void Stop60HZ(void);
void UpdateVFrame(void);
void clearwin(void);
void drawscreenwin(void);
void initwinvideo(void);

extern u1 MouseButton;
extern u1 T36HZEnabled;
#endif

#endif
