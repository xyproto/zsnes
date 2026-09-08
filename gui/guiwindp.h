// Looks good
#ifndef GUIWINDP_H
#define GUIWINDP_H

#include "../types.h"

extern char CSDescDisplay[20];
extern char CSInputDisplay[12];
extern char* GUICustomResTextPtr[2];
extern char* GUIPathsTab1Ptr[4];
extern char* GUIPathsTab2Ptr[5];
extern char* GUIPathsTab3Ptr[4];
extern char* GUIMovieTextPtr[1];
extern u1 CheatCompareValue;
extern u1 CheatSearchStatus;
extern u1 CheatWinMode;
extern u1 GUICheatPosA;
extern u1 GUICheatPosB;
extern u1 GUIComboData[50];
extern u1 GUIComboLHorz;
extern u1 GUIComboPNum;
extern u1 GUIComboPos;
extern u1 GUINumCombo;
extern u4 CurCStextpos;
extern u4 GUICSStA[3];
extern u4 GUICSStC[3];
extern u4 GUICStA[3];
extern u4 GUIComboKey;
extern u4 GUIDumpingTab[];
extern u4 GUIInputTabs[];
extern u4 GUIMovieTabs[];
extern u4 GUIOptionTabs[];
extern u4 GUIPathTabs[];
extern u4 GUIVideoTabs[];
extern u4 GUIVntscTab[];
extern u4 GUIccombcursloc;
extern u4 GUIccomblcursloc;
extern u4 GUIccombviewloc;
extern u4 GUIcurrentcheatcursloc;
extern u4 GUIcurrentcheatviewloc;
extern u4 GUIcurrentcheatwin;
extern u4 GUIcurrentchtsrccursloc;
extern u4 GUIcurrentchtsrcviewloc;
extern u4 NumCheatSrc;
extern u4 NumCombo;
extern u4 NumComboGlob;
extern u4 NumComboLocl;
extern u4 SrcMask[4];
extern u4 curaddrvalcs;
extern u4 curentryval;
extern u4 curvaluecs;

/* Panel layout.
 *
 * Widget positions used to be literal pixel numbers written down twice: once
 * where the panel is drawn and again where its clicks are tested, in another
 * file. Nothing tied the two together, so moving a control meant editing both
 * and hoping they matched - and twice they did not, leaving controls that drew
 * correctly and ignored the mouse.
 *
 * Instead a panel describes itself as a list of rows and asks where they go.
 * Both the drawing and the click handling ask the same question and get the
 * same answer, so they cannot drift apart: a row added, resized or removed
 * moves everything below it on both sides at once.
 *
 * A row is either an item with a height, a fixed gap, or a gap that expands.
 * The expanding ones share out whatever the fixed rows leave over, which is
 * what keeps a panel balanced when a section appears or disappears with the
 * video mode.
 */
enum { GUI_ITEM,
    GUI_GAP,
    GUI_EXPAND };

typedef struct {
    s4 h; /* an item's height, or a gap's minimum */
    u1 kind;
} GUIRow;

/* Place `n` rows between `top` and `bottom`, writing every row's y into out. */
void GUIStackLayout(GUIRow const* rows, u4 n, s4 top, s4 bottom, s4* out);

/* The CRT panel's rows, in the order they are stacked. Both DisplayGUIVideo
   and DisplayGUIVideoClick lay this out and read the same answers. */
enum {
    CRT_ROW_SCANLABEL,
    CRT_ROW_SCAN,
    CRT_GAP1,
    CRT_ROW_VIBLABEL,
    CRT_ROW_VIB,
    CRT_GAP2,
    CRT_ROW_BLOOMLABEL,
    CRT_ROW_BLOOM,
    CRT_GAP3,
    CRT_ROW_OUTLABEL,
    CRT_ROW_HDR,
    CRT_GAP4,
    CRT_ROW_NOTE,
    CRT_ROW_COUNT
};

void GUICrtRows(s4 out[CRT_ROW_COUNT]);

#endif
