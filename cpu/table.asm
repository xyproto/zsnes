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
EXTSYM ngpalcon2b,ngpalcon4b
EXTSYM mosjmptab16b,mosdraw1016b,mosdraw1116b,mosdraw1216b,mosdraw1316b
EXTSYM mosdraw1416b,mosdraw1516b,mosdraw1616b,mosdraw216b,mosdraw316b
EXTSYM mosdraw416b,mosdraw516b,mosdraw616b,mosdraw716b,mosdraw816b
EXTSYM mosdraw916b
EXTSYM mosjmptab16bt,mosdraw1016bt,mosdraw1116bt,mosdraw1216bt,mosdraw1316bt
EXTSYM mosdraw1416bt,mosdraw1516bt,mosdraw1616bt,mosdraw216bt,mosdraw316bt
EXTSYM mosdraw416bt,mosdraw516bt,mosdraw616bt,mosdraw716bt,mosdraw816bt
EXTSYM mosdraw916bt
EXTSYM mosjmptab16btms,mosdraw1016btms,mosdraw1116btms,mosdraw1216btms,mosdraw1316btms
EXTSYM mosdraw1416btms,mosdraw1516btms,mosdraw1616btms,mosdraw216btms,mosdraw316btms
EXTSYM mosdraw416btms,mosdraw516btms,mosdraw616btms,mosdraw716btms,mosdraw816btms
EXTSYM mosdraw916btms
EXTSYM mosjmptab16bntms,mosdraw1016bntms,mosdraw1116bntms,mosdraw1216bntms,mosdraw1316bntms
EXTSYM mosdraw1416bntms,mosdraw1516bntms,mosdraw1616bntms,mosdraw216bntms,mosdraw316bntms
EXTSYM mosdraw416bntms,mosdraw516bntms,mosdraw616bntms,mosdraw716bntms,mosdraw816bntms
EXTSYM mosdraw916bntms
EXTSYM tableA,tableB,tableC,tableD,tableE,tableF,tableG,tableH
EXTSYM DPageR8,DPageW8,DPageR16,DPageW16
EXTSYM SDD1Enable
EXTSYM JoyAOrig,JoyANow,JoyBOrig,JoyBNow,JoyCOrig,JoyCNow,JoyDOrig,JoyDNow

%include "cpu/65816d.inc"
%include "cpu/address.inc"
%include "cpu/addrni.inc"
%include "cpu/e65816.inc"
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

section .text

NEWSYM eopINVALID
    ret

section .data

;*******************************************************
; Cpu Cycles                    Sets the CPU cycle table
;*******************************************************
NEWSYM cpucycle
         db 8, 6, 8, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5
         db 2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 2, 2, 6, 4, 7, 5
         db 6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 4, 4, 6, 5
         db 2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 2, 2, 4, 4, 7, 5
         db 7, 6, 2, 4, 7, 3, 5, 6, 3, 2, 2, 3, 3, 4, 6, 5
         db 2, 5, 5, 7, 7, 4, 6, 6, 2, 4, 3, 2, 4, 4, 7, 5
         db 6, 6, 6, 4, 3, 3, 5, 6, 4, 2, 2, 6, 5, 4, 6, 5
         db 2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5
         db 2, 6, 3, 4, 3, 3, 3, 6, 2, 2, 2, 3, 4, 4, 4, 5
         db 2, 6, 5, 7, 4, 4, 4, 6, 2, 5, 2, 2, 4, 5, 5, 5
         db 2, 6, 2, 4, 3, 3, 3, 6, 2, 2, 2, 4, 4, 4, 4, 5
         db 2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5
         db 2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 4, 5
         db 2, 5, 5, 7, 6, 4, 6, 6, 2, 4, 3, 3, 6, 4, 7, 5
         db 2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5
         db 2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5
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
