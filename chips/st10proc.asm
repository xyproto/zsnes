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
EXTSYM setaramdata,ST010DoCommand

SECTION .data
NEWSYM SetaCmdEnable,    dd 0     ; Seta ST010/ST011 command enable register. Maybe also status.
SECTION .text

;; TODO - should return ROM for > 8000h
NEWSYM setaaccessbankr8
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM setaaccessbankw8
    test ecx,8000h
    jnz .nosetenablew8  ; ignore ROM writes
    and ecx,0fffh
    mov ebx,[setaramdata]
    mov [ebx+ecx],al
    cmp byte[ebx+021h], 80h
    jnz .nosetenablew8
    ccallv ST010DoCommand
.nosetenablew8
    xor ebx,ebx
    ret

;; We ignore the case where it wraps into ROM reads - should never happen
NEWSYM setaaccessbankr16
    mov ebx,[setaramdata]
    and ecx,0fffh
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM setaaccessbankw16
    test ecx,8000h
    jnz .nosetenablew16  ; ignore ROM writes
    cmp ecx,7fffh
    jne .noromw16
    mov ebx,[setaramdata]
    mov [ebx+0fffh],al  ; only write ram part in, not rom part
    jmp short .nosetenablew16
.noromw16
    and ecx,0fffh
    mov ebx,[setaramdata]
    cmp ecx,0fffh
    jne .nowrapw16
    mov [ebx+ecx],al
    xchg ah,al
    mov [ebx],al
    jmp short .nosetenablew16
.nowrapw16
    mov [ebx+ecx],ax
    cmp byte[ebx+021h], 80h
    jnz .nosetenablew16
    ccallv ST010DoCommand
.nosetenablew16
    xor ebx,ebx
    ret


NEWSYM setaaccessbankr8a
    xor al,al
    cmp ecx,4000h
    jae .nosetenabler8a
    and ecx, 3
    mov al,[SetaCmdEnable+ecx]
.nosetenabler8a
    xor ebx,ebx
    ret

NEWSYM setaaccessbankw8a
    cmp ecx,4000h
    jae .nosetenablew8a
    and ecx, 03h
    mov [SetaCmdEnable+ecx],al
.nosetenablew8a
    xor ebx,ebx
    ret

NEWSYM setaaccessbankr16a
    xor ax,ax
    cmp ecx,4000h
    jae .nosetenabler16a
    and ecx,3
    mov al,[SetaCmdEnable+ecx]
    xchg ah,al
    inc ecx
    and ecx,3
    mov al,[SetaCmdEnable+ecx]
.nosetenabler16a
    xor ebx,ebx
    ret

NEWSYM setaaccessbankw16a
    cmp ecx,4000h
    jae .nosetenablew16a
    and ecx,3
    mov [SetaCmdEnable+ecx],ah  ; high byte first (big-endian, matches r16a)
    inc ecx
    and ecx,3
    mov [SetaCmdEnable+ecx],al  ; low byte second
.nosetenablew16a
    xor ebx,ebx
    ret

