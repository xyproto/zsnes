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
  ccallv GUIOuttextwin2, %1, %2 + 5 + %8, %3 + 4 + %9, %6
  add byte[GUItextcolor],15
  ccallv GUIOuttextwin2, %1, %2 + 4 + %8, %3 + 3 + %9, %6
  jmp %%yes
%%no2
  sub byte[GUItextcolor],18
  ccallv GUIOuttextwin2, %1, %2 + 6 + %8, %3 + 5 + %9, %6
  add byte[GUItextcolor],15
  ccallv GUIOuttextwin2, %1, %2 + 5 + %8, %3 + 4 + %9, %6
  add byte[GUItextcolor],3
%%yes
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

%macro GUIDisplayTextG 4                  ; Green Text&Shadow
  mov byte[GUItextcolor],223
  ccallv GUIOuttextwin2, %1, %2, %3, %4
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
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

%macro GUIDisplayCheckboxun 7             ; Set Var. Checkbox (Text Underline)
  mov dword[GUITemp],GUIIconDataCheckBoxUC
  cmp byte[%4],%5
  jne %%nocheckbox
  mov dword[GUITemp],GUIIconDataCheckBoxC
%%nocheckbox
  ccallv GUIDisplayIconWin, %1, %2, %3, [GUITemp]
  GUIDisplayTextu %1,%2+15,%3+5,%6,%7
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
  ccallv GUIDisplayIconWin, %1, [GUITemp], %4 - 4, GUIIconDataSlideBar
  %8 %5,%6    ; proc2 == alters text correctly and puts pointer in esi
  mov [GUITemp],esi  ; Display Value (Green)
  GUIDisplayTextG %1,%2+%3+6,%4-1,[GUITemp]
%endmacro

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
  ccallv GUIOuttextwin2, %1, %2 + 5, %3 + 2, GUIComboText3
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  ccallv GUIOuttextwin2, %1, %2 + 4, %3 + 1, GUIComboText3
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
  ccallv GUIOuttextwin2, %1, %2 + 2, %3 + 2, GUIComboText3b
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  ccallv GUIOuttextwin2, %1, %2 + 1, %3 + 1, GUIComboText3b
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

%macro DrawSlideBar 10
  ccallv DrawSlideBarWin, %1, %2, %3 + 8, %4, %5, %6, %7 - 16, %8
  mov al,[TextColorSp+2]
  mov [GUItextcolor],al
  cmp byte[GUICHold],%9
  jne .noholda
  add byte[GUIWincoladd],3
.noholda
  ccallv GUIDisplayIconWin, %1, %2, %3, GUIIconDataUpArrow
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
  ccallv GUIDisplayIconWin, %1, %2, %3 + %7 - 8, GUIIconDataDownArrow
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
  ccallv GUIOuttextwin2, 9, %1 + 3, %2 + 2, GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  ccallv GUIOuttextwin2, 9, %1 + 2, %2 + 1, GUIGameDisplayKy
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
  ccallv GUIOuttextwin2, %1, %2 + 4, %3 + 3, GUIGameDisplayKy
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je %%zero
  mov byte[GUItextcolor],222
%%zero
  ccallv GUIOuttextwin2, %1, %2 + 3, %3 + 2, GUIGameDisplayKy
  DGOptnsBorderBox %1,%2,%3
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

SECTION .text

; Key types: Up, Down, Left, Right, A, B, X, Y, L, R (Press/Relase/P+R)
; Frame delays: 1 frame, 2, 3, 4, 5, 1 sec., 2, 3 ,4 ,5

NEWSYM DisplayGUICombo
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
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIComboText3
  mov byte[GUItextcolor],221
  cmp byte[GUIWincoladd],0
  je .zerocol
  mov byte[GUItextcolor],222
.zerocol
  dec ecx
  dec eax
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIComboText3

  popad
  ccallv GUIDisplayIconWin, 16, ecx, edx, esi
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
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIScrolBufA
  push ecx
  add ecx,128
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIScrolBufB
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
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIScrolBufA
  add ecx,128
  ccallv GUIOuttextwin2, 16, ecx, eax, GUIScrolBufB
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
NEWSYM GUIComboKey, resd 1
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
NEWSYM GUIComboTextH, db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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
  ccall GetTime
  cmp eax,360
  ja .noEE
  mov eax,1
  ret
.noEE
  mov eax,0
  ret

