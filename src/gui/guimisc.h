#ifndef GUIMISC_H
#define GUIMISC_H

// Joystick setting (display) routines, SNES Reset Function

#include "../types.h"

void SetAllKeys(void);
void CalibrateDev1(void);
void SetDevice(void);
void GUIDoReset(void);
void GetCoords(u2 port);
void GetCoords3(u2 port);

extern u1 JoyExists2;
extern u1 JoyExists;
extern u4 JoyMaxX;
extern u4 JoyMaxY;
extern u4 JoyMinX;
extern u4 JoyMinY;
extern u4 JoyX2;
extern u4 JoyX;
extern u4 JoyY2;
extern u4 JoyY;

#endif
