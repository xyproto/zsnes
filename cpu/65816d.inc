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



EXTSYM flagnz,flago,flagc,execloopdeb

;*******************************************************
; Flag Setting Macros
;*******************************************************



; The 65816 cores are C (cpu/ops65816.h and its two instantiations). All
; that is left here is the tail every ported opcode ends with, which the
; `cop` thunk expands - it still has to be assembly because it dispatches
; the next opcode rather than returning.

%macro endloop 0
   jmp execloopdeb
%endmacro
