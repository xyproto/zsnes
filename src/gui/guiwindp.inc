;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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



EXTSYM VERSION_STR,placedate

SECTION .data
NEWSYM mencoderExists, db 0

SECTION .text

%macro DrawGUILineSc 2
  mov dword[GUIcolscaleval],%2
  mov edx,%1
  push eax
  push ecx
  call GUIHLines
  pop ecx
  pop eax
  inc ebx
%endmacro

%macro DrawGUILine 0
  push eax
  push ecx
  call GUIHLine
  pop ecx
  pop eax
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
  call GUIDrawShadow2
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
  GUIOuttextwin %2
  mov ebx,[GUIwinposy+%1*4]
  add ebx,2
  mov edx,[GUIwinposx+%1*4]
  add edx,2
  mov byte[GUItextcolor],220
  cmp byte[GUIWincoladd],0
  je .zero2
  sub byte[GUItextcolor],6
.zero2
  GUIOuttextwin %2
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,dword[GUIwinsizex+%1*4]
  sub eax,10
  GUIDisplayIcon GUIIconDataClose
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

%macro DrawGUIWinBox2 5
  mov eax,[GUIwinposx+%1*4]
  mov ecx,eax
  add eax,%2
  add ecx,%3+1
  mov dl,%5
  cmp byte[GUIWincoladd],0
  je %%zero
  inc dl
%%zero
  mov esi,%4
  add ebx,dword[GUIwinposy+%1*4]
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

%macro DrawGUIButton 9
  mov dl,[GUItextcolor]
  sub dl,5
  cmp byte[GUICBHold],%7
  jne %%noa
  sub dl,13
%%noa
  DrawGUIWinBox %1,%2,%3,%4,%3,dl
  sub dl,3
  cmp byte[GUICBHold],%7
  jne %%nob
  add dl,5
%%nob
  DrawGUIWinBox %1,%2,%3,%2,%5,dl
  sub dl,3
  cmp byte[GUICBHold],%7
  jne %%noc
  add dl,5
%%noc
  DrawGUIWinBox %1,%2+1,%3+1,%4-2,%5-2,dl
  sub dl,3
  cmp byte[GUICBHold],%7
  jne %%nod
  add dl,5
%%nod
  DrawGUIWinBox %1,%4+1,%3+1,%4-2,%5-2,dl
  sub dl,3
  cmp byte[GUICBHold],%7
  jne %%noe
  add dl,5
%%noe
  DrawGUIWinBox %1,%2,%5,%4-1,%5,dl
  cmp byte[GUICBHold],%7
  je near %%no2
  sub byte[GUItextcolor],15
  GUIOuttextwin2 %1,%2+5+%8,%3+4+%9,%6
  add byte[GUItextcolor],15
  GUIOuttextwin2 %1,%2+4+%8,%3+3+%9,%6
  jmp %%yes
%%no2
  sub byte[GUItextcolor],18
  GUIOuttextwin2 %1,%2+6+%8,%3+5+%9,%6
  add byte[GUItextcolor],15
  GUIOuttextwin2 %1,%2+5+%8,%3+4+%9,%6
  add byte[GUItextcolor],3
%%yes
%endmacro

%macro GUIOuttextwin2load 3
  mov eax,[eax]
  mov [cloadnpos],eax
  inc dword[cloadnposb]
  mov byte[GUItextcolor],223
  GUIOuttextwin2l %1,%2,%3,[cloadnpos]
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2l %1,%2-1,%3-1,[cloadnpos]
  add dword[cloadnpos],32
  dec dword[cloadnleft]
%endmacro

%macro GUIOuttextwinloadfile 3
  test dword[cloadnleft],80000000h
  jnz near %%skip
  mov eax,[cloadnposb]
  cmp eax,[GUIfileentries]
  jae near %%skip
  mov eax,[cloadnposb]
  shl eax,2
  add eax,[selected_names]
  GUIOuttextwin2load  %1,%2,%3
%%skip
%endmacro

%macro GUIOuttextwinloaddir 3
  test dword[cloadnleft],80000000h
  jnz near %%skip
  mov eax,[cloadnposb]
  cmp eax,[GUIdirentries]
  jae near %%skip
  mov eax,[cloadnposb]
  shl eax,2
  add eax,[d_names]
  add eax,8
  GUIOuttextwin2load  %1,%2,%3
%%skip
%endmacro

%macro GUIDisplayButtonHoleTu 7
  mov dword[GUITemp],GUIIconDataButtonHole
  cmp %4,%5
  jne %%nofill
  mov dword[GUITemp],GUIIconDataButtonFill
%%nofill
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
  GUIDisplayTextu %1,%2+15,%3+3,%6,%7
%endmacro

%macro GUIDisplayButtonHole 5
  mov dword[GUITemp],GUIIconDataButtonHole
  cmp %4,%5
  jne %%nofill
  mov dword[GUITemp],GUIIconDataButtonFill
%%nofill
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
%endmacro

%macro GUIDisplayText 4                   ; Text&Shadow
  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je %%text
  mov byte[GUItextcolor],196
%%text
GUIOuttextwin2 %1,%2,%3,%4
  add byte[GUItextcolor],15
GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIDisplayTextY 4                  ; Yellow Text&Shadow
  mov al,[GUIWincol]
  mov [GUItextcolor],al
  GUIOuttextwin2 %1,%2,%3,%4
  mov byte[GUItextcolor],163
  cmp byte[GUIWincoladd],0
  je %%texty
  mov byte[GUItextcolor],164
%%texty
  GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIDisplayTextu 5                  ; Text&Shadow With Underline
  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je %%textu
  mov byte[GUItextcolor],196
%%textu
GUIOuttextwin2u %1,%2,%3,%4,%5
  add byte[GUItextcolor],15
GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIDisplayTextG 4                  ; Green Text&Shadow
  mov byte[GUItextcolor],223
  GUIOuttextwin2 %1,%2,%3,%4
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIDisplayCheckboxP 4              ; Toggled Checkbox (Plain, no text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
%endmacro

%macro GUIDisplayCheckbox 5               ; Toggled Checkbox (Text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
  GUIDisplayText %1,%2+15,%3+5,%5
%endmacro

%macro GUIDisplayCheckboxTn 6             ; Variable Checkbox (Text)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],%5
  jne %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
  GUIDisplayText %1,%2+15,%3+5,%6
%endmacro

%macro GUIDisplayCheckboxu 6              ; Toggled Checkbox (Text Underline)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],0
  je %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
  GUIDisplayTextu %1,%2+15,%3+5,%5,%6
%endmacro

%macro GUIDisplayCheckboxun 7             ; Set Var. Checkbox (Text Underline)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],%5
  jne %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
  GUIDisplayTextu %1,%2+15,%3+5,%6,%7
%endmacro

%macro GUIDisplayCheckboxPn 5             ; Set Var. Checkbox
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],%5
  jne %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  GUIDisplayIconWin %1,%2,%3,[GUITemp]
%endmacro

%macro GUIDisplayBBoxS 6                   ; Black Box W/Border
  mov dword[GUIWincol],148+10              ; Minus right side
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
  call GUIOutputStringwin
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
  call GUIOutputStringwin
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
  call GUIOutputStringwin
  pop edx
  pop ebx
  add byte[GUItextcolor],15
  dec edx
  dec ebx
  SetVidbufLoc
  mov edi,%1
  add edi,ebp
  call GUIOutputStringwin
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

%macro GUIDrawSlider 8 ; win #id, minX, width, Ypos, var, text, proc1, proc2
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2
  add ebx,%4
  mov ecx,eax
  add ecx,%3
  xor edx,edx
  mov dl,215
  sub dl,[GUIWincoladd]
  DrawGUILine
  inc ecx
  inc eax
  sub dl,13
  DrawGUILine
  xor eax,eax
  %7 %5       ; proc1 == alters var correctly and puts result in al
  mov dword[GUITemp],%2-4
  add [GUITemp],eax
  GUIDisplayIconWin %1,[GUITemp],%4-4,GUIIconDataSlideBar
  %8 %5,%6    ; proc2 == alters text correctly and puts pointer in esi
  mov [GUITemp],esi  ; Display Value (Green)
  GUIDisplayTextG %1,%2+%3+6,%4-1,[GUITemp]
%endmacro

; DGUIDisplay
DisplayGUILoad:
  GUIDrawWindowBox 1,GUILoadDisp

%ifdef __MSDOS__
  GUIDisplayTextY 1,6,157,GUILoadText2
  GUIDisplayText 1,21,166,GUILoadText3
%else
  GUIDisplayText 1,21,166,GUILoadText3b
%endif
  GUIDisplayText 1,21,174,GUILoadText4
%ifdef __MSDOS__
  GUIDisplayText 1,21,182,GUILoadText5
%endif
  GUIDisplayText 1,6,16,GUILoadText7
  GUIDisplayText 1,161,16,GUILoadText8
  GUIDisplayText 1,146,172,GUILoadTextB

  push ecx
  push esi
  xor ecx,ecx
  mov esi,[ZRomPath]
.strlenloop
  cmp byte[esi+ecx],0
  je .foundend
  inc ecx
  jmp .strlenloop
.foundend
  cmp ecx,39
  jb .notoverflow
  add esi,ecx
  sub esi,39
.notoverflow
  mov [GUITemp],esi
  pop esi
  pop ecx
  GUIDisplayText 1,6,138,[GUITemp]

  mov byte[GUItextcolor],202
  cmp byte[GUIWincoladd],0
  je .text
  mov byte[GUItextcolor],196
.text
  mov dword[cloadmaxlen],39
%ifndef __MSDOS__
  cmp dword[GUIcurrentfilewin],0
  je near .nodir
  mov eax,[GUIcurrentdircursloc]
  add eax,2
  shl eax,2
  add eax,[d_names]
  mov eax,[eax]
  mov [GUITemp],eax
  GUIOuttextwin2l 1,6,158,[GUITemp]
  add byte[GUItextcolor],15
  GUIOuttextwin2l 1,5,157,[GUITemp]
  jmp .nofiles
.nodir
  cmp dword[GUIfileentries],0
  je near .nofiles
  mov eax,[GUIcurrentcursloc]
  cmp eax,[GUIfileentries]
  jae near .nofiles
  shl eax,2
  add eax,[selected_names]
  mov eax,[eax]
  mov [cloadnpos],eax
  GUIOuttextwin2l 1,6,158,[cloadnpos]
  add byte[GUItextcolor],15
  GUIOuttextwin2l 1,5,157,[cloadnpos]
.nofiles                           ; Button
%endif
  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .text2
  mov byte[GUItextcolor],111
.text2
  DrawGUIButton 1,186,165,228,176,GUILoadText9,1,0,0

  ; The Three Boxes
  GUIDisplayBBoxS 1,5,25,144,134,167    ; 126 = 6 * 21, 112 = 7 * 16
  GUIDisplayBBoxS 1,160,25,228,134,167  ; 78 =  6 * 13
  GUIDisplayBBox 1,5,145,228,152,167    ; 126 = 6 * 21, 112 = 7 * 16

  xor ebx,ebx                        ; Flash Code?
  mov bl,[GUILoadPos]
  mov byte[GUILoadTextA+ebx],0
  test byte[GUILDFlash],8
  jnz .nound
  mov byte[GUILoadTextA+ebx],'_'
  mov byte[GUILoadTextA+ebx+1],0
.nound

  mov ebx,[GUIcurrentcursloc]           ; check if its in the Files box?
  sub ebx,[GUIcurrentviewloc]
  cmp dword[GUIcurrentfilewin],0
  je .files
  mov ebx,[GUIcurrentdircursloc]
  sub ebx,[GUIcurrentdirviewloc]
.files

  mov eax,ebx                         ; Draw 2 more boxes?
  lea ebx,[ebx*4]
  add ebx,eax
  add ebx,eax
  add ebx,eax
  add ebx,27
  cmp dword[GUIcurrentfilewin],0
  jne .dir
  DrawGUIWinBox2 1,5,144,7,224
  jmp .files2
.dir
  DrawGUIWinBox2 1,160,228,7,224
.files2

  mov byte[GUItextcolor],223             ; Green Shadow
  GUIOuttextwin2 1,8,148, GUILoadTextA

  cmp dword[GUIfileentries],0
  jne .yesfiles
  mov dword[GUIcurrentfilewin],1
.yesfiles

  mov eax,[GUIfileentries]
  sub eax,[GUIcurrentviewloc]
  mov [cloadnleft],eax
  mov eax,[GUIcurrentviewloc]
  mov [cloadnposb],eax
  mov dword[cloadmaxlen],23

  GUIOuttextwinloadfile 1,8,29              ; Text/Shadow for Filename Box
  GUIOuttextwinloadfile 1,8,29+7
  GUIOuttextwinloadfile 1,8,29+7*2
  GUIOuttextwinloadfile 1,8,29+7*3
  GUIOuttextwinloadfile 1,8,29+7*4
  GUIOuttextwinloadfile 1,8,29+7*5
  GUIOuttextwinloadfile 1,8,29+7*6
  GUIOuttextwinloadfile 1,8,29+7*7
  GUIOuttextwinloadfile 1,8,29+7*8
  GUIOuttextwinloadfile 1,8,29+7*9
  GUIOuttextwinloadfile 1,8,29+7*10
  GUIOuttextwinloadfile 1,8,29+7*11
  GUIOuttextwinloadfile 1,8,29+7*12
  GUIOuttextwinloadfile 1,8,29+7*13
  GUIOuttextwinloadfile 1,8,29+7*14

  mov eax,[GUIdirentries]
  sub eax,[GUIcurrentdirviewloc]
  mov [cloadnleft],eax
  mov eax,[GUIcurrentdirviewloc]
  mov [cloadnposb],eax
  mov dword[cloadmaxlen],11

  GUIOuttextwinloaddir 1,164,29              ; Text/Shadow for DIR Box
  GUIOuttextwinloaddir 1,164,29+7
  GUIOuttextwinloaddir 1,164,29+7*2
  GUIOuttextwinloaddir 1,164,29+7*3
  GUIOuttextwinloaddir 1,164,29+7*4
  GUIOuttextwinloaddir 1,164,29+7*5
  GUIOuttextwinloaddir 1,164,29+7*6
  GUIOuttextwinloaddir 1,164,29+7*7
  GUIOuttextwinloaddir 1,164,29+7*8
  GUIOuttextwinloaddir 1,164,29+7*9
  GUIOuttextwinloaddir 1,164,29+7*10
  GUIOuttextwinloaddir 1,164,29+7*11
  GUIOuttextwinloaddir 1,164,29+7*12
  GUIOuttextwinloaddir 1,164,29+7*13
  GUIOuttextwinloaddir 1,164,29+7*14

  mov byte[GUItextcolor],221              ; Green Text
  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],222
.zero4
  GUIOuttextwin2 1,7,147, GUILoadTextA

  xor ebx,ebx
  mov bl,[GUILoadPos]
  mov byte[GUILoadTextA+ebx],0

  GUIDisplayButtonHole 1,9,163,byte[GUIloadfntype],0 ; Radio Buttons
  GUIDisplayButtonHole 1,9,171,byte[GUIloadfntype],1
%ifdef __MSDOS__
  GUIDisplayButtonHole 1,9,179,byte[GUIloadfntype],2
%endif

  GUIDisplayCheckboxTn 1,10,187,showallext,1,GUILoadText6         ; Checkboxes
  GUIDisplayCheckboxTn 1,144,177,ForceROMTiming,1,GUILoadTextC
  GUIDisplayCheckboxTn 1,144,187,ForceROMTiming,2,GUILoadTextD
  GUIDisplayCheckboxTn 1,184,177,ForceHiLoROM,1,GUILoadTextE
  GUIDisplayCheckboxTn 1,184,187,ForceHiLoROM,2,GUILoadTextF

  ; Slidebar for Files
  ; win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
  DrawSlideBarWin 1,146,33,[GUIcurrentviewloc],[GUIfileentries],15,94,GUILStA
  cmp byte[GUICHold],1
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin 1,146,25,GUIIconDataUpArrow
  cmp byte[GUICHold],1
  jne .noholda2
  sub byte[GUIWincoladd],3
.noholda2
  cmp byte[GUICHold],2
  jne .noholdb
  add byte[GUIWincoladd],3
.noholdb
  GUIDisplayIconWin 1,146,127,GUIIconDataDownArrow
  cmp byte[GUICHold],2
  jne .noholdb2
  sub byte[GUIWincoladd],3
.noholdb2

  ; Slidebar for DIR
  DrawSlideBarWin 1,230,33,[GUIcurrentdirviewloc],[GUIdirentries],15,94,GUILStB
  cmp byte[GUICHold],3
  jne .noholdc
  add byte[GUIWincoladd],3
.noholdc
  GUIDisplayIconWin 1,230,25,GUIIconDataUpArrow
  cmp byte[GUICHold],3
  jne .noholdc2
  sub byte[GUIWincoladd],3
.noholdc2
  cmp byte[GUICHold],4
  jne .noholdd
  add byte[GUIWincoladd],3
.noholdd
  GUIDisplayIconWin 1,230,127,GUIIconDataDownArrow
  cmp byte[GUICHold],4
  jne .noholdd2
  sub byte[GUIWincoladd],3
.noholdd2
  ret

SECTION .data
GUILoadText1 db 'SELECT FILE TO LOAD:',0
GUILoadText2 db 'DISPLAY TYPE:',0
GUILoadText3 db 'WIN9X LONG FILENAME',0
GUILoadText3b db 'LONG FILENAME',0
GUILoadText4 db 'SNES HEADER NAME',0
GUILoadText5 db 'DOS 8.3 FORMAT',0
GUILoadText6 db 'SHOW ALL EXTENSIONS',0
GUILoadText7 db 'FILENAME',0
GUILoadText8 db 'DIRECTORY',0
GUILoadText9 db 'LOAD',0
NEWSYM GUILoadTextA, times 38 db 0
GUILoadTextB db 'FORCE',0
GUILoadTextC db 'NTSC',0
GUILoadTextD db 'PAL',0
GUILoadTextE db 'LOROM',0
GUILoadTextF db 'HIROM',0
SECTION .bss
GUILStA resd 3
GUILStB resd 3
NEWSYM GUILoadPos, resb 1

SECTION .text

DisplayGUIReset:              ; Reset Confirmation
  GUIDrawWindowBox 12,GUIResetDisp

  cmp byte[GUIWincoladd],0         ;Red Box around buttons
  je .zero
  mov byte[GUItextcolor],211
.zero
  mov dl,224
  cmp byte[GUIWincoladd],0
  je .zero4
  inc dl
.zero4
  cmp byte[GUICResetPos],0
  je near .resetboxa
  DrawGUIWinBox 12,79,29,117,42,dl
  jmp .resetboxb
.resetboxa
  DrawGUIWinBox 12,19,29,57,42,dl
.resetboxb

  DrawGUIButton 12,20,30,56,41,GUIResetText2,2,0,0   ; Buttons
  DrawGUIButton 12,80,30,116,41,GUIResetText3,3,0,0

  GUIDisplayTextY 12,6,16,GUIResetText1
  ret

SECTION .data
GUIResetText1 db 'RESET: ARE YOU SURE ?',0
GUIResetText2 db 'YES',0
GUIResetText3 db 'NO',0

SECTION .text

DisplayGUIStates:                         ; Save/Load State Confirmation
  GUIDrawWindowBox 14,GUIStatesDisp

  cmp byte[GUIWincoladd],0             ; Red Box around buttons
  je .zero
  mov byte[GUItextcolor],211
.zero
  mov dl,224
  cmp byte[GUIWincoladd],0
  je .zero4
  inc dl
.zero4
  cmp byte[GUICStatePos],0
  je near .resetboxa
  DrawGUIWinBox 14,79,29,117,42,dl
  jmp .resetboxb
.resetboxa
  DrawGUIWinBox 14,19,29,57,42,dl
.resetboxb

  DrawGUIButton 14,20,30,56,41,GUIStatesText3,10,0,0    ; Buttons
  DrawGUIButton 14,80,30,116,41,GUIStatesText4,11,0,0

  cmp byte[GUIStatesText5],1              ; Determines Load or Save box
  je near .load
  GUIDisplayTextY 14,6,16,GUIStatesText1
  jmp .save
