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

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1+
	push eax
	ccall %1
	pop eax
%endmacro
EXTSYM obc1_address,obc1_byte,SetOBC1,GetOBC1
EXTSYM regaccessbankr16,regaccessbankr8,regaccessbankw16,regaccessbankw8
EXTSYM memaccessbankr16,memaccessbankr8,memaccessbankw16,memaccessbankw8

SECTION .text

%macro RouteAccess 1
    test ecx,8000h
    jnz memaccessbank%1
    cmp ecx,6000h
    jb regaccessbank%1
%endmacro

NEWSYM OBC1Read8b
    RouteAccess r8
    mov [obc1_address],cx
    ccallv GetOBC1
    mov al,[obc1_byte]
    ret

NEWSYM OBC1Write8b
    RouteAccess w8
    mov [obc1_address],cx
    mov [obc1_byte],al
    ccallv SetOBC1
    ret

NEWSYM OBC1Read16b
    RouteAccess r16
    mov [obc1_address],cx
    ccallv GetOBC1
    mov al,[obc1_byte]
    inc word[obc1_address]
    ccallv GetOBC1
    mov ah,[obc1_byte]
    ret

NEWSYM OBC1Write16b
    RouteAccess w16
    mov [obc1_address],cx
    mov [obc1_byte],al
    ccallv SetOBC1
    mov [obc1_byte],ah
    inc word[obc1_address]
    ccallv SetOBC1
    ret
