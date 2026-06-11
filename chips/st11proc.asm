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
EXTSYM seta11_address,seta11_byte,setaramdata,ST011_DR
EXTSYM ST011_MapR_60,ST011_MapW_60,ST011_MapW_68;ST011_MapR_68

SECTION .text


NEWSYM Seta11Read8_68
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov al,[ebx+ecx]
    mov [ST011_DR],al
    xor ebx,ebx
    ret

NEWSYM Seta11Write8_68
    test ecx,8000h
    jnz .nosetenablew8 ; ignore ROM writes
    mov [seta11_address],cx
    mov [seta11_byte],al
    ccallv ST011_MapW_68
.nosetenablew8
    ret

NEWSYM Seta11Read16_68
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov ax,[ebx+ecx]
    mov [ST011_DR],ah
    xor ebx,ebx
    ret

NEWSYM Seta11Write16_68
    test ecx,8000h
    jnz .nosetenablew16 ; ignore ROM writes
    mov [seta11_address],cx
    mov [seta11_byte],al
    ccallv ST011_MapW_68
    mov [seta11_byte],ah        ; high byte for second write (was missing — bug fix)
    inc word[seta11_address]
    ccallv ST011_MapW_68
.nosetenablew16
    ret


NEWSYM Seta11Read8_60
    xor al,al
    cmp ecx,4000h
    jae .nosetenabler8
    and ecx,3
    mov [seta11_address],cx
    ccallv ST011_MapR_60
    mov al,[seta11_byte]
 .nosetenabler8
    ret

NEWSYM Seta11Write8_60
    cmp ecx,4000h
    jae .nosetenablew8
    and ecx,3
    mov [seta11_address],cx
    mov [seta11_byte],al
    ccallv ST011_MapW_60
.nosetenablew8
    ret

NEWSYM Seta11Read16_60
    xor ax,ax
    cmp ecx,4000h
    jae .nosetenabler16
    and ecx,3
    mov [seta11_address],cx
    ccallv ST011_MapR_60
    mov al,[seta11_byte]
    inc word[seta11_address]
    and word[seta11_address],3
    ccallv ST011_MapR_60
    mov ah,[seta11_byte]
.nosetenabler16
    ret

NEWSYM Seta11Write16_60
    cmp ecx,4000h
    jae .nosetenablew16
    and ecx,3
    mov [seta11_address],cx
    mov [seta11_byte],al
    ccallv ST011_MapW_60
    mov [seta11_byte],ah
    inc word[seta11_address]
    and word[seta11_address],3
    ccallv ST011_MapW_60
.nosetenablew16
    ret