.load
  GUIDisplayTextY 14,6,16,GUIStatesText2
.save
  ret

SECTION .data
GUIStatesText1 db 'OKAY TO SAVE STATE?',0
GUIStatesText2 db 'OKAY TO LOAD STATE?',0
GUIStatesText3 db 'YES',0
GUIStatesText4 db 'NO',0
GUIStatesText5 db 0

SECTION .text

DisplayGUIChoseSave:                    ; Pick Save State
  GUIDrawWindowBox 2,GUIStateSelDisp

  GUIDisplayTextY 2,6,16,GUIChoseSaveText1
  mov byte[GUIChoseSaveText2],'0'
  GUIDisplayText 2,21,31,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'1'
  GUIDisplayText 2,41,31,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'2'
  GUIDisplayText 2,61,31,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'3'
  GUIDisplayText 2,81,31,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'4'
  GUIDisplayText 2,101,31,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'5'
  GUIDisplayText 2,21,46,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'6'
  GUIDisplayText 2,41,46,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'7'
  GUIDisplayText 2,61,46,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'8'
  GUIDisplayText 2,81,46,GUIChoseSaveText2
  mov byte[GUIChoseSaveText2],'9'
  GUIDisplayText 2,101,46,GUIChoseSaveText2

  mov dword[GUITemp],GUIIconDataButtonHole           ; Radio Buttons
  mov ebx,[statefileloc]
  mov edx,[ZStateName]
  mov al,[edx+ebx]
  mov [GUIChoseSaveText2],al
  cmp byte[edx+ebx-1],'s'
  jne .nots
  GUIDisplayButtonHole 2,10,28,byte[GUIChoseSaveText2],'t'
  jmp .next1
.nots
  GUIDisplayButtonHole 2,10,28,byte[GUIChoseSaveText2],'0'
.next1
  GUIDisplayButtonHole 2,30,28,byte[GUIChoseSaveText2],'1'
  GUIDisplayButtonHole 2,50,28,byte[GUIChoseSaveText2],'2'
  GUIDisplayButtonHole 2,70,28,byte[GUIChoseSaveText2],'3'
  GUIDisplayButtonHole 2,90,28,byte[GUIChoseSaveText2],'4'
  GUIDisplayButtonHole 2,10,43,byte[GUIChoseSaveText2],'5'
  GUIDisplayButtonHole 2,30,43,byte[GUIChoseSaveText2],'6'
  GUIDisplayButtonHole 2,50,43,byte[GUIChoseSaveText2],'7'
  GUIDisplayButtonHole 2,70,43,byte[GUIChoseSaveText2],'8'
  GUIDisplayButtonHole 2,90,43,byte[GUIChoseSaveText2],'9'
  ret

SECTION .data
GUIChoseSaveText1 db 'SELECT SAVE SLOT:',0
GUIChoseSaveText2 db '-',0

SECTION .text

%macro DGOptnsBorderBox 3
  mov dword[GUIWincol],148+10
  cmp byte[cwindrawn],1
  jne %%noone
  mov dword[GUIWincol],148+5
%%noone
  cmp byte[cwindrawn],0
  jne %%nozero
  mov dword[GUIWincol],148
%%nozero
  mov dl,[GUIWincol]
  DrawGUIWinBox %1,%2+1,%3,%2+18,%3,dl
  mov dl,[GUIWincol]
  inc dl
  DrawGUIWinBox %1,%2,%3+1,%2-1,%3+5,dl
  mov dl,[GUIWincol]
  add dl,4
  DrawGUIWinBox %1,%2+1,%3+8,%2+18,%3-8,dl
  mov dl,[GUIWincol]
  add dl,3
  DrawGUIWinBox %1,%2+22,%3+1,%2-23,%3+5,dl
%endmacro

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
  GUIOuttextwin2 9,%1+3,%2+2,GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 9,%1+2,%2+1,GUIGameDisplayKy
%endmacro

%macro DDrawBox 4
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2+1
  add ebx,%3+1
  mov ecx,eax
  add ecx,20
  mov dl,167
  mov esi,7
%%loop
  DrawGUILine
  dec esi
  jnz %%loop
  mov eax,%4
  mov ebx,[ScanCodeListing+eax*3]
  mov [GUIGameDisplayKy],ebx
  mov byte[GUIGameDisplayKy+3],0
  mov byte[GUItextcolor],223
  GUIOuttextwin2 %1,%2+4,%3+3,GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 %1,%2+3,%3+2,GUIGameDisplayKy
  DGOptnsBorderBox %1,%2,%3
%endmacro

%macro DGOptnsDrawBox2 3
  mov eax,[GUIwinposx+3*4]
  mov ebx,[GUIwinposy+3*4]
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
  GUIOuttextwin2 3,%1+3,%2+2,GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 3,%1+2,%2+1,GUIGameDisplayKy
%endmacro

%macro GUIInputDispAll 1
  GUIInputDispKey 45,105,%1upk    ; Up
  GUIInputDispKey 45,115,%1downk  ; Down
  GUIInputDispKey 45,125,%1leftk  ; Left
  GUIInputDispKey 45,135,%1rightk ; Right
  GUIInputDispKey 45,145,%1startk ; Start
  GUIInputDispKey 45,155,%1selk   ; Select
  GUIInputDispKey 85,105,%1Ak     ; A
  GUIInputDispKey 85,115,%1Bk     ; B
  GUIInputDispKey 85,125,%1Xk     ; X
  GUIInputDispKey 85,135,%1Yk     ; Y
  GUIInputDispKey 85,145,%1Lk     ; L
  GUIInputDispKey 85,155,%1Rk     ; R

  GUIInputDispKey 125,105,%1Xtk   ; X Turbo
  GUIInputDispKey 125,115,%1Ytk   ; Y Turbo
  GUIInputDispKey 125,125,%1Ltk   ; L Turbo
  GUIInputDispKey 165,105,%1Atk   ; A Turbo
  GUIInputDispKey 165,115,%1Btk   ; B Turbo
  GUIInputDispKey 165,125,%1Rtk   ; R Turbo

  GUIInputDispKey 125,145,%1DLk   ; Down-Left
  GUIInputDispKey 125,155,%1DRk   ; Down-Right
  GUIInputDispKey 165,145,%1ULk   ; Up-Left
  GUIInputDispKey 165,155,%1URk   ; Up-Right
%endmacro

%macro GUIInputDispKey 3
  mov eax,[%3]
  mov [GUIInputTextE],eax
  DGOptnsDrawBox2 %1,%2,[GUIInputTextE]
%endmacro

DisplayGUIInput:    ; Player Select, Joystick/keyboard Type List, Button Assignments

  GUIDrawWindowBox 3,GUIInputDisp
  mov eax,[GUIInputTabs]
  dec al
  mov [cplayernum],al
  GUIDrawTArea 3
  GUIDrawTabs GUIInputTabs

  GUIDisplayTextY 3,6,26,GUIInputTextA
  movzx eax,byte[cplayernum]
  mov ebx,[GUIInputRefP+eax*4]
  mov al,[ebx]
  mov al,[GUIInputRefB+eax]
  mov ebx,eax
  shl eax,4
  add eax,ebx
  add eax,GUIInputNames
  mov [GUITemp],eax
  GUIDisplayTextY 3,6+54,83,[GUITemp]   ; CDV
  GUIDisplayTextY 3,6,83,GUIInputTextA1

  GUIDisplayTextY 3,6,97,GUIInputTextB
  GUIDisplayText 3,6,107,GUIInputTextB1
  GUIDisplayText 3,6,117,GUIInputTextB2
  GUIDisplayText 3,6,127,GUIInputTextB3
  GUIDisplayText 3,6,137,GUIInputTextB4
  GUIDisplayText 3,6,147,GUIInputTextB5
  GUIDisplayText 3,6,157,GUIInputTextB6

  GUIDisplayText 3,76,107,GUIInputTextC1
  GUIDisplayText 3,76,117,GUIInputTextC2
  GUIDisplayText 3,76,127,GUIInputTextC3
  GUIDisplayText 3,76,137,GUIInputTextC4
  GUIDisplayText 3,76,147,GUIInputTextC5
  GUIDisplayText 3,76,157,GUIInputTextC6

  GUIDisplayTextY 3,116,97,GUIInputTextC
  GUIDisplayText 3,156,107,GUIInputTextC1
  GUIDisplayText 3,156,117,GUIInputTextC2
  GUIDisplayText 3,156,127,GUIInputTextC6
  GUIDisplayText 3,116,107,GUIInputTextC3
  GUIDisplayText 3,116,117,GUIInputTextC4
  GUIDisplayText 3,116,127,GUIInputTextC5

  GUIDisplayTextY 3,113,137,GUIInputTextD
  GUIDisplayText 3,153,147,GUIInputTextD1
  GUIDisplayText 3,153,157,GUIInputTextD2
  GUIDisplayText 3,113,147,GUIInputTextD3
  GUIDisplayText 3,113,157,GUIInputTextD4

%ifdef __MSDOS__
  GUIDisplayText 3,20,167,GUIInputTextE4

  mov dword[GUITemp],GUIIconDataCheckBoxUC              ; Checkbox
  cmp byte[cplayernum],0
  jne .notp1
  cmp byte[pl1p209],0
  je .nocheckbox2
  jmp .checkbox2
.notp1
  cmp byte[cplayernum],1
  jne .notp2
  cmp byte[pl2p209],0
  je .nocheckbox2
  jmp .checkbox2
.notp2
  cmp byte[cplayernum],2
  jne .notp3
  cmp byte[pl3p209],0
  je .nocheckbox2
  jmp .checkbox2
.notp3
  cmp byte[cplayernum],3
  jne .notp4
  cmp byte[pl4p209],0
  je .nocheckbox2
  jmp .checkbox2
.notp4
  cmp byte[pl5p209],0
  je .nocheckbox2
.checkbox2
  mov dword[GUITemp],GUIIconDataCheckBoxC
.nocheckbox2
  GUIDisplayIconWin 3,5,162,[GUITemp]
%endif

  DrawGUIButton 3,123,34,153,45,GUIInputTextE1,14,0,0   ; Buttons
  DrawGUIButton 3,123,50,177,61,GUIInputTextE2,40,0,0
%ifdef __MSDOS__
  DrawGUIButton 3,123,66,183,77,GUIInputTextE3,15,0,0
%endif

  GUIDisplayBBoxS 3,5,34,107,77,167        ; Main Box
  mov ebx,[GUIcurrentinputcursloc]
  sub ebx,[GUIcurrentinputviewloc]
  mov eax,ebx
  lea ebx,[ebx*8]
  add ebx,36
  DrawGUIWinBox2 3,5,107,7,224

  mov eax,[GUIcurrentinputviewloc]         ; Text&Shadow inside Main Box
  shl eax,4
  add eax,[GUIcurrentinputviewloc]
  add eax,GUIInputNames
  mov [GUITemp],eax
  GUIDisplayTextG 3,11,38,[GUITemp]
  add dword[GUITemp],17
  GUIDisplayTextG 3,11,38+8,[GUITemp]
  add dword[GUITemp],17
  GUIDisplayTextG 3,11,38+8*2,[GUITemp]
  add dword[GUITemp],17
  GUIDisplayTextG 3,11,38+8*3,[GUITemp]
  add dword[GUITemp],17
  GUIDisplayTextG 3,11,38+8*4,[GUITemp]

  ; Sidebar
  DrawSlideBarWin 3,109,42,[GUIcurrentinputviewloc],[NumInputDevices],5,28,GUIIStA
  cmp byte[GUICHold],9
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin 3,109,34,GUIIconDataUpArrow
  cmp byte[GUICHold],9
  jne .noholda2
  sub byte[GUIWincoladd],3
.noholda2
  cmp byte[GUICHold],10
  jne .noholdb
  add byte[GUIWincoladd],3
.noholdb
  GUIDisplayIconWin 3,109,70,GUIIconDataDownArrow
  cmp byte[GUICHold],10
  jne .noholdb2
  sub byte[GUIWincoladd],3
.noholdb2

 ; Hotkey Boxes
  cmp byte[cplayernum],0
  jne near .notpl1
  GUIInputDispAll pl1
.notpl1
  cmp byte[cplayernum],1
  jne near .notpl2
  GUIInputDispAll pl2
.notpl2
  cmp byte[cplayernum],2
  jne near .notpl3
  GUIInputDispAll pl3
.notpl3
  cmp byte[cplayernum],3
  jne near .notpl4
  GUIInputDispAll pl4
.notpl4
  cmp byte[cplayernum],4
  jne near .notpl5
  GUIInputDispAll pl5
.notpl5

  DGOptnsBorderBox 3,44,104    ; Box borders, keep them at bottom
  DGOptnsBorderBox 3,44,114
  DGOptnsBorderBox 3,44,124
  DGOptnsBorderBox 3,44,134
  DGOptnsBorderBox 3,44,144
  DGOptnsBorderBox 3,44,154
  DGOptnsBorderBox 3,84,104
  DGOptnsBorderBox 3,84,114
  DGOptnsBorderBox 3,84,124
  DGOptnsBorderBox 3,84,134
  DGOptnsBorderBox 3,84,144
  DGOptnsBorderBox 3,84,154
  DGOptnsBorderBox 3,124,104
  DGOptnsBorderBox 3,124,114
  DGOptnsBorderBox 3,124,124
  DGOptnsBorderBox 3,124,144
  DGOptnsBorderBox 3,124,154
  DGOptnsBorderBox 3,164,104
  DGOptnsBorderBox 3,164,114
  DGOptnsBorderBox 3,164,124
  DGOptnsBorderBox 3,164,144
  DGOptnsBorderBox 3,164,154
  ret

SECTION .data
GUIInputTabs dd 1,5
  db '#1',0
  db '#2',0
  db '#3',0
  db '#4',0
  db '#5',0

GUIInputRefF db 0,1,2,3,5,18,6,7,8,9,10,11,14,15,16,17
GUIInputRefB db 0,1,2,3,3,4,6,7,8,9,10,11,11,11,12,13,14,15,5
GUIInputRefP dd pl1contrl,pl2contrl,pl3contrl,pl4contrl,pl5contrl

GUIInputTextA db 'DEVICE:',0
GUIInputTextA1 db 'CURRENT:',0

GUIInputTextB db 'KEYS:',0
GUIInputTextB1 db '    UP',0
GUIInputTextB2 db '  DOWN',0
GUIInputTextB3 db '  LEFT',0
GUIInputTextB4 db ' RIGHT',0
GUIInputTextB5 db ' START',0
GUIInputTextB6 db 'SELECT',0

GUIInputTextC db 'TURBO:',0
GUIInputTextC1 db 'A',0
GUIInputTextC2 db 'B',0
GUIInputTextC3 db 'X',0
GUIInputTextC4 db 'Y',0
GUIInputTextC5 db 'L',0
GUIInputTextC6 db 'R',0

GUIInputTextD db 'DIAGONALS:',0
GUIInputTextD1 db 'UL',0
GUIInputTextD2 db 'UR',0
GUIInputTextD3 db 'DL',0
GUIInputTextD4 db 'DR',0

GUIInputTextE dd 0
GUIInputTextE1 db 'SET',0
GUIInputTextE2 db 'SET KEYS',0
GUIInputTextE3 db 'CALIBRATE',0
GUIInputTextE4 db 'USE JOYSTICK PORT 209H',0

SECTION .bss
GUIIStA resd 3
GUIcurrentinputviewloc resd 1
GUIcurrentinputcursloc resd 1

SECTION .text

DisplayGUIOption:       ; Emu Options
  GUIDrawWindowBox 4,GUIOptionDisp

  GUIDisplayCheckboxu 4,11,11,newengen,GUIOptionText3,0
  GUIDisplayCheckboxu 4,11,21,MMXSupport,GUIOptionText4,0
  GUIDisplayCheckboxu 4,11,31,pl12s34,GUIOptionText5,0
%ifdef __MSDOS__
  GUIDisplayCheckboxu 4,11,41,SidewinderFix,GUIOptionText6,1
%else
  GUIDisplayCheckboxu 4,11,41,PauseFocusChange,GUIOptionText6b,13
%endif
  GUIDisplayCheckboxu 4,11,51,FPSAtStart,GUIOptionText7,0
  GUIDisplayCheckboxu 4,11,61,Turbo30hz,GUIOptionText8,2
  GUIDisplayCheckboxu 4,11,71,TimerEnable,GUIOptionText9,7
  GUIDisplayCheckboxu 4,11,81,TwelveHourClock,GUIOptionText9a,0
  GUIDisplayCheckboxu 4,11,91,ClockBox,GUIOptionText9b,13
  GUIDisplayCheckboxu 4,11,101,bgfixer,GUIOptionTextA,0
  GUIDisplayCheckboxu 4,11,111,SmallMsgText,GUIOptionTextB,1
  GUIDisplayCheckboxu 4,11,121,AutoPatch,GUIOptionTextC,12
  GUIDisplayCheckboxu 4,11,131,RomInfo,GUIOptionTextD,15
  GUIDisplayCheckboxu 4,11,141,DisplayInfo,GUIOptionTextDa,17
  GUIDisplayCheckboxu 4,11,151,AllowUDLR,GUIOptionTextE,0
%ifdef __WIN32__
  GUIDisplayCheckboxu 4,11,161,HighPriority,GUIOptionTextF,6
  GUIDisplayCheckboxu 4,11,171,DisableScreenSaver,GUIOptionTextZ,1
%endif
  ret

SECTION .data
GUIOptionText3 db 'NEW GFX ENGINE',0
GUIOptionText4 db 'MMX SUPPORT',0
GUIOptionText5 db 'USE PL3/4 AS PL1/2',0
GUIOptionText6 db 'SIDEWINDER FIX',0
GUIOptionText6b db 'PAUSE EMU IN BACKGROUND',0
GUIOptionText7 db 'FPS AT PROGRAM START',0
GUIOptionText8 db 'USE 30HZ FOR TURBO',0
GUIOptionText9 db 'ENABLE CLOCK',0
GUIOptionText9a db '12 HOUR MODE',0
GUIOptionText9b db 'SHOW CLOCK BOX',0
GUIOptionTextA db 'OLD GFX MODE 2',0
GUIOptionTextB db 'USE SMALL MESSAGE TEXT',0
GUIOptionTextC db 'ENABLE AUTO-PATCH',0
GUIOptionTextD db 'ENABLE ROMINFO.TXT',0
GUIOptionTextDa db 'SHOW ROM INFO ON LOAD',0
GUIOptionTextE db 'ALLOW U+D/L+R',0
GUIOptionTextF db 'USE A HIGHER PRIORITY',0
GUIOptionTextZ db 'DISABLE POWER MANAGEMENT',0 ; keep as last option, Win Only

SECTION .text

%macro NTSCslidSet 1 ; slider variable
  mov al,[%1]
  add al,100
%endmacro

%macro NTSCslidText 2 ; slider var, text
  mov dword[%2],'    '
  mov esi,%2+3
  mov al,[%1]
  mov bl,10
  test al,128
  jz %%asciilp
  xor al,255
  inc al
%%asciilp                            ; turns decimal into ascii
  xor ah,ah
  div bl
  add ah,48
  mov [esi],ah
  dec esi
  cmp al,0
  jne %%asciilp
  test byte[%1],128
  jz %%pos
  mov byte[esi],'-'
%%pos
  mov esi,%2
%endmacro

DisplayGUIVideo:
  cmp byte[TripBufAvail],0        ; Check features
  jne .notexttb
  mov byte[Triplebufen],0
.notexttb
  cmp byte[MMXSupport],1
  jne .2xSaIdis
  cmp byte[newgfx16b],0
  je .2xSaIdis
  jmp .no2xSaIdis
.2xSaIdis
  mov byte[En2xSaI],0
  mov byte[hqFilter],0
.no2xSaIdis
  cmp byte[En2xSaI],0
  je .no2xsaien
  mov byte[Triplebufen],0
  mov byte[hqFilter],0
  mov byte[scanlines],0
  mov byte[antienab],0
.no2xsaien
  cmp byte[hqFilter],0
  je .nohqen
  mov byte[En2xSaI],0
  mov byte[scanlines],0
  mov byte[antienab],0
