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

/* The Filters panel. Heights are the gaps between the rows as they stand, so
   the layout comes out where it already was; what changes is that the numbers
   live here instead of being repeated in the click handling. */
enum {
    FILT_ROW_LABEL,
    FILT_ROW_TOP, /* bilinear or interpolation, and the NTSC box */
    FILT_ROW_SAI1,
    FILT_ROW_SAI2,
    FILT_ROW_HQLEVEL,
    FILT_ROW_MISCLABEL,
    FILT_ROW_MISC,
    FILT_ROW_SYNCLABEL,
    FILT_ROW_SYNC,
    FILT_ROW_DISPLABEL,
    FILT_ROW_DISP,
    FILT_ROW_COUNT
};

void GUIFilterRows(s4 out[FILT_ROW_COUNT]);

/* The Monitors panel. The list is one row of the stack, six entries deep. */
enum { MON_PITCH = 12,
    MON_MAX = 6 };
enum { MON_ROW_LABEL,
    MON_ROW_LIST,
    MON_GAP,
    MON_ROW_NOTE,
    MON_ROW_COUNT };

void GUIMonitorRows(s4 out[MON_ROW_COUNT]);

/* The Modes panel's right-hand column: the Set button, the legend, and the
   custom resolution boxes under it. */
enum {
    MODE_ROW_SET,
    MODE_ROW_LEGEND,
    MODE_ROW_LEGEND1,
    MODE_ROW_LEGEND2,
    MODE_ROW_LEGEND3,
    MODE_ROW_LEGEND4,
    MODE_ROW_LEGEND5,
    MODE_ROW_LEGEND6,
    MODE_ROW_CUSTOM,
    MODE_ROW_CUSTOMBOX,
    MODE_ROW_COUNT
};

void GUIModeRows(s4 out[MODE_ROW_COUNT]);

/* The Sound panel. Two of its rows are groups of evenly spaced entries, so the
   stack holds the whole group and the entries step through it by SND_PITCH. */
enum { SND_PITCH = 10,
    SND_OPTS = 6,
    SND_LIST = 4 };
enum {
    SND_ROW_LABEL,
    SND_ROW_OPTS, /* the six on/off boxes */
    SND_ROW_RATELABEL,
    SND_ROW_RATEBOX,
    SND_ROW_VOLLABEL,
    SND_ROW_VOL,
    SND_ROW_LISTLABEL, /* INTERPOLATION: and LOWPASS: share the row */
    SND_ROW_LIST,
    SND_ROW_COUNT
};

void GUISoundRows(s4 out[SND_ROW_COUNT]);

#endif
