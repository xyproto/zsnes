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



EXTSYM guipresstest

SECTION .text

%macro GUIClickArea 5
  cmp eax,%1
  jl %5
  cmp edx,%2
  jl %5
  cmp eax,%3
  jg %5
  cmp edx,%4
  jg %5
%endmacro

%ifdef __MSDOS__
%macro GUIClickCButton3 3
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  xor %3,1
  DOSClearScreenMouse
  ret
%%noclick
%endmacro

%macro DOSClearScreenMouse 0
  call DOSClearScreen
  push eax
  push ecx
  mov eax,[vidbufferofsb]
  mov ecx,288*128
%%loop
  mov dword[eax],0
  add eax,4
  dec ecx
  jnz %%loop
  pop ecx
  pop eax
  ret
%endmacro
%endif

%macro GUIClickCButton5MMXC 4
  GUIClickArea %1+1,%2+3,%1+6,%2+8,%%noclick
  cmp byte[MMXSupport],0
  je %%clear
  cmp %3,%4
  jne %%set
  xor %3,%4
  jmp %%clear
%%set
  mov %3,%4
%%clear
  ret
%%noclick
%endmacro

SECTION .bss
NEWSYM GUIlastdispval, resd 1
NEWSYM GUIdispmode,    resd 1
NEWSYM GUIlastypos,    resd 1
NEWSYM GUIlastcpos,    resd 1
NEWSYM GUIlastvpos,    resd 1
SECTION .text

%macro GUITextBoxInputB 5
  GUIClickArea %1,%2,%3,%4,%%noclick
  mov byte[GUINetTextm2],%5
%%noclick
%endmacro

%macro GUIPButtonHole2 6
  GUIClickArea %1+1,%2+1,%1+7,%2+7,%%noclick
  mov %3,%4
  mov %5,%6
%%noclick
%endmacro

%macro DGOptnsProcBoxb 3
  GUIClickArea %1,%2,%1+19,%2+6,%%skip
  push edi
  ccall guipresstest
  mov ebx, eax
  pop edi
  cmp ebx,1
  je %%cancelkey
  cmp ebx,3Bh
  je %%cancelkey
  mov %3,ebx
  jmp %%donekey
%%cancelkey
  cmp ebx,1
  jne %%noesc
  mov byte[pressed+1],2
%%noesc
  xor ebx,ebx
  mov %3,ebx
%%donekey
  ret
%%skip
%endmacro

; Mouse implementation

SECTION .bss
NEWSYM GUINumValue, resd 1
NEWSYM GUIBlankVar, resd 1
SECTION .data
NEWSYM NTSCPresetVar, dd 4
