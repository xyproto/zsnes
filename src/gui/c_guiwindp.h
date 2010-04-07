#ifndef C_GUIWINDP_H
#define C_GUIWINDP_H

void DisplayGUICheat(void);     // Add/Browse Cheats menu
void DisplayGUIChoseSave(void); // Pick Save State
void DisplayGUIInput(void);     // Player Select, Joystick/keyboard Type List, Button Assignments
void DisplayGUILoad(void);
void DisplayGUIOption(void);    // Emu Options
void DisplayGUIReset(void);     // Reset Confirmation
void DisplayGUISound(void);     // Sound Related Options
void DisplayGUIStates(void);    // Save/Load State Confirmation
void DisplayGUIVideo(void);

extern u1 GUIStatesText5;

#endif