NEWSYM DisplayGUIAddOns                   ;Select Special Controllers
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

  GUIDisplayText 17,12,78,GUIAddOnText8Alt
  GUIDisplayText 17,103,78,GUIAddOnText9Alt

  GUIDisplayCheckboxu 17,9,83,mouse1lh,GUIAddOnTextDAlt,0
  GUIDisplayCheckboxu 17,100,83,mouse2lh,GUIAddOnTextDAlt,1

  GUIDisplayTextY 17,12,100,GUIAddOnTextAAlt
  GUIDisplayText 17,12,110,GUIAddOnTextBAlt
  GUIDisplayText 17,103,110,GUIAddOnTextCAlt
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

  GUIDisplayText 17,12,78,GUIAddOnText8
  GUIDisplayText 17,103,78,GUIAddOnText9

  GUIDisplayCheckboxu 17,9,83,mouse1lh,GUIAddOnTextD,0
  GUIDisplayCheckboxu 17,100,83,mouse2lh,GUIAddOnTextD,1

  GUIDisplayTextY 17,12,100,GUIAddOnTextA
  GUIDisplayText 17,12,110,GUIAddOnTextB
  GUIDisplayText 17,103,110,GUIAddOnTextC
.endEE
  DDrawBox 17,73,75,[KeyExtraEnab1]
  DDrawBox 17,164,75,[KeyExtraEnab2]

  DDrawBox 17,73,107,[SSAutoFire]
  DDrawBox 17,164,107,[SSPause]
  ret

SECTION .data
GUIAddOnText1 db 'PORT 1:',0
GUIAddOnText2 db 'PORT 2:',0
GUIAddOnText3 db 'GAMEPAD',0
GUIAddOnText4 db 'MOUSE',0
GUIAddOnText5 db 'SUPER SCOPE',0
GUIAddOnText6 db '1 JUSTIFIER',0
GUIAddOnText7 db '2 JUSTIFIERS',0
GUIAddOnText8 db 'CYCLE P1:',0
GUIAddOnText9 db 'CYCLE P2:',0
GUIAddOnTextA db 'SUPER SCOPE KEYS:',0
GUIAddOnTextB db 'AUTO-FIRE',0
GUIAddOnTextC db 'SS PAUSE',0
GUIAddOnTextD db 'LEFT HANDED',0

;EE
GUIAddOnText1Alt db 83,76,79,84,32,49,58,0
GUIAddOnText2Alt db 83,76,79,84,32,50,58,0
GUIAddOnText3Alt db 79,70,70,0
GUIAddOnText4Alt db 87,72,73,84,69,32,66,82,69,65,68,0
GUIAddOnText5Alt db 82,89,69,32,66,82,69,65,68,0
GUIAddOnText6Alt db 87,72,69,65,84,32,66,82,69,65,68,0
GUIAddOnText7Alt db 69,78,71,76,73,83,72,32,77,85,70,70,73,78,0
GUIAddOnText8Alt db 80,79,80,32,84,65,82,84,0
GUIAddOnText9Alt db 87,65,70,70,76,69,0
GUIAddOnTextAAlt db 67,79,70,70,69,69,63,0
GUIAddOnTextBAlt db 67,82,69,65,77,0
GUIAddOnTextCAlt db 83,85,71,65,82,0
GUIAddOnTextDAlt db 81,85,73,67,75,32,84,79,65,83,84,0
GUIAddOnEETopBar db 84,79,65,83,84,69,82,32,79,80,84,73,79,78,83,0

SECTION .text

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

GUISaveTextZ1 db '+',0
GUISaveTextZ2 db '-',0

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

NEWSYM DisplayGUISpeed                    ;Speed Options
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
  DrawGUIButton 21,118,13,126,21,GUISpeedTextY,78,-2,-1
  DrawGUIButton 21,129,13,137,21,GUISpeedTextZ,79,-2,-1
  jmp .ratioborders

.auto                                          ; AFR Max Frameskip +/- Box
  GUIDisplayBBox 21,96,13,114,20,167
  mov al,[maxskip]
  add al,48
  mov [GUISpeedTextX],al
  GUIDisplayTextG 21,107,15,GUISpeedTextX

  mov byte[GUItextcolor],217
  cmp byte[GUIWincoladd],0
  je .zero5
  mov byte[GUItextcolor],211
.zero5
  DrawGUIButton 21,118,13,126,21,GUISpeedTextY,78,-2,-1
  DrawGUIButton 21,129,13,137,21,GUISpeedTextZ,79,-2,-1
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
GUINetDisp db 'REMOTE',0,0,0,0,0,0,0,0,0
GUIComboDisp db 'KEY COMBINATION EDITOR',0
GUIAddOnsDisp db 'DEVICES SELECTOR',0
GUIAddOnsDispEE db 'DEVICES SELECTOR',0
GUIChipConfigDisp db 'CONFIGURE CHIPS',0
GUIPathsDisp db 'SETUP PATHS',0
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