.nohqen

  GUIDrawWindowBox 5,GUIVideoDisp      ; Video Modes List/Options

  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUINTVID+eax],0
  jne .ntsc
  mov byte[NTSCFilter],0
  mov dword[GUIVntscTab],0
  cmp byte[GUIVideoTabs],0
  jne .ntsc
  mov byte[GUIVideoTabs],1
.ntsc
  GUIDrawTArea 5
  GUIDrawTabs GUIVideoTabs
  cmp byte[NTSCFilter],0
  je near .nontsc
  GUIDrawTabs GUIVntscTab
.nontsc

  cmp dword[GUIVideoTabs],1
  jne near .notmodestab
  DrawGUIButton 5,128,60,164,71,GUIVideoText8,4,0,0        ; Set Button

%ifndef __MSDOS__                              ; Legend
  GUIDisplayText 5,8,116,GUIVideoTextB
  GUIDisplayText 5,10,125,GUIVideoTextB1
  GUIDisplayText 5,10,132,GUIVideoTextB2
  GUIDisplayText 5,10,139,GUIVideoTextB3
%endif
%ifdef __UNIXSDL__
  GUIDisplayText 5,10,146,GUIVideoTextB4
%endif

  GUIDisplayBBoxS 5,5,26,115,109,167           ; Video Modes Box
  DrawSlideBarWin 5,117,34,[GUIcurrentvideoviewloc],[NumVideoModes],10,68,GUIVStA
  cmp byte[GUICHold],5                        ; Scrollbar
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin 5,117,26,GUIIconDataUpArrow
  cmp byte[GUICHold],5
  jne .noholdb
  sub byte[GUIWincoladd],3
.noholdb
  cmp byte[GUICHold],6
  jne .noholdc
  add byte[GUIWincoladd],3
.noholdc
  GUIDisplayIconWin 5,117,102,GUIIconDataDownArrow
  cmp byte[GUICHold],6
  jne .noholdd
  sub byte[GUIWincoladd],3
.noholdd

  mov ebx,[GUIcurrentvideocursloc]          ; Box
  sub ebx,[GUIcurrentvideoviewloc]
  lea ebx,[ebx*8]
  add ebx,28
  DrawGUIWinBox2 5,5,115,7,224

  mov byte[GUItextcolor],223               ; Shadow in Box
  mov eax,[GUIcurrentvideoviewloc]
  mov ebx,eax
  shl eax,4
  add eax,ebx
  add eax,ebx
  add eax,GUIVideoModeNames
  mov [GUITemp],eax
  GUIDisplayTextG 5,11,30,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*2,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*3,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*4,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*5,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*6,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*7,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*8,[GUITemp]
  add dword[GUITemp],18
  GUIDisplayTextG 5,11,30+8*9,[GUITemp]

  xor eax,eax                      ; Mode Value
  mov al,[cvidmode]
  mov ebx,eax
  shl eax,4
  add eax,ebx
  add eax,ebx
  add eax,GUIVideoModeNames
  mov [GUITemp],eax
  GUIDisplayTextY 5,7,194,GUIVideoTextE
  GUIDisplayTextY 5,61,194,[GUITemp]
.notmodestab

  cmp dword[GUIVideoTabs],2
  jne near .notfilterstab
  xor eax,eax                                  ; Scanlines text
  mov al,[cvidmode]
  cmp byte[GUISLVID+eax],0
  je near .notext2
  GUIDisplayText 5,13,111,GUIVideoTextC
.notext2

;Checkboxes
  xor eax,eax                                  ; Interpolations
  mov al,[cvidmode]
  cmp byte[GUIINVID+eax],0
  je near .notext3
  GUIDisplayCheckboxu 5,11,153,antienab,GUIVideoText3,0
.notext3
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUII2VID+eax],0
  je near .nocheckboxi2
  GUIDisplayCheckboxu 5,11,153,antienab,GUIVideoText3,0
.nocheckboxi2

  xor eax,eax                                  ; Eagle Interp.
  mov al,[cvidmode]
  cmp byte[GUIEAVID+eax],0
  je near .notext4
  GUIDisplayCheckboxu 5,11,153,antienab,GUIVideoText4,9
.notext4

  xor eax,eax                                  ; Full/Widescreen
  mov al,[cvidmode]
  cmp byte[GUIFSVID+eax],0
  je near .notext5
  cmp byte[GUIWSVID+eax],0
  je near .notext4a
  GUIDisplayCheckboxu 5,11,88,ScreenScale,GUIVideoText5b,6
  jmp .notext5
.notext4a
  GUIDisplayCheckboxu 5,11,88,ScreenScale,GUIVideoText5,6
.notext5

  xor eax,eax                                   ; Small Screen
  mov al,[cvidmode]
  cmp byte[GUISSVID+eax],0
  je near .nocheckbox3b
  GUIDisplayCheckboxu 5,11,98,smallscreenon,GUIVideoText6,1
.nocheckbox3b

  xor eax,eax                                        ; hq*x
  mov al,[cvidmode]
  cmp byte[MMXSupport],0
  je near .notexthq4xa
  cmp byte[GUIHQ2X+eax],0
  je near .notexthq2xa
  GUIDisplayCheckboxu 5,125,163,hqFilter,GUIVideoTexthq2x,1
  jmp .notexthq4xa
.notexthq2xa
  cmp byte[GUIHQ3X+eax],0
  je near .notexthq3xa
  GUIDisplayCheckboxu 5,125,163,hqFilter,GUIVideoTexthq3x,1
  jmp .notexthq4xa
.notexthq3xa
  cmp byte[GUIHQ4X+eax],0
  je near .notexthq4xa
  GUIDisplayCheckboxu 5,125,163,hqFilter,GUIVideoTexthq4x,1
.notexthq4xa

  xor eax,eax                                    ; scanlines
  mov al,[cvidmode]
  cmp byte[GUISLVID+eax],0
  je near .nocheckboxb
  GUIDisplayCheckboxun 5,21,114,scanlines,1,GUIVideoTextC1,0
.nocheckboxb
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUIHSVID+eax],0
  je near .nocheckboxw
  GUIDisplayCheckboxun 5,61,114,scanlines,3,GUIVideoTextC2,0
  GUIDisplayCheckboxun 5,101,114,scanlines,2,GUIVideoTextC3,0
.nocheckboxw

  xor eax,eax                                  ; triple buffs
  mov al,[cvidmode]
  cmp byte[GUITBWVID+eax],0
  je near .nocheckboxslw2
  GUIDisplayCheckboxu 5,11,183,TripleBufferWin,GUIVideoText7,0
.nocheckboxslw2
  xor eax,eax
  mov al,[cvidmode]
  cmp byte[GUITBVID+eax],0
  je near .nocheckboxtbb
  cmp byte[TripBufAvail],0
  je near .nocheckboxtbb
  GUIDisplayCheckboxu 5,11,183,Triplebufen,GUIVideoText7,0
.nocheckboxtbb

  xor eax,eax                                  ; 2xSaI & Supers
  mov al,[cvidmode]
  cmp byte[GUI2xVID+eax],0
  je near .nocheckboxx
  cmp byte[MMXSupport],0
  je near .nocheckboxx
  GUIDisplayCheckboxun 5,11,123,En2xSaI,1,GUIVideoTextc,1
  GUIDisplayCheckboxun 5,11,133,En2xSaI,2,GUIVideoTextd,6
  GUIDisplayCheckboxun 5,11,143,En2xSaI,3,GUIVideoTextd2,0
.nocheckboxx

  xor eax,eax                                   ; Bilinear
  mov al,[cvidmode]
  cmp byte[GUIBIFIL+eax],0
  je near .nocheckboxe
  GUIDisplayCheckboxu 5,11,153,BilinearFilter,GUIVideoTextbf,0
.nocheckboxe

  xor eax,eax                                    ; NTSC filter
  mov al,[cvidmode]
  cmp byte[GUINTVID+eax],0
  je near .nocheckboxbf
  GUIDisplayCheckboxu 5,125,173,NTSCFilter,GUIVideoTexte,0
.nocheckboxbf

  xor eax,eax                                    ; Hires Mode7
  mov al,[cvidmode]
  cmp byte[GUIM7VID+eax],0
  je near .nocheckboxm72
  cmp byte[newengen],0
  je near .nocheckboxm72
  GUIDisplayCheckboxu 5,11,163,Mode7HiRes16b,GUIVideoText8b,0
.nocheckboxm72

%ifdef __WIN32__
  xor eax,eax                                    ; Keep 4:3 Ratio
  mov al,[cvidmode]
  cmp byte[GUIDSMODE+eax],0
  jne near .checkboxk43
  cmp byte[GUISMODE+eax],0
  jne near .checkboxk43
  jmp .nocheckboxk43
.checkboxk43
  GUIDisplayCheckboxu 5,125,153,Keep4_3Ratio,GUIVideoTextK43,8
.nocheckboxk43
%endif

  GUIDisplayCheckboxu 5,11,173,GrayscaleMode,GUIVideoText8c,0
%ifndef __UNIXSDL__
  GUIDisplayCheckboxu 5,125,183,vsyncon,GUIVideoText9,0
%endif
.notfilterstab

  cmp dword[GUIVntscTab],1
  jne near .notntsctab
  DrawGUIButton 5,5,166,42,177,GUIVideoTextG8,37,0,0        ; Button

  GUIDisplayCheckboxu 5,5,25,NTSCBlend,GUIVideoTextG1,0
  GUIDisplayCheckboxu 5,135,25,NTSCRef,GUIVideoTextG9,0

  GUIDisplayTextY 5,7,46,GUIVideoTextG2
  GUIDisplayTextY 5,7,66,GUIVideoTextG3
  GUIDisplayTextY 5,7,86,GUIVideoTextG4
  GUIDisplayTextY 5,7,106,GUIVideoTextG5
  GUIDisplayTextY 5,7,126,GUIVideoTextG6
  GUIDisplayTextY 5,7,146,GUIVideoTextG7

  GUIDrawSlider 5,8,200,56,NTSCHue,GUIVideoTextG,NTSCslidSet,NTSCslidText
  GUIDrawSlider 5,8,200,76,NTSCSat,GUIVideoTextG,NTSCslidSet,NTSCslidText
  GUIDrawSlider 5,8,200,96,NTSCCont,GUIVideoTextG,NTSCslidSet,NTSCslidText
  GUIDrawSlider 5,8,200,116,NTSCBright,GUIVideoTextG,NTSCslidSet,NTSCslidText
  GUIDrawSlider 5,8,200,136,NTSCSharp,GUIVideoTextG,NTSCslidSet,NTSCslidText
  GUIDrawSlider 5,8,200,156,NTSCWarp,GUIVideoTextG,NTSCslidSet,NTSCslidText
.notntsctab
  ret

SECTION .data
GUIVideoTabs dd 1,2
  db 'MODES',0
  db 'FILTERS',0
GUIVntscTab dd 0,1
  db 'NTSC',0

GUIVideoTextB db 'LEGEND:',0
GUIVideoTextB1 db 'S = STRETCH  R = KEEP RATIO (8:7)',0
GUIVideoTextB2 db ' D = ALLOW SPECIAL FILTERS',0
GUIVideoTextB3 db ' W = WIN          F = FULL',0
GUIVideoTextB4 db '      O = USES OPENGL',0

GUIVideoTextC db 'SCANLINES:',0
GUIVideoTextC1 db 'FULL',0
GUIVideoTextC2 db '50%',0
GUIVideoTextC3 db '25%',0

GUIVideoTextE db 'CURRENT:',0

GUIVideoTextc db '2XSAI ENGINE',0
GUIVideoTextd db 'SUPER EAGLE',0
GUIVideoTextd2 db 'SUPER 2XSAI',0
GUIVideoTextbf db 'BILINEAR FILTER',0
GUIVideoText3 db 'INTERPOLATION',0      ; -y
GUIVideoText4 db 'EAGLE ENGINE',0       ; -y
GUIVideoTextscale2x db 'SCALE2X',0
GUIVideoTexthq2x db 'HQ2X',0
GUIVideoTexthq3x db 'HQ3X',0
GUIVideoTexthq4x db 'HQ4X',0
GUIVideoText5 db 'FULL SCREEN',0        ; -c
GUIVideoText5b db 'WIDE SCREEN',0       ; -c
GUIVideoText6 db 'SMALL SCREEN',0       ; -c
GUIVideoText7 db 'TRIPLE BUFFERING',0   ; -c
GUIVideoText8b db 'HI-RES MODE 7',0     ; -c
GUIVideoText8c db 'GRAYSCALE MODE',0    ; -c
GUIVideoText8 db 'SET',0                ; set button
GUIVideoText9 db 'VSYNC',0              ; -c
GUIVideoTexte db 'NTSC',0
GUIVideoTextK43 db 'USE 4:3 RATIO',0

GUIVideoTextG db '----%',0
GUIVideoTextG1 db 'BLEND FRAMES',0
GUIVideoTextG2 db 'HUE:',0
GUIVideoTextG3 db 'SATURATION:',0
GUIVideoTextG4 db 'CONTRAST:',0
GUIVideoTextG5 db 'BRIGHTNESS:',0
GUIVideoTextG6 db 'SHARPNESS:',0
GUIVideoTextG7 db 'HUE WARPING:',0
GUIVideoTextG8 db 'RESET',0
GUIVideoTextG9 db 'REFRESH',0

SECTION .bss
GUIVStA resd 3
GUIcurrentvideoviewloc resd 1
GUIcurrentvideocursloc resd 1

SECTION .text

%macro VolslidSet 1 ; slider variable
  mov al,[%1]
%endmacro

%macro VolslidText 2 ; slider var, text
  mov word[%2],'  '
  mov esi,%2+3
  mov al,[%1]
  mov bl,10
%%asciilp                            ; turns decimal into ascii
  dec esi
  xor ah,ah
  div bl
  add ah,48
  mov [esi],ah
  cmp al,0
  jne %%asciilp
  mov esi,%2
%endmacro

DisplayGUISound:                     ; Sound Related Options
  GUIDrawWindowBox 6,GUISoundDisp

  GUIDisplayTextY 6,6,16,GUISoundTextA
  GUIDisplayCheckboxu 6,11,21,soundon,GUISoundTextA1,0
%ifdef __MSDOS__
  GUIDisplayCheckboxu 6,102,21,Force8b,GUISoundTextA4,0
%endif
  GUIDisplayCheckboxu 6,11,30,StereoSound,GUISoundTextA2,0
  GUIDisplayTextY 6,6,53,GUISoundTextB
  GUIDisplayTextY 6,6,76,GUISoundTextC
%ifdef __WIN32__
  GUIDisplayCheckboxu 6,11,101,PrimaryBuffer,GUISoundTextD1,4
%endif
  GUIDisplayCheckboxu 6,11,111,RevStereo,GUISoundTextD2,2
  GUIDisplayCheckboxu 6,11,121,Surround,GUISoundTextD3,1
  GUIDisplayCheckboxu 6,11,131,SoundBufEn,GUISoundTextD4,6
  GUIDisplayCheckboxu 6,11,141,SPCDisable,GUISoundTextD5,0

  GUIDisplayTextY 6,6,158,GUISoundTextE
  GUIDisplayCheckboxun 6,11,163,SoundInterpType,1,GUISoundTextE1,0
  GUIDisplayCheckboxun 6,11,173,SoundInterpType,2,GUISoundTextE2,0
  cmp byte[MMXSupport],0
  je near .no8ptinterp
  GUIDisplayCheckboxun 6,11,183,SoundInterpType,3,GUISoundTextE3,0
.no8ptinterp

  GUIDisplayTextY 6,106,158,GUISoundTextF
  GUIDisplayCheckboxun 6,111,163,LowPassFilterType,1,GUISoundTextF1,1
  GUIDisplayCheckboxun 6,111,173,LowPassFilterType,2,GUISoundTextF2,1
  cmp byte[MMXSupport],0
  je near .nohiqualityfilter
  GUIDisplayCheckboxun 6,111,183,LowPassFilterType,3,GUISoundTextF3,0
.nohiqualityfilter

%ifdef __MSDOS__
  cmp byte[SoundQuality],2
  jbe .okay
  cmp byte[SoundQuality],4
  je .okay
  cmp byte[StereoSound],1
  jne .okay
  cmp byte[SBHDMA],0
  jne .okay
  cmp byte[vibracard], 1
  je .okay
  GUIDisplayBBox 6,15,61,69,69,167
  GUIDisplayTextG 6,23,64,GUISoundTextB0
  jmp .nosrate
.okay
%endif
  GUIDisplayBBox 6,15,61,69,69,167            ;Sampling Rate Box
  mov eax,[SoundQuality]
  lea eax,[eax*8+GUISoundTextB1]
  mov [GUITemp],eax
  GUIDisplayTextG 6,23,64,[GUITemp]
.nosrate

  GUIDrawSlider 6,15,100,91,MusicRelVol,GUISoundTextC1,VolslidSet,VolslidText
  ret

SECTION .data
GUISoundTextA db 'SOUND SWITCHES:',0
GUISoundTextA1 db 'ENABLE SOUND',0
GUISoundTextA2 db 'STEREO SOUND',0
GUISoundTextA4 db 'FORCE 8-BIT',0        ; DOS Only

GUISoundTextB db 'SAMPLING RATE:',0
GUISoundTextB0 db 'N/A',0
GUISoundTextB1 db ' 8000HZ',0
GUISoundTextB2 db '11025HZ',0
GUISoundTextB3 db '22050HZ',0
GUISoundTextB4 db '44100HZ',0
GUISoundTextB5 db '16000HZ',0
GUISoundTextB6 db '32000HZ',0
GUISoundTextB7 db '48000HZ',0

GUISoundTextC db 'VOLUME LEVEL:',0
GUISoundTextC1 db '---%',0

GUISoundTextD1 db 'USE PRIMARY BUFFER',0 ; Win Only
GUISoundTextD2 db 'REVERSE STEREO',0
GUISoundTextD3 db 'SURROUND SOUND',0
GUISoundTextD4 db 'SOUND BUFFERING',0
GUISoundTextD5 db 'DISABLE SPC EMULATION',0

GUISoundTextE db 'INTERPOLATION:',0
GUISoundTextE1 db 'GAUSSIAN',0
GUISoundTextE2 db 'CUBIC SPLINE',0
GUISoundTextE3 db '8-POINT',0

GUISoundTextF db 'LOWPASS:',0
GUISoundTextF1 db 'SIMPLE',0
GUISoundTextF2 db 'DYNAMIC',0
GUISoundTextF3 db 'HI QUALITY',0

SECTION .text

DisplayGUICheatConv:
  mov eax,[ccheatnpos]
  add eax,4
  xor ebx,ebx
  mov ecx,3
  mov edx,GUICheatTextZ3
.loop
  mov bl,[eax]
  and bl,0Fh
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx+1],bl
  mov bl,[eax]
  and bl,0F0h
  shr bl,4
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx],bl
  add edx,2
  dec eax
  dec ecx
  jnz .loop
  mov eax,[ccheatnpos]
  inc eax
  mov edx,GUICheatTextZ3+7
  mov bl,[eax]
  and bl,0Fh
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx+1],bl
  mov bl,[eax]
  and bl,0F0h
  shr bl,4
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx],bl
  mov eax,[ccheatnpos]
  add eax,5
  mov edx,GUICheatTextZ3+10
  mov bl,[eax]
  and bl,0Fh
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx+1],bl
  mov bl,[eax]
  and bl,0F0h
  shr bl,4
  mov bl,[GUICheatTextZ4+ebx]
  mov [edx],bl
  mov eax,[ccheatnpos]
  test byte[eax-28],80h
  jnz .src
  test byte[eax],04h
  jz .on
  mov byte[GUICheatTextZ3+13],'O'
  mov byte[GUICheatTextZ3+14],'F'
  mov byte[GUICheatTextZ3+15],'F'
  jmp .off
.on
  test byte[eax],80h
  jnz .repl
  mov byte[GUICheatTextZ3+13],'O'
  mov byte[GUICheatTextZ3+14],'N'
  mov byte[GUICheatTextZ3+15],' '
  jmp .off
.repl
  mov byte[GUICheatTextZ3+13],'R'
  mov byte[GUICheatTextZ3+14],'P'
  mov byte[GUICheatTextZ3+15],'L'
  jmp .off
