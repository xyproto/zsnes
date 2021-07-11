#ifndef GUIFUNCS_H
#define GUIFUNCS_H

#include <stdbool.h>

#include "../types.h"

bool Keep43Check(void);
u4 GUILoadKeysNavigate(u1 gui_key_extended);
void CheatCodeLoad(void);
void CheatCodeSave(void);
void GUIGenericJumpTo(void);
void GUILoadData(void);
void GUILoadKeysJumpTo(void);
void GUIQuickLoadUpdate(void);
void GUISaveVars(void);
void GetLoadData(void);
void GetMovieForcedLength(void);
void LoadCheatSearchFile(void);
void SaveCheatSearchFile(void);
void SetMovieForcedLength(void);
void dumpsound(void);
void loadquickfname(u1 slot);

char const* const* horizon_get(u4 distance);

#ifndef __MSDOS__
void GetCustomXY(void);
void SetCustomXY(void);

extern char GUICustomX[5];
extern char GUICustomY[5];
#endif

extern char GUIMovieForcedText[11];
extern char** d_names; // Directory Names
extern char** selected_names; // Used to point to requested one
extern s4 GUIJT_entries;
extern s4 GUIJT_offset;
extern s4 GUIJT_viewable;
extern s4 GUIcurrentcursloc; // current cursor position (GUI)
extern s4 GUIcurrentdircursloc; // current dir position (GUI)
extern s4 GUIcurrentdirviewloc; // current directory position
extern s4 GUIcurrentviewloc; // current file position
extern s4 GUIdirentries;
extern s4 GUIfileentries;
extern s4* GUIJT_currentcursloc;
extern s4* GUIJT_currentviewloc;
extern u4 GUIcurrentfilewin;

#endif
