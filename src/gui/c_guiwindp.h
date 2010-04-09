#ifndef C_GUIWINDP_H
#define C_GUIWINDP_H

void DisplayGUIAbout(void);     // Displays the About Box
void DisplayGUICheat(void);     // Add/Browse Cheats menu
void DisplayGUIChoseSave(void); // Pick Save State
void DisplayGUIInput(void);     // Player Select, Joystick/keyboard Type List, Button Assignments
void DisplayGUILoad(void);
void DisplayGUIMovies(void);    // Movie Record/Playback options
void DisplayGUIOption(void);    // Emu Options
void DisplayGUIOptns(void);     // GUI Options
void DisplayGUIReset(void);     // Reset Confirmation
void DisplayGUISave(void);      // Save & Rewind options/Hotkeys
void DisplayGUISearch(void);
void DisplayGUISound(void);     // Sound Related Options
void DisplayGUISpeed(void);     // Speed Options
void DisplayGUIStates(void);    // Save/Load State Confirmation
void DisplayGUIVideo(void);
void DisplayGameOptns(void);    // Misc Key Window
void DisplayNetOptns(void);

extern u1 GUIStatesText5;

#endif
