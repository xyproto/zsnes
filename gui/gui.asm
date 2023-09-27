;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;https://zsnes.bountysource.com
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;version 2 as published by the Free Software Foundation.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



; Sorry.  The GUI code is a total mess.  One problem I encountered is that
;   there seems to be a bug in nasm with using math calculations combined
;   with macros within macros, so in some macro definitions, I had to
;   add/subtract extra values in certain macros to get the GUI to align
;   correctly.
; The GUI is drawn is mostly manually.  What I mean is such as
;   coding 5 boxes to represent a shaded box for each box.  Although that
;   can be simplified using macros to draw 5 boxes, that unknown bug in nasm
;   showed up which prevented me from using macros extensively.  I was
;   thinking of changing it to an object-based GUI for easier coding,
;   but that was decided far into the development of the GUI itself
;   and that I also never expected this code to become open-source, so I
;   decided not to proceed with it.

; Routine StartGUI is the function called to start the GUI and also
;   contains the execution main loop of the GUI.  If you want to completely
;   replace the GUI, just add a function call from that function, then
;   jump to endprog to exit ZSNES or continueprog to continue with the
;   gameplay.  Do not replace StartGUI with a function since it is not
;   a function, but rather a label that is being jumped to.

%include "macros.mac"

EXTSYM pressed

EXTSYM SnapPath,SPCPath,BSXPath,SGPath,STPath,GNextPath
EXTSYM SRAMPath
EXTSYM IPSPath,MoviePath,CHTPath,ComboPath,INPPath,SStatePath
EXTSYM GUIMovieForcedText

EXTSYM GUICustomX,GUICustomY

%include "gui/guiwindp.inc"

; Things to do :
;
; .checkmenuboxclick
; gray scale = 32 .. 63
; shadow = 96 .. 127
; blue scale = 148 .. 167, 168 .. 187
; gray scale = 189 .. 220 (32+137)

; |  Game        Config     Cheat     MultiPlay    Misc
;-------------------------------------------------------
;    Load        Input      Add Code  Modem        Misc Keys
;    Run         -----      Browse    IPX          GUI Opts
;    Reset       Devices    Search                 Movie Opt
;    -----       Chip Cfg                          Key Comb.
;    Save State  -----                             Save Cfg
;    Load State  Options                           -----
;    Pick State  Video                             About
;    -----       Sound
;    Quit        Paths
;                Saves
;                Speed

; NetPlay only has "Internet" for Windows/Linux

; Windows : 1 = Load
;           2 = Chose State
;           3 = Input Device Window
;           4 = Options
;           5 = Video
;           6 = Sound
;           7 = Cheat
;           8 = Net Options
;           9 = Game Options
;           10 = GUI Options
;           11 = About
;           12 = Reset Confirmation
;           13 = Cheat Search
;           14 = SaveState Confirmation
;           15 = Movies
;           16 = Key Combo
;           17 = Devices
;           18 = Chip Config
;           19 = Paths
;           20 = Saves
;           21 = Speed

SECTION .data

NEWSYM GUIPrevMenuData,
  db 1,'1.                            ',0
  db 1,'2.                            ',0
  db 1,'3.                            ',0
  db 1,'4.                            ',0
  db 1,'5.                            ',0
  db 1,'6.                            ',0
  db 1,'7.                            ',0
  db 1,'8.                            ',0
  db 1,'9.                            ',0
  db 1,'0.                            ',0
  db 0,'------------',0
  db 1,'FREEZE DATA: OFF   ',0
  db 1,'CLEAR ALL DATA     ',0

NEWSYM ForceROMTiming, db 0
NEWSYM ForceHiLoROM, db 0
NEWSYM CalibXmin, dd 0
NEWSYM CalibXmax, dd 0
NEWSYM CalibYmin, dd 0
NEWSYM CalibYmax, dd 0
NEWSYM CalibXmin209, dd 0
NEWSYM CalibXmax209, dd 0
NEWSYM CalibYmin209, dd 0
NEWSYM CalibYmax209, dd 0

NEWSYM EEgg, db 0

SECTION .bss
NEWSYM CombinDataGlob, resb 3300
NEWSYM CombinDataLocl, resb 3300

NEWSYM GUIwinorder, resb 22
NEWSYM GUIwinactiv, resb 22

NEWSYM GUIcmenupos,  resb 1
NEWSYM GUIescpress,  resb 1
NEWSYM GUIpmenupos,  resb 1
NEWSYM GUIcrowpos,   resd 1
NEWSYM GUIpclicked,  resb 1
NEWSYM GUImouseposx, resd 1
NEWSYM GUImouseposy, resd 1
NEWSYM GUICYLocPtr,  resd 1
NEWSYM GUIMenuL,     resd 1
NEWSYM GUIMenuR,     resd 1
NEWSYM GUIMenuD,     resd 1
NEWSYM GUIQuit,      resb 1
NEWSYM GUIHold,      resb 1
NEWSYM GUIHoldx,     resd 1
NEWSYM GUIHoldy,     resd 1
NEWSYM GUIHoldxm,    resd 1
NEWSYM GUIHoldym,    resd 1
NEWSYM cwindrawn,    resb 1
NEWSYM GUIHoldXlimL, resd 1
NEWSYM GUIHoldXlimR, resd 1
NEWSYM GUIHoldYlim,  resd 1
NEWSYM GUIHoldYlimR, resd 1
NEWSYM cloadmaxlen,  resd 1
NEWSYM cplayernum,   resb 1
NEWSYM GUIScrolTim1, resd 1
NEWSYM GUIScrolTim2, resd 1
NEWSYM GUICHold,     resd 1
NEWSYM GUICBHold,    resd 1
NEWSYM GUICBHold2,   resd 1
NEWSYM GUIDClickTL,  resd 1
NEWSYM GUIDClCWin,   resd 1
NEWSYM GUIDClCEntry, resd 1
NEWSYM GUICResetPos, resd 1
NEWSYM GUICStatePos, resd 1
NEWSYM GUICCFlash,   resb 1
NEWSYM GUILDFlash,   resb 1

NEWSYM CheatOn, resd 1
NEWSYM NumCheats, resd 1
NEWSYM cheatdataprev, resb 28 ; leave contents blank
NEWSYM cheatdata, resb 28*255+56 ; toggle, value, address, pvalue, name(22)

NEWSYM GUIOn,       resb 1
NEWSYM GUIOn2,      resb 1
NEWSYM GUIReset,    resb 1
NEWSYM CurPalSelect, resb 1