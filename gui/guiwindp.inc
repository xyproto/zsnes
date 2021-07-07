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



SECTION .data
NEWSYM GUIInputTabs, dd 1,5
  db '#1',0
  db '#2',0
  db '#3',0
  db '#4',0
  db '#5',0

NEWSYM GUIOptionTabs, dd 1,2
  db 'BASIC',0
  db 'MORE',0

NEWSYM GUIVideoTabs, dd 1,2
  db 'MODES',0
  db 'FILTERS',0
NEWSYM GUIVntscTab, dd 0,2
  db 'NTSC',0
  db 'ADV NTSC',0

SECTION .bss
NEWSYM GUICheatPosA, resb 1
NEWSYM GUICheatPosB, resb 1
NEWSYM GUICStA, resd 3
NEWSYM GUIcurrentcheatviewloc, resd 1
NEWSYM GUIcurrentcheatcursloc, resd 1
NEWSYM GUIcurrentcheatwin,     resd 1

NEWSYM CheatWinMode, resb 1
NEWSYM CheatSearchStatus, resb 1
NEWSYM CheatCompareValue, resd 1
NEWSYM NumCheatSrc,       resd 1
NEWSYM GUIcurrentchtsrcviewloc, resd 1
NEWSYM GUIcurrentchtsrccursloc, resd 1
NEWSYM CurCStextpos,      resd 1

SECTION .data
NEWSYM SrcMask, dd 0FFh,0FFFFh,0FFFFFFh,0FFFFFFFFh
NEWSYM CSInputDisplay, db '_',0,'         ',0
NEWSYM CSDescDisplay,  db '                  ',0,0

SECTION .bss
NEWSYM curentryval, resd 1
NEWSYM curaddrvalcs, resd 1
NEWSYM curvaluecs, resd 1
NEWSYM GUICSStA, resd 3

NEWSYM GUINCStA, resd 3

SECTION .data

NEWSYM GUIMovieTabs, dd 1,1
  db 'CONTROLS',0
NEWSYM GUIDumpingTab, dd 0,1
  db 'DUMPING',0

SECTION .bss
NEWSYM GUIccombviewloc, resd 1
NEWSYM GUIccombcursloc, resd 1
NEWSYM GUIccomblcursloc, resd 1
NEWSYM NumCombo, resd 1
NEWSYM NumComboGlob, resd 1
NEWSYM NumComboLocl, resd 1
NEWSYM GUICSStC, resd 3
NEWSYM GUIComboKey, resd 1
NEWSYM GUIComboPos, resb 1
NEWSYM GUIComboPNum, resb 1
NEWSYM GUIComboLHorz, resb 1
NEWSYM GUINumCombo, resb 1
NEWSYM GUIComboData, resb 50

SECTION .data
NEWSYM GUIPathTabs, dd 1,3
  db 'GENERAL',0
  db 'MORE',0
  db 'BIOS+CARTS',0

NEWSYM GUIPathsTab1Ptr, dd SRAMPath,SStatePath,MoviePath,IPSPath
NEWSYM GUIPathsTab2Ptr, dd SnapPath,SPCPath,CHTPath,ComboPath,INPPath
NEWSYM GUIPathsTab3Ptr, dd BSXPath,STPath,GNextPath,SGPath
NEWSYM GUIMovieTextPtr, dd GUIMovieForcedText
%ifndef __MSDOS__
NEWSYM GUICustomResTextPtr, dd GUICustomX,GUICustomY
%endif
