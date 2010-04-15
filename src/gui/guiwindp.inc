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

%macro DrawGUIWinBox 6
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2
  add ebx,%3
  mov ecx,eax
  add ecx,%4-%2+1
  mov dl,%6
  mov esi,%5-%3+1
%%loop
  DrawGUILine
  dec esi
  jnz %%loop
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

%macro GUIDisplayButtonHoleTu 7
  mov dword[GUITemp],GUIIconDataButtonHole
  cmp %4,%5
  jne %%nofill
  mov dword[GUITemp],GUIIconDataButtonFill
%%nofill
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
  GUIDisplayTextu %1,%2+15,%3+3,%6,%7
%endmacro

%macro GUIDisplayButtonHole 5
  mov dword[GUITemp],GUIIconDataButtonHole
  cmp %4,%5
  jne %%nofill
  mov dword[GUITemp],GUIIconDataButtonFill
%%nofill
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
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

%macro GUIDisplayBBox 6                     ; Black Box W/Border
  mov dword[GUIWincol],148+10
  cmp byte[cwindrawn],1
  jne %%noone
  mov dword[GUIWincol],148+5
%%noone
  cmp byte[cwindrawn],0
  jne %%nozero
  mov dword[GUIWincol],148
%%nozero
  DrawGUIWinBox %1,%2,%3,%4,%5,%6
  mov dl,[GUIWincol]
  DrawGUIWinBox %1,%2,%3-3+2,%4,%3+1,dl
  mov dl,[GUIWincol]
  inc dl
  DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
  mov dl,[GUIWincol]
  add dl,4
  DrawGUIWinBox %1,%2,%5+1,%4,%5-1,dl
  mov dl,[GUIWincol]
  add dl,3
  DrawGUIWinBox %1,%4+2,%3,%4-3,%5,dl
%endmacro

%macro SetVidbufLoc 0
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
%endmacro

%macro GUIDrawTArea 1 ; win #id
  mov dl,[GUIWincol]
  inc dl
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add ebx,10
  mov ecx,eax
  add ecx,[GUIwinsizex+%1*4]
  mov esi,12
%%loop1
  DrawGUILine
  dec esi
  jnz %%loop1
  add dl,3
  inc eax
  DrawGUILine
  sub ebx,13 ; set ebx to minY
  dec eax    ; set eax to minX
%endmacro

%macro DrawTabOn 1 ; %1 = array, ebp = offset, eax = minX, ebx = minY
  push edi
  mov esi,%1
  mov ecx,8
  add ecx,eax
%%skip
  cmp byte[esi+ebp],0
  jne %%read1
  inc ebp
  jmp %%skip
%%read1
  cmp byte[esi+ebp],0
  je %%ok1
  inc esi
  add ecx,6
  jmp %%read1
%%ok1
  mov dl,[GUIWincol]
  add dl,4
  inc eax
  DrawGUILine
  sub dl,2
  mov esi,12
%%draw1
  DrawGUILine
  dec esi
  jnz %%draw1
  sub ebx,12
  push ecx    ; save X offset for next drawing step
  dec eax
  mov ecx,eax
  mov esi,11
  inc dl
%%draw2
  DrawGUILine
  dec esi
  jnz %%draw2
  sub ebx,11
  push ebx    ; save Y offset for next drawing step
  mov cl,[GUIWincol]
  mov [GUItextcolor],cl
  add ebx,4
  push ebx    ; save label Yoff
  mov edx,eax
  add edx,6
  push edx    ; save label Xoff
  SetVidbufLoc
  mov edi,%1
  add edi,ebp
  ccallv GUIOutputStringwin, edx, esi, edi
  pop edx
  pop ebx
  mov byte[GUItextcolor],163
  cmp byte[GUIWincoladd],0
  je %%nochange
  mov byte[GUItextcolor],164
