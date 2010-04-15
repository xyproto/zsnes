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



EXTSYM GUIHLine
EXTSYM GUIDisplayIconWin,GUIOuttextwin2

SECTION .data
NEWSYM mencoderExists, db 0
NEWSYM lameExists, db 0

SECTION .text

%macro DrawGUILine 0
  ccallv GUIHLine, eax, ecx, ebx, edx
  inc ebx
%endmacro

%macro DrawGUIWinBox3 6
  mov ecx,%4
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add ecx,eax
  add eax,%2
  add ebx,%3
  inc ecx
  mov dl,%6
  mov esi,%5-%3+1
%%loop
  DrawGUILine
  dec esi
  jnz %%loop
%endmacro

%macro GUIDisplayCheckboxP 4              ; Toggled Checkbox (Plain, no text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
%endmacro

%macro GUIDisplayCheckboxPn 5             ; Set Var. Checkbox
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],%5
  jne %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
%endmacro

%macro GUIOuttextwin2b 4        ; Special, yellow text
  mov al,[TextColorSp]
  mov [GUItextcolor],al
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  mov al,[TextColorSp+1]
  mov [GUItextcolor],al
  ccallv GUIOuttextwin2, %1, %2 - 1, %3 - 1, %4
%endmacro

SECTION .data
NEWSYM GUILoadTextA, times 38 db 0
SECTION .bss
NEWSYM GUILStA, resd 3
NEWSYM GUILStB, resd 3
NEWSYM GUILoadPos, resb 1

SECTION .data
NEWSYM GUIChoseSaveText2, db '-',0
NEWSYM GUIChoseSlotTextX, db '-',0

SECTION .text

%macro DGOptnsDrawBox 3
  mov eax,[GUIwinposx+9*4]
  mov ebx,[GUIwinposy+9*4]
  add eax,%1
  add ebx,%2
  mov ecx,eax
  add ecx,20
  mov dl,167
  mov esi,7
%%loop
  DrawGUILine
  dec esi
  jnz %%loop
  mov eax,%3
  mov ebx,[ScanCodeListing+eax*3]
  mov [GUIGameDisplayKy],ebx
  mov byte[GUIGameDisplayKy+3],0
  mov byte[GUItextcolor],223
  ccallv GUIOuttextwin2, 9, %1 + 3, %2 + 2, GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  ccallv GUIOuttextwin2, 9, %1 + 2, %2 + 1, GUIGameDisplayKy
%endmacro

SECTION .data
NEWSYM GUIInputTabs, dd 1,5
  db '#1',0
  db '#2',0
  db '#3',0
  db '#4',0
  db '#5',0

NEWSYM GUIInputRefP, dd pl1contrl,pl2contrl,pl3contrl,pl4contrl,pl5contrl

NEWSYM GUIFreshInputSelect, db 1

SECTION .bss
NEWSYM GUIIStA, resd 3
NEWSYM GUIcurrentinputviewloc, resd 1
NEWSYM GUIcurrentinputcursloc, resd 1

SECTION .data
NEWSYM GUIOptionTabs, dd 1,2
  db 'BASIC',0
  db 'MORE',0

NEWSYM ShowMMXSupport, db 2

NEWSYM GUIVideoTabs, dd 1,2
  db 'MODES',0
  db 'FILTERS',0
NEWSYM GUIVntscTab, dd 0,2
  db 'NTSC',0
  db 'ADV NTSC',0

SECTION .bss
NEWSYM GUIVStA, resd 3
NEWSYM GUIcurrentvideoviewloc, resd 1
NEWSYM GUIcurrentvideocursloc, resd 1

SECTION .data
NEWSYM GUICheatTextZ1, db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 14 characters
NEWSYM GUICheatTextZ2, db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 18 characters
GUICheatTextZ5 db '      ',0

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
NEWSYM CSStartEntry,      resd 1
NEWSYM CSCurEntry,      resd 1
NEWSYM CheatSearchXPos,   resd 1
NEWSYM CheatSearchYPos,   resd 1
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

SECTION .data

; G = General Use

NEWSYM GUICSrcTextG1, db '          ',0       ; All variables

SECTION .bss
CCursLoc resd 1
CViewLoc resd 1
NViewLoc resd 1
TViewLoc resd 1

NEWSYM GUINCStA, resd 3
FileNameMod resb 512
CurCStringPos resd 1

