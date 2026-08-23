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
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif

EXTSYM DLR,DLFN

SECTION .text

; All that is left of this file. calldl16t is the generic register bridge: it
; loads the block a C caller filled in, calls through DLFN, and stores the
; registers back, so a ported caller can reach what assembly remains with the
; register ABI intact. It goes when nothing on the other side is assembly.
NEWSYM calldl16t
    push ebx
    push esi
    push edi
    push ebp
    mov ebx, [DLR+4]
    mov ecx, [DLR+8]
    mov edx, [DLR+12]
    mov esi, [DLR+16]
    mov edi, [DLR+20]
    mov ebp, [DLR+24]
    mov eax, [DLR]
    call dword[DLFN]
    mov [DLR], eax
    mov [DLR+4], ebx
    mov [DLR+8], ecx
    mov [DLR+12], edx
    mov [DLR+16], esi
    mov [DLR+20], edi
    mov [DLR+24], ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