%%nochange
  dec edx
  dec ebx
  SetVidbufLoc
  mov edi,%1
  add edi,ebp
  ccallv GUIOutputStringwin, edx, esi, edi
  mov byte[GUItextcolor],217 ; restore normal colour
  pop ebx     ; restore Yoff for drawing step
  pop eax
  inc eax     ; restore and set Xoff for drawing step
  mov ecx,eax
  mov dl,[GUIWincol]
  add dl,3
  mov esi,12
%%draw3
  DrawGUILine
  dec esi
  jnz %%draw3
  sub ebx,13
  inc eax
%%read2
  mov esi,%1
  cmp byte[esi+ebp],0
  je %%ok2
  inc ebp
  jmp %%read2
%%ok2
  pop edi
%endmacro

%macro DrawTabOff 1 ; %1 = array, ebp = offset, eax = minX, ebx = minY
  push edi
  mov esi,%1
  mov ecx,8
  add ecx,eax
%%skip
  cmp byte[esi+ebp],0
  jne %%read1
  inc ebp
  jmp %%skip
%%read1
  cmp byte[esi+ebp],0
  je %%ok1
  inc esi
  add ecx,6
  jmp %%read1
%%ok1
  mov dl,[GUIWincol]
  add dl,3
  inc eax
  inc ebx
  DrawGUILine
  dec dl
  push ecx    ; save Xoff for next drawing step
  dec eax
  mov ecx,eax
  mov esi,10
%%draw1
  DrawGUILine
  dec esi
  jnz %%draw1
  sub ebx,10
  push ebx    ; save Yoff for next drawing step
  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je %%nochange
  mov byte[GUItextcolor],196
%%nochange
  add ebx,4
  push ebx    ; save label Yoff
  mov edx,eax
  add edx,6
  push edx    ; save label Xoff
  SetVidbufLoc
  mov edi,%1
  add edi,ebp
  ccallv GUIOutputStringwin, edx, esi, edi
  pop edx
  pop ebx
  add byte[GUItextcolor],15
  dec edx
  dec ebx
  SetVidbufLoc
  mov edi,%1
  add edi,ebp
  ccall GUIOutputStringwin, edx, esi, edi
  mov edx, eax
  pop ebx     ; restore Yoff for drawing step
  pop eax
  inc eax     ; restore and set Xoff for drawing step
  mov ecx,eax
  mov dl,[GUIWincol]
  mov esi,10
%%draw2
  DrawGUILine
  dec esi
  jnz %%draw2
  sub ebx,12
  inc eax
%%read2
  mov esi,%1
  cmp byte[esi+ebp],0
  je %%ok2
  inc ebp
  jmp %%read2
%%ok2
  pop edi
%endmacro

%macro GUIDrawTabs 1 ; tabs/label array
  mov ecx,[%1+4] ; total #
  cmp ecx,0
  je near %%nodraw
  mov esi,[%1]   ; active tab
  mov ebp,8  ; set array offset at top of labels
%%loop2
  dec esi    ; check if tab is the current one
  push ecx
  push esi
  jnz near %%disabled
  DrawTabOn %1  ; draws tab, updates eax, ebx & ebp for next tab...
  jmp near %%done
%%disabled
  DrawTabOff %1 ; ... and autosizes the tab for its label
%%done
  pop esi
  pop ecx
  dec ecx
  jnz %%loop2
%%nodraw
%endmacro

%macro GUIOuttextwin2b 4        ; Special, yellow text
  mov al,[TextColorSp]
  mov [GUItextcolor],al
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  mov al,[TextColorSp+1]
  mov [GUItextcolor],al
  ccallv GUIOuttextwin2, %1, %2 - 1, %3 - 1, %4
%endmacro

%macro GUIOuttextwin2c 4        ; Boxed, green text
  mov byte[GUItextcolor],223
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  push esi
  mov esi,%2
  dec esi
  ccallv GUIOuttextwin2, %1, esi, %3 - 1, %4
  pop esi
%endmacro

%macro BlinkCursorCheck 2
  cmp dword[GUIInputBox],%2+1
  jne %%notselected
  mov ebx,[%1+%2*4] 
