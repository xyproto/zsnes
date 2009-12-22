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

; Tools for the GUI (string functions are also used elsewhere)

EXTSYM GUIOutputString,GUIOutputStringwinl,OutputUnder
EXTSYM GUIoutputiconwin

SECTION .text

%macro GUIOuttextwin 1
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
  ccallv GUIOutputStringwin, edx, esi, %1
%endmacro

%macro GUIOuttextwin2 4
  mov edx,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add edx,%2
  add ebx,%3
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
  ccallv GUIOutputStringwin, edx, esi, %4
%endmacro

%macro GUIOuttextwin2l 4
  mov edx,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add edx,%2
  add ebx,%3
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
  ccallv GUIOutputStringwinl, edx, esi, %4
%endmacro

%macro GUIOuttextwin2u 5
  mov edx,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add edx,%2
  add ebx,%3
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
  mov edi,%4
  add edx,%5*6
  ccallv OutputUnder, edx, esi
  sub edx,%5*6
  ccallv GUIOutputStringwin, edx, esi, edi
%endmacro

%macro GUIDisplayIcon 1
  ccallv GUIoutputiconwin, eax, ebx, %1
%endmacro

%macro GUIDisplayIconWin 4
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2
  add ebx,%3
  ccallv GUIoutputiconwin, eax, ebx, %4
%endmacro

GUIDrawSlideBar:
  cmp eax,-10
  jl .ret
  cmp eax,256
  jg .ret
  jmp .noret
.ret
  ret
.noret
  mov edi,[vidbuffer]
  mov [vbuflimtop],edi
  mov edi,[vidbuffer]
  add edi,288*224
  sub edi,16
  mov [vbuflimbot],edi
  mov edi,eax
  mov eax,ebx
  shl ebx,8
  shl eax,5
  add edi,ebx
  add edi,eax
  add edi,[vidbuffer]
  add edi,16
  xor eax,eax
  dec ecx
.loop
  cmp edx,0
  je near .toparea
  cmp ecx,0
  je near .bottomarea
  cmp ah,1
  je .filleddisp
  cmp edi,[vbuflimtop]
  jb near .nodraw
  cmp edi,[vbuflimbot]
  ja near .nodraw
  mov al,197
  sub al,byte[GUIWincoladd]
  mov [edi],al
  sub al,2
  mov [edi+1],al
  dec al
  mov [edi+2],al
  dec al
  mov [edi+3],al
  mov [edi+4],al
  inc al
  mov [edi+5],al
  inc al
  mov [edi+6],al
  add al,2
  mov [edi+7],al
  jmp .fin
.filleddisp
  cmp edi,[vbuflimtop]
  jb near .nodraw
  cmp edi,[vbuflimbot]
  ja near .nodraw
  mov al,202
  sub al,byte[GUIWincoladd]
  mov [edi],al
  sub al,2
  mov [edi+1],al
  mov [edi+2],al
  mov [edi+3],al
  mov [edi+4],al
  mov [edi+5],al
  mov [edi+6],al
  sub al,2
  mov [edi+7],al
  jmp .fin
.toparea
  cmp edi,[vbuflimtop]
  jb near .nodraw
  cmp edi,[vbuflimbot]
  ja near .nodraw
  mov al,202
  sub al,byte[GUIWincoladd]
  mov [edi],al
  add al,2
  mov [edi+1],al
  mov [edi+2],al
  mov [edi+3],al
  mov [edi+4],al
  mov [edi+5],al
  mov [edi+6],al
  mov [edi+7],al
  mov ah,1
  jmp .fin
.bottomarea
  cmp edi,[vbuflimtop]
  jb near .nodraw
  cmp edi,[vbuflimbot]
  ja near .nodraw
  mov al,6
  mov al,196
  sub al,byte[GUIWincoladd]
  mov [edi],al
  mov [edi+1],al
  mov [edi+2],al
  mov [edi+3],al
  mov [edi+4],al
  mov [edi+5],al
  mov [edi+6],al
  add al,2
  mov [edi+7],al
  mov ah,0
.fin
.nodraw
  add edi,288
  dec ecx
  dec edx
  dec esi
  jnz near .loop
  ret

%macro DrawSlideBarWin 8
  ; win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
  mov ebx,%5
  mov ecx,%4
  ; if (Screen Size > List Size) List Size = Screen Size
  cmp ebx,%6
  jae %%noscrbig
  mov ebx,%6
%%noscrbig
  ; If (ListLoc + ScreenSize > ListSize) ListLoc = ListSize - ScreenSize
  mov eax,ecx
  add eax,%6
  cmp eax,ebx
  jbe %%nosetlloc
  mov ecx,ebx
  sub ecx,%6
%%nosetlloc
  ; Scrollbarsize = (Screen Size/List size)*Bar Size
  push ebx
  mov eax,%6
  mov ebx,%7
  mul ebx
  pop ebx
  div ebx
  ; If (Scrollbarsize < 5) Scrollbarsize = 5
  cmp eax,5
  jae %%validscroll
  mov eax,5
%%validscroll
  ; StartY = (Bar Size-Scrollbarsize)*(List Loc/(List size-ScreenSize))
  push eax
  sub eax,%7
  neg eax
  mul ecx
  sub ebx,%6
  cmp ebx,0
  je %%zero
  div ebx
  jmp %%notzero
%%zero
  xor eax,eax
%%notzero
  mov edx,eax
  pop eax
  mov ecx,eax
  add ecx,edx
  ; ecx = endy, edx = starty
  mov esi,%7
  sub esi,ecx
  add esi,edx
  dec esi
  mov [%8],esi
  mov [%8+4],edx
  mov [%8+8],ecx
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2
  add ebx,%3
  mov esi,%7
  call GUIDrawSlideBar
%endmacro