GUIGameOptnsColA resb 1
GUIGameOptnsColB resb 1
NEWSYM GUIGameDisplayKy, resd 1

SECTION .data

NEWSYM GUIMovieTabs, dd 1,1
  db 'CONTROLS',0
NEWSYM GUIDumpingTab, dd 0,1
  db 'DUMPING',0

NEWSYM CMovieExt, db 'v'

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
NEWSYM GUIComboTextH, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

NEWSYM GUIIconDataComboPressRelease
  db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
  db 0  ,166,0  ,166,166,166,0  ,0  ,0  ,0
  db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
  db 166,166,166,0  ,166,0  ,0  ,0  ,0  ,0
  db 0  ,166,0  ,0  ,166,0  ,0  ,0  ,0  ,0
  db 0  ,234,234,234,234,0  ,0  ,0  ,0  ,0
  db 234,234,234,234,234,234,0  ,0  ,0  ,0
  db 234,234,234,234,234,234,0  ,0  ,0  ,0
  db 235,234,234,234,234,235,0  ,0  ,0  ,0
  db 0  ,235,235,235,235,0  ,0  ,0  ,0  ,0

NEWSYM GUIIconDataComboPress
  db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,166,166,166,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,236,236,236,236,0  ,0  ,0  ,0  ,0
  db 236,236,236,236,236,236,0  ,0  ,0  ,0
  db 236,236,236,236,236,236,0  ,0  ,0  ,0
  db 237,236,236,236,236,237,0  ,0  ,0  ,0
  db 0  ,237,237,237,237,0  ,0  ,0  ,0  ,0

NEWSYM GUIIconDataComboRelease
  db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,166,166,166,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,166,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,238,238,238,238,0  ,0  ,0  ,0  ,0
  db 238,238,238,238,238,238,0  ,0  ,0  ,0
  db 238,238,238,238,238,238,0  ,0  ,0  ,0
  db 239,238,238,238,238,239,0  ,0  ,0  ,0
  db 0  ,239,239,239,239,0  ,0  ,0  ,0  ,0

NEWSYM GUIIconDataComboFrame
  db 0  ,0  ,0  ,0  ,58 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,56 ,46 ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,54 ,44 ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,52 ,0  ,42 ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,50 ,40 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,48 ,38 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,46 ,36 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,44 ,0  ,34 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,42 ,32 ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,40 ,32 ,0  ,0  ,0  ,0  ,0  ,0

NEWSYM GUIIconDataComboSecond
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,51 ,45 ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,51 ,45 ,0  ,0  ,0  ,0  ,0  ,0
  db 0  ,54 ,52 ,48 ,46 ,0  ,0  ,0  ,0  ,0
  db 53 ,46 ,50 ,50 ,48 ,45 ,0  ,0  ,0  ,0
  db 52 ,50 ,45 ,48 ,46 ,44 ,0  ,0  ,0  ,0
  db 51 ,50 ,50 ,46 ,50 ,43 ,0  ,0  ,0  ,0
  db 50 ,50 ,50 ,50 ,50 ,42 ,0  ,0  ,0  ,0
  db 0  ,48 ,46 ,44 ,42 ,0  ,0  ,0  ,0  ,0
  db 0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0

NEWSYM GUIPathTabs, dd 1,3
  db 'GENERAL',0
  db 'MORE',0
  db 'BIOS+CARTS',0

NEWSYM GUIBlinkCursor, db 0,0
NEWSYM GUIBlinkCursorLoop, db 0
NEWSYM GUIPathsTab1Ptr, dd SRAMPath,SStatePath,MoviePath,IPSPath
NEWSYM GUIPathsTab2Ptr, dd SnapPath,SPCPath,CHTPath,ComboPath,INPPath
NEWSYM GUIPathsTab3Ptr, dd BSXPath,STPath,GNextPath,SGPath
NEWSYM GUIMovieTextPtr, dd GUIMovieForcedText
%ifndef __MSDOS__
NEWSYM GUICustomResTextPtr, dd GUICustomX,GUICustomY
%endif

;General data
GUINetDisp db 'REMOTE',0,0,0,0,0,0,0,0,0

SECTION .bss
GUIItemBoxColor resd 1
NEWSYM TextColorSp, resd 1

SECTION .data
NEWSYM GUIMenuItem
  db 'GAME  ',0
  db 'CONFIG',0
  db 'CHEAT ',0
  db 'NETPLAY',0
  db 'MISC  ',0
  db 25,0