%%notselected
%endmacro

%macro GUIOuttextwin2d 7        ; Boxed, green text, limited to 5th param
  push ecx
  mov ecx,%4                  ;Move pointer to text into ecx
%%zero
  cmp byte[ecx],0 ; Check for null in string
  je %%one
  inc ecx
  jmp %%zero
%%one
  push eax
  mov eax,ecx ; Move pointer to null into eax
  sub ecx,%5  ; Move ecx pointer back by length to display
  cmp ecx,%4
  jae %%two
  mov ecx,%4  ; If pointer is before beginning, reset pointer to beginning
%%two
  sub eax,ecx ; Subtract pointer from \0 pointer gives us string length
  push eax
  GUIOuttextwin2c %1,%2,%3,ecx
  pop eax
  push ebx
  pop ecx
  BlinkCursorCheck %6,%7
  cmp ebx,%4
  pop ebx
  jne near %%fixreg
  inc byte[GUIBlinkCursorLoop]
  cmp byte[GUIBlinkCursorLoop],30
  jne %%four
  mov byte[GUIBlinkCursorLoop],0
  cmp byte[GUIBlinkCursor],' '
  je %%three
  mov byte[GUIBlinkCursor],' '
  jmp %%four
%%three
  mov byte[GUIBlinkCursor],'_'
%%four
  imul eax,6 ; 6 pixels
  add eax,%2
  push ecx
  mov ecx,eax
  GUIOuttextwin2c %1,ecx,%3,GUIBlinkCursor
  pop ecx
%%fixreg
  pop eax
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

SECTION .text

NEWSYM DisplayGUIPaths
  GUIDrawWindowBox 19,GUIPathsDisp
  GUIDrawTArea 19
  GUIDrawTabs GUIPathTabs

  cmp dword[GUIPathTabs],1
  jne near .notgeneral

  GUIDisplayText 19,8,31,GUIPathsTextA1           ; Text
  GUIDisplayText 19,8,66,GUIPathsTextA2
  GUIDisplayText 19,8,101,GUIPathsTextA3
  GUIDisplayText 19,8,136,GUIPathsTextA4
  GUIDisplayText 19,8,171,GUIPathsTextA5

  GUIDisplayBBox  19,8,41,236,51,167              ; Input boxes
  GUIDisplayBBox  19,8,76,236,86,167
  GUIDisplayBBox  19,8,111,236,121,167
  GUIDisplayBBox  19,8,146,236,156,167
                                                  ; Green Text
  GUIOuttextwin2d 19,10,45,SRAMPath,37,GUIPathsTab1Ptr,0
  GUIOuttextwin2d 19,10,80,SStatePath,37,GUIPathsTab1Ptr,1
  GUIOuttextwin2d 19,10,115,MoviePath,37,GUIPathsTab1Ptr,2
  GUIOuttextwin2d 19,10,150,IPSPath,37,GUIPathsTab1Ptr,3

  ; Display Radio buttons
  GUIDisplayText 19,23,181,GUIPathsTextA5A
  GUIDisplayText 19,103,181,GUIPathsTextA5B

  GUIDisplayButtonHole 19,8,178,byte[RelPathBase],0
  GUIDisplayButtonHole 19,88,178,byte[RelPathBase],1

  cmp dword[GUIInputBox],0
  jne near .noinputboxselected
  GUIDisplayButtonHoleTu 19,8,178,byte[RelPathBase],0,GUIPathsTextA5A,0
  GUIDisplayButtonHoleTu 19,88,178,byte[RelPathBase],1,GUIPathsTextA5B,0
