#ifndef GUIWINDP_H
#define GUIWINDP_H

#include "../types.h"

extern char   CMovieExt;
extern char   CSDescDisplay[20];
extern char   CSInputDisplay[12];
extern char   GUICSrcTextG1[11];
extern char   GUICheatTextZ1[16];
extern char   GUICheatTextZ2[23];
extern char   GUIChoseSaveText2[2];
extern char   GUIChoseSlotTextX[2];
extern char   GUIComboTextH[21];
extern char   GUILoadTextA[38];
extern char*  GUICustomResTextPtr[2];
extern char*  GUIPathsTab1Ptr[4];
extern char*  GUIPathsTab2Ptr[5];
extern char*  GUIPathsTab3Ptr[4];
extern char** GUIMovieTextPtr;
extern u1     CheatCompareValue;
extern u1     CheatSearchStatus;
extern u1     CheatWinMode;
extern u1     CurCStextpos;
extern u1     GUICheatPosA;
extern u1     GUICheatPosB;
extern u1     GUIComboData[50];
extern u1     GUIComboLHorz;
extern u1     GUIComboPNum;
extern u1     GUIComboPos;
extern u1     GUIFreshInputSelect;
extern u1     GUILoadPos;
extern u1     GUINumCombo;
extern u1     ShowMMXSupport;
extern u1     lameExists;
extern u1     mencoderExists;
extern u1*    GUIInputRefP[5];
extern u4     CSStartEntry;
extern u4     CheatSearchXPos;
extern u4     CheatSearchYPos;
extern u4     GUICSStA[3];
extern u4     GUICSStC[3];
extern u4     GUICStA[3];
extern u4     GUIComboKey;
extern u4     GUIDumpingTab[2];
extern u4     GUIIStA[3];
extern u4     GUIInputTabs[];
extern u4     GUILStA[3];
extern u4     GUILStB[3];
extern u4     GUIMovieTabs[];
extern u4     GUIOptionTabs[];
extern u4     GUIPathTabs[];
extern u4     GUIVStA[3];
extern u4     GUIVideoTabs[];
extern u4     GUIVntscTab[];
extern u4     GUIccombcursloc;
extern u4     GUIccomblcursloc;
extern u4     GUIccombviewloc;
extern u4     GUIcurrentcheatcursloc;
extern u4     GUIcurrentcheatviewloc;
extern u4     GUIcurrentcheatwin;
extern u4     GUIcurrentchtsrccursloc;
extern u4     GUIcurrentchtsrcviewloc;
extern u4     GUIcurrentinputcursloc;
extern u4     GUIcurrentinputviewloc;
extern u4     GUIcurrentvideocursloc;
extern u4     GUIcurrentvideoviewloc;
extern u4     NumCheatSrc;
extern u4     NumCombo;
extern u4     NumComboGlob;
extern u4     NumComboLocl;
extern u4     SrcMask[4];
extern u4     curaddrvalcs;
extern u4     curentryval;
extern u4     curvaluecs;

#endif
