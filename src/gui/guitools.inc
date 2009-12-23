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

EXTSYM GUIOutputString,GUIoutputiconwin

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
