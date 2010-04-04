#ifndef C_GUIWINDP_H
#define C_GUIWINDP_H

void DisplayGUIOption(void); // Emu Options
void DisplayGUIReset(void);  // Reset Confirmation
void DisplayGUISound(void);  // Sound Related Options
void DisplayGUIStates(void); // Save/Load State Confirmation
void DisplayGUIVideo(void);

extern u1 GUIStatesText5;

#endif
