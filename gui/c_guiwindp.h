#ifndef C_GUIWINDP_H
#define C_GUIWINDP_H

void DisplayGUIAbout(void); // Displays the About Box
void DisplayGUIAddOns(void); // Select Special Controllers
void DisplayGUICheat(void); // Add/Browse Cheats menu
void DisplayGUIChipConfig(void);
void DisplayGUIChoseSave(void); // Pick Save State
void DisplayGUICombo(void);
void DisplayGUIInput(void); // Player Select, Joystick/keyboard Type List, Button Assignments
void DisplayGUILoad(void);
void DisplayGUIMovies(void); // Movie Record/Playback options
void DisplayGUIOption(void); // Emu Options
void DisplayGUIOptns(void); // GUI Options
void DisplayGUIPaths(void);
void DisplayGUIReset(void); // Reset Confirmation
void DisplayGUISave(void); // Save & Rewind options/Hotkeys
void DisplayGUISearch(void);
void DisplayGUISound(void); // Sound Related Options
void DisplayGUISpeed(void); // Speed Options
void DisplayGUIStates(void); // Save/Load State Confirmation
void DisplayGUIVideo(void);
void DisplayGameOptns(void); // Misc Key Window
void DisplayNetOptns(void);

extern char CMovieExt;
extern char GUICheatTextZ1[];
extern char GUICheatTextZ2[];
extern char GUIChoseSaveText2[2];
extern char GUIChoseSlotTextX[2];
extern char GUIComboTextH[21];
extern char GUILoadTextA[38];
extern u1 GUIFreshInputSelect;
extern u1 GUILoadPos;
extern u1 GUIStatesText5;
extern u1 GUIWincoladd;
extern u1 ShowMMXSupport;
extern u1* const GUIInputRefP[];
extern u4 GUIIStA[3];
extern u4 GUILStA[3];
extern u4 GUILStB[3];
extern u4 GUIVStA[3];
extern u4 GUIcurrentinputcursloc;
extern u4 GUIcurrentinputviewloc;
extern u4 GUIcurrentvideocursloc;
extern u4 GUIcurrentvideoviewloc;

#endif
