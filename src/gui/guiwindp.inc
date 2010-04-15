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



EXTSYM GUIHLine,GUIDrawShadow2
EXTSYM GUIDisplayIconWin,GUIOuttextwin2u,GUIOuttextwin2
EXTSYM GUIoutputiconwin,GUIOuttextwin

SECTION .data
NEWSYM mencoderExists, db 0
NEWSYM lameExists, db 0

SECTION .text

%macro DrawGUILine 0
  ccallv GUIHLine, eax, ecx, ebx, edx
  inc ebx
%endmacro

%macro DrawTitleBar 0
  mov edx,46+157+6
  sub edx,[GUIWincoladd]
  DrawGUILine
  mov edx,42+157+4+4
  sub edx,[GUIWincoladd]
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  dec edx
  DrawGUILine
  mov edx,38+157+4
  sub edx,[GUIWincoladd]
  DrawGUILine
  sub ebx,10
  mov edx,44+157+4
  sub edx,[GUIWincoladd]
  mov esi,9
  push ecx
  mov ecx,eax
%%loop2
  DrawGUILine
  dec esi
  jnz %%loop2
  pop ecx
  sub ebx,8
  mov edx,40+157+4
  sub edx,[GUIWincoladd]
  mov esi,9
  push eax
  mov eax,ecx
%%loop3
  DrawGUILine
  dec esi
  jnz %%loop3
  pop eax
%endmacro

%macro drawshadow2 3
  mov edx,[GUIwinposy+%1*4]
  cmp edx,223
  jg near .noshadow
  cmp edx,0
  jge .noshadclip
  mov edx,0
.noshadclip
  mov ebx,edx
  add ebx,%3
  add ebx,9
  cmp ebx,0
  jl near .noshadow
  cmp ebx,223
  jng .noshadclip2
  mov ebx,223
.noshadclip2
  sub ebx,edx
  inc ebx
  mov edi,edx
  shl edi,8
  mov eax,edx
  shl eax,5
  add edi,eax
  mov edx,[GUIwinposx+%1*4]
  mov ecx,edx
  cmp edx,255
  jg .noshadow
  cmp edx,-3
  jnl .noshadclip3
  mov edx,-3
.noshadclip3
  add ecx,%2
  cmp ecx,0
  jl .noshadow
  cmp ecx,255
  jng .noshadclip4
  mov ecx,255
.noshadclip4
  sub ecx,edx
  inc ecx
  add edi,edx
  add edi,[vidbuffer]
  add edi,288*3+3+16
  ccallv GUIDrawShadow2, edi, ecx, ebx
.noshadow
%endmacro

%macro GUIDrawWindowBox 2
  mov dword[GUIWincoladd],4
  mov dword[GUIWincol],148+10
  cmp byte[cwindrawn],1
  jne .noone
  mov dword[GUIWincol],148+5
.noone
  cmp byte[cwindrawn],0
  jne .nozero
  mov dword[GUIWincoladd],0
  mov dword[GUIWincol],148
.nozero
  drawshadow2 %1,dword[GUIwinsizex+%1*4],dword[GUIwinsizey+%1*4]
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  mov ecx,eax
  add ecx,dword[GUIwinsizex+%1*4]
  DrawTitleBar
  mov esi,[GUIwinsizey+%1*4]
  dec esi
  mov edx,[GUIWincol]
  add edx,2
.loop
  DrawGUILine
  dec esi
  jnz .loop
  mov edx,[GUIWincol]
  DrawGUILine
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add ebx,10
  mov ecx,eax
  mov esi,[GUIwinsizey+%1*4]
  mov edx,[GUIWincol]
  add edx,3
  dec esi
.loop2
  DrawGUILine
  dec esi
  jnz .loop2
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add ebx,10
  add eax,dword[GUIwinsizex+%1*4]
  mov ecx,eax
  mov esi,[GUIwinsizey+%1*4]
  mov dl,[GUIWincol]
  inc dl
.loop3
  DrawGUILine
  dec esi
  jnz .loop3
  mov ebx,[GUIwinposy+%1*4]
  add ebx,3
  mov edx,[GUIwinposx+%1*4]
  add edx,3
  mov byte[GUItextcolor],184
  ccallv GUIOuttextwin, edx, ebx, %2
  mov ebx,[GUIwinposy+%1*4]
  add ebx,2
  mov edx,[GUIwinposx+%1*4]
  add edx,2
  mov byte[GUItextcolor],220
  cmp byte[GUIWincoladd],0
  je .zero2
  sub byte[GUItextcolor],6