.src
  mov byte[GUICheatTextZ3+13],'S'
  mov byte[GUICheatTextZ3+14],'R'
  mov byte[GUICheatTextZ3+15],'C'
.off
  mov ecx,20
  add eax,8
  mov edx,GUICheatTextZ3+17
.loop2
  mov bl,[eax]
  mov [edx],bl
  inc eax
  inc edx
  dec ecx
  jnz .loop2
  ret

%macro GUIOuttextwin2cheat 2
  test dword[ccheatnleft],80000000h
  jnz near %%skip
  call DisplayGUICheatConv
  mov byte[GUItextcolor],223
  GUIOuttextwin2 7,%1,%2,GUICheatTextZ3
  mov byte[GUItextcolor],221               ;Text
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 7,%1-1,%2-1,GUICheatTextZ3
  add dword[ccheatnpos],28
  dec dword[ccheatnleft]
%%skip
%endmacro

DisplayGUICheat:       ; Add/Browse Cheats menu
  GUIDrawWindowBox 7,GUICheatDisp

  GUIDisplayText 7,6,13,GUICheatTextA
  GUIDisplayText 7,6,132,GUICheatTextC1
  GUIDisplayText 7,6,143,GUICheatTextC2
  GUIDisplayText 7,11,154,GUICheatTextD1
  GUIDisplayText 7,11,164,GUICheatTextD2
  GUIDisplayText 7,11,172,GUICheatTextD3
  GUIDisplayText 7,11,180,GUICheatTextD4

  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero
  mov byte[GUItextcolor],211
.zero
  DrawGUIButton 7,5,113,47,124,GUICheatTextB1,5,0,0      ; Draw Buttons
  DrawGUIButton 7,52,113,94,124,GUICheatTextB2,6,0,0
  DrawGUIButton 7,99,113,141,124,GUICheatTextB3,7,0,0
  DrawGUIButton 7,146,113,188,124,GUICheatTextB4,8,0,0
  DrawGUIButton 7,193,113,235,124,GUICheatTextB5,33,0,0
  DrawGUIButton 7,212,134,236,145,GUICheatTextB6,9,0,0

  GUIDisplayBBoxS 7,5,20,229,108,167           ; Draw Cheat Box

  cmp dword[GUIcurrentcheatwin],0     ; Red Highlight for Cheats box
  jne near .nowinbox
  mov ebx,[GUIcurrentcheatcursloc]
  sub ebx,[GUIcurrentcheatviewloc]
  mov eax,ebx
  lea ebx,[ebx*8]
  sub ebx,eax
  add ebx,22
  DrawGUIWinBox2 7,5,229,7,224
.nowinbox

  mov dword[ccheatnpos],cheatdata         ; Green Text
  mov eax,[NumCheats]
  sub eax,[GUIcurrentcheatviewloc]
  dec eax
  mov [ccheatnleft],eax
  mov eax,[GUIcurrentcheatviewloc]
  mov ebx,eax
  shl eax,5
  sub eax,ebx
  sub eax,ebx
  sub eax,ebx
  sub eax,ebx
  add [ccheatnpos],eax
  GUIOuttextwin2cheat 12,24
  GUIOuttextwin2cheat 12,24+7
  GUIOuttextwin2cheat 12,24+7*2
  GUIOuttextwin2cheat 12,24+7*3
  GUIOuttextwin2cheat 12,24+7*4
  GUIOuttextwin2cheat 12,24+7*5
  GUIOuttextwin2cheat 12,24+7*6
  GUIOuttextwin2cheat 12,24+7*7
  GUIOuttextwin2cheat 12,24+7*8
  GUIOuttextwin2cheat 12,24+7*9
  GUIOuttextwin2cheat 12,24+7*10
  GUIOuttextwin2cheat 12,24+7*11

;Scrollbar
  DrawSlideBarWin 7,231,28,[GUIcurrentcheatviewloc],[NumCheats],12,73,GUICStA
  cmp byte[GUICHold],7
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin 7,231,20,GUIIconDataUpArrow
  cmp byte[GUICHold],7
  jne .noholda2
  sub byte[GUIWincoladd],3
.noholda2
  cmp byte[GUICHold],8
  jne .noholdb
  add byte[GUIWincoladd],3
.noholdb
  GUIDisplayIconWin 7,231,101,GUIIconDataDownArrow
  cmp byte[GUICHold],8
  jne .noholdb2
  sub byte[GUIWincoladd],3
.noholdb2

  mov ebx,[GUIcurrentcheatcursloc]    ; Draw input boxes
  sub ebx,[GUIcurrentcheatviewloc]
  mov eax,ebx
  lea ebx,[ebx*4]
  add ebx,eax
  add ebx,eax
  add ebx,eax
  add ebx,22

  mov dl,167                           ;Code Box
  cmp dword[GUIcurrentcheatwin],1
  jne .notopibox
  mov dl,226
  cmp byte[GUIWincoladd],0
  je .nocoladd
  inc dl
.nocoladd
.notopibox
  GUIDisplayBBox 7,82,129,172,136,dl

  mov dl,167                           ;Descrip. Box
  cmp dword[GUIcurrentcheatwin],2
  jne .nobotibox
  mov dl,226
  cmp byte[GUIWincoladd],0
  je .nocoladd2
  inc dl
.nocoladd2
.nobotibox
  GUIDisplayBBox 7,82,140,196,147,dl

  GUIDisplayTextG 7,84,132,GUICheatTextZ1           ;Green Text&Shadow
  GUIDisplayTextG 7,84,143,GUICheatTextZ2

  xor eax,eax                 ; Code for movement of cursor
  mov al,[GUICheatPosA]
  mov byte[GUICheatTextZ1+eax],0
  xor ebx,ebx
  mov bl,[GUICheatPosB]
  mov byte[GUICheatTextZ2+ebx],0
  test byte[GUICCFlash],8
  jnz .nound
  cmp dword[GUIcurrentcheatwin],1
  je .win1
  cmp dword[GUIcurrentcheatwin],2
  je .win2
  jmp .nound
.win1
  mov byte[GUICheatTextZ1+eax],'_'
  jmp .nound
.win2
  mov byte[GUICheatTextZ2+ebx],'_'
.nound

  GUIDisplayCheckbox 7,11,186,AutoLoadCht,GUICheatTextE1          ; Checkbox
  cmp dword[GUIcurrentcheatwin],0     ; Draw underline only if you
  jne near .noshortcut                ; don't have an input box selected
  GUIDisplayCheckboxu 7,11,186,AutoLoadCht,GUICheatTextE1,0
.noshortcut
  ret

SECTION .data
GUICheatTextA db 'ADDRESS CV PV TGL DESCRIPTION',0   ; Top

GUICheatTextB1 db 'REMOVE',0           ; Buttons
GUICheatTextB2 db 'TOGGLE',0
GUICheatTextB3 db 'SAVE',0
GUICheatTextB4 db 'LOAD',0
GUICheatTextB5 db 'FIX',0
GUICheatTextB6 db 'ADD',0

GUICheatTextC1 db 'ENTER CODE:',0     ; Text by input boxes
GUICheatTextC2 db 'DESCRIPTION:',0

GUICheatTextD1 db 'VALID CODES: GAME GENIE, PAR, AND GF',0  ; Info for User
GUICheatTextD2 db 'NOTE: YOU MAY HAVE TO RESET THE GAME',0
GUICheatTextD3 db 'AFTER ENTERING THE CODE. REMEMBER TO',0
GUICheatTextD4 db 'INSERT THE "-" FOR GAME GENIE CODES.',0

GUICheatTextE1 db 'AUTO-LOAD .CHT FILE AT GAME LOAD',0  ; Checkbox Text

GUICheatTextZ1 db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 14 characters
GUICheatTextZ2 db '_',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 18 characters
GUICheatTextZ3 db '000000 00 00 OFF BLAHBLAH---',0,0,0,0,0,0,0,0,0,0,0,0
GUICheatTextZ4 db '0123456789ABCDEF'
GUICheatTextZ5 db '      ',0

SECTION .bss
GUICheatPosA resb 1
GUICheatPosB resb 1
GUICStA resd 3
GUIcurrentcheatviewloc resd 1
NEWSYM GUIcurrentcheatcursloc, resd 1
GUIcurrentcheatwin     resd 1
ccheatnpos             resd 1
ccheatnleft            resd 1

SECTION .text

DrawWindowSearch:
  GUIDrawWindowBox 13,GUISearchDisp
  ret

CSRemoveFlash:
.loop
  cmp byte[esi],'_'
  je .flash
  cmp byte[esi],0
  je .notflash
  inc esi
  jmp .loop
.flash
  mov byte[esi],0
.notflash
  ret

CSAddFlash:
.loop
  cmp byte[esi],'_'
  je .flash
  cmp byte[esi],0
  je .notflash
  inc esi
  jmp .loop
.notflash
  mov byte[esi],'_'
  mov byte[esi+1],0
.flash
  ret

DisplayGUISearch:
  cmp byte[CheatWinMode],1    ; Determine which CS window we're on
  je near Incheatmode
  cmp byte[CheatWinMode],2
  je near Cheatmodeview
  cmp byte[CheatWinMode],3
  je near Cheatmodeadd
  mov dword[GUIwinsizex+13*4],170
  mov dword[GUIwinsizey+13*4],150
  call DrawWindowSearch

  GUIDisplayTextY 13,6,16,GUICSrcTextA1
  ; Radio Buttons
  GUIDisplayButtonHoleTu 13,11,28,byte[CheatSrcByteSize],0,GUICSrcTextA1a,0
  GUIDisplayButtonHoleTu 13,11,38,byte[CheatSrcByteSize],1,GUICSrcTextA1b,0
  GUIDisplayButtonHoleTu 13,11,48,byte[CheatSrcByteSize],2,GUICSrcTextA1c,0
  GUIDisplayButtonHoleTu 13,11,58,byte[CheatSrcByteSize],3,GUICSrcTextA1d,0
  GUIDisplayButtonHoleTu 13,11,73,byte[CheatSrcByteBase],0,GUICSrcTextA1e,0
  GUIDisplayButtonHoleTu 13,11,83,byte[CheatSrcByteBase],1,GUICSrcTextA1f,0

  GUIDisplayTextY 13,6,101,GUICSrcTextA2
  GUIDisplayButtonHoleTu 13,11,113,byte[CheatSrcSearchType],0,GUICSrcTextA2a,0
  GUIDisplayButtonHoleTu 13,11,123,byte[CheatSrcSearchType],1,GUICSrcTextA2b,0

  mov byte[GUItextcolor],217          ; Button
  cmp byte[GUIWincoladd],0
  je .zero2
  mov byte[GUItextcolor],211
.zero2
  DrawGUIButton 13,95,140,140,152,GUICSrcTextA3,50,0,1
  ret

Incheatmode:                              ; Return and Re-search Window
  mov dword[GUIwinsizex+13*4],180
  mov dword[GUIwinsizey+13*4],150
  call DrawWindowSearch

  mov byte[GUItextcolor],217            ; Text And Shadow
  cmp byte[GUIWincoladd],0
  je .zero2
  mov byte[GUItextcolor],211
.zero2
  cmp byte[CheatSearchStatus],1
  jne near CheatSearching
  GUIDisplayText 13,5,20,GUICSrcTextC1
  GUIDisplayText 13,5,30,GUICSrcTextC2
  GUIDisplayText 13,5,40,GUICSrcTextC3
  GUIDisplayText 13,5,50,GUICSrcTextC4
  jmp DisplayChtSrcRes.nosearch

CheatSearching:                       ; Exact Value Search
  cmp byte[CheatSrcSearchType],1
  je near CheatSearchingComp

  GUIDisplayText 13,5,20,GUICSrcTextB1
  GUIDisplayText 13,5,65,GUICSrcTextG1a

  GUIDisplayBBox 13,10,40,80,47,167           ; Input Box

  test byte[GUICCFlash],8            ; Flash Cursor Code?
  jnz .nound
  mov esi,CSInputDisplay
  call CSRemoveFlash
.nound

  mov byte[GUItextcolor],223             ; Green Shadow
  cmp byte[CSOverValue],1
  jne .notdark
  mov byte[GUItextcolor],202             ; Alt Color
.notdark
  GUIOuttextwin2 13,13,42,CSInputDisplay

  mov byte[GUItextcolor],221            ; Green Text
  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],222
.zero4
  cmp byte[CSOverValue],1
  jne .notdark2
  mov byte[GUItextcolor],207             ; Alt Color
.notdark2
  GUIOuttextwin2 13,12,41,CSInputDisplay

  mov esi,CSInputDisplay          ; More flash?
  call CSAddFlash

  xor eax,eax                 ; Find Max Size
  mov al,[CheatSrcByteSize]
  mov eax,[SrcMask+eax*4]
  mov esi,GUICSrcTextG1
  cmp byte[CheatSrcByteBase],1
  je .hex
  call convertnum
  jmp .dec
.hex
  xor ecx,ecx
  mov cl,[CheatSrcByteSize]
  inc ecx
  call converthex
.dec
  GUIDisplayText 13,71,65,GUICSrcTextG1          ; Max Size Text
  jmp DisplayChtSrcRes

CheatSearchingComp:           ; Comparative search
  GUIDisplayTextY 13,6,16,GUICSrcTextD
  GUIDisplayButtonHoleTu 13,11,33,byte[CheatCompareValue],0,GUICSrcTextD1,0
  GUIDisplayButtonHoleTu 13,11,43,byte[CheatCompareValue],1,GUICSrcTextD2,1
  GUIDisplayButtonHoleTu 13,11,53,byte[CheatCompareValue],2,GUICSrcTextD3,2
  GUIDisplayButtonHoleTu 13,11,63,byte[CheatCompareValue],3,GUICSrcTextD4,5
  jmp DisplayChtSrcRes

FindChtSrcRes:                        ; Calculate search results
  inc edi
  mov eax,[vidbuffer]
  add eax,129600+65536*2
  mov ecx,16384
  xor esi,esi
  xor ebx,ebx
.searchloop
  mov dl,[eax]
  mov dh,8
.sloop2
  test dl,1
  jz .notset
  inc ebx
  dec edi
  jz .found
.notset
  inc esi
  shr dl,1
  dec dh
  jnz .sloop2
  inc eax
  dec ecx
  jnz .searchloop
  mov eax,ebx
  ret
.found
  mov [CSStartEntry],esi
  jmp .notset

DisplayChtSrcRes:                    ; Buttons (Restart/View/Search)
  DrawGUIButton 13,120,140,170,152,GUICSrcTextG2a,53,0,1
.nosearch
  DrawGUIButton 13,10,140,60,152,GUICSrcTextG2b,51,0,1
  DrawGUIButton 13,70,140,110,152,GUICSrcTextG2c,52,0,1

  xor edi,edi                      ; Call and display # of results
  call FindChtSrcRes
  mov esi,GUICSrcTextG1
  call convertnum
  GUIDisplayText 13,12,125,GUICSrcTextG1b
  GUIDisplayText 13,97,125,GUICSrcTextG1
  mov dword[GUIcurrentchtsrcviewloc],0
  mov dword[GUIcurrentchtsrccursloc],0
  ret

Cheatmodeview:                       ; View ResultsWindow
  mov byte[GUICSrcTextE+12],32
  cmp byte[CheatSrcByteSize],3
  jne .yesprev
  cmp byte[CheatSrcByteBase],0
  jne .yesprev
  mov byte[GUICSrcTextE+12],0
.yesprev
  mov dword[GUIwinsizex+13*4],185
  mov dword[GUIwinsizey+13*4],150
  call DrawWindowSearch

  GUIDisplayText 13,10,12,GUICSrcTextE   ; Text

  GUIDisplayBBoxS 13,5,20,171,108,167      ; Box

  mov edi,[GUIcurrentchtsrcviewloc]
  call FindChtSrcRes
  mov [NumCheatSrc],eax
  mov [ccheatnpos],esi

  mov byte[GUItextcolor],223              ; Display Window Contents
  mov eax,[NumCheatSrc]
  sub eax,[GUIcurrentchtsrcviewloc]
  cmp eax,12
  jbe .notof
  mov eax,12
.notof
  mov [ccheatnleft],eax
  mov [ccheatnleftb],eax

  mov dword[CheatSearchYPos],24
  mov dword[CheatSearchXPos],10
  mov eax,[CSStartEntry]
  mov [CSCurEntry],eax

  cmp dword[ccheatnleft],0
  je near .noentry
  mov byte[CheatLooped],0
  mov ebx,[GUIcurrentchtsrccursloc]
  sub ebx,[GUIcurrentchtsrcviewloc]
  mov [curentryleft],ebx
  mov eax,ebx
  lea ebx,[ebx*8]
  sub ebx,eax
  add ebx,22
  DrawGUIWinBox2 13,5,171,7,224
.nextdisplay
  cmp dword[curentryleft],0
  jne .notzero
  mov eax,[CSCurEntry]
  mov [curentryval],eax
.notzero
  dec dword[curentryleft]

  mov esi,GUICSrcTextG1
  mov ecx,3
  mov eax,[CSCurEntry]
  add eax,7E0000h
  call converthex
  GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextG1

  mov esi,GUICSrcTextG1
  mov ebx,[wramdata]
  add ebx,[CSCurEntry]
  mov eax,[ebx]
  cmp byte[CheatSrcByteBase],0
  je .dec
  xor ecx,ecx
  mov cl,[CheatSrcByteSize]
  inc ecx
  call converthex
  jmp .hex
.dec
  xor ebx,ebx
  mov bl,[CheatSrcByteSize]
  and eax,[SrcMask+ebx*4]
  call convertnum
.hex
  add dword[CheatSearchXPos],42
  GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextG1

  add dword[CheatSearchXPos],60
  cmp byte[GUICSrcTextE+12],0
  je near .noprevious
  mov esi,GUICSrcTextG1
  mov ebx,[vidbuffer]
  add ebx,129600
  add ebx,[CSCurEntry]
  mov eax,[ebx]
  cmp byte[CheatSrcByteBase],0
  je .decb
  xor ecx,ecx
  mov cl,[CheatSrcByteSize]
  inc ecx
  call converthex
  jmp .hexb
.decb
  xor ebx,ebx
  mov bl,[CheatSrcByteSize]
  and eax,[SrcMask+ebx*4]
  call convertnum
.hexb
  GUIOuttextwin2 13,[CheatSearchXPos],[CheatSearchYPos],GUICSrcTextG1
.noprevious
  sub dword[CheatSearchXPos],102
  add dword[CheatSearchYPos],7

.nextentry                                      ; Search for next entry
  inc dword[CSCurEntry]
  mov eax,[CSCurEntry]
  mov ecx,eax
  and ecx,7
  mov edx,1
  shr eax,3
  shl edx,cl
  add eax,dword[vidbuffer]
  add eax,129600+65536*2
  test byte[eax],dl
  jz .nextentry
  dec dword[ccheatnleft]
  jnz near .nextdisplay
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],222
.zero4
  mov dword[CheatSearchYPos],23
  mov dword[CheatSearchXPos],11
  mov eax,[CSStartEntry]
  mov [CSCurEntry],eax
  mov eax,[ccheatnleftb]
  mov [ccheatnleft],eax
  inc byte[CheatLooped]
  cmp byte[CheatLooped],2
  jne near .nextdisplay
.noentry
; Slidebar
  ; win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
  DrawSlideBarWin 13,173,28,[GUIcurrentchtsrcviewloc],[NumCheatSrc],12,73,GUICSStA
  cmp byte[GUICHold],11
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin 13,173,20,GUIIconDataUpArrow
  cmp byte[GUICHold],11
  jne .noholda2
  sub byte[GUIWincoladd],3
.noholda2
  cmp byte[GUICHold],12
  jne .noholdb
  add byte[GUIWincoladd],3
.noholdb
  GUIDisplayIconWin 13,173,101,GUIIconDataDownArrow
  cmp byte[GUICHold],12
  jne .noholdb2
  sub byte[GUIWincoladd],3
.noholdb2
  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero3
  mov byte[GUItextcolor],211
.zero3
  DrawGUIButton 13,70,140,130,152,GUICSrcTextG2d,54,0,1
  DrawGUIButton 13,140,140,180,152,GUICSrcTextG2e,55,0,1
  ret

