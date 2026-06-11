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
EXTSYM SA1tableA,SA1tableB,SA1tableC,SA1tableD,SA1tableE
EXTSYM SA1tableF,SA1tableG,SA1tableH,SA1tablead
EXTSYM cpucycle,SA1UpdateDPage,intrset

%include "cpu/s65816d.inc"
%include "cpu/saddress.inc"
%include "cpu/saddrni.inc"
%include "cpu/se65816.inc"

; global variables
;tableA  times 256             ; Table addresses (M:0,X:0,D:0)
;tableB  times 256             ; Table addresses (M:1,X:0,D:0)
;tableC  times 256             ; Table addresses (M:0,X:1,D:0)
;tableD  times 256             ; Table addresses (M:1,X:1,D:0)
;tableE  times 256             ; Table addresses (M:0,X:0,D:1)
;tableF  times 256             ; Table addresses (M:1,X:0,D:1)
;tableG  times 256             ; Table addresses (M:0,X:1,D:1)
;tableH  times 256             ; Table addresses (M:1,X:1,D:1)
;tablead times 256             ; Table address location according to P
;memtabler8 times 256          ; Memory Bank Locations for reading 8-bit
;memtablew8 times 256          ; Memory Bank Locations for writing 8-bit
;memtabler16 times 256          ; Memory Bank Locations for reading 16-bit
;memtablew16 times 256          ; Memory Bank Locations for reading 16-bit

; global variables

SECTION .text

eopINVALID
    ret
