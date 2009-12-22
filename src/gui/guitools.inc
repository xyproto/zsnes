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

EXTSYM GUIOutputString,GUIoutputcharwin

SECTION .text

NEWSYM GUIOutputStringwin
  xor eax,eax
.more
  mov al,[edi]
  or al,al
  jz .nomore
  mov al,[ASCII2Font+eax]
  cmp edx,-8
  jl .nodraw
  cmp edx,255
  jg .nodraw
  push esi
  add esi,edx
  ccallv GUIoutputcharwin, esi, eax
  pop esi
.nodraw
  add edx,6
  inc edi
  jmp .more
.nomore
  ret

GUIOutputStringwinl:
  xor eax,eax
  mov ecx,[cloadmaxlen]
.more
  mov al,[edi]
%ifndef __MSDOS__
  cmp al,'%'
  jne .noperc
  call ConvertPercValue
.noperc
%endif
  or al,al
  jz .nomore
  mov al,[ASCII2Font+eax]
  cmp edx,-8
  jl .nodraw
  cmp edx,255
  jg .nodraw
  push esi
  add esi,edx
  ccallv GUIoutputcharwin, esi, eax
  pop esi
.nodraw
  add edx,6
  inc edi
  dec ecx
  jz .nomore
  jmp .more
.nomore
  ret

%ifndef __MSDOS__
ConvertPercValue:
  mov al,[edi+2]
  cmp al,'a'
  jb .notlower
  cmp al,'z'
  ja .notlower
  sub al,'a'-'A'
.notlower
  mov ah,[edi+1]
  cmp ah,'a'
  jb .notlowerb
  cmp ah,'z'
  ja .notlowerb
  sub ah,'a'-'A'
.notlowerb
  cmp al,'0'
  jb .noperc
  cmp al,'9'
  jbe .yesperc
  cmp al,'F'
  ja .noperc
  cmp al,'A'
  jb .noperc
.yesperc
  cmp ah,'0'
  jb .noperc
  cmp ah,'9'
  jbe .yesperc2
  cmp ah,'F'
  ja .noperc
  cmp ah,'A'
  jb .noperc
.yesperc2
  sub al,'0'
  cmp al,9
  jbe .nodec
  sub al,'A'-'0'-10
.nodec
  sub ah,'0'
  cmp ah,9
  jbe .nodecb
  sub ah,'A'-'0'-10
.nodecb
  shl ah,4
  or al,ah
  add edi,2
  and eax,0FFh
  ret
.noperc
  movzx eax,byte[edi]
  ret
%endif

OutputUnder:
  cmp edx,-8
  jl .nodraw
  cmp edx,255
  jg .nodraw
  mov dword[esi+edx+5*288-1],0E8E8E8E8h
  mov byte[esi+edx+5*288+3],0E8h
.nodraw
  ret

%macro GUIOuttextwin 1
  mov esi,ebx
  mov eax,ebx
  shl esi,8
  shl eax,5
  add esi,eax
  add esi,16
  add esi,[vidbuffer]
  mov edi,%1
  call GUIOutputStringwin
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
  mov edi,%4
  call GUIOutputStringwin
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
  mov edi,%4
  call GUIOutputStringwinl
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
  call OutputUnder
  sub edx,%5*6
  call GUIOutputStringwin
%endmacro

GUIoutputiconwin:
  cmp eax,-9
  jl .nodraw
  cmp eax,256
  jl .draw
.nodraw
  ret
.draw
  mov esi,ebx
  mov ecx,ebx
  shl esi,8
  shl ecx,5
  add esi,ecx
  add esi,16
  add esi,[vidbuffer]
  add esi,eax
  mov cl,10
.loopa
  mov ch,10
  cmp esi,[vidbuffer]
  jb .below
  mov edx,[vidbuffer]
  add edx,224*288
  cmp esi,edx
  jb .loopb
.below
  add esi,10
  jmp .nowrite2
.loopb
  mov al,[edi]
  or al,al
  jz .transp
  cmp al,189
  ja .col2
  cmp byte[GUIWincoladd],0
  je .col3
  inc al
  jmp .col3
.col2
  sub al,[GUIWincoladd]
  dec al
.col3
  mov [esi],al
.transp
  inc edi
  inc esi
  dec ch
  jnz .loopb
.nowrite2
  add esi,278
  dec cl
  jnz .loopa
  ret

%macro GUIDisplayIcon 1
  mov edi,%1
  call GUIoutputiconwin
%endmacro

%macro GUIDisplayIconWin 4
  mov eax,[GUIwinposx+%1*4]
  mov ebx,[GUIwinposy+%1*4]
  add eax,%2
  add ebx,%3
  mov edi,%4
  call GUIoutputiconwin
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