Cheatmodeadd:                                 ; Add Window
  mov dword[GUIwinsizex+13*4],170
  mov dword[GUIwinsizey+13*4],165
  call DrawWindowSearch

  GUIDisplayText 13,5,20,GUICSrcTextF1      ; Text
  GUIDisplayText 13,5,45,GUICSrcTextF2
  GUIDisplayText 13,5,70,GUICSrcTextF3

  GUIDisplayCheckbox 13,8,139,CheatUpperByteOnly,GUICSrcTextF4   ; Checkbox

  GUIDisplayBBox 13,10,30,80,37,167             ; Boxes
  GUIDisplayBBox 13,10,55,126,62,167
  GUIDisplayBBox 13,10,80,80,120,167

  DrawGUIButton 13,60,155,120,167,GUICSrcTextG2d,56,0,1        ; Buttons
  DrawGUIButton 13,130,155,160,167,GUICSrcTextG2e,57,0,1

  GUIDisplayText 13,5,130,GUICSrcTextG1a              ; Max Value Text
  xor eax,eax
  mov al,[CheatSrcByteSize]
  mov eax,[SrcMask+eax*4]
  mov esi,GUICSrcTextG1
  cmp byte[CheatSrcByteBase],1
  je .hex
  call convertnum
  jmp .dec
.hex
  xor ecx,ecx
  mov cl,[CheatSrcByteSize]
  inc ecx
  call converthex
.dec
  GUIDisplayText 13,71,130,GUICSrcTextG1

  cmp byte[CurCStextpos],0               ; Cheat Input
  jne near .nofirstbox
  test byte[GUICCFlash],8
  jnz .nound
.nofirstbox
  mov esi,CSInputDisplay
  call CSRemoveFlash
.nound
  mov byte[GUItextcolor],223
  cmp byte[CSOverValue],1
  jne .notdark
  mov byte[GUItextcolor],202
.notdark
  GUIOuttextwin2 13,13,32,CSInputDisplay
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],222
.zero4
  cmp byte[CSOverValue],1
  jne .notdark2
  mov byte[GUItextcolor],207
.notdark2
  GUIOuttextwin2 13,12,31,CSInputDisplay
  mov esi,CSInputDisplay
  call CSAddFlash

  cmp byte[CurCStextpos],1               ; Cheat Desc. Input
  jne .noundb
  test byte[GUICCFlash],8
  jnz .noundb
  mov esi,CSDescDisplay
  call CSAddFlash
.noundb
  GUIDisplayTextG 13,13,57,CSDescDisplay
  mov esi,CSDescDisplay
  call CSRemoveFlash

  cmp byte[CSOverValue],1
  je near .nodisplay
  cmp byte[CSInputDisplay],'_'
  je near .nodisplay

  mov dword[CheatSearchYPos],83           ; PAR Code?
  mov eax,[curentryval]
  mov [curaddrvalcs],eax
  mov eax,[CSCurValue]
  mov [curvaluecs],eax
  xor ecx,ecx
  mov cl,[CheatSrcByteSize]
  inc cl
  cmp byte[CheatUpperByteOnly],0
  je .cspardisploop
  mov cl,1
.cspardispagain
  cmp dword[curvaluecs],0FFh
  jbe .cspardisploop
  shr dword[curvaluecs],8
  inc dword[curaddrvalcs]
  jmp .cspardispagain
.cspardisploop                               ; Max Value Display?
  push ecx
  mov esi,GUICSrcTextG1
  mov ecx,3
  mov eax,[curaddrvalcs]
  add eax,7E0000h
  call converthex
  mov esi,GUICSrcTextG1+6
  mov ecx,1
  mov al,[curvaluecs]
  call converthex
  shr dword[curvaluecs],8
  mov byte[GUItextcolor],223
  inc dword[CheatSearchYPos]
  GUIOuttextwin2 13,13,[CheatSearchYPos],GUICSrcTextG1
  dec dword[CheatSearchYPos]
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zero4c
  mov byte[GUItextcolor],222
.zero4c
  GUIOuttextwin2 13,12,[CheatSearchYPos],GUICSrcTextG1
  add dword[CheatSearchYPos],10
  inc dword[curaddrvalcs]
  pop ecx
  dec ecx
  jnz near .cspardisploop
.nodisplay
  ret

SECTION .bss
NEWSYM CheatWinMode, resb 1
NEWSYM CheatSearchStatus, resb 1
CheatCompareValue resd 1
NumCheatSrc       resd 1
GUIcurrentchtsrcviewloc resd 1
GUIcurrentchtsrccursloc resd 1
CSStartEntry      resd 1
CSCurEntry      resd 1
CheatSearchXPos   resd 1
CheatSearchYPos   resd 1
CheatLooped       resd 1
ccheatnleftb      resd 1
CurCStextpos      resd 1

SECTION .data
SrcMask dd 0FFh,0FFFFh,0FFFFFFh,0FFFFFFFFh
CSInputDisplay db '_',0,'         ',0
CSDescDisplay  db '                  ',0,0

SECTION .bss
curentryval resd 1
curentryleft resd 1
curaddrvalcs resd 1
curvaluecs resd 1
GUICSStA resd 3

SECTION .data

; A = Opening Screen
; B = Exact Value Search
; C = Comparison Search Message
; D = Comparison Search Method
; E = View Results
; F = Add Cheat
; G = General Use

GUICSrcTextA1 db 'SELECT SIZE AND FORMAT:',0
GUICSrcTextA1a db '1 BYTE  [0..255]',0
GUICSrcTextA1b db '2 BYTES [0..65535]',0
GUICSrcTextA1c db '3 BYTES [0..16777215]',0
GUICSrcTextA1d db '4 BYTES [0..4294967295]',0
GUICSrcTextA1e db 'DEC (BASE 10)',0
GUICSrcTextA1f db 'HEX (BASE 16)',0
GUICSrcTextA2 db 'SELECT SEARCH TYPE:',0
GUICSrcTextA2a db 'EXACT VALUE SEARCH',0
GUICSrcTextA2b db 'COMPARATIVE SEARCH',0
GUICSrcTextA3 db 'START',0                    ; Button

GUICSrcTextB1 db 'ENTER VALUE:',0             ; Make Yellow

GUICSrcTextC1 db 'NOW RETURN TO YOUR GAME',0
GUICSrcTextC2 db 'AND COME BACK WHEN ',0
GUICSrcTextC3 db 'THE NEXT SEARCH',0
GUICSrcTextC4 db 'SHOULD BE PROCESSED',0

GUICSrcTextD db 'SELECT COMPARISON:',0
GUICSrcTextD1 db 'NEW VALUE IS > OLD VALUE',0
GUICSrcTextD2 db 'NEW VALUE IS < OLD VALUE',0
GUICSrcTextD3 db 'NEW VALUE IS = OLD VALUE',0
GUICSrcTextD4 db 'NEW VALUE IS != OLD VALUE',0

GUICSrcTextE db 'ADDR   VALUE     PVALUE',0

GUICSrcTextF1 db 'ENTER NEW VALUE:',0
GUICSrcTextF2 db 'ENTER CHEAT DESCRIPTION:',0
GUICSrcTextF3 db 'PAR CODE EQUIVALENT:',0
GUICSrcTextF4 db 'USE ONLY UPPER BYTE',0

GUICSrcTextG1 db '          ',0       ; All variables
GUICSrcTextG1a db 'MAX VALUE:',0
GUICSrcTextG1b db '# OF RESULTS:',0
GUICSrcTextG2a db 'SEARCH',0          ; Buttons
GUICSrcTextG2b db 'RESTART',0
GUICSrcTextG2c db 'VIEW',0
GUICSrcTextG2d db 'RETURN',0
GUICSrcTextG2e db 'ADD',0

SECTION .bss
CCursLoc resd 1
CViewLoc resd 1
NViewLoc resd 1
TViewLoc resd 1

SECTION .text

DisplayNetOptns:
  ret

SECTION .bss
NEWSYM GUINCStA, resd 3
FileNameMod resb 512
CurCStringPos resd 1

SECTION .text

DisplayGameOptns:        ; Misc Key Window
  GUIDrawWindowBox 9,GUIGameDisp

  GUIDisplayTextY 9,6,16,GUIGameOptnsTextA
  mov byte[GUIGameOptnsTextA1+2],'1'
  GUIDisplayText 9,9,25,GUIGameOptnsTextA1
  mov byte[GUIGameOptnsTextA1+2],'2'
  GUIDisplayText 9,9+45,25,GUIGameOptnsTextA1
  mov byte[GUIGameOptnsTextA1+2],'3'
  GUIDisplayText 9,9+90,25,GUIGameOptnsTextA1
  mov byte[GUIGameOptnsTextA1+2],'4'
  GUIDisplayText 9,9+135,25,GUIGameOptnsTextA1
  GUIDisplayText 9,9+180,25,GUIGameOptnsTextA2

  GUIDisplayTextY 9,6,34,GUIGameOptnsTextB
  mov byte[GUIGameOptnsTextB1+2],'1'
  GUIDisplayText 9,9,43,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'2'
  GUIDisplayText 9,9+45,43,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'3'
  GUIDisplayText 9,9+45*2,43,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'4'
  GUIDisplayText 9,9+45*3,43,GUIGameOptnsTextB1
  GUIDisplayText 9,9+45*4,43,GUIGameOptnsTextB2
  mov byte[GUIGameOptnsTextB1+2],'5'
  GUIDisplayText 9,9,52,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'6'
  GUIDisplayText 9,9+45,52,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'7'
  GUIDisplayText 9,9+45*2,52,GUIGameOptnsTextB1
  mov byte[GUIGameOptnsTextB1+2],'8'
  GUIDisplayText 9,9+45*3,52,GUIGameOptnsTextB1
  GUIDisplayText 9,9+45*4,52,GUIGameOptnsTextB3

  GUIDisplayTextY 9,6,61,GUIGameOptnsTextC
  GUIDisplayText 9,9,72,GUIGameOptnsTextC1
  GUIDisplayText 9,9+52,72,GUIGameOptnsTextC2
  GUIDisplayText 9,9+109,72,GUIGameOptnsTextC3
  GUIDisplayText 9,9+160,72,GUIGameOptnsTextC4
  GUIDisplayText 9,9,82,GUIGameOptnsTextC5
  GUIDisplayText 9,9+52,82,GUIGameOptnsTextC6
  GUIDisplayText 9,137,82,GUIGameOptnsTextC7

  GUIDisplayTextY 9,6,93,GUIGameOptnsTextD1
  GUIDisplayText 9,9,102,GUIGameOptnsTextD1a
  GUIDisplayText 9,9,112,GUIGameOptnsTextD1b
  GUIDisplayText 9,9,122,GUIGameOptnsTextD1c
%ifndef __MSDOS__
  GUIDisplayText 9,9,132,GUIGameOptnsTextD1d
%endif

  GUIDisplayTextY 9,119,93,GUIGameOptnsTextD2
  GUIDisplayText 9,122,102,GUIGameOptnsTextD2a
  GUIDisplayText 9,122,112,GUIGameOptnsTextD2b
  GUIDisplayText 9,122,122,GUIGameOptnsTextD2c
  GUIDisplayText 9,122,132,GUIGameOptnsTextD2d
  GUIDisplayText 9,122,142,GUIGameOptnsTextD2e

  ; Draw black boxes
  DDrawBox 9,26,22,[KeyBGDisble0]
  DDrawBox 9,71,22,[KeyBGDisble1]
  DDrawBox 9,116,22,[KeyBGDisble2]
  DDrawBox 9,161,22,[KeyBGDisble3]
  DDrawBox 9,206,22,[KeySprDisble]

  DDrawBox 9,26,40,[KeyDisableSC0]
  DDrawBox 9,71,40,[KeyDisableSC1]
  DDrawBox 9,116,40,[KeyDisableSC2]
  DDrawBox 9,161,40,[KeyDisableSC3]
  DDrawBox 9,213,40,[KeyVolUp]
  DDrawBox 9,26,49,[KeyDisableSC4]
  DDrawBox 9,71,49,[KeyDisableSC5]
  DDrawBox 9,116,49,[KeyDisableSC6]
  DDrawBox 9,161,49,[KeyDisableSC7]
  DDrawBox 9,213,49,[KeyVolDown]

  DDrawBox 9,32,69,[KeyQuickLoad]
  DDrawBox 9,90,69,[KeyQuickRst]
  DDrawBox 9,141,69,[KeyQuickExit]
  DDrawBox 9,199,69,[KeyQuickClock]
  DDrawBox 9,32,79,[KeyQuickChat]
  DDrawBox 9,109,79,[KeyQuickSnapShot]
  DDrawBox 9,185,79,[KeyQuickSaveSPC]

  DDrawBox 9,77,99,[KeyUsePlayer1234]
  DDrawBox 9,77,109,[KeyResetAll]
  DDrawBox 9,77,119,[KeyDisplayFPS]
%ifndef __MSDOS__
  DDrawBox 9,77,129,[KeyDisplayBatt]
%endif

  DDrawBox 9,190,99,[KeyNewGfxSwt]
  DDrawBox 9,190,109,[KeyWinDisble]
  DDrawBox 9,190,119,[KeyOffsetMSw]
  DDrawBox 9,190,129,[KeyIncreaseGamma]
  DDrawBox 9,190,139,[KeyDecreaseGamma]
  ret

SECTION .bss
GUIGameOptnsColA resb 1
GUIGameOptnsColB resb 1
GUIGameDisplayKy resd 1

SECTION .data

GUIGameOptnsTextA db 'BG DISABLES:',0
GUIGameOptnsTextA1 db 'BG#',0
GUIGameOptnsTextA2 db 'OBJ',0

GUIGameOptnsTextB db 'SOUND KEYS:',0
GUIGameOptnsTextB1 db 'CH#',0
GUIGameOptnsTextB2 db '+VOL',0
GUIGameOptnsTextB3 db '-VOL',0

GUIGameOptnsTextC db 'QUICK KEYS:',0
GUIGameOptnsTextC1 db 'LOAD',0
GUIGameOptnsTextC2 db 'RESET',0
GUIGameOptnsTextC3 db 'EXIT',0
GUIGameOptnsTextC4 db 'CLOCK',0
GUIGameOptnsTextC5 db 'CHAT',0
GUIGameOptnsTextC6 db 'SNAPSHOT',0
GUIGameOptnsTextC7 db 'SAVE SPC',0

GUIGameOptnsTextD1 db 'MISC TOGGLES:',0
GUIGameOptnsTextD1a db 'USE PL12/34',0
GUIGameOptnsTextD1b db 'PANIC KEY',0
GUIGameOptnsTextD1c db 'DISPLAY FPS',0
GUIGameOptnsTextD1d db 'BATT POWER',0

GUIGameOptnsTextD2 db 'GFX TOGGLES:',0
GUIGameOptnsTextD2a db 'NEW GFX ENG',0
GUIGameOptnsTextD2b db 'BG WINDOW',0
GUIGameOptnsTextD2c db 'OFFSET MODE',0
GUIGameOptnsTextD2d db '+ GAMMA',0
GUIGameOptnsTextD2e db '- GAMMA',0


SECTION .text

%macro GUICslidSet 1 ; slider var
  mov al,[%1]
  shl eax,2
  add eax,2
%endmacro

%macro GUICslidText 2 ; slider var, text
  mov byte[%2],' '
  mov esi,%2+2
  mov al,[%1]
  mov bl,10
%%asciilp                            ; turns decimal into ascii
  dec esi
  xor ah,ah
  div bl
  add ah,48
  mov [esi],ah
  cmp al,0
  jne %%asciilp
  mov esi,%2
%endmacro

DisplayGUIOptns:              ; GUI Options
%ifdef __WIN32__              ; If Windows, extend window down
  mov dword[GUIwinsizey+10*4],192
%endif
  GUIDrawWindowBox 10,GUIGUIDisp

  ; Setup Colors
  cmp byte[CurPalSelect],1
  je .yespal2b
  cmp byte[CurPalSelect],2
  je .yespal3b
  mov al,[GUIRAdd]
  mov [TRVal2],al
  mov al,[GUIGAdd]
  mov [TGVal2],al
  mov al,[GUIBAdd]
  mov [TBVal2],al
  jmp .nopal1b
.yespal2b
  mov al,[GUITRAdd]
  mov [TRVal2],al
  mov al,[GUITGAdd]
  mov [TGVal2],al
  mov al,[GUITBAdd]
  mov [TBVal2],al
  jmp .nopal1b
.yespal3b
  mov al,[GUIWRAdd]
  mov [TRVal2],al
  mov al,[GUIWGAdd]
  mov [TGVal2],al
  mov al,[GUIWBAdd]
  mov [TBVal2],al
.nopal1b

  GUIDrawSlider 10,25,127,124,TRVal2,GUIGUIOptnsTextD2,GUICslidSet,GUICslidText
  GUIDrawSlider 10,25,127,136,TGVal2,GUIGUIOptnsTextD2,GUICslidSet,GUICslidText
  GUIDrawSlider 10,25,127,148,TBVal2,GUIGUIOptnsTextD2,GUICslidSet,GUICslidText

  GUIDisplayTextY 10,6,16,GUIGUIOptnsTextA

  GUIDisplayCheckboxu 10,124,13,newfont,GUIGUIOptnsTextFont,12

  GUIDisplayCheckboxu 10,12,23,mousewrap,GUIGUIOptnsTextA1a,0     ; Checkboxes
  GUIDisplayCheckboxu 10,12,33,mouseshad,GUIGUIOptnsTextA1b,6
  GUIDisplayCheckboxu 10,12,43,esctomenu,GUIGUIOptnsTextA1c,7
  GUIDisplayCheckboxun 10,12,53,resetposn,0,GUIGUIOptnsTextA1d,5
  ; ^ "Save Win Pos", but the option is really for Reset Pos
  GUIDisplayCheckboxu 10,12,63,GUIClick,GUIGUIOptnsTextA1e,1
  GUIDisplayCheckboxu 10,12,73,JoyPad1Move,GUIGUIOptnsTextA1f,6
  GUIDisplayCheckboxu 10,124,23,GUIEnableTransp,GUIGUIOptnsTextA2a,0
  GUIDisplayCheckboxu 10,124,33,FilteredGUI,GUIGUIOptnsTextA2b,0
  GUIDisplayCheckboxun 10,124,43,GUIEffect,1,GUIGUIOptnsTextA2c,1
  GUIDisplayCheckboxun 10,124,53,GUIEffect,2,GUIGUIOptnsTextA2d,3
  GUIDisplayCheckboxun 10,124,63,GUIEffect,3,GUIGUIOptnsTextA2e,13
  GUIDisplayCheckboxun 10,124,73,GUIEffect,4,GUIGUIOptnsTextA2f,2
%ifdef __WIN32__
  GUIDisplayCheckboxu 10,12,83,MouseWheel,GUIGUIOptnsTextA2g,1
  GUIDisplayCheckboxu 10,124,83,TrapMouseCursor,GUIGUIOptnsTextA2h,3
%endif
  GUIDisplayTextY 10,6,111,GUIGUIOptnsTextB
  GUIDisplayText 10,60,111,GUIGUIOptnsTextB1
  GUIDisplayText 10,100,111,GUIGUIOptnsTextB2
  GUIDisplayText 10,147,111,GUIGUIOptnsTextB3
%ifdef __WIN32__
  GUIDisplayTextY 10,6,161,GUIGUIOptnsTextC
  GUIDisplayCheckboxu 10,12,168,AlwaysOnTop,GUIGUIOptnsTextC1,0
  GUIDisplayCheckboxu 10,12,178,SaveMainWindowPos,GUIGUIOptnsTextC2,2
  GUIDisplayCheckboxu 10,12,188,AllowMultipleInst,GUIGUIOptnsTextC3,1
%endif

  mov byte[GUIGUIOptnsTextD1],'R'
  GUIDisplayText 10,16,123,GUIGUIOptnsTextD1
  mov byte[GUIGUIOptnsTextD1],'G'
  GUIDisplayText 10,16,135,GUIGUIOptnsTextD1
  mov byte[GUIGUIOptnsTextD1],'B'
  GUIDisplayText 10,16,147,GUIGUIOptnsTextD1

  GUIDisplayButtonHole 10,48,108,byte[CurPalSelect],0    ; Radio Buttons
  GUIDisplayButtonHole 10,88,108,byte[CurPalSelect],1
  GUIDisplayButtonHole 10,135,108,byte[CurPalSelect],2
  ret

