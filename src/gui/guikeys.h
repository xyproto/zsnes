#ifndef GUIKEYS_H
#define GUIKEYS_H

#include "../types.h"

extern void GUIGetInputLine();
extern void GUILoadKeys();
extern void GUIResetKeys();
extern void GUIStateKeys();
extern void GUIStateSelKeys();
extern void GUIWaitForKey();
extern void InsertSearchCharacter();
extern void InsertSearchDescription();

extern u1  CSOverValue;
extern u1* GUIEditStringcLen;
extern u4  CSCurValue;
extern u4  GUIEditStringLTxt;
extern u4  GUIEditStringLstb;
extern u4  GUIEditStringcWin;
extern u4  GUIInputBox;
extern u4  GUIInputLimit;

#endif