.noinputboxselected
.notgeneral

  cmp dword[GUIPathTabs],2
  jne near .notmorepaths

  GUIDisplayText 19,8,31,GUIPathsTextB1
  GUIDisplayText 19,8,66,GUIPathsTextB2
  GUIDisplayText 19,8,101,GUIPathsTextB3
  GUIDisplayText 19,8,136,GUIPathsTextB4
  GUIDisplayText 19,8,171,GUIPathsTextB5

  GUIDisplayBBox  19,8,41,236,51,167
  GUIDisplayBBox  19,8,76,236,86,167
  GUIDisplayBBox  19,8,111,236,121,167
  GUIDisplayBBox  19,8,146,236,156,167
  GUIDisplayBBox  19,8,181,236,191,167

  GUIOuttextwin2d 19,10,45,SnapPath,37,GUIPathsTab2Ptr,0
  GUIOuttextwin2d 19,10,80,SPCPath,37,GUIPathsTab2Ptr,1
  GUIOuttextwin2d 19,10,115,CHTPath,37,GUIPathsTab2Ptr,2
  GUIOuttextwin2d 19,10,150,ComboPath,37,GUIPathsTab2Ptr,3
  GUIOuttextwin2d 19,10,185,INPPath,37,GUIPathsTab2Ptr,4
.notmorepaths

  cmp dword[GUIPathTabs],3
  jne near .notbc

  GUIDisplayText 19,8,31,GUIPathsTextC1
  GUIDisplayText 19,8,66,GUIPathsTextC2
  GUIDisplayText 19,8,101,GUIPathsTextC3
  GUIDisplayText 19,8,136,GUIPathsTextC4

  GUIDisplayBBox  19,8,41,236,51,167
  GUIDisplayBBox  19,8,76,236,86,167
  GUIDisplayBBox  19,8,111,236,121,167
  GUIDisplayBBox  19,8,146,236,156,167

  GUIOuttextwin2d 19,10,45,BSXPath,37,GUIPathsTab3Ptr,0
  GUIOuttextwin2d 19,10,80,STPath,37,GUIPathsTab3Ptr,1
  GUIOuttextwin2d 19,10,115,GNextPath,37,GUIPathsTab3Ptr,2
  GUIOuttextwin2d 19,10,150,SGPath,37,GUIPathsTab3Ptr,3
.notbc
  ret

SECTION .data
GUIPathTabs dd 1,3
  db 'GENERAL',0
  db 'MORE',0
  db 'BIOS+CARTS',0

GUIPathsTextA1 db 'SAVES:',0
GUIPathsTextA2 db 'SAVESTATES:',0
GUIPathsTextA3 db 'MOVIES:',0
GUIPathsTextA4 db 'IPS:',0
GUIPathsTextA5 db 'RELATIVE PATH BASE:',0
GUIPathsTextA5A db 'CONFIG DIR',0
GUIPathsTextA5B db 'ROM DIR',0

GUIPathsTextB1 db 'SNAPSHOTS:',0
GUIPathsTextB2 db 'SPCS:',0
GUIPathsTextB3 db 'CHEATS:',0
GUIPathsTextB4 db 'COMBOS:',0
GUIPathsTextB5 db 'GAME SPECIFIC INPUT:',0

GUIPathsTextC1 db 'BS-X:',0
GUIPathsTextC2 db 'SUFAMI TURBO:',0
GUIPathsTextC3 db 'SD GUNDAM G-NEXT:',0
GUIPathsTextC4 db 'SAME GAME:',0

NEWSYM GUIBlinkCursor, db 0,0
NEWSYM GUIBlinkCursorLoop, db 0
GUIPathsTab1Ptr dd SRAMPath,SStatePath,MoviePath,IPSPath
GUIPathsTab2Ptr dd SnapPath,SPCPath,CHTPath,ComboPath,INPPath
GUIPathsTab3Ptr dd BSXPath,STPath,GNextPath,SGPath
NEWSYM GUIMovieTextPtr, dd GUIMovieForcedText
%ifndef __MSDOS__
NEWSYM GUICustomResTextPtr, dd GUICustomX,GUICustomY
%endif

;General data
GUINetDisp db 'REMOTE',0,0,0,0,0,0,0,0,0
GUIChipConfigDisp db 'CONFIGURE CHIPS',0
GUIPathsDisp db 'SETUP PATHS',0

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