SECTION .data
GUIGUIOptnsTextA db 'GUI SWITCHES:',0
GUIGUIOptnsTextA1a db 'MOUSE WRAP',0
GUIGUIOptnsTextA1b db 'MOUSE SHADOW',0
GUIGUIOptnsTextA1c db 'ESC TO GAME MENU',0
GUIGUIOptnsTextA1d db 'SAVE WIN POSITNS',0
GUIGUIOptnsTextA1e db 'RCLICK OPENS GUI',0
GUIGUIOptnsTextA1f db 'CTRL GUI W/GPAD1',0

GUIGUIOptnsTextA2a db 'TRANSPARENT MSGS',0
GUIGUIOptnsTextA2b db 'FILTERED GUI',0
GUIGUIOptnsTextA2c dd 'SNOW EFFECT',0
GUIGUIOptnsTextA2d dd 'WATER EFFECT A',0
GUIGUIOptnsTextA2e dd 'WATER EFFECT B',0
GUIGUIOptnsTextA2f dd 'BURNING EFFECT',0
GUIGUIOptnsTextA2g db 'WHEEL MOUSE',0
GUIGUIOptnsTextA2h db 'TRAP MOUSE CURSOR',0

GUIGUIOptnsTextB db 'COLOR:',0
GUIGUIOptnsTextB1 db 'BACK',0
GUIGUIOptnsTextB2 db 'TITLE',0
GUIGUIOptnsTextB3 db 'WIN',0

GUIGUIOptnsTextC dd 'MAIN WINDOW OPTIONS:',0
GUIGUIOptnsTextC1 dd 'ALWAYS ON TOP',0
GUIGUIOptnsTextC2 db 'SAVE MAIN WINDOW POSITION',0
GUIGUIOptnsTextC3 db 'ALLOW MULTIPLE INSTANCES',0

GUIGUIOptnsTextD1 db '@',0
GUIGUIOptnsTextD2 db '  ',0

GUIGUIOptnsTextFont db 'USE CUSTOM FONT',0

SECTION .text

DisplayGUIAbout:          ; Displays the About Box
  ;This will attach compile date onto the end of GUIGUIAboutText1
  pushad
  mov eax,GUIGUIAboutTextA1
  mov [VERSION_STR],eax
  call placedate
  popad

  GUIDrawWindowBox 11,GUIAboutDisp
  cmp byte[EEgg],1
  je near .eegg
  GUIDisplayText 11,6,16,GUIGUIAboutTextA1       ; Text
  GUIDisplayText 11,6,26,GUIGUIAboutTextA2
  GUIDisplayTextY 11,6,36,GUIGUIAboutTextB
  GUIDisplayText 11,6,46,GUIGUIAboutTextB1
  GUIDisplayText 11,6,56,GUIGUIAboutTextB2
  GUIDisplayTextY 11,6,66,GUIGUIAboutTextC
  GUIDisplayText 11,6,76,GUIGUIAboutTextC1
  GUIDisplayText 11,6,86,GUIGUIAboutTextC2
  GUIDisplayText 11,6,96,GUIGUIAboutTextC3
  GUIDisplayText 11,6,106,GUIGUIAboutTextC4
  GUIDisplayText 11,6,116,GUIGUIAboutTextC5
  GUIDisplayText 11,6,131,GUIGUIAboutTextD1
  GUIDisplayText 11,6,141,GUIGUIAboutTextD2
  GUIDisplayText 11,6,151,GUIGUIAboutTextD3
  GUIDisplayText 11,6,161,GUIGUIAboutTextD4
  GUIDisplayText 11,6,171,GUIGUIAboutTextD5
  GUIDisplayText 11,6,181,GUIGUIAboutTextD6
  GUIDisplayText 11,6,191,GUIGUIAboutTextD7

  mov byte[GUItextcolor],217                       ; Buttons
  cmp byte[GUIWincoladd],0
  je .zero3
  mov byte[GUItextcolor],211
.zero3
  DrawGUIButton 11,90,22,175,32,GUIGUIAboutTextE1,65,0,0
  DrawGUIButton 11,90,33,175,43,GUIGUIAboutTextE2,66,0,0
  ret

.eegg                                ;Playground
  GUIDisplayText 11,42,36,GUIGUIAboutTextF
  GUIDisplayText 11,30,96,GUIGUIAboutTextFa
  GUIDisplayText 11,39,106,GUIGUIAboutTextFb
  ret

SECTION .data
GUIGUIAboutTextA1 db 'ZSNES V',ZVERSION,'             ',0 ;Need room for date
%ifdef __MSDOS__
GUIGUIAboutTextA2 db 'DOS VERSION',0
%elifdef __WIN32__
GUIGUIAboutTextA2 db 'WIN VERSION',0
%elifdef __UNIXSDL__
GUIGUIAboutTextA2 db 'SDL VERSION',0
%endif

GUIGUIAboutTextB  db 'CODED BY:  ',0
GUIGUIAboutTextB1 db '    ZSKNIGHT      _DEMO_',0
GUIGUIAboutTextB2 db '    PAGEFAULT     NACH',0

GUIGUIAboutTextC  db 'ASSISTANT CODERS:',0
GUIGUIAboutTextC1 db '    PHAROS        STATMAT',0
GUIGUIAboutTextC2 db '    TEUF          HPSOLO',0
GUIGUIAboutTextC3 db '    THEODDONE33   SILOH',0
GUIGUIAboutTextC4 db '    IPHER         GRINVADER',0
GUIGUIAboutTextC5 db '    JONAS QUINN',0

GUIGUIAboutTextD1 db 'ZSNES comes with ABSOLUTELY',0
GUIGUIAboutTextD2 db 'NO WARRANTY.  This is free',0
GUIGUIAboutTextD3 db 'software, and you are welcome',0
GUIGUIAboutTextD4 db 'to redistribute it under',0
GUIGUIAboutTextD5 db 'certain conditions; please',0
GUIGUIAboutTextD6 db 'read ',39,'LICENSE.TXT',39,0
GUIGUIAboutTextD7 db 'thoroughly before doing so.',0

GUIGUIAboutTextE1 db 'WWW.ZSNES.COM',0
GUIGUIAboutTextE2 db 'DOCUMENTATION',0

GUIGUIAboutTextF db 'HIDDEN MESSAGE!',0
GUIGUIAboutTextFa db 'PRESS ',39,'E',39,' TO RETURN ',0
GUIGUIAboutTextFb db 'TO THE ABOUT BOX',0

SECTION .text

DisplayGUIMovies:    ; Movie Record/Playback options
  GUIDrawWindowBox 15,GUIMovieDisp   ; Display Window

  cmp byte[MovieRecordWinVal],0
  je near .displayboxa

  GUIDisplayText 15,9,16,GUIMovieTextX1       ; Overwrite Message Box
  GUIDisplayText 15,9,26,GUIMovieTextX2
  GUIDisplayText 15,9,41,GUIMovieTextX3

  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],211
.zero4
  DrawGUIButton 15,17,55,59,66,GUIMovieTextX4,19,0,0 ; Yes/No Buttons
  DrawGUIButton 15,70,55,112,66,GUIMovieTextX5,20,0,0
  ret

.displayboxa                            ; Main Window
  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero
  mov byte[GUItextcolor],211
.zero
  DrawGUIButton 15,7,50,49,61,GUIMovieTextA1,16,0,0  ; Draw Buttons
  DrawGUIButton 15,53,50,95,61,GUIMovieTextA2,17,0,0
  DrawGUIButton 15,101,50,145,61,GUIMovieTextA3,18,0,0
  DrawGUIButton 15,151,50,195,61,GUIMovieTextA4,32,0,0
  DrawGUIButton 15,7,68,109,79,GUIMovieTextA5,29,0,0
  DrawGUIButton 15,7,84,109,95,GUIMovieTextA6,30,0,0
  DrawGUIButton 15,7,100,109,111,GUIMovieTextA7,31,0,0
  DrawGUIButton 15,130,165,198,176,GUIMovieTextA8,34,0,0

  GUIDisplayTextY 15,8,16,GUIMovieTextB        ; Slot text
  mov byte[GUIMovieTextB1],'0'
  GUIDisplayText 15,22,28,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'1'
  GUIDisplayText 15,42,28,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'2'
  GUIDisplayText 15,62,28,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'3'
  GUIDisplayText 15,82,28,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'4'
  GUIDisplayText 15,102,28,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'5'
  GUIDisplayText 15,22,38,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'6'
  GUIDisplayText 15,42,38,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'7'
  GUIDisplayText 15,62,38,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'8'
  GUIDisplayText 15,82,38,GUIMovieTextB1
  mov byte[GUIMovieTextB1],'9'
  GUIDisplayText 15,102,38,GUIMovieTextB1

  GUIDisplayTextY 15,115,16,GUIMovieTextC           ; "Start From" Section
  GUIDisplayButtonHoleTu 15,115,25,byte[MovieStartMethod],0,GUIMovieTextC1,0
  GUIDisplayButtonHoleTu 15,152,25,byte[MovieStartMethod],1,GUIMovieTextC2,0
  GUIDisplayButtonHoleTu 15,200,25,byte[MovieStartMethod],2,GUIMovieTextC3,0
  GUIDisplayButtonHoleTu 15,115,35,byte[MovieStartMethod],3,GUIMovieTextC4,6

  GUIDisplayTextY 15,8,120,GUIMovieTextD           ; Video Section
  GUIDisplayButtonHoleTu 15,9,128,byte[MovieVideoMode],0,GUIMovieTextD1,1
  GUIDisplayButtonHoleTu 15,9,138,byte[MovieVideoMode],1,GUIMovieTextD2,2
  GUIDisplayButtonHoleTu 15,9,148,byte[MovieVideoMode],2,GUIMovieTextD3,0
  GUIDisplayButtonHoleTu 15,9,158,byte[MovieVideoMode],3,GUIMovieTextD4,9
  GUIDisplayButtonHoleTu 15,9,168,byte[MovieVideoMode],4,GUIMovieTextD5,0
  GUIDisplayButtonHoleTu 15,9,178,byte[MovieVideoMode],5,GUIMovieTextD6,0

  GUIDisplayTextY 15,129,120,GUIMovieTextE           ; Audio Section
  GUIDisplayCheckboxu 15,130,128,MovieAudio,GUIMovieTextE1,0
  cmp byte[MovieAudio],1
  jne near .nomux
  GUIDisplayCheckboxu 15,130,138,MovieAudioCompress,GUIMovieTextE3,2
  cmp byte[MovieVideoMode],2
  jl near .nomux
  GUIDisplayCheckboxu 15,130,148,MovieVideoAudio,GUIMovieTextE2,11
.nomux

  mov dword[GUIMovieTextZ+8],'INAC'    ; Determine and Display Status
  mov dword[GUIMovieTextZ+12],'TIVE'
  mov dword[GUIMovieTextZ+16],'    '
  cmp byte[MovieProcessing],1
  jne .noplay
  mov dword[GUIMovieTextZ+8],'PLAY'
  mov dword[GUIMovieTextZ+12],'ING '
  mov dword[GUIMovieTextZ+16],'    '
.noplay
  cmp byte[MovieProcessing],2
  jne .norec
  mov dword[GUIMovieTextZ+8],'RECO'
  mov dword[GUIMovieTextZ+12],'RDIN'
  mov dword[GUIMovieTextZ+16],'G   '
.norec
  cmp byte[MovieProcessing],3
  jne .nooldplay
  mov dword[GUIMovieTextZ+8],'OLD '
  mov dword[GUIMovieTextZ+12],'PLAY'
  mov dword[GUIMovieTextZ+16],'ING '
.nooldplay
  GUIDisplayTextY 15,6,192,GUIMovieTextZ
  ; Display Radio buttons
  GUIDisplayButtonHole 15,10,25,byte[CMovieExt],'v'
  GUIDisplayButtonHole 15,30,25,byte[CMovieExt],'1'
  GUIDisplayButtonHole 15,50,25,byte[CMovieExt],'2'
  GUIDisplayButtonHole 15,70,25,byte[CMovieExt],'3'
  GUIDisplayButtonHole 15,90,25,byte[CMovieExt],'4'
  GUIDisplayButtonHole 15,10,35,byte[CMovieExt],'5'
  GUIDisplayButtonHole 15,30,35,byte[CMovieExt],'6'
  GUIDisplayButtonHole 15,50,35,byte[CMovieExt],'7'
  GUIDisplayButtonHole 15,70,35,byte[CMovieExt],'8'
  GUIDisplayButtonHole 15,90,35,byte[CMovieExt],'9'

  DDrawBox 15,115,70,[KeyInsrtChap]        ; Chapter Keyboard Shortcut Boxes
  DDrawBox 15,115,86,[KeyPrevChap]
  DDrawBox 15,115,102,[KeyNextChap]

  GUIDisplayCheckboxu 15,144,67,MovieDisplayFrame,GUIMovieTextB2,0 ; Checkbox
  ret

SECTION .data
GUIMovieTextA1 db 'PLAY',0
GUIMovieTextA2 db 'RECORD',0
GUIMovieTextA3 db 'STOP',0
GUIMovieTextA4 db 'APPEND',0
GUIMovieTextA5 db 'INSERT CHAPTER',0
GUIMovieTextA6 db 'PREVIOUS CHAPTER',0
GUIMovieTextA7 db 'NEXT CHAPTER',0
GUIMovieTextA8 db 'DUMP MOVIE',0

GUIMovieTextB db 'SELECT MOVIE:',0
GUIMovieTextB1 db ' ',0
GUIMovieTextB2 db 'DISPLAY FRAMES',0

GUIMovieTextC db 'RECORD FROM:',0
GUIMovieTextC1 db 'NOW',0
GUIMovieTextC2 db 'POWER',0
GUIMovieTextC3 db 'RESET',0
GUIMovieTextC4 db 'POWER+SRAM CLEAR',0

GUIMovieTextD db 'DUMP VIDEO OPTIONS',0
GUIMovieTextD1 db 'NO VIDEO',0
GUIMovieTextD2 db 'RAW VIDEO',0
GUIMovieTextD3 db 'FFV1',0
GUIMovieTextD4 db 'X264 LOSSLESS',0
GUIMovieTextD5 db 'XVID LOSSLESS',0
GUIMovieTextD6 db 'CUSTOM',0

GUIMovieTextE db 'DUMP AUDIO OPTIONS',0
GUIMovieTextE1 db 'AUDIO',0
GUIMovieTextE2 db 'MERGE WITH VIDEO',0
GUIMovieTextE3 db 'COMPRESS',0

GUIMovieTextX1 db 'WARNING: THIS MOVIE',0  ; Overwrite Window
GUIMovieTextX2 db 'FILE ALREADY EXISTS',0
GUIMovieTextX3 db 'OKAY TO OVERWRITE ?',0
GUIMovieTextX4 db 'YES',0
GUIMovieTextX5 db 'NO',0

GUIMovieTextZ db 'STATUS:              ',0

NEWSYM CMovieExt, db 'v'

SECTION .text

%macro DrawBorderedBox 6;
  ; draw borders
  mov dl,[GUIWincol]
  DrawGUIWinBox %1,%2,%6,%4,%3,dl   ; freezes if you replace %6 with %3-1
  mov dl,[GUIWincol]
  inc dl
  DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
  mov dl,[GUIWincol]
  add dl,4
  DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
  mov dl,[GUIWincol]
  add dl,3
  DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
  DrawGUIWinBox %1,%2,%3,%4,%5,167
%endmacro

%macro DrawBorderedBoxB 7       ; Special function for combo displays
  ; draw borders
  mov dl,[GUIWincol]
  DrawGUIWinBox %1,%2,%6,%4,%3,dl
  mov dl,[GUIWincol]
  inc dl
  DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
  mov dl,[GUIWincol]
  add dl,4
  DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
  mov dl,[GUIWincol]
  add dl,3
  DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
  DrawGUIWinBox %1,%2,%3,%4,%5,167
  mov byte[GUIComboText3],%7
  mov byte[GUItextcolor],223
  GUIOuttextwin2 %1,%2+5,%3+2,GUIComboText3
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 %1,%2+4,%3+1,GUIComboText3
%endmacro

%macro DrawBorderedBoxB2 7       ; Special function for combo displays
  ; draw borders
  mov dl,[GUIWincol]
  DrawGUIWinBox %1,%2,%6,%4,%3,dl
  mov dl,[GUIWincol]
  inc dl
  DrawGUIWinBox %1,%2-1,%3,%2,%5,dl
  mov dl,[GUIWincol]
  add dl,4
  DrawGUIWinBox %1,%2,%5,%4,%5+1,dl
  mov dl,[GUIWincol]
  add dl,3
  DrawGUIWinBox %1,%4,%3,%4+1,%5,dl
  DrawGUIWinBox %1,%2,%3,%4,%5,167
  mov byte[GUIComboText3b+1],%7
  mov byte[GUItextcolor],223
  GUIOuttextwin2 %1,%2+2,%3+2,GUIComboText3b
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  GUIOuttextwin2 %1,%2+1,%3+1,GUIComboText3b
%endmacro

%macro InitTextColor 0
  mov al,217
  cmp byte[GUIWincoladd],0
  je .zero3
  mov al,211
.zero3
  sub al,15
  mov [TextColorSp+2],al
  mov al,[GUIWincol]
  mov [TextColorSp],al
  mov al,163
  cmp byte[GUIWincoladd],0
  je .zero
  mov al,164
  mov byte[GUItextcolor],164
.zero
  mov [TextColorSp+1],al
%endmacro

%macro GUIOuttextwin2b 4        ; Special, yellow text
  mov al,[TextColorSp]
  mov [GUItextcolor],al
  GUIOuttextwin2 %1,%2,%3,%4
  mov al,[TextColorSp+1]
  mov [GUItextcolor],al
  GUIOuttextwin2 %1,%2-1,%3-1,%4
%endmacro

%macro GUIOuttextwin2c 4        ; Boxed, green text
  mov byte[GUItextcolor],223
  push ecx                    ;Needed for the macro below
  GUIOuttextwin2 %1,%2,%3,%4
  pop ecx
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  push esi
  mov esi,%2
  dec esi
  GUIOuttextwin2 %1,esi,%3-1,%4
  pop esi
%endmacro

%macro GUIOuttextwin2d 5        ; Boxed, green text, limited to 5th param
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
  call BlinkCursorCheck
  cmp ebx,%4
  pop ebx
  jne near %%fixreg
  cmp byte[GUIBlinkCursor],' '
  je %%three
  mov byte[GUIBlinkCursor],' '
  jmp %%four
%%three
  mov byte[GUIBlinkCursor],'_'
%%four
  imul eax,6
  add eax,%2
  push ecx
  mov ecx,eax
  GUIOuttextwin2c %1,ecx,%3,GUIBlinkCursor
  pop ecx
%%fixreg
  pop eax
%endmacro

%macro DrawSlideBar 10
  DrawSlideBarWin %1,%2,%3+8,%4,%5,%6,%7-16,%8
  mov al,[TextColorSp+2]
  mov [GUItextcolor],al
  cmp byte[GUICHold],%9
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  GUIDisplayIconWin %1,%2,%3,GUIIconDataUpArrow
  cmp byte[GUICHold],%9
  jne .noholda2
  sub byte[GUIWincoladd],3
.noholda2
  mov al,[TextColorSp+2]
  mov [GUItextcolor],al
  cmp byte[GUICHold],%10
  jne .noholdb
  add byte[GUIWincoladd],3
.noholdb
  GUIDisplayIconWin %1,%2,%3+%7-8,GUIIconDataDownArrow
  cmp byte[GUICHold],%10
  jne .noholdb2
  sub byte[GUIWincoladd],3
.noholdb2
%endmacro

%macro DetermineChar 2
  cmp al,%1
  jne %%control
  mov al,%2
%%control
%endmacro

; Key types: Up, Down, Left, Right, A, B, X, Y, L, R (Press/Relase/P+R)
; Frame delays: 1 frame, 2, 3, 4, 5, 1 sec., 2, 3 ,4 ,5

