#ifndef GUIWINDP_H
#define GUIWINDP_H

#include "../types.h"

extern void DisplayGUIAbout();
extern void DisplayGUIAddOns();
extern void DisplayGUICheat();
extern void DisplayGUIChipConfig();
extern void DisplayGUIChoseSave();
extern void DisplayGUICombo();
extern void DisplayGUIInput();
extern void DisplayGUILoad();
extern void DisplayGUIMovies();
extern void DisplayGUIOption();
extern void DisplayGUIOptns();
extern void DisplayGUIPaths();
extern void DisplayGUIReset();
extern void DisplayGUISave();
extern void DisplayGUISearch();
extern void DisplayGUISound();
extern void DisplayGUISpeed();
extern void DisplayGUIStates();
extern void DisplayGUIVideo();
extern void DisplayGameOptns();
extern void DisplayNetOptns();

extern char CMovieExt;
extern char GUIMenuItem[];
extern u1   CheatSearchStatus;
extern u1   CheatWinMode;
extern u1   GUILoadPos;
extern u1   ShowMMXSupport;
extern u4   GUIcurrentcheatwin;
extern u4   GUIcurrentvideocursloc;
extern u4   GUIcurrentvideoviewloc;
extern u4   NumCombo;
extern u4   NumComboGlob;
extern u4   NumComboLocl;

#endif
