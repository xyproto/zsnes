/* C replacement for gui/gui.asm — data declarations only.
 * Compiled instead of gui/gui.asm when NO_ASM=1.
 * Covers both gui/gui.asm and the data from gui/guiwindp.inc (which
 * gui.asm %includes). */

#include "../types.h"
#include "../cfg.h"
#include "guifuncs.h"

/* ── .data — variables with explicit initial values ── */

/* 13-entry menu list, each entry: u8 enabled-flag, text, u8 0-terminator */
char GUIPrevMenuData[] = {
    1, '1', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '2', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '3', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '4', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '5', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '6', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '7', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '8', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '9', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    1, '0', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
       ' ', ' ', ' ', ' ', ' ', 0,
    0, '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', 0,
    1, 'F', 'R', 'E', 'E', 'Z', 'E', ' ', 'D', 'A', 'T', 'A', ':', ' ',
       'O', 'F', 'F', ' ', ' ', ' ', 0,
    1, 'C', 'L', 'E', 'A', 'R', ' ', 'A', 'L', 'L', ' ', 'D', 'A', 'T',
       'A', ' ', ' ', ' ', ' ', ' ', 0,
};

u1  ForceROMTiming  = 0;
u1  ForceHiLoROM    = 0;
u4  CalibXmin       = 0;
u4  CalibXmax       = 0;
u4  CalibYmin       = 0;
u4  CalibYmax       = 0;
u4  CalibXmin209    = 0;
u4  CalibXmax209    = 0;
u4  CalibYmin209    = 0;
u4  CalibYmax209    = 0;
u1  EEgg            = 0;

u4  GUIoldhand9o    = 0;
u2  GUIoldhand9s    = 0;
u4  GUIoldhand8o    = 0;
u2  GUIoldhand8s    = 0;
u1  GUIt1ccSwap     = 0;
u1  GUIskipnextkey42 = 0;

/* ── .bss — zero-initialised buffers ── */

u1  CombinDataGlob[3300];
u1  CombinDataLocl[3300];
u1  GUIwinorder[22];
u1  GUIwinactiv[22];
u1  GUIcmenupos;
u1  GUIescpress;
u1  GUIpmenupos;
u4  GUIcrowpos;
u1  GUIpclicked;
u4  GUImouseposx;
u4  GUImouseposy;
u4  GUICYLocPtr;
u4  GUIMenuL;
u4  GUIMenuR;
u4  GUIMenuD;
u1  GUIQuit;
u1  GUIHold;
u4  GUIHoldx;
u4  GUIHoldy;
u4  GUIHoldxm;
u4  GUIHoldym;
u1  cwindrawn;
u4  GUIHoldXlimL;
u4  GUIHoldXlimR;
u4  GUIHoldYlim;
u4  GUIHoldYlimR;
u4  cloadmaxlen;
u1  cplayernum;
u4  GUIScrolTim1;
u4  GUIScrolTim2;
u4  GUICHold;
u4  GUICBHold;
u4  GUICBHold2;
u4  GUIDClickTL;
u4  GUIDClCWin;
u4  GUIDClCEntry;
u4  GUICResetPos;
u4  GUICStatePos;
u1  GUICCFlash;
u1  GUILDFlash;

u4  CheatOn;
u4  NumCheats;
u1  cheatdataprev[28];
u1  cheatdata[28 * 255 + 56];

u1  GUIOn;
u1  GUIOn2;
u1  GUIReset;
u1  CurPalSelect;

/* ── Symbols from gui/guiwindp.inc ── */

/* BSS — zero-initialised */
u1  GUICheatPosA;
u1  GUICheatPosB;
u4  GUICStA[3];
u4  GUIcurrentcheatviewloc;
u4  GUIcurrentcheatcursloc;
u4  GUIcurrentcheatwin;
u1  CheatWinMode;
u1  CheatSearchStatus;
u4  CheatCompareValue;
u4  NumCheatSrc;
u4  GUIcurrentchtsrcviewloc;
u4  GUIcurrentchtsrccursloc;
u4  CurCStextpos;
u4  curentryval;
u4  curaddrvalcs;
u4  curvaluecs;
u4  GUICSStA[3];
u4  GUINCStA[3];
u4  GUIccombviewloc;
u4  GUIccombcursloc;
u4  GUIccomblcursloc;
u4  NumCombo;
u4  NumComboGlob;
u4  NumComboLocl;
u4  GUICSStC[3];
u4  GUIComboKey;
u1  GUIComboPos;
u1  GUIComboPNum;
u1  GUIComboLHorz;
u1  GUINumCombo;
u1  GUIComboData[50];

/* Initialized data from guiwindp.inc.
 * Each tab descriptor: u32 current_tab, u32 count, then NUL-terminated
 * tab-name strings.  Declared extern as u4[] in guiwindp.h so the memory
 * must start on a 4-byte boundary and the first 8 bytes are u32 values. */
__attribute__((aligned(4)))
char GUIInputTabs[] = {
    '\x01','\0','\0','\0',  '\x05','\0','\0','\0',
    '#','1','\0',  '#','2','\0',  '#','3','\0',  '#','4','\0',  '#','5','\0'
};
__attribute__((aligned(4)))
char GUIOptionTabs[] = {
    '\x01','\0','\0','\0',  '\x02','\0','\0','\0',
    'B','A','S','I','C','\0',  'M','O','R','E','\0'
};
__attribute__((aligned(4)))
char GUIVideoTabs[] = {
    '\x01','\0','\0','\0',  '\x02','\0','\0','\0',
    'M','O','D','E','S','\0',  'F','I','L','T','E','R','S','\0'
};
__attribute__((aligned(4)))
char GUIVntscTab[] = {
    '\x00','\0','\0','\0',  '\x02','\0','\0','\0',
    'N','T','S','C','\0',  'A','D','V',' ','N','T','S','C','\0'
};
__attribute__((aligned(4)))
char GUIMovieTabs[] = {
    '\x01','\0','\0','\0',  '\x01','\0','\0','\0',
    'C','O','N','T','R','O','L','S','\0'
};
__attribute__((aligned(4)))
char GUIDumpingTab[] = {
    '\x00','\0','\0','\0',  '\x01','\0','\0','\0',
    'D','U','M','P','I','N','G','\0'
};
__attribute__((aligned(4)))
char GUIPathTabs[] = {
    '\x01','\0','\0','\0',  '\x03','\0','\0','\0',
    'G','E','N','E','R','A','L','\0',  'M','O','R','E','\0',
    'B','I','O','S','+','C','A','R','T','S','\0'
};

u4  SrcMask[4] = { 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF };
char CSInputDisplay[] = { '_', '\0', ' ',' ',' ',' ',' ',' ',' ',' ',' ', '\0' };
char CSDescDisplay[]  = { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                          ' ',' ',' ',' ',' ',' ',' ',' ', '\0', '\0' };

/* Pointer arrays — point at path/text buffers defined in cfg.c / guifuncs.c */
char *GUIPathsTab1Ptr[4] = { SRAMPath, SStatePath, MoviePath, IPSPath };
char *GUIPathsTab2Ptr[5] = { SnapPath, SPCPath, CHTPath, ComboPath, INPPath };
char *GUIPathsTab3Ptr[4] = { BSXPath, STPath, GNextPath, SGPath };
char **GUIMovieTextPtr   = (char **)GUIMovieForcedText;
char *GUICustomResTextPtr[2] = { GUICustomX, GUICustomY };