DisplayGUICombo:
  mov eax,[GUIccombcursloc]
  cmp [GUIccomblcursloc],eax
  je .nolastcurschange
  mov [GUIccomblcursloc],eax
  ; copy contents into temporary variables
  mov esi,[GUIccombcursloc]
  shl esi,6
  add esi,[GUIccombcursloc]
  add esi,[GUIccombcursloc]
  add esi,CombinDataGlob
  cmp byte[GUIComboGameSpec],0
  je .nogamespecb
  sub esi,CombinDataGlob
  add esi,CombinDataLocl
.nogamespecb
  ; copy 20 bytes to GUIComboTextH
  mov ecx,20
  mov edi,GUIComboTextH
  rep movsb
  ; copy 42 bytes to GUIComboData
  mov ecx,42
  mov edi,GUIComboData
  rep movsb
  mov ax,[esi]
  mov [GUIComboKey],ax
  mov al,[esi+2]
  mov [GUIComboPNum],al
  mov al,[esi+3]
  mov [GUIComboLHorz],al
  ; determine length of combo data
  mov eax,GUIComboData
  xor ecx,ecx
.notfin
  cmp byte[eax],0
  je .fin
  inc eax
  inc ecx
  jmp .notfin
.fin
  mov [GUINumCombo],cl
.nolastcurschange

  ; copy into data if description equal
  mov esi,[GUIccombcursloc]
  shl esi,6
  add esi,[GUIccombcursloc]
  add esi,[GUIccombcursloc]
  add esi,CombinDataGlob
  cmp byte[GUIComboGameSpec],0
  je .nogamespecc
  sub esi,CombinDataGlob
  add esi,CombinDataLocl
.nogamespecc
  mov ecx,20
  mov edi,GUIComboTextH
.comploop
  mov al,[esi]
  cmp al,[edi]
  jne near .notequal
  or al,al
  jz .donecomp
  inc esi
  inc edi
  dec ecx
  jnz .comploop
.donecomp
  call ComboClip
  mov edi,[GUIccombcursloc]
  shl edi,6
  add edi,[GUIccombcursloc]
  add edi,[GUIccombcursloc]
  add edi,CombinDataGlob
  cmp byte[GUIComboGameSpec],0
  je .nogamespecd
  sub edi,CombinDataGlob
  add edi,CombinDataLocl
.nogamespecd
  mov ecx,20
  mov esi,GUIComboTextH
  rep movsb
  ; copy 42 bytes to GUIComboData
  mov ecx,42
  mov esi,GUIComboData
  rep movsb
  mov ax,[GUIComboKey]
  mov [edi],ax
  mov al,[GUIComboPNum]
  mov [edi+2],al
  mov al,[GUIComboLHorz]
  mov [edi+3],al
.notequal
  InitTextColor

  cmp byte[GUIWincoladd],0
  je .zero4
  mov byte[GUItextcolor],211
.zero4

  GUIDrawWindowBox 16,GUIComboDisp

  DrawBorderedBox 16,10,20,190,80,19
  DrawBorderedBox 16,37,91,157,99,90
  DrawBorderedBox 16,10,91,32,99,90
  DrawBorderedBox 16,10,110,220,146,109

  ; Draw SlideBar
  ; win#,X,Y start,List Loc,List size,# Lines,Bar Size(Y),UpArrowResource#,DownArrowRes#
  DrawSlideBar 16,192,20,[GUIccombviewloc],[NumCombo],8,61,GUICSStC,13,14

  ; Draw control boxes
  DrawBorderedBoxB 16,75,150,85,157,149,251
  DrawBorderedBoxB 16,89,150,99,157,149,252
  DrawBorderedBoxB 16,103,150,113,157,149,253
  DrawBorderedBoxB 16,117,150,127,157,149,254
  DrawBorderedBoxB 16,131,150,141,157,149,'A'
  DrawBorderedBoxB 16,145,150,155,157,149,'B'
  DrawBorderedBoxB 16,159,150,169,157,149,'X'
  DrawBorderedBoxB 16,173,150,183,157,149,'Y'
  DrawBorderedBoxB 16,187,150,197,157,149,'L'
  DrawBorderedBoxB 16,201,150,211,157,149,'R'
  DrawBorderedBoxB2 16,215,150,227,157,149,'T'
  DrawBorderedBoxB2 16,231,150,243,157,149,'L'

  DrawBorderedBoxB 16,75,160,85,167,159,251
  DrawBorderedBoxB 16,89,160,99,167,159,252
  DrawBorderedBoxB 16,103,160,113,167,159,253
  DrawBorderedBoxB 16,117,160,127,167,159,254
  DrawBorderedBoxB 16,131,160,141,167,159,'A'
  DrawBorderedBoxB 16,145,160,155,167,159,'B'
  DrawBorderedBoxB 16,159,160,169,167,159,'X'
  DrawBorderedBoxB 16,173,160,183,167,159,'Y'
  DrawBorderedBoxB 16,187,160,197,167,159,'L'
  DrawBorderedBoxB 16,201,160,211,167,159,'R'
  DrawBorderedBoxB2 16,215,160,227,167,159,'T'
  DrawBorderedBoxB2 16,231,160,243,167,159,'L'

  DrawBorderedBoxB 16,75,170,85,177,169,251
  DrawBorderedBoxB 16,89,170,99,177,169,252
  DrawBorderedBoxB 16,103,170,113,177,169,253
  DrawBorderedBoxB 16,117,170,127,177,169,254
  DrawBorderedBoxB 16,131,170,141,177,169,'A'
  DrawBorderedBoxB 16,145,170,155,177,169,'B'
  DrawBorderedBoxB 16,159,170,169,177,169,'X'
  DrawBorderedBoxB 16,173,170,183,177,169,'Y'
  DrawBorderedBoxB 16,187,170,197,177,169,'L'
  DrawBorderedBoxB 16,201,170,211,177,169,'R'
  DrawBorderedBoxB2 16,215,170,227,177,169,'T'
  DrawBorderedBoxB2 16,231,170,243,177,169,'L'

  DrawBorderedBoxB 16,10,189,20,196,188,'1'
  DrawBorderedBoxB 16,24,189,34,196,188,'2'
  DrawBorderedBoxB 16,38,189,48,196,188,'3'
  DrawBorderedBoxB 16,52,189,62,196,188,'4'
  DrawBorderedBoxB 16,66,189,76,196,188,'5'
  DrawBorderedBoxB 16,80,189,90,196,188,'9'

  DrawBorderedBoxB 16,107,189,117,196,188,250
  DrawBorderedBoxB 16,121,189,131,196,188,'1'
  DrawBorderedBoxB 16,135,189,145,196,188,'2'
  DrawBorderedBoxB 16,149,189,159,196,188,'3'
  DrawBorderedBoxB 16,163,189,173,196,188,'4'
  DrawBorderedBoxB 16,177,189,187,196,188,'5'

  DrawBorderedBoxB 16,204,189,218,196,188,255

  GUIDisplayText 16,10,13,GUIComboText1        ; Text
  GUIDisplayText 16,138,13,GUIComboText2
  GUIDisplayText 16,38,84,GUIComboTextE
  GUIDisplayText 16,10,84,GUIComboTextF
  GUIDisplayText 16,10,103,GUIComboText4
  GUIDisplayText 16,10,152,GUIComboText5
  GUIDisplayText 16,10,162,GUIComboText6
  GUIDisplayText 16,10,172,GUIComboText7
  GUIDisplayText 16,10,182,GUIComboText8
  GUIDisplayText 16,114,182,GUIComboText9
  GUIDisplayText 16,204,182,GUIComboTextG
  GUIDisplayText 16,145,85,GUIComboTextI

  DrawGUIButton 16,202,20,246,31,GUIComboTextA,60,-1,0    ; Buttons
  DrawGUIButton 16,202,35,246,46,GUIComboTextB,61,-1,0
  DrawGUIButton 16,202,50,246,61,GUIComboTextC,62,-1,0
  DrawGUIButton 16,202,65,246,76,GUIComboTextD,63,-1,0

  ; Calculate Text Cursor Position / Draw Box Text
  xor eax,eax
.nozerocp
  cmp byte[GUIComboTextH+eax],0
  je .zerocp
  inc eax
  jmp .nozerocp
.zerocp
  mov [GUIComboPos],al
  test byte[GUICCFlash],8
  jnz .nound
  mov byte[GUIComboTextH+eax],'_'
  mov byte[GUIComboTextH+eax+1],0
.nound
  GUIOuttextwin2c 16,39,94,GUIComboTextH
  xor eax,eax
  mov al,[GUIComboPos]
  mov byte[GUIComboTextH+eax],0

  ; Display Current Combo Key
  mov eax,[GUIComboKey]
  mov ebx,[ScanCodeListing+eax*3]
  mov [GUIGameDisplayKy],ebx
  mov byte[GUIGameDisplayKy+3],0
  GUIOuttextwin2c 16,14,94,GUIGameDisplayKy

  ; Buttons
  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero20
  mov byte[GUItextcolor],211
.zero20
  DrawGUIButton 16,202,20,246,31,GUIComboTextA,60,-1,0
  DrawGUIButton 16,202,35,246,46,GUIComboTextB,61,-1,0
  DrawGUIButton 16,202,50,246,61,GUIComboTextC,62,-1,0
  DrawGUIButton 16,202,65,246,76,GUIComboTextD,63,-1,0

  ; Radio Switches
  GUIDisplayButtonHole 16,158,82,byte[GUIComboPNum],0
  GUIDisplayButtonHole 16,176,82,byte[GUIComboPNum],1
  GUIDisplayButtonHole 16,194,82,byte[GUIComboPNum],2
  GUIDisplayButtonHole 16,212,82,byte[GUIComboPNum],3
  GUIDisplayButtonHole 16,230,82,byte[GUIComboPNum],4
  ; Checkboxes
  GUIDisplayCheckbox 16,163,88,GUIComboLHorz,GUIComboTextJ
  GUIDisplayCheckbox 16,163,96,GUIComboGameSpec,GUIComboTextK

  ; Draw Combination Keys (Each 15x11 -> 210x36)
  xor eax,eax
  mov al,[GUINumCombo]
  or eax,eax
  jz near .comboend
  xor ebx,ebx
  mov ecx,11
  mov edx,112
.comboiconloop
  push eax
  mov al,[GUIComboData+ebx]
  dec al
  mov esi,GUIIconDataComboPressRelease
  cmp al,12
  jb .doneicon
  mov esi,GUIIconDataComboPress
  sub al,12
  cmp al,12
  jb .doneicon
  mov esi,GUIIconDataComboRelease
  sub al,12
  cmp al,12
  jb .doneicon
  mov esi,GUIIconDataComboFrame
  cmp al,17
  jne .not9
  mov al,20
  jmp .doneicon
.not9
  cmp al,18
  jb .doneicon
  sub al,7
  mov esi,GUIIconDataComboSecond
  cmp al,11
  jne .notback
  mov al,250-37
  jmp .doneicon
.notback
.doneicon
  cmp al,11
  jbe .control
  add al,37
.control
  DetermineChar 0,251
  DetermineChar 1,252
  DetermineChar 2,253
  DetermineChar 3,254
  DetermineChar 4,'A'
  DetermineChar 5,'B'
  DetermineChar 6,'X'
  DetermineChar 7,'Y'
  DetermineChar 8,'L'
  DetermineChar 9,'R'
  DetermineChar 10,'T'
  DetermineChar 11,'E'
  mov [GUIComboText3],al
  pushad
  mov eax,edx
  add ecx,8
  add eax,5

  mov byte[GUItextcolor],223
  push ecx
  push eax
  GUIOuttextwin2 16,ecx,eax,GUIComboText3
  pop eax
  pop ecx
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zerocol
  mov byte[GUItextcolor],222
.zerocol
  dec ecx
  dec eax
  GUIOuttextwin2 16,ecx,eax,GUIComboText3

  popad
  push ecx
  push edx
  push ebx
  GUIDisplayIconWin 16,ecx,edx,esi
  pop ebx
  pop edx
  pop ecx
  pop eax
  add ecx,15
  cmp ecx,11+15*14
  jne .notoverflowx
  sub ecx,15*14
  add edx,11
.notoverflowx
  inc ebx
  dec eax
  jnz near .comboiconloop
.comboend

  ; Display Bordered Box
  cmp dword[NumCombo],0
  je near .nowinbox
  mov ebx,[GUIccombcursloc]
  sub ebx,[GUIccombviewloc]
  mov eax,ebx
  lea ebx,[ebx*8]
  sub ebx,eax
  add ebx,23
  DrawGUIWinBox2 16,10,190,7,224
.nowinbox

  ; Display Scroll Lines
  ; Copy Description to GUIScrolBufA, Others to GUIScrolBufB
  mov edi,[GUIccombviewloc]
  shl edi,6
  add edi,[GUIccombviewloc]
  add edi,[GUIccombviewloc]
  add edi,CombinDataGlob
  cmp byte[GUIComboGameSpec],0
  je .nogamespec
  sub edi,CombinDataGlob
  add edi,CombinDataLocl
.nogamespec

  mov ecx,12
  mov eax,25
  mov ebx,[NumCombo]
  sub ebx,[GUIccombviewloc]
  cmp ebx,0
  jle near .noscrolldisplay
  cmp ebx,8
  jbe .scrolloop
  mov ebx,8
.scrolloop
  push ebx
  push edi
  push eax

  xor eax,eax
  mov ax,[edi+62]
  mov eax,[ScanCodeListing+eax*3]
  mov [GUIScrolBufB],eax
  mov byte[GUIScrolBufB+3],32
  mov dword[GUIScrolBufB+4],20202020h
  mov al,[edi+64]
  add al,49
  mov [GUIScrolBufB+4],al
  mov byte[GUIScrolBufB+7],'N'
  cmp byte[edi+65],0
  je .yesnoff
  mov byte[GUIScrolBufB+7],'Y'
.yesnoff
  mov bl,20
  mov eax,GUIScrolBufA
.scloopb
  mov bh,[edi]
  mov [eax],bh
  inc edi
  inc eax
  dec bl
  jnz .scloopb
  pop eax

  mov byte[GUItextcolor],223
  push ecx
  push eax
  GUIOuttextwin2 16,ecx,eax,GUIScrolBufA
  pop eax
  pop ecx
  push ecx
  push eax
  add ecx,128
  GUIOuttextwin2 16,ecx,eax,GUIScrolBufB
  pop eax
  pop ecx
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zerocolb
  mov byte[GUItextcolor],222
.zerocolb
  push ecx
  push eax
  dec ecx
  dec eax
  push ecx
  push eax
  GUIOuttextwin2 16,ecx,eax,GUIScrolBufA
  pop eax
  pop ecx
  add ecx,128
  GUIOuttextwin2 16,ecx,eax,GUIScrolBufB
  pop eax
  pop ecx
  pop edi
  pop ebx
  add eax,7
  add edi,66
  dec ebx
  jnz near .scrolloop
.noscrolldisplay
  ret

SECTION .bss
GUIccombviewloc resd 1
GUIccombcursloc resd 1
GUIccomblcursloc resd 1
NEWSYM NumCombo, resd 1
NEWSYM NumComboGlob, resd 1
NEWSYM NumComboLocl, resd 1
GUICSStC resd 3
GUIComboKey resd 1
GUIComboPos resb 1
GUIComboPNum resb 1
GUIComboLHorz resb 1
NEWSYM GUINumCombo, resb 1
GUIComboData resb 50
GUIScrolBufA resb 21
GUIScrolBufB resb 10

SECTION .data
GUIComboText1 db 'DESCRIPTION',0
GUIComboText2 db 'KEY P# LH',0
GUIComboText3 db ' ',0
GUIComboText3b db 'S ',0
GUIComboText4 db 'COMBINATION KEYS:',0
GUIComboText5 db 'PRESS+REL',0
GUIComboText6 db 'PRESS ONLY',0
GUIComboText7 db 'REL ONLY',0
GUIComboText8 db 'FRAME DELAY',0
GUIComboText9 db 'SECOND DELAY',0
GUIComboTextA db 'CLEAR',0
GUIComboTextB db 'ADD',0
GUIComboTextC db 'REPLACE',0
GUIComboTextD db 'DELETE',0
GUIComboTextE db 'DESCRIPTION:',0
GUIComboTextF db 'KEY:',0
GUIComboTextG db 'DEL',0
GUIComboTextH db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
GUIComboTextI db 'P#  1  2  3  4  5',0
GUIComboTextJ db 254,' = LAST ',253,'/',254,0
GUIComboTextK db 'GAME SPECFIC',0

GUIIconDataComboPressRelease:
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

GUIIconDataComboPress:
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

GUIIconDataComboRelease:
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

GUIIconDataComboFrame:
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

GUIIconDataComboSecond:
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

EEMode:
  pushad
  call GetTime
  cmp eax,360
  popad
  ja .noEE
  mov eax,1
  ret
.noEE
  mov eax,0
  ret

DisplayGUIAddOns:                   ;Select Special Controllers
  GUIDrawWindowBox 17,GUIAddOnsDisp

  call EEMode
  cmp eax,0
  je near .noEE                    ;You know it!

  push eax
  push ebx
  push ecx
  mov eax,GUIAddOnEETopBar
  mov ebx,GUIAddOnsDisp
.loopA
  mov cl,[eax]
  mov [ebx],cl
  inc eax
  inc ebx
  cmp cl,0
  jne .loopA
  pop ecx
  pop ebx
  pop eax

  GUIDisplayText 17,20,15,GUIAddOnText1Alt
  GUIDisplayText 17,112,15,GUIAddOnText2Alt

  GUIDisplayButtonHoleTu 17,9,23,byte[device1],0,GUIAddOnText3Alt,0
  GUIDisplayButtonHoleTu 17,9,33,byte[device1],1,GUIAddOnText4Alt,0

  GUIDisplayButtonHoleTu 17,100,23,byte[device2],0,GUIAddOnText3Alt,1
  GUIDisplayButtonHoleTu 17,100,33,byte[device2],1,GUIAddOnText4Alt,1
  GUIDisplayButtonHoleTu 17,100,43,byte[device2],2,GUIAddOnText5Alt,0
  GUIDisplayButtonHoleTu 17,100,53,byte[device2],3,GUIAddOnText6Alt,0
  GUIDisplayButtonHoleTu 17,100,63,byte[device2],4,GUIAddOnText7Alt,0
  jmp .endEE
.noEE                                ;Regular

  push eax
  push ebx
  push ecx
  mov eax,GUIAddOnsDispEE
  mov ebx,GUIAddOnsDisp
.loopB
  mov cl,[eax]
  mov [ebx],cl
  inc eax
  inc ebx
  cmp cl,0
  jne .loopB
  pop ecx
  pop ebx
  pop eax

  GUIDisplayTextY 17,20,15,GUIAddOnText1
  GUIDisplayTextY 17,112,15,GUIAddOnText2

  GUIDisplayButtonHoleTu 17,9,23,byte[device1],0,GUIAddOnText3,0
  GUIDisplayButtonHoleTu 17,9,33,byte[device1],1,GUIAddOnText4,0

  GUIDisplayButtonHoleTu 17,100,23,byte[device2],0,GUIAddOnText3,1
  GUIDisplayButtonHoleTu 17,100,33,byte[device2],1,GUIAddOnText4,1
  GUIDisplayButtonHoleTu 17,100,43,byte[device2],2,GUIAddOnText5,0
  GUIDisplayButtonHoleTu 17,100,53,byte[device2],3,GUIAddOnText6,0
  GUIDisplayButtonHoleTu 17,100,63,byte[device2],4,GUIAddOnText7,0
.endEE

  DDrawBox 17,18,75,[KeyExtraEnab1]
  DDrawBox 17,109,75,[KeyExtraEnab2]
  ret

SECTION .data
GUIAddOnText1 db 'PORT 1:',0
GUIAddOnText2 db 'PORT 2:',0
GUIAddOnText3 db 'GAMEPAD',0
GUIAddOnText4 db 'MOUSE',0
GUIAddOnText5 db 'SUPER SCOPE',0
GUIAddOnText6 db '1 JUSTIFIER',0
GUIAddOnText7 db '2 JUSTIFIERS',0

