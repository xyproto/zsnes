#ifndef C_GUI_H
#define C_GUI_H

#include "../types.h"

void GUI36hzcall(void);
void StartGUI(void);
void guimencodermsg(void);
void guilamemsg(void);
void guiprevideo(void);
void guipostvideo(void);
void CheckMenuItemHelp(u4 id);
void GUITryMenuItem(void);
void GUIProcStates(void);
void GUIProcReset(void);
void GUIUnBuffer(void);
void GUISetPal(void);
void convertnum(char *dst, u4 val);
void converthex(char *dst, u4 val, u4 n /* bytes */);
void guicheaterror(void);
void DisplayBoxes(void);
void DisplayMenu(void);

void GUIBox3D(u4 const x1, u4 const y1, u4 const x2, u4 const y2);
void GUIOuttextShadowed(u4 const x, u4 const y, char const *const text);

u1 *GetAnyPressedKey(void);

extern u1 GUIFontData1[141][5];
extern u1 GUIwinptr;
extern u1 MouseDis;
extern u1 MousePRClick;
extern u1 ShowTimer;
extern u1 SnowVelDist[800];
extern u1 const GUIFontData[141][5];
extern u1 savecfgforce;
extern u2 GUICPC[256];
extern u2 SnowData[800];
extern u4 GUIwinsizex[22];
extern u4 GUIwinsizey[22];
extern u4 MsgGiftLeft;
extern u4 NumSnow;
extern u4 SantaPos;
extern u4 SnowTimer;

#endif