.zero2
  ccallv GUIOuttextwin, edx, ebx, %2
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,dword[GUIwinsizex+%1*4]
  sub eax,10
  ccallv GUIoutputiconwin, eax, ebx, GUIIconDataClose
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

%macro GUIDisplayText 4                   ; Text&Shadow
  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je %%text
  mov byte[GUItextcolor],196
%%text
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  add byte[GUItextcolor],15
  ccallv GUIOuttextwin2, %1, %2 - 1, %3 - 1, %4
%endmacro

%macro GUIDisplayTextY 4                  ; Yellow Text&Shadow
  mov al,[GUIWincol]
  mov [GUItextcolor],al
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  mov byte[GUItextcolor],163
  cmp byte[GUIWincoladd],0
  je %%texty
  mov byte[GUItextcolor],164
%%texty
  ccallv GUIOuttextwin2, %1, %2 - 1, %3 - 1, %4
%endmacro

%macro GUIDisplayTextu 5                  ; Text&Shadow With Underline
  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je %%textu
  mov byte[GUItextcolor],196
%%textu
  ccallv GUIOuttextwin2u, %1,%2,%3,%4,%5
  add byte[GUItextcolor],15
  ccallv GUIOuttextwin2, %1, %2 - 1, %3 - 1, %4
%endmacro

%macro GUIDisplayCheckboxP 4              ; Toggled Checkbox (Plain, no text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
%endmacro

%macro GUIDisplayCheckbox 5               ; Toggled Checkbox (Text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
  GUIDisplayText %1,%2+15,%3+5,%5
%endmacro

%macro GUIDisplayCheckboxu 6              ; Toggled Checkbox (Text Underline)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
  GUIDisplayTextu %1,%2+15,%3+5,%5,%6
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

SECTION .text

NEWSYM EEMode
  ccall GetTime
  cmp eax,360
  ja .noEE
  mov eax,1
  ret
.noEE
  mov eax,0
  ret

NEWSYM DisplayGUIChipConfig
  GUIDrawWindowBox 18,GUIChipConfigDisp

  call EEMode                     ; You know it!
  cmp eax,0
  je near .noEE
  GUIDisplayText 18,10,38,GUIChipText1Alt
  GUIDisplayCheckbox 18,9,43,nssdip1,GUIChipText2Alt
  GUIDisplayCheckbox 18,59,43,nssdip2,GUIChipText3Alt
  GUIDisplayCheckbox 18,109,43,nssdip3,GUIChipText4Alt
  GUIDisplayCheckbox 18,9,53,nssdip4,GUIChipText5Alt
  GUIDisplayCheckbox 18,59,53,nssdip5,GUIChipText6Alt
  GUIDisplayCheckbox 18,109,53,nssdip6,GUIChipText7Alt
  jmp .endEE
.noEE                                ; Regular Shadow
  GUIDisplayTextY 18,10,38,GUIChipTextA
  GUIDisplayCheckboxu 18,9,43,nssdip1,GUIChipTextA1,4   ;Checkboxes
  GUIDisplayCheckboxu 18,59,43,nssdip2,GUIChipTextA2,4
  GUIDisplayCheckboxu 18,109,43,nssdip3,GUIChipTextA3,4
  GUIDisplayCheckboxu 18,9,53,nssdip4,GUIChipTextA4,4
  GUIDisplayCheckboxu 18,59,53,nssdip5,GUIChipTextA5,4
  GUIDisplayCheckboxu 18,109,53,nssdip6,GUIChipTextA6,4
.endEE
  ret

SECTION .data
GUIChipTextA db 'SUPER SYSTEM:',0
GUIChipTextA1 db 'DIP 1',0
GUIChipTextA2 db 'DIP 2',0
GUIChipTextA3 db 'DIP 3',0
GUIChipTextA4 db 'DIP 4',0
GUIChipTextA5 db 'DIP 5',0
GUIChipTextA6 db 'DIP 6',0

;EE
GUIChipText1Alt db 69,88,84,82,65,83,0
GUIChipText2Alt db 83,65,76,84,0
GUIChipText3Alt db 71,65,82,76,73,67,0
GUIChipText4Alt db 80,69,80,80,69,82,0
GUIChipText5Alt db 67,72,69,69,83,69,0
GUIChipText6Alt db 66,85,84,84,69,82,0
GUIChipText7Alt db 67,65,84,83,85,80,0

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
GUIChipConfigDisp db 'CONFIGURE CHIPS',0

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