;EE
GUIAddOnText1Alt db 83,76,79,84,32,49,58,0
GUIAddOnText2Alt db 83,76,79,84,32,50,58,0
GUIAddOnText3Alt db 79,70,70,0
GUIAddOnText4Alt db 87,72,73,84,69,32,66,82,69,65,68,0
GUIAddOnText5Alt db 82,89,69,32,66,82,69,65,68,0
GUIAddOnText6Alt db 69,78,71,76,73,83,72,32,77,85,70,70,73,78,0
GUIAddOnText7Alt db 87,65,70,70,76,69,0
GUIAddOnEETopBar db 84,79,65,83,84,69,82,32,79,80,84,73,79,78,83,0

SECTION .text

DisplayGUIChipConfig:
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

BlinkCursorCheck:
  cmp dword[GUIInputBox],0
  jne .checkboxes
  mov ebx,0
  ret
.checkboxes
  cmp dword[GUIInputBox],1
  jne .notBox1
  mov ebx,SRAMDir
  ret
.notBox1
  cmp dword[GUIInputBox],2
  jne .notBox2
  mov ebx,SnapPath
  ret
.notBox2
  cmp dword[GUIInputBox],3
  jne .notBox3
  mov ebx,SPCPath
  ret
.notBox3
  cmp dword[GUIInputBox],4
  jne .notBox4
  mov ebx,BSXPath
  ret
.notBox4
  cmp dword[GUIInputBox],5
  jne .notBox5
  mov ebx,SGPath
  ret
.notBox5
  cmp dword[GUIInputBox],6
  jne .notBox6
  mov ebx,STPath
  ret
.notBox6
  cmp dword[GUIInputBox],7
  jne .notBox7
  mov ebx,GNextPath
  ret
.notBox7
  cmp dword[GUIInputBox],8
  jne .notBox8
  mov ebx,FEOEZPath
  ret
.notBox8
  cmp dword[GUIInputBox],9
  jne .notBox9
  mov ebx,SJNSPath
  ret
.notBox9
  cmp dword[GUIInputBox],10
  jne .notBox10
  mov ebx,MDHPath
  ret
.notBox10
  cmp dword[GUIInputBox],11
  jne .notBox11
  mov ebx,SPL4Path
.notBox11
  ret

DisplayGUIPaths:
  GUIDrawWindowBox 19,GUIPathsDisp

  GUIDisplayText 19,8,18,GUIPathsText1         ; Text
  GUIDisplayText 19,8,38,GUIPathsText2
  GUIDisplayText 19,8,58,GUIPathsText3
  GUIDisplayText 19,60,78,GUIPathsText4
  GUIDisplayText 19,8,88,GUIPathsText5
  GUIDisplayText 19,135,88,GUIPathsText6
  GUIDisplayText 19,8,113,GUIPathsText8
  GUIDisplayText 19,135,113,GUIPathsText7
  GUIDisplayText 19,45,138,GUIPathsText9
  GUIDisplayText 19,8,148,GUIPathsTextA
  GUIDisplayText 19,135,148,GUIPathsTextB
  GUIDisplayText 19,8,173,GUIPathsTextC
  GUIDisplayText 19,135,173,GUIPathsTextD

  GUIDisplayBBox  19,75,15,231,25,167            ;Input boxes
  GUIDisplayBBox  19,75,35,231,45,167
  GUIDisplayBBox  19,75,55,231,65,167
  GUIDisplayBBox  19,8,95,106,105,167
  GUIDisplayBBox  19,8,120,106,130,167
  GUIDisplayBBox  19,135,95,235,105,167
  GUIDisplayBBox  19,135,120,235,130,167
  GUIDisplayBBox  19,8,155,106,165,167
  GUIDisplayBBox  19,8,180,106,190,167
  GUIDisplayBBox  19,135,155,235,165,167
  GUIDisplayBBox  19,135,180,235,190,167

  GUIOuttextwin2d 19,77,19,SRAMDir,25            ; Green Text
  GUIOuttextwin2d 19,77,39,SnapPath,25
  GUIOuttextwin2d 19,77,59,SPCPath,25
  GUIOuttextwin2d 19,10,99,BSXPath,15
  GUIOuttextwin2d 19,10,124,SGPath,15
  GUIOuttextwin2d 19,137,99,STPath,15
  GUIOuttextwin2d 19,137,124,GNextPath,15
  GUIOuttextwin2d 19,10,159,FEOEZPath,15
  GUIOuttextwin2d 19,10,184,MDHPath,15
  GUIOuttextwin2d 19,137,159,SJNSPath,15
  GUIOuttextwin2d 19,137,184,SPL4Path,15
  ret

SECTION .data
GUIPathsText1 db 'SAVES:',0
GUIPathsText2 db 'SNAPSHOTS:',0
GUIPathsText3 db 'SPCS:',0
GUIPathsText4 db '--BIOS/BASE CARTS--',0
GUIPathsText5 db 'BS-X:',0
GUIPathsText6 db 'SUFAMI TURBO:',0
GUIPathsText7 db 'SD GUNDAM G-NEXT:',0
GUIPathsText8 db 'SAME GAME:',0
GUIPathsText9 db '--SPC7110 GRAPHIC PACKS--',0
GUIPathsTextA db 'FEOEZ:',0
GUIPathsTextB db 'FEOEZ - SJNS:',0
GUIPathsTextC db 'MDH:',0
GUIPathsTextD db 'SPL4:',0
NEWSYM GUIBlinkCursor, db 0,0

SECTION .text

DisplayGUISave:                    ;Save & Rewind options/Hotkeys
  GUIDrawWindowBox 20,GUISaveDisp

  GUIDisplayText 20,8,19,GUISaveTextA1           ; Text
  GUIDisplayText 20,8,31,GUISaveTextA2
  GUIDisplayText 20,9,150,GUISaveTextC2
  GUIDisplayText 20,9+57,150,GUISaveTextC3
  GUIDisplayText 20,9+114,150,GUISaveTextC4
  GUIDisplayText 20,9,159,GUISaveTextC5

  GUIDisplayTextY 20,6,114,GUISaveTextC
  mov byte[GUISaveTextC1+2],'0'
  GUIDisplayText 20,9,123,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'1'
  GUIDisplayText 20,9+45,123,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'2'
  GUIDisplayText 20,9+45*2,123,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'3'
  GUIDisplayText 20,9+45*3,123,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'4'
  GUIDisplayText 20,9,132,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'5'
  GUIDisplayText 20,9+45,132,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'6'
  GUIDisplayText 20,9+45*2,132,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'7'
  GUIDisplayText 20,9+45*3,132,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'8'
  GUIDisplayText 20,9,141,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'9'
  GUIDisplayText 20,9+45,141,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'+'
  GUIDisplayText 20,9+45*2,141,GUISaveTextC1
  mov byte[GUISaveTextC1+2],'-'
  GUIDisplayText 20,9+45*3,141,GUISaveTextC1

  GUIDisplayCheckboxu 20,11,40,AutoIncSaveSlot,GUISaveTextB1,5 ;Checkboxes
  GUIDisplayCheckboxu 20,11,50,SRAMSave5Sec,GUISaveTextB2,5
  GUIDisplayCheckboxu 20,11,60,LatestSave,GUISaveTextB3,0
  GUIDisplayCheckboxu 20,11,70,AutoState,GUISaveTextB4,0
  GUIDisplayCheckboxu 20,11,80,SRAMState,GUISaveTextB5,0
  GUIDisplayCheckboxu 20,11,90,PauseLoad,GUISaveTextB6,0
  GUIDisplayCheckboxu 20,11,100,PauseRewind,GUISaveTextB7,12

  GUIDisplayBBox 20,150,17,165,24,167     ; Rewind States Box
  mov ah,[RewindStates]
  mov al,48
  add ah,48
.asciiloop
  cmp ah,58
  jb .hex2asciidone
  inc al
  sub ah,10
  jmp .asciiloop
.hex2asciidone
  mov [GUISaveTextZ3],ax
  GUIDisplayTextG 20,154,19,GUISaveTextZ3

  GUIDisplayBBox 20,150,29,165,36,167     ; Second/Rewind Box
  mov ah,[RewindFrames]
  mov al,48
  add ah,48
.asciiloop2
  cmp ah,58
  jb .hex2asciidone2
  inc al
  sub ah,10
  jmp .asciiloop2
.hex2asciidone2
  mov [GUISaveTextZ3],ax
  GUIDisplayTextG 20,154,31,GUISaveTextZ3

  DDrawBox 20,26,120,[KeyStateSlc0]           ; Boxes for State section
  DDrawBox 20,71,120,[KeyStateSlc1]
  DDrawBox 20,116,120,[KeyStateSlc2]
  DDrawBox 20,161,120,[KeyStateSlc3]
  DDrawBox 20,26,129,[KeyStateSlc4]
  DDrawBox 20,71,129,[KeyStateSlc5]
  DDrawBox 20,116,129,[KeyStateSlc6]
  DDrawBox 20,161,129,[KeyStateSlc7]
  DDrawBox 20,26,138,[KeyStateSlc8]
  DDrawBox 20,71,138,[KeyStateSlc9]
  DDrawBox 20,116,138,[KeyIncStateSlot]
  DDrawBox 20,161,138,[KeyDecStateSlot]
  DDrawBox 20,32,147,[KeySaveState]
  DDrawBox 20,89,147,[KeyLoadState]
  DDrawBox 20,146,147,[KeyStateSelct]
  DDrawBox 20,45,156,[KeyRewind]

  mov byte[GUItextcolor],217               ;Buttons
  cmp byte[GUIWincoladd],0
  je .zero
  mov byte[GUItextcolor],211
.zero
  DrawGUIButton 20,173,17,181,25,GUISaveTextZ1,70,-2,-1  ; + Rewind States
  DrawGUIButton 20,184,17,192,25,GUISaveTextZ2,71,-2,-1  ; - Rewind States
  DrawGUIButton 20,173,29,181,37,GUISaveTextZ1,72,-2,-1  ; + Second/Rewind
  DrawGUIButton 20,184,29,192,37,GUISaveTextZ2,73,-2,-1  ; - Second/Rewind
  ret

SECTION .data
GUISaveTextA1 db '# OF REWIND STATES',0
GUISaveTextA2 db '1/5 SECONDS PER REWIND',0

GUISaveTextB1 db 'AUTO INCREMENT SAVE SLOT',0
GUISaveTextB2 db 'SRAM CHECK+SAVE',0
GUISaveTextB3 db 'START AT LATEST SAVE',0
GUISaveTextB4 db 'AUTO STATE SAVE/LOAD',0
GUISaveTextB5 db 'LOAD SAVESTATE W/SRAM',0
GUISaveTextB6 db 'PAUSE AFTER LOADING STATE',0
GUISaveTextB7 db 'PAUSE AFTER REWIND',0

GUISaveTextC db 'STATE SHORTCUTS:',0
GUISaveTextC1 db 'ST#',0 ; Used for numbers 0-9 and +/-
GUISaveTextC2 db 'SAVE',0
GUISaveTextC3 db 'LOAD',0
GUISaveTextC4 db 'PICK',0
GUISaveTextC5 db 'REWIND',0

GUISaveTextZ1 db '+',0
GUISaveTextZ2 db '-',0
GUISaveTextZ3 db '  ',0

SECTION .text

%macro SpdslidSet 1 ; slider var
  mov al,[%1]
  shl eax,1
%endmacro

%macro SpdslidText 2 ; slider var, text
  mov dword[%2],0
  mov esi,%2+2
  mov al,[%1]   ; currently emuspeed ranges from 0 to 58
  mov bl,10
  cmp al,29     ; this will turn it into '/30' to '30x'
  jb %%slowmo
  mov byte[esi],'x' ; adds 'x' after the ff value
  sub al,28
  jmp %%asciilp
%%slowmo
  inc esi
  mov byte[%2],1    ; slomo indicator
  mov al,30
  sub al,[%1]
%%asciilp           ; turns decimal into ascii
  dec esi
  xor ah,ah
  div bl
  add ah,48
  mov [esi],ah
  cmp al,0
  jne %%asciilp
  cmp byte[%2],1
  jne %%done
  dec esi
  mov byte[esi],'/' ; adds '/' before the slomo value
%%done
%endmacro

DisplayGUISpeed:                    ;Speed Options
  GUIDrawWindowBox 21,GUISpeedDisp

  cmp byte[frameskip],0
  jne near .noautoshad
  GUIDisplayText 21,6,15,GUISpeedTextA1
  GUIDisplayText 21,40,79,GUISpeedTextB3
  GUIDisplayText 21,40,99,GUISpeedTextB5
  GUIDisplayTextY 21,8,164,GUISpeedTextD
  jmp near .endautoshad
.noautoshad
  GUIDisplayText 21,6,15,GUISpeedTextA1b
  GUIDisplayText 21,40,79,GUISpeedTextB3b
  GUIDisplayText 21,40,99,GUISpeedTextB5b
.endautoshad
  GUIDisplayText 21,6,26,GUISpeedTextA2
  GUIDisplayText 21,6,37,GUISpeedTextA3

  GUIDisplayTextY 21,8,49,GUISpeedTextB
  GUIDisplayText 21,40,59,GUISpeedTextB1
  GUIDisplayText 21,40,69,GUISpeedTextB2
  GUIDisplayText 21,40,89,GUISpeedTextB4
  GUIDisplayText 21,40,109,GUISpeedTextB6
  GUIDisplayText 21,40,119,GUISpeedTextB7

  cmp byte[frameskip],0                             ; Shortcut Boxes
  jne near .frate
  DDrawBox 21,10,76,[KeyEmuSpeedUp]
  DDrawBox 21,10,96,[KeyEmuSpeedDown]
  jmp near .endsc
.frate
  DDrawBox 21,10,76,[KeyFRateUp]
  DDrawBox 21,10,96,[KeyFRateDown]
.endsc
  DDrawBox 21,10,56,[KeyFastFrwrd]
  DDrawBox 21,10,66,[KeySlowDown]
  DDrawBox 21,10,86,[KeyResetSpeed]
  DDrawBox 21,10,106,[EMUPauseKey]
  DDrawBox 21,10,116,[INCRFrameKey]

  GUIDisplayCheckboxu 21,11,135,FastFwdToggle,GUISpeedTextC1,0
  GUIDisplayCheckboxun 21,11,145,frameskip,0,GUISpeedTextC2,0

  GUIDisplayBBox 21,96,24,114,31,167      ; FF Ratio Box
  mov ah,[FFRatio]
  add ah,2
  mov al,48
  add ah,48
.asciiloop
  cmp ah,58
  jb .hex2asciidone
  inc al
  sub ah,10
  jmp .asciiloop
.hex2asciidone
  cmp al,48
  jne .yestens
  mov al,32
.yestens
  mov [GUISpeedTextZ3],ax
  GUIDisplayTextG 21,101,26,GUISpeedTextZ3

  GUIDisplayBBox 21,96,35,114,42,167      ; SD Ratio Box
  mov ah,[SDRatio]
  add ah,2
  mov al,48
  add ah,48
.asciiloop2
  cmp ah,58
  jb .hex2asciidone2
  inc al
  sub ah,10
  jmp .asciiloop2
.hex2asciidone2
  cmp al,48
  jne .yestens2
  mov al,32
.yestens2
  mov [GUISpeedTextZ3],ax
  GUIDisplayTextG 21,101,37,GUISpeedTextZ3

  mov byte[GUItextcolor],217               ;Buttons
  cmp byte[GUIWincoladd],0
  je .zero88
  mov byte[GUItextcolor],211
.zero88
  DrawGUIButton 21,118,24,126,32,GUISaveTextZ1,74,-2,-1  ; + Rewind States
  DrawGUIButton 21,129,24,137,32,GUISaveTextZ2,75,-2,-1  ; - Rewind States
  DrawGUIButton 21,118,35,126,43,GUISaveTextZ1,76,-2,-1  ; + Second/Rewind
  DrawGUIButton 21,129,35,137,43,GUISaveTextZ2,77,-2,-1  ; - Second/Rewind

  cmp byte[frameskip],0
  jne near .noslider
  GUIDrawSlider 21,7,116,175,EmuSpeed,GUISpeedTextD1,SpdslidSet,SpdslidText
.noslider
  cmp byte[frameskip],0                    ; Determine if AutoFR is enabled
  je near .auto

  GUIDisplayBBox 21,96,13,114,20,167           ; Non AFR FrameRate +/- Box
  mov al,[frameskip]
  add al,47
  mov [GUISpeedTextX],al
  GUIDisplayTextG 21,107,15,GUISpeedTextX

  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero6
  mov byte[GUItextcolor],211
.zero6
  DrawGUIButton 21,118,13,126,21,GUISpeedTextY,12,-2,-1
  DrawGUIButton 21,129,13,137,21,GUISpeedTextZ,13,-2,-1
  jmp .ratioborders

.auto                                          ; AFR Max Frameskip +/- Box
  GUIDisplayBBox 21,96,13,114,20,167
  mov al,[maxskip]
  add al,48
  mov [GUISpeedTextX],al
  mov byte[GUItextcolor],223
  GUIOuttextwin2 21,107,15,GUISpeedTextX
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zero5
  mov byte[GUItextcolor],222
.zero5
  GUIOuttextwin2 21,106,14,GUISpeedTextX
  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero7
  mov byte[GUItextcolor],211
.zero7
  DrawGUIButton 21,118,13,126,21,GUISpeedTextY,12,-2,-1
  DrawGUIButton 21,129,13,137,21,GUISpeedTextZ,13,-2,-1
.ratioborders
  ret

SECTION .data
GUISpeedTextA1 db 'MAX FRAME SKIP',0
GUISpeedTextA1b db 'FRAME RATE',0
GUISpeedTextA2 db 'FASTFWD RATIO x',0
GUISpeedTextA3 db 'SLOWDWN RATIO /',0

GUISpeedTextB db 'SHORTCUTS:',0
GUISpeedTextB1 db 'FAST FORWARD',0
GUISpeedTextB2 db 'SLOW DOWN',0
GUISpeedTextB3 db '+ EMU SPEED',0
GUISpeedTextB3b db '+ FRAME RATE',0
GUISpeedTextB4 db 'RESET SPEED',0
GUISpeedTextB5 db '- EMU SPEED',0
GUISpeedTextB5b db '- FRAME RATE',0
GUISpeedTextB6 db 'PAUSE GAME',0
GUISpeedTextB7 db 'INCR FRAME',0

GUISpeedTextC1 db 'TOGGLED FFWD/SLWDWN',0
GUISpeedTextC2 db 'AUTO FRAME RATE',0

GUISpeedTextD db 'EMU SPEED:',0
GUISpeedTextD1 db '---',0

GUISpeedTextZ3 db '--',0
GUISpeedTextX db '-',0
GUISpeedTextY db '+',0
GUISpeedTextZ db '-',0

;General data
GUILoadDisp db 'LOAD GAME',0
GUIResetDisp db 'RESET GAME',0
GUIStateSelDisp db 'STATE SELECT',0
GUIInputDisp db 'INPUT DEVICE',0
GUIOptionDisp db 'OPTIONS',0
GUISoundDisp db 'SOUND CONFIG',0
GUIVideoDisp db 'VIDEO CONFIG',0
GUICheatDisp db 'CHEAT',0
GUISearchDisp db 'CHEAT SEARCH',0
GUINetDisp db 'REMOTE',0,0,0,0,0,0,0,0,0
GUIGameDisp db 'MISC KEYS',0
GUIGUIDisp db 'GUI OPTIONS',0
GUIAboutDisp db 'ABOUT',0
GUIMovieDisp db 'MOVIE OPTIONS',0
GUIStatesDisp db 'STATE CONFIRM',0
GUIComboDisp db 'KEY COMBINATION EDITOR',0
GUIAddOnsDisp db 'DEVICES SELECTOR',0
GUIAddOnsDispEE db 'DEVICES SELECTOR',0
GUIChipConfigDisp db 'CONFIGURE CHIPS',0
GUIPathsDisp db 'SETUP PATHS',0
GUISaveDisp db 'SAVE OPTIONS',0
GUISpeedDisp db 'SPEED OPTIONS',0

SECTION .bss
GUIItemBoxColor resd 1
TextColorSp resd 1

SECTION .data
NEWSYM GUIMenuItem
  db 'GAME  ',0
  db 'CONFIG',0
  db 'CHEAT ',0
  db 'NETPLAY',0
  db 'MISC  ',0
  db 25,0
