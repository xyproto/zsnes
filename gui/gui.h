#ifndef GUI_H
#define GUI_H

#include "../macros.h"
#include "../types.h"

typedef struct ComboData {
    char name[20];
    u1 combo[42];
    u2 key;
    u1 player;
    u1 ff;
} ComboData;
STATIC_ASSERT(sizeof(ComboData) == 66);

extern ComboData CombinDataGlob[50];
extern ComboData CombinDataLocl[50];

extern char GUIPrevMenuData[];
extern s4 GUIDClCEntry;
extern u1 CheatOn;
extern u1 CurPalSelect;
extern u1 EEgg;
extern u1 ForceHiLoROM;
extern u1 ForceROMTiming;
extern u1 GUICBHold2;
extern u1 GUICBHold;
extern u1 GUICCFlash;
extern u1 GUICResetPos;
extern u1 GUICStatePos;
extern u1 GUIHold;
extern u1 GUILDFlash;
extern u1 GUIOn2;
extern u1 GUIOn;
extern u1 GUIQuit;
extern u1 GUIReset;
extern u1 GUIcmenupos;
extern u1 GUIcrowpos;
extern u1 GUIescpress;
extern u1 GUIpclicked;
extern u1 GUIpmenupos;
extern u1 GUIwinactiv[];
extern u1 GUIwinorder[22];
extern u1 cheatdata[28 * 255 + 56];
extern u1 cplayernum;
extern u1 cwindrawn;
extern u1* GUICYLocPtr;
extern u2 GUImouseposx;
extern u2 GUImouseposy;
extern u4 CalibXmax209;
extern u4 CalibXmax;
extern u4 CalibXmin209;
extern u4 CalibXmin;
extern u4 CalibYmax209;
extern u4 CalibYmax;
extern u4 CalibYmin209;
extern u4 CalibYmin;
extern u4 GUICHold;
extern u4 GUIDClCWin;
extern u4 GUIDClickTL;
extern u4 GUIHoldXlimL;
extern u4 GUIHoldXlimR;
extern u4 GUIHoldYlim;
extern u4 GUIHoldYlimR;
extern u4 GUIHoldx;
extern u4 GUIHoldxm;
extern u4 GUIHoldy;
extern u4 GUIHoldym;
extern u4 GUIMenuD;
extern u4 GUIMenuL;
extern u4 GUIMenuR;
extern u4 GUIScrolTim1;
extern u4 GUIScrolTim2;
extern u4 NumCheats;
extern u4 cloadmaxlen;

#endif
