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


; Body of an SA-1 opcode ported to C (cpu/ops65816_sa1.h). Same seam as the
; 65816's: pushad hands the register file to the C, which reads and writes it
; in place. `endloop` here is the SA-1's, which counts cycles inline.
%macro cop 1
    pushad
    mov eax, esp
    ccall %1, eax
    popad
    endloop
%endmacro

EXTSYM c_SA1COp61m8nd,c_SA1COp61m16nd,c_SA1COp61m8d,c_SA1COp61m16d
EXTSYM c_SA1COp63m8nd,c_SA1COp63m16nd,c_SA1COp63m8d,c_SA1COp63m16d
EXTSYM c_SA1COp65m8nd,c_SA1COp65m16nd,c_SA1COp65m8d,c_SA1COp65m16d
EXTSYM c_SA1COp67m8nd,c_SA1COp67m16nd,c_SA1COp67m8d,c_SA1COp67m16d
EXTSYM c_SA1COp69m8nd,c_SA1COp69m16nd,c_SA1COp69m8d,c_SA1COp69m16d
EXTSYM c_SA1COp6Dm8nd,c_SA1COp6Dm16nd,c_SA1COp6Dm8d,c_SA1COp6Dm16d
EXTSYM c_SA1COp6Fm8nd,c_SA1COp6Fm16nd,c_SA1COp6Fm8d,c_SA1COp6Fm16d
EXTSYM c_SA1COp71m8nd,c_SA1COp71m16nd,c_SA1COp71m8d,c_SA1COp71m16d
EXTSYM c_SA1COp72m8nd,c_SA1COp72m16nd,c_SA1COp72m8d,c_SA1COp72m16d
EXTSYM c_SA1COp73m8nd,c_SA1COp73m16nd,c_SA1COp73m8d,c_SA1COp73m16d
EXTSYM c_SA1COp75m8nd,c_SA1COp75m16nd,c_SA1COp75m8d,c_SA1COp75m16d
EXTSYM c_SA1COp77m8nd,c_SA1COp77m16nd,c_SA1COp77m8d,c_SA1COp77m16d
EXTSYM c_SA1COp79m8nd,c_SA1COp79m16nd,c_SA1COp79m8d,c_SA1COp79m16d
EXTSYM c_SA1COp7Dm8nd,c_SA1COp7Dm16nd,c_SA1COp7Dm8d,c_SA1COp7Dm16d
EXTSYM c_SA1COp7Fm8nd,c_SA1COp7Fm16nd,c_SA1COp7Fm8d,c_SA1COp7Fm16d
EXTSYM c_SA1COp21m8,c_SA1COp21m16,c_SA1COp23m8,c_SA1COp23m16
EXTSYM c_SA1COp25m8,c_SA1COp25m16,c_SA1COp27m8,c_SA1COp27m16
EXTSYM c_SA1COp29m8,c_SA1COp29m16,c_SA1COp2Dm8,c_SA1COp2Dm16
EXTSYM c_SA1COp2Fm8,c_SA1COp2Fm16,c_SA1COp31m8,c_SA1COp31m16
EXTSYM c_SA1COp32m8,c_SA1COp32m16,c_SA1COp33m8,c_SA1COp33m16
EXTSYM c_SA1COp35m8,c_SA1COp35m16,c_SA1COp37m8,c_SA1COp37m16
EXTSYM c_SA1COp39m8,c_SA1COp39m16,c_SA1COp3Dm8,c_SA1COp3Dm16
EXTSYM c_SA1COp3Fm8,c_SA1COp3Fm16,c_SA1COp06m8,c_SA1COp06m16
EXTSYM c_SA1COp0Am8,c_SA1COp0Am16,c_SA1COp0Em8,c_SA1COp0Em16
EXTSYM c_SA1COp16m8,c_SA1COp16m16,c_SA1COp1Em8,c_SA1COp1Em16
EXTSYM c_SA1COp90,c_SA1COpB0,c_SA1COpF0,c_SA1COp30
EXTSYM c_SA1COpD0,c_SA1COp10,c_SA1COp80,c_SA1COp50
EXTSYM c_SA1COp70,c_SA1COp89m8,c_SA1COp89m16,c_SA1COp24m8
EXTSYM c_SA1COp24m16,c_SA1COp2Cm8,c_SA1COp2Cm16,c_SA1COp34m8
EXTSYM c_SA1COp34m16,c_SA1COp3Cm8,c_SA1COp3Cm16,c_SA1COp00
EXTSYM c_SA1COp82,c_SA1COp18,c_SA1COpD8,c_SA1COp58
EXTSYM c_SA1COpB8,c_SA1COpC1m8,c_SA1COpC1m16,c_SA1COpC3m8
EXTSYM c_SA1COpC3m16,c_SA1COpC5m8,c_SA1COpC5m16,c_SA1COpC7m8
EXTSYM c_SA1COpC7m16,c_SA1COpC9m8,c_SA1COpC9m16,c_SA1COpCDm8
EXTSYM c_SA1COpCDm16,c_SA1COpCFm8,c_SA1COpCFm16,c_SA1COpD1m8
EXTSYM c_SA1COpD1m16,c_SA1COpD2m8,c_SA1COpD2m16,c_SA1COpD3m8
EXTSYM c_SA1COpD3m16,c_SA1COpD5m8,c_SA1COpD5m16,c_SA1COpD7m8
EXTSYM c_SA1COpD7m16,c_SA1COpD9m8,c_SA1COpD9m16,c_SA1COpDDm8
EXTSYM c_SA1COpDDm16,c_SA1COpDFm8,c_SA1COpDFm16,c_SA1COp02
EXTSYM c_SA1COpE0x8,c_SA1COpE0x16,c_SA1COpE4x8,c_SA1COpE4x16
EXTSYM c_SA1COpECx8,c_SA1COpECx16,c_SA1COpC0x8,c_SA1COpC0x16
EXTSYM c_SA1COpC4x8,c_SA1COpC4x16,c_SA1COpCCx8,c_SA1COpCCx16
EXTSYM c_SA1COp3Am8,c_SA1COp3Am16,c_SA1COpCEm8,c_SA1COpCEm16
EXTSYM c_SA1COpC6m8,c_SA1COpC6m16,c_SA1COpD6m8,c_SA1COpD6m16
EXTSYM c_SA1COpDEm8,c_SA1COpDEm16,c_SA1COpCAx8,c_SA1COpCAx16
EXTSYM c_SA1COp88x8,c_SA1COp88x16,c_SA1COp41m8,c_SA1COp41m16
EXTSYM c_SA1COp43m8,c_SA1COp43m16,c_SA1COp45m8,c_SA1COp45m16
EXTSYM c_SA1COp47m8,c_SA1COp47m16,c_SA1COp49m8,c_SA1COp49m16
EXTSYM c_SA1COp4Dm8,c_SA1COp4Dm16,c_SA1COp4Fm8,c_SA1COp4Fm16
EXTSYM c_SA1COp51m8,c_SA1COp51m16,c_SA1COp52m8,c_SA1COp52m16
EXTSYM c_SA1COp53m8,c_SA1COp53m16,c_SA1COp55m8,c_SA1COp55m16
EXTSYM c_SA1COp57m8,c_SA1COp57m16,c_SA1COp59m8,c_SA1COp59m16
EXTSYM c_SA1COp5Dm8,c_SA1COp5Dm16,c_SA1COp5Fm8,c_SA1COp5Fm16
EXTSYM c_SA1COp1Am8,c_SA1COp1Am16,c_SA1COpEEm8,c_SA1COpEEm16
EXTSYM c_SA1COpE6m8,c_SA1COpE6m16,c_SA1COpF6m8,c_SA1COpF6m16
EXTSYM c_SA1COpFEm8,c_SA1COpFEm16,c_SA1COpE8x8,c_SA1COpE8x16
EXTSYM c_SA1COpC8x8,c_SA1COpC8x16,c_SA1COpDC,c_SA1COp4C
EXTSYM c_SA1COp6C,c_SA1COp7C,c_SA1COp5C,c_SA1COp22
EXTSYM c_SA1COp20,c_SA1COpFC,c_SA1COpA1m8,c_SA1COpA1m16
EXTSYM c_SA1COpA3m8,c_SA1COpA3m16,c_SA1COpA5m8,c_SA1COpA5m16
EXTSYM c_SA1COpA7m8,c_SA1COpA7m16,c_SA1COpA9m8,c_SA1COpA9m16
EXTSYM c_SA1COpADm8,c_SA1COpADm16,c_SA1COpAFm8,c_SA1COpAFm16
EXTSYM c_SA1COpB1m8,c_SA1COpB1m16,c_SA1COpB2m8,c_SA1COpB2m16
EXTSYM c_SA1COpB3m8,c_SA1COpB3m16,c_SA1COpB5m8,c_SA1COpB5m16
EXTSYM c_SA1COpB7m8,c_SA1COpB7m16,c_SA1COpB9m8,c_SA1COpB9m16
EXTSYM c_SA1COpBDm8,c_SA1COpBDm16,c_SA1COpBFm8,c_SA1COpBFm16
EXTSYM c_SA1COpA2x8,c_SA1COpA2x16,c_SA1COpA6x8,c_SA1COpA6x16
EXTSYM c_SA1COpAEx8,c_SA1COpAEx16,c_SA1COpB6x8,c_SA1COpB6x16
EXTSYM c_SA1COpBEx8,c_SA1COpBEx16,c_SA1COpA0x8,c_SA1COpA0x16
EXTSYM c_SA1COpA4x8,c_SA1COpA4x16,c_SA1COpACx8,c_SA1COpACx16
EXTSYM c_SA1COpB4x8,c_SA1COpB4x16,c_SA1COpBCx8,c_SA1COpBCx16
EXTSYM c_SA1COp46m8,c_SA1COp46m16,c_SA1COp4Am8,c_SA1COp4Am16
EXTSYM c_SA1COp4Em8,c_SA1COp4Em16,c_SA1COp56m8,c_SA1COp56m16
EXTSYM c_SA1COp5Em8,c_SA1COp5Em16,c_SA1COp54,c_SA1COp44
EXTSYM c_SA1COpEA,c_SA1COp01m8,c_SA1COp01m16,c_SA1COp03m8
EXTSYM c_SA1COp03m16,c_SA1COp05m8,c_SA1COp05m16,c_SA1COp07m8
EXTSYM c_SA1COp07m16,c_SA1COp09m8,c_SA1COp09m16,c_SA1COp0Dm8
EXTSYM c_SA1COp0Dm16,c_SA1COp0Fm8,c_SA1COp0Fm16,c_SA1COp11m8
EXTSYM c_SA1COp11m16,c_SA1COp12m8,c_SA1COp12m16,c_SA1COp13m8
EXTSYM c_SA1COp13m16,c_SA1COp15m8,c_SA1COp15m16,c_SA1COp17m8
EXTSYM c_SA1COp17m16,c_SA1COp19m8,c_SA1COp19m16,c_SA1COp1Dm8
EXTSYM c_SA1COp1Dm16,c_SA1COp1Fm8,c_SA1COp1Fm16,c_SA1COpF4
EXTSYM c_SA1COpD4,c_SA1COp62,c_SA1COp48m8,c_SA1COp48m16
EXTSYM c_SA1COp8B,c_SA1COp0B,c_SA1COp4B,c_SA1COp08
EXTSYM c_SA1COpDAx8,c_SA1COpDAx16,c_SA1COp5Ax8,c_SA1COp5Ax16
EXTSYM c_SA1COp68m8,c_SA1COp68m16,c_SA1COpAB,c_SA1COp2B
EXTSYM c_SA1COp28,c_SA1COpFAx8,c_SA1COpFAx16,c_SA1COp7Ax8
EXTSYM c_SA1COp7Ax16,c_SA1COpC2,c_SA1COp26m8,c_SA1COp26m16
EXTSYM c_SA1COp2Am8,c_SA1COp2Am16,c_SA1COp2Em8,c_SA1COp2Em16
EXTSYM c_SA1COp36m8,c_SA1COp36m16,c_SA1COp3Em8,c_SA1COp3Em16
EXTSYM c_SA1COp66m8,c_SA1COp66m16,c_SA1COp6Am8,c_SA1COp6Am16
EXTSYM c_SA1COp6Em8,c_SA1COp6Em16,c_SA1COp76m8,c_SA1COp76m16
EXTSYM c_SA1COp7Em8,c_SA1COp7Em16,c_SA1COp40,c_SA1COp6B
EXTSYM c_SA1COp60,c_SA1COpE1m8nd,c_SA1COpE1m16nd,c_SA1COpE1m8d
EXTSYM c_SA1COpE1m16d,c_SA1COpE3m8nd,c_SA1COpE3m16nd,c_SA1COpE3m8d
EXTSYM c_SA1COpE3m16d,c_SA1COpE5m8nd,c_SA1COpE5m16nd,c_SA1COpE5m8d
EXTSYM c_SA1COpE5m16d,c_SA1COpE7m8nd,c_SA1COpE7m16nd,c_SA1COpE7m8d
EXTSYM c_SA1COpE7m16d,c_SA1COpE9m8nd,c_SA1COpE9m16nd,c_SA1COpE9m8d
EXTSYM c_SA1COpE9m16d,c_SA1COpEDm8nd,c_SA1COpEDm16nd,c_SA1COpEDm8d
EXTSYM c_SA1COpEDm16d,c_SA1COpEFm8nd,c_SA1COpEFm16nd,c_SA1COpEFm8d
EXTSYM c_SA1COpEFm16d,c_SA1COpF1m8nd,c_SA1COpF1m16nd,c_SA1COpF1m8d
EXTSYM c_SA1COpF1m16d,c_SA1COpF2m8nd,c_SA1COpF2m16nd,c_SA1COpF2m8d
EXTSYM c_SA1COpF2m16d,c_SA1COpF3m8nd,c_SA1COpF3m16nd,c_SA1COpF3m8d
EXTSYM c_SA1COpF3m16d,c_SA1COpF5m8nd,c_SA1COpF5m16nd,c_SA1COpF5m8d
EXTSYM c_SA1COpF5m16d,c_SA1COpF7m8nd,c_SA1COpF7m16nd,c_SA1COpF7m8d
EXTSYM c_SA1COpF7m16d,c_SA1COpF9m8nd,c_SA1COpF9m16nd,c_SA1COpF9m8d
EXTSYM c_SA1COpF9m16d,c_SA1COpFDm8nd,c_SA1COpFDm16nd,c_SA1COpFDm8d
EXTSYM c_SA1COpFDm16d,c_SA1COpFFm8nd,c_SA1COpFFm16nd,c_SA1COpFFm8d
EXTSYM c_SA1COpFFm16d,c_SA1COp38,c_SA1COpF8,c_SA1COp78
EXTSYM c_SA1COpE2,c_SA1COp81m8,c_SA1COp81m16,c_SA1COp83m8
EXTSYM c_SA1COp83m16,c_SA1COp85m8,c_SA1COp85m16,c_SA1COp87m8
EXTSYM c_SA1COp87m16,c_SA1COp8Dm8,c_SA1COp8Dm16,c_SA1COp8Fm8
EXTSYM c_SA1COp8Fm16,c_SA1COp91m8,c_SA1COp91m16,c_SA1COp92m8
EXTSYM c_SA1COp92m16,c_SA1COp93m8,c_SA1COp93m16,c_SA1COp95m8
EXTSYM c_SA1COp95m16,c_SA1COp97m8,c_SA1COp97m16,c_SA1COp99m8
EXTSYM c_SA1COp99m16,c_SA1COp9Dm8,c_SA1COp9Dm16,c_SA1COp9Fm8
EXTSYM c_SA1COp9Fm16,c_SA1COp86x8,c_SA1COp86x16,c_SA1COp8Ex8
EXTSYM c_SA1COp8Ex16,c_SA1COp96x8,c_SA1COp96x16,c_SA1COp84x8
EXTSYM c_SA1COp84x16,c_SA1COp8Cx8,c_SA1COp8Cx16,c_SA1COp94x8
EXTSYM c_SA1COp94x16,c_SA1COpDB,c_SA1COp64m8,c_SA1COp64m16
EXTSYM c_SA1COp74m8,c_SA1COp74m16,c_SA1COp9Cm8,c_SA1COp9Cm16
EXTSYM c_SA1COp9Em8,c_SA1COp9Em16,c_SA1COp14m8,c_SA1COp14m16
EXTSYM c_SA1COp1Cm8,c_SA1COp1Cm16,c_SA1COp04m8,c_SA1COp04m16
EXTSYM c_SA1COp0Cm8,c_SA1COp0Cm16,c_SA1COpAAx8,c_SA1COpAAx16
EXTSYM c_SA1COpA8x8,c_SA1COpA8x16,c_SA1COp5B,c_SA1COp1B
EXTSYM c_SA1COp7B,c_SA1COp3B,c_SA1COpBAx8,c_SA1COpBAx16
EXTSYM c_SA1COp8Am8,c_SA1COp8Am16,c_SA1COp9A,c_SA1COp9Bx8
EXTSYM c_SA1COp9Bx16,c_SA1COp98m8,c_SA1COp98m16,c_SA1COpBBx8
EXTSYM c_SA1COpBBx16,c_SA1COpCB,c_SA1COpEB,c_SA1COpFB
EXTSYM c_SA1COp42

%include "cpu/s65816d.inc"
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
