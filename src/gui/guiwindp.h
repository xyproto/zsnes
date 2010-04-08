#ifndef GUIWINDP_H
#define GUIWINDP_H

#include "../types.h"

extern void CSAddFlash();
extern void CSRemoveFlash();
extern void CheatSearching();
extern void DisplayChtSrcRes_nosearch();
extern void DisplayGUIAbout();
extern void DisplayGUIAddOns();
extern void DisplayGUIChipConfig();
extern void DisplayGUICombo();
extern void DisplayGUIMovies();
extern void DisplayGUIOptns();
extern void DisplayGUIPaths();
extern void DisplayGUISave();
extern void DisplayGUISpeed();
extern void DisplayGameOptns();
extern void DisplayNetOptns();
extern void FindChtSrcRes();

extern char  CMovieExt;
extern char  CSDescDisplay[20];
extern char  CSInputDisplay[12];
extern char  GUIBlinkCursor[2];
extern char  GUICSrcTextG1[11];
extern char  GUICSrcTextG1a[];
extern char  GUICheatTextZ1[16];
extern char  GUICheatTextZ2[23];
extern char  GUIChoseSaveText2[2];
extern char  GUIChoseSlotTextX[2];
extern char  GUIComboTextH[21];
extern char  GUIGameDisplayKy[4];
extern char  GUILoadTextA[38];
extern char  GUIMenuItem[];
extern char* GUICustomResTextPtr[2];
extern u1    CheatSearchStatus;
extern u1    CheatWinMode;
extern u1    CurCStextpos;
extern u1    GUIBlinkCursorLoop;
extern u1    GUICheatPosA;
extern u1    GUICheatPosB;
extern u1    GUIFreshInputSelect;
extern u1    GUILoadPos;
extern u1    GUINumCombo;
extern u1    ShowMMXSupport;
extern u1    lameExists;
extern u1    mencoderExists;
extern u1*   GUIInputRefP[5];
extern u1*   ccheatnpos;
extern u4    CSCurEntry;
extern u4    CSStartEntry;
extern u4    CheatSearchXPos;
extern u4    CheatSearchYPos;
extern u4    GUICSStA[3];
extern u4    GUICStA[3];
extern u4    GUIComboKey;
extern u4    GUIIStA[3];
extern u4    GUIInputTabs[];
extern u4    GUILStA[3];
extern u4    GUILStB[3];
extern u4    GUIOptionTabs[];
extern u4    GUIVStA[3];
extern u4    GUIVideoTabs[];
extern u4    GUIVntscTab[];
extern u4    GUIcurrentcheatcursloc;
extern u4    GUIcurrentcheatviewloc;
extern u4    GUIcurrentcheatwin;
extern u4    GUIcurrentchtsrccursloc;
extern u4    GUIcurrentchtsrcviewloc;
extern u4    GUIcurrentinputcursloc;
extern u4    GUIcurrentinputviewloc;
extern u4    GUIcurrentvideocursloc;
extern u4    GUIcurrentvideoviewloc;
extern u4    NumCheatSrc;
extern u4    NumCombo;
extern u4    NumComboGlob;
extern u4    NumComboLocl;
extern u4    SrcMask[4];
extern u4    ccheatnleft;
extern u4    ccheatnleftb;
extern u4    curaddrvalcs;
extern u4    curentryval;
extern u4    curvaluecs;

#endif
