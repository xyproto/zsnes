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

%macro ALIGN32 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro

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

; All that is left in this file are the I/O register handlers it includes; the
; 65816 core itself is C (cpu/c_ops65816.c and cpu/c_table.c). These have to be
; declared here, above the %includes: on PE/COFF EXTSYM also emits a %define,
; which only applies below the line it appears on.
EXTSYM xa,curexecstate,nmistatus
EXTSYM sndrot,sndrot2,INTEnab
EXTSYM VIRQLoc
EXTSYM bg1ptr,bg2ptr,bg3ptr
EXTSYM bg4ptr,bg1ptrb,bg2ptrb,bg3ptrb
EXTSYM bg4ptrb,bg1ptrc,bg2ptrc,bg3ptrc
EXTSYM bg4ptrc,bg1ptrd,bg2ptrd,bg3ptrd
EXTSYM bg4ptrd
EXTSYM bg1objptr,bg2objptr,bg3objptr
EXTSYM bg4objptr
EXTSYM vramaddr
EXTSYM resolutn
EXTSYM latchx
EXTSYM latchy
EXTSYM mode7A,mode7B
EXTSYM compmult
EXTSYM extlatch,HIRQLoc
EXTSYM ioportval,iohvlatch
EXTSYM hdmastartsc,hdmarestart,hdmadelay,nohdmaframe
EXTSYM h_dot_counter
EXTSYM SPC700read,SPC700write
EXTSYM hblank
EXTSYM bgscrolPrev
EXTSYM multchange,m7byte
; 65816 opcodes ported to cpu/ops65816.h.


%include "cpu/regs.mac"
%include "cpu/regsw.mac"
%include "cpu/regs.inc"     ; start problem here
%include "cpu/regsw.inc"

section .data

; global variables
;tableA  times 256 dd 0             ; Table addresses (M:0,X:0,D:0)
;tableB  times 256 dd 0             ; Table addresses (M:1,X:0,D:0)
;tableC  times 256 dd 0             ; Table addresses (M:0,X:1,D:0)
;tableD  times 256 dd 0             ; Table addresses (M:1,X:1,D:0)
;tableE  times 256 dd 0             ; Table addresses (M:0,X:0,D:1)
;tableF  times 256 dd 0             ; Table addresses (M:1,X:0,D:1)
;tableG  times 256 dd 0             ; Table addresses (M:0,X:1,D:1)
;tableH  times 256 dd 0             ; Table addresses (M:1,X:1,D:1)
;tablead times 256 dd 0             ; Table address location according to P
;memtabler8 times 256 dd 0          ; Memory Bank Locations for reading 8-bit
;memtablew8 times 256 dd 0          ; Memory Bank Locations for writing 8-bit
;memtabler16 times 256 dd 0         ; Memory Bank Locations for reading 16-bit
;memtablew16 times 256 dd 0         ; Memory Bank Locations for reading 16-bit

; eopINVALID and the cpucycle table are ported to C (cpu/c_table.c).

; 28 | 26 | 28 | 24 | 25 | 23 | 25 | 26 | 13 | 22 | 12 | 14 | 36 | 34 | 36 | 45
; 22 | 25 | 25 | 27 | 25 | 24 | 26 | 26 | 12 | 34 | 12 | 12 | 36 | 34 | 37 | 45
; 36 | 26 | 48 | 24 | 23 | 23 | 25 | 26 | 14 | 22 | 12 | 15 | 34 | 34 | 36 | 45
; 22 | 25 | 25 | 27 | 24 | 24 | 26 | 26 | 12 | 34 | 12 | 12 | 34 | 34 | 37 | 45
; 17 | 26 | 22 | 24 | 37 | 23 | 25 | 26 | 13 | 22 | 12 | 13 | 33 | 34 | 36 | 45
; 22 | 25 | 25 | 27 | 37 | 24 | 26 | 26 | 12 | 34 | 13 | 12 | 44 | 34 | 37 | 45
; 16 | 26 | 36 | 24 | 23 | 23 | 25 | 26 | 14 | 22 | 12 | 16 | 35 | 34 | 36 | 45
; 22 | 25 | 25 | 27 | 24 | 24 | 26 | 26 | 12 | 34 | 14 | 12 | 36 | 34 | 37 | 45
; 22 | 26 | 33 | 24 | 23 | 23 | 23 | 26 | 12 | 22 | 12 | 13 | 34 | 34 | 34 | 45
; 22 | 26 | 25 | 27 | 24 | 24 | 24 | 26 | 12 | 35 | 12 | 12 | 34 | 35 | 35 | 45
; 22 | 26 | 22 | 24 | 23 | 23 | 23 | 26 | 12 | 22 | 12 | 14 | 34 | 34 | 34 | 45
; 22 | 25 | 25 | 27 | 24 | 24 | 24 | 26 | 12 | 34 | 12 | 12 | 34 | 34 | 34 | 45
; 22 | 26 | 23 | 24 | 23 | 23 | 25 | 26 | 12 | 22 | 12 | 13 | 34 | 34 | 34 | 45
; 22 | 25 | 25 | 27 | 26 | 24 | 26 | 26 | 12 | 34 | 13 | 13 | 36 | 34 | 37 | 45
; 22 | 26 | 23 | 24 | 23 | 23 | 25 | 26 | 12 | 22 | 12 | 13 | 34 | 34 | 36 | 45
; 22 | 25 | 25 | 27 | 35 | 24 | 26 | 26 | 12 | 34 | 14 | 12 | 36 | 34 | 37 | 45
