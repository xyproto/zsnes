#ifndef GUI_H
#define GUI_H

#include "../types.h"

extern void DrawSnow();
extern void GUIUnBuffer();
extern void InitGUI();

extern char  GUIPrevMenuData[];
extern char* GUICMessage;
extern u1    CheatOn;
extern u1    GUICCFlash;
extern u1    GUIFontData1[141][5];
extern u1    GUIFontData[141][5];
extern u1    GUIHold;
extern u1    GUILDFlash;
extern u1    GUIOn2;
extern u1    GUIOn;
extern u1    GUIPalConv;
extern u1    GUIQuit;
extern u1    GUIReset;
extern u1    GUIcmenupos;
extern u1    GUIcrowpos;
extern u1    GUIcwinpress;
extern u1    GUIescpress;
extern u1    GUIpclicked;
extern u1    GUIpmenupos;
extern u1    GUItextcolor[5];
extern u1    GUIwinactiv[];
extern u1    GUIwinorder[22];
extern u1    GUIwinptr;
extern u1    MouseDis;
extern u1    MousePRClick;
extern u1    OkaySC;
extern u1    ShowTimer;
extern u1    SnowVelDist[800];
extern u1    cwindrawn;
extern u1*   GUICYLocPtr;
extern u2    PrevResoln;
extern u2    SnowData[800];
extern u4    GUICTimer;
extern u4    GUIDClickTL;
extern u4    GUIMenuD;
extern u4    GUIMenuL;
extern u4    GUIMenuR;
extern u4    GUIScrolTim1;
extern u4    GUIt1cc;
extern u4    GUIwinposxo[22];
extern u4    GUIwinposyo[22];
extern u4    NumCheats;
extern u4    SnowMover;
extern u4    SnowTimer;
extern u4    StartLL;
extern u4    StartLR;

#endif
