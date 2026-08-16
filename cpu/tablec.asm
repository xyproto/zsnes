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
EXTSYM tableAc,tableBc,tableCc,tableDc,tableEc
EXTSYM tableFc,tableGc,tableHc,tableadc


; Body of a debug-core opcode ported to C (cpu/ops65816_dbg.h). Same seam as
; the other two cores; `endloop` here is the debug one, which steps the SPC700
; and counts cycles inline.
%macro cop 1
    pushad
    mov eax, esp
    ccall %1, eax
    popad
    endloop
%endmacro

EXTSYM c_dbgCOp61m8nd,c_dbgCOp61m16nd,c_dbgCOp61m8d,c_dbgCOp61m16d
EXTSYM c_dbgCOp63m8nd,c_dbgCOp63m16nd,c_dbgCOp63m8d,c_dbgCOp63m16d
EXTSYM c_dbgCOp65m8nd,c_dbgCOp65m16nd,c_dbgCOp65m8d,c_dbgCOp65m16d
EXTSYM c_dbgCOp67m8nd,c_dbgCOp67m16nd,c_dbgCOp67m8d,c_dbgCOp67m16d
EXTSYM c_dbgCOp69m8nd,c_dbgCOp69m16nd,c_dbgCOp69m8d,c_dbgCOp69m16d
EXTSYM c_dbgCOp6Dm8nd,c_dbgCOp6Dm16nd,c_dbgCOp6Dm8d,c_dbgCOp6Dm16d
EXTSYM c_dbgCOp6Fm8nd,c_dbgCOp6Fm16nd,c_dbgCOp6Fm8d,c_dbgCOp6Fm16d
EXTSYM c_dbgCOp71m8nd,c_dbgCOp71m16nd,c_dbgCOp71m8d,c_dbgCOp71m16d
EXTSYM c_dbgCOp72m8nd,c_dbgCOp72m16nd,c_dbgCOp72m8d,c_dbgCOp72m16d
EXTSYM c_dbgCOp73m8nd,c_dbgCOp73m16nd,c_dbgCOp73m8d,c_dbgCOp73m16d
EXTSYM c_dbgCOp75m8nd,c_dbgCOp75m16nd,c_dbgCOp75m8d,c_dbgCOp75m16d
EXTSYM c_dbgCOp77m8nd,c_dbgCOp77m16nd,c_dbgCOp77m8d,c_dbgCOp77m16d
EXTSYM c_dbgCOp79m8nd,c_dbgCOp79m16nd,c_dbgCOp79m8d,c_dbgCOp79m16d
EXTSYM c_dbgCOp7Dm8nd,c_dbgCOp7Dm16nd,c_dbgCOp7Dm8d,c_dbgCOp7Dm16d
EXTSYM c_dbgCOp7Fm8nd,c_dbgCOp7Fm16nd,c_dbgCOp7Fm8d,c_dbgCOp7Fm16d
EXTSYM c_dbgCOp21m8,c_dbgCOp21m16,c_dbgCOp23m8,c_dbgCOp23m16
EXTSYM c_dbgCOp25m8,c_dbgCOp25m16,c_dbgCOp27m8,c_dbgCOp27m16
EXTSYM c_dbgCOp29m8,c_dbgCOp29m16,c_dbgCOp2Dm8,c_dbgCOp2Dm16
EXTSYM c_dbgCOp2Fm8,c_dbgCOp2Fm16,c_dbgCOp31m8,c_dbgCOp31m16
EXTSYM c_dbgCOp32m8,c_dbgCOp32m16,c_dbgCOp33m8,c_dbgCOp33m16
EXTSYM c_dbgCOp35m8,c_dbgCOp35m16,c_dbgCOp37m8,c_dbgCOp37m16
EXTSYM c_dbgCOp39m8,c_dbgCOp39m16,c_dbgCOp3Dm8,c_dbgCOp3Dm16
EXTSYM c_dbgCOp3Fm8,c_dbgCOp3Fm16,c_dbgCOp06m8,c_dbgCOp06m16
EXTSYM c_dbgCOp0Am8,c_dbgCOp0Am16,c_dbgCOp0Em8,c_dbgCOp0Em16
EXTSYM c_dbgCOp16m8,c_dbgCOp16m16,c_dbgCOp1Em8,c_dbgCOp1Em16
EXTSYM c_dbgCOp90,c_dbgCOpB0,c_dbgCOpF0,c_dbgCOp30
EXTSYM c_dbgCOpD0,c_dbgCOp10,c_dbgCOp80,c_dbgCOp50
EXTSYM c_dbgCOp70,c_dbgCOp89m8,c_dbgCOp89m16,c_dbgCOp24m8
EXTSYM c_dbgCOp24m16,c_dbgCOp2Cm8,c_dbgCOp2Cm16,c_dbgCOp34m8
EXTSYM c_dbgCOp34m16,c_dbgCOp3Cm8,c_dbgCOp3Cm16,c_dbgCOp00
EXTSYM c_dbgCOp82,c_dbgCOp18,c_dbgCOpD8,c_dbgCOp58
EXTSYM c_dbgCOpB8,c_dbgCOpC1m8,c_dbgCOpC1m16,c_dbgCOpC3m8
EXTSYM c_dbgCOpC3m16,c_dbgCOpC5m8,c_dbgCOpC5m16,c_dbgCOpC7m8
EXTSYM c_dbgCOpC7m16,c_dbgCOpC9m8,c_dbgCOpC9m16,c_dbgCOpCDm8
EXTSYM c_dbgCOpCDm16,c_dbgCOpCFm8,c_dbgCOpCFm16,c_dbgCOpD1m8
EXTSYM c_dbgCOpD1m16,c_dbgCOpD2m8,c_dbgCOpD2m16,c_dbgCOpD3m8
EXTSYM c_dbgCOpD3m16,c_dbgCOpD5m8,c_dbgCOpD5m16,c_dbgCOpD7m8
EXTSYM c_dbgCOpD7m16,c_dbgCOpD9m8,c_dbgCOpD9m16,c_dbgCOpDDm8
EXTSYM c_dbgCOpDDm16,c_dbgCOpDFm8,c_dbgCOpDFm16,c_dbgCOp02
EXTSYM c_dbgCOpE0x8,c_dbgCOpE0x16,c_dbgCOpE4x8,c_dbgCOpE4x16
EXTSYM c_dbgCOpECx8,c_dbgCOpECx16,c_dbgCOpC0x8,c_dbgCOpC0x16
EXTSYM c_dbgCOpC4x8,c_dbgCOpC4x16,c_dbgCOpCCx8,c_dbgCOpCCx16
EXTSYM c_dbgCOp3Am8,c_dbgCOp3Am16,c_dbgCOpCEm8,c_dbgCOpCEm16
EXTSYM c_dbgCOpC6m8,c_dbgCOpC6m16,c_dbgCOpD6m8,c_dbgCOpD6m16
EXTSYM c_dbgCOpDEm8,c_dbgCOpDEm16,c_dbgCOpCAx8,c_dbgCOpCAx16
EXTSYM c_dbgCOp88x8,c_dbgCOp88x16,c_dbgCOp41m8,c_dbgCOp41m16
EXTSYM c_dbgCOp43m8,c_dbgCOp43m16,c_dbgCOp45m8,c_dbgCOp45m16
EXTSYM c_dbgCOp47m8,c_dbgCOp47m16,c_dbgCOp49m8,c_dbgCOp49m16
EXTSYM c_dbgCOp4Dm8,c_dbgCOp4Dm16,c_dbgCOp4Fm8,c_dbgCOp4Fm16
EXTSYM c_dbgCOp51m8,c_dbgCOp51m16,c_dbgCOp52m8,c_dbgCOp52m16
EXTSYM c_dbgCOp53m8,c_dbgCOp53m16,c_dbgCOp55m8,c_dbgCOp55m16
EXTSYM c_dbgCOp57m8,c_dbgCOp57m16,c_dbgCOp59m8,c_dbgCOp59m16
EXTSYM c_dbgCOp5Dm8,c_dbgCOp5Dm16,c_dbgCOp5Fm8,c_dbgCOp5Fm16
EXTSYM c_dbgCOp1Am8,c_dbgCOp1Am16,c_dbgCOpEEm8,c_dbgCOpEEm16
EXTSYM c_dbgCOpE6m8,c_dbgCOpE6m16,c_dbgCOpF6m8,c_dbgCOpF6m16
EXTSYM c_dbgCOpFEm8,c_dbgCOpFEm16,c_dbgCOpE8x8,c_dbgCOpE8x16
EXTSYM c_dbgCOpC8x8,c_dbgCOpC8x16,c_dbgCOpDC,c_dbgCOp4C
EXTSYM c_dbgCOp6C,c_dbgCOp7C,c_dbgCOp5C,c_dbgCOp22
EXTSYM c_dbgCOp20,c_dbgCOpFC,c_dbgCOpA1m8,c_dbgCOpA1m16
EXTSYM c_dbgCOpA3m8,c_dbgCOpA3m16,c_dbgCOpA5m8,c_dbgCOpA5m16
EXTSYM c_dbgCOpA7m8,c_dbgCOpA7m16,c_dbgCOpA9m8,c_dbgCOpA9m16
EXTSYM c_dbgCOpADm8,c_dbgCOpADm16,c_dbgCOpAFm8,c_dbgCOpAFm16
EXTSYM c_dbgCOpB1m8,c_dbgCOpB1m16,c_dbgCOpB2m8,c_dbgCOpB2m16
EXTSYM c_dbgCOpB3m8,c_dbgCOpB3m16,c_dbgCOpB5m8,c_dbgCOpB5m16
EXTSYM c_dbgCOpB7m8,c_dbgCOpB7m16,c_dbgCOpB9m8,c_dbgCOpB9m16
EXTSYM c_dbgCOpBDm8,c_dbgCOpBDm16,c_dbgCOpBFm8,c_dbgCOpBFm16
EXTSYM c_dbgCOpA2x8,c_dbgCOpA2x16,c_dbgCOpA6x8,c_dbgCOpA6x16
EXTSYM c_dbgCOpAEx8,c_dbgCOpAEx16,c_dbgCOpB6x8,c_dbgCOpB6x16
EXTSYM c_dbgCOpBEx8,c_dbgCOpBEx16,c_dbgCOpA0x8,c_dbgCOpA0x16
EXTSYM c_dbgCOpA4x8,c_dbgCOpA4x16,c_dbgCOpACx8,c_dbgCOpACx16
EXTSYM c_dbgCOpB4x8,c_dbgCOpB4x16,c_dbgCOpBCx8,c_dbgCOpBCx16
EXTSYM c_dbgCOp46m8,c_dbgCOp46m16,c_dbgCOp4Am8,c_dbgCOp4Am16
EXTSYM c_dbgCOp4Em8,c_dbgCOp4Em16,c_dbgCOp56m8,c_dbgCOp56m16
EXTSYM c_dbgCOp5Em8,c_dbgCOp5Em16,c_dbgCOp54,c_dbgCOp44
EXTSYM c_dbgCOpEA,c_dbgCOp01m8,c_dbgCOp01m16,c_dbgCOp03m8
EXTSYM c_dbgCOp03m16,c_dbgCOp05m8,c_dbgCOp05m16,c_dbgCOp07m8
EXTSYM c_dbgCOp07m16,c_dbgCOp09m8,c_dbgCOp09m16,c_dbgCOp0Dm8
EXTSYM c_dbgCOp0Dm16,c_dbgCOp0Fm8,c_dbgCOp0Fm16,c_dbgCOp11m8
EXTSYM c_dbgCOp11m16,c_dbgCOp12m8,c_dbgCOp12m16,c_dbgCOp13m8
EXTSYM c_dbgCOp13m16,c_dbgCOp15m8,c_dbgCOp15m16,c_dbgCOp17m8
EXTSYM c_dbgCOp17m16,c_dbgCOp19m8,c_dbgCOp19m16,c_dbgCOp1Dm8
EXTSYM c_dbgCOp1Dm16,c_dbgCOp1Fm8,c_dbgCOp1Fm16,c_dbgCOpF4
EXTSYM c_dbgCOpD4,c_dbgCOp62,c_dbgCOp48m8,c_dbgCOp48m16
EXTSYM c_dbgCOp8B,c_dbgCOp0B,c_dbgCOp4B,c_dbgCOp08
EXTSYM c_dbgCOpDAx8,c_dbgCOpDAx16,c_dbgCOp5Ax8,c_dbgCOp5Ax16
EXTSYM c_dbgCOp68m8,c_dbgCOp68m16,c_dbgCOpAB,c_dbgCOp2B
EXTSYM c_dbgCOp28,c_dbgCOpFAx8,c_dbgCOpFAx16,c_dbgCOp7Ax8
EXTSYM c_dbgCOp7Ax16,c_dbgCOpC2,c_dbgCOp26m8,c_dbgCOp26m16
EXTSYM c_dbgCOp2Am8,c_dbgCOp2Am16,c_dbgCOp2Em8,c_dbgCOp2Em16
EXTSYM c_dbgCOp36m8,c_dbgCOp36m16,c_dbgCOp3Em8,c_dbgCOp3Em16
EXTSYM c_dbgCOp66m8,c_dbgCOp66m16,c_dbgCOp6Am8,c_dbgCOp6Am16
EXTSYM c_dbgCOp6Em8,c_dbgCOp6Em16,c_dbgCOp76m8,c_dbgCOp76m16
EXTSYM c_dbgCOp7Em8,c_dbgCOp7Em16,c_dbgCOp40,c_dbgCOp6B
EXTSYM c_dbgCOp60,c_dbgCOpE1m8nd,c_dbgCOpE1m16nd,c_dbgCOpE1m8d
EXTSYM c_dbgCOpE1m16d,c_dbgCOpE3m8nd,c_dbgCOpE3m16nd,c_dbgCOpE3m8d
EXTSYM c_dbgCOpE3m16d,c_dbgCOpE5m8nd,c_dbgCOpE5m16nd,c_dbgCOpE5m8d
EXTSYM c_dbgCOpE5m16d,c_dbgCOpE7m8nd,c_dbgCOpE7m16nd,c_dbgCOpE7m8d
EXTSYM c_dbgCOpE7m16d,c_dbgCOpE9m8nd,c_dbgCOpE9m16nd,c_dbgCOpE9m8d
EXTSYM c_dbgCOpE9m16d,c_dbgCOpEDm8nd,c_dbgCOpEDm16nd,c_dbgCOpEDm8d
EXTSYM c_dbgCOpEDm16d,c_dbgCOpEFm8nd,c_dbgCOpEFm16nd,c_dbgCOpEFm8d
EXTSYM c_dbgCOpEFm16d,c_dbgCOpF1m8nd,c_dbgCOpF1m16nd,c_dbgCOpF1m8d
EXTSYM c_dbgCOpF1m16d,c_dbgCOpF2m8nd,c_dbgCOpF2m16nd,c_dbgCOpF2m8d
EXTSYM c_dbgCOpF2m16d,c_dbgCOpF3m8nd,c_dbgCOpF3m16nd,c_dbgCOpF3m8d
EXTSYM c_dbgCOpF3m16d,c_dbgCOpF5m8nd,c_dbgCOpF5m16nd,c_dbgCOpF5m8d
EXTSYM c_dbgCOpF5m16d,c_dbgCOpF7m8nd,c_dbgCOpF7m16nd,c_dbgCOpF7m8d
EXTSYM c_dbgCOpF7m16d,c_dbgCOpF9m8nd,c_dbgCOpF9m16nd,c_dbgCOpF9m8d
EXTSYM c_dbgCOpF9m16d,c_dbgCOpFDm8nd,c_dbgCOpFDm16nd,c_dbgCOpFDm8d
EXTSYM c_dbgCOpFDm16d,c_dbgCOpFFm8nd,c_dbgCOpFFm16nd,c_dbgCOpFFm8d
EXTSYM c_dbgCOpFFm16d,c_dbgCOp38,c_dbgCOpF8,c_dbgCOp78
EXTSYM c_dbgCOpE2,c_dbgCOp81m8,c_dbgCOp81m16,c_dbgCOp83m8
EXTSYM c_dbgCOp83m16,c_dbgCOp85m8,c_dbgCOp85m16,c_dbgCOp87m8
EXTSYM c_dbgCOp87m16,c_dbgCOp8Dm8,c_dbgCOp8Dm16,c_dbgCOp8Fm8
EXTSYM c_dbgCOp8Fm16,c_dbgCOp91m8,c_dbgCOp91m16,c_dbgCOp92m8
EXTSYM c_dbgCOp92m16,c_dbgCOp93m8,c_dbgCOp93m16,c_dbgCOp95m8
EXTSYM c_dbgCOp95m16,c_dbgCOp97m8,c_dbgCOp97m16,c_dbgCOp99m8
EXTSYM c_dbgCOp99m16,c_dbgCOp9Dm8,c_dbgCOp9Dm16,c_dbgCOp9Fm8
EXTSYM c_dbgCOp9Fm16,c_dbgCOp86x8,c_dbgCOp86x16,c_dbgCOp8Ex8
EXTSYM c_dbgCOp8Ex16,c_dbgCOp96x8,c_dbgCOp96x16,c_dbgCOp84x8
EXTSYM c_dbgCOp84x16,c_dbgCOp8Cx8,c_dbgCOp8Cx16,c_dbgCOp94x8
EXTSYM c_dbgCOp94x16,c_dbgCOpDB,c_dbgCOp64m8,c_dbgCOp64m16
EXTSYM c_dbgCOp74m8,c_dbgCOp74m16,c_dbgCOp9Cm8,c_dbgCOp9Cm16
EXTSYM c_dbgCOp9Em8,c_dbgCOp9Em16,c_dbgCOp14m8,c_dbgCOp14m16
EXTSYM c_dbgCOp1Cm8,c_dbgCOp1Cm16,c_dbgCOp04m8,c_dbgCOp04m16
EXTSYM c_dbgCOp0Cm8,c_dbgCOp0Cm16,c_dbgCOpAAx8,c_dbgCOpAAx16
EXTSYM c_dbgCOpA8x8,c_dbgCOpA8x16,c_dbgCOp5B,c_dbgCOp1B
EXTSYM c_dbgCOp7B,c_dbgCOp3B,c_dbgCOpBAx8,c_dbgCOpBAx16
EXTSYM c_dbgCOp8Am8,c_dbgCOp8Am16,c_dbgCOp9A,c_dbgCOp9Bx8
EXTSYM c_dbgCOp9Bx16,c_dbgCOp98m8,c_dbgCOp98m16,c_dbgCOpBBx8
EXTSYM c_dbgCOpBBx16,c_dbgCOpCB,c_dbgCOpEB,c_dbgCOpFB
EXTSYM c_dbgCOp42

%include "cpu/65816dc.inc"
%include "cpu/e65816c.inc"

; global variables

SECTION .text

eopINVALID
    ret
