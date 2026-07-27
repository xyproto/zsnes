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
EXTSYM FxTable,FxTableb,FxTablec,SfxB,SfxCPB,SfxCROM,SfxCarry,SfxOverflow
EXTSYM SfxR0,SfxR14,SfxR15,SfxRomBuffer,SfxSignZero,withr15sk

; Seam to the handlers ported to C (chips/c_fxemu2b.c, chips/fx_ops.h).
EXTSYM FxSeamPC,FxSeamSrc,FxSeamDst,FxSeamCX
EXTSYM c_FxOpb05,c_FxOpb06,c_FxOpb07,c_FxOpb08,c_FxOpb09,c_FxOpb0A
EXTSYM c_FxOpb0B,c_FxOpb0C,c_FxOpb0D,c_FxOpb0E,c_FxOpb0F
EXTSYM c_FxOpc05,c_FxOpc06,c_FxOpc07,c_FxOpc08,c_FxOpc09,c_FxOpc0A
EXTSYM c_FxOpc0B,c_FxOpc0C,c_FxOpc0D,c_FxOpc0E,c_FxOpc0F
EXTSYM c_FxOpb10,c_FxOpb11,c_FxOpb12,c_FxOpb13,c_FxOpb14,c_FxOpb15
EXTSYM c_FxOpb16,c_FxOpb17,c_FxOpb18,c_FxOpb19,c_FxOpb1A,c_FxOpb1B
EXTSYM c_FxOpb1C,c_FxOpb1D,c_FxOpb1E,c_FxOpb1F,c_FxOpb3D,c_FxOpb3E
EXTSYM c_FxOpb3F,c_FxOpbB0,c_FxOpbB1,c_FxOpbB2,c_FxOpbB3,c_FxOpbB4
EXTSYM c_FxOpbB5,c_FxOpbB6,c_FxOpbB7,c_FxOpbB8,c_FxOpbB9,c_FxOpbBA
EXTSYM c_FxOpbBB,c_FxOpbBC,c_FxOpbBD,c_FxOpbBE,c_FxOpbBF,c_FxOpc10
EXTSYM c_FxOpc11,c_FxOpc12,c_FxOpc13,c_FxOpc14,c_FxOpc15,c_FxOpc16
EXTSYM c_FxOpc17,c_FxOpc18,c_FxOpc19,c_FxOpc1A,c_FxOpc1B,c_FxOpc1C
EXTSYM c_FxOpc1D,c_FxOpc1E,c_FxOpc1F,c_FxOpc3D,c_FxOpc3E,c_FxOpc3F
EXTSYM c_FxOpcB0,c_FxOpcB1,c_FxOpcB2,c_FxOpcB3,c_FxOpcB4,c_FxOpcB5
EXTSYM c_FxOpcB6,c_FxOpcB7,c_FxOpcB8,c_FxOpcB9,c_FxOpcBA,c_FxOpcBB
EXTSYM c_FxOpcBC,c_FxOpcBD,c_FxOpcBE,c_FxOpcBF

%include "chips/fxemu2.mac"

%macro ccall 1-*
	push ecx
	push edx
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%if %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

; Spill the core's live registers to the seam block, run a C handler body, then
; reload them. ebp is the program counter, esi/edi the source/destination
; register pointers, ecx the opcode byte (cl) plus the ALT mode (ch); the C body
; may change any of them, exactly as an asm body could.
%macro fxcop 1
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    ccall %1
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    ret
%endmacro

SECTION .text

; void FxDispatch(u4 const *table) - the other half of the seam: run one opcode
; through the given dispatch table with the register ABI live. ecx indexes the
; table as (ALT mode << 8) | opcode, which reaches FxTable/FxTableA1/A2/A3 as
; one adjacent block.
NEWSYM FxDispatch
    push ebx
    push esi
    push edi
    push ebp
    mov eax, [esp+20]
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    call [eax+ecx*4]
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret




NEWSYM FxOpb05      ; BRA    branch always      ; Verified.
   fxcop c_FxOpb05

NEWSYM FxOpb06      ; BGE    branch on greater or equals        ; Verified.
   fxcop c_FxOpb06

NEWSYM FxOpb07      ; BLT    branch on lesss than       ; Verified.
   fxcop c_FxOpb07

NEWSYM FxOpb08      ; BNE    branch on not equal        ; Verified.
   fxcop c_FxOpb08

NEWSYM FxOpb09      ; BEQ    branch on equal (z=1)      ; Verified.
   fxcop c_FxOpb09

NEWSYM FxOpb0A      ; BPL    branch on plus     ; Verified.
   fxcop c_FxOpb0A

NEWSYM FxOpb0B      ; BMI    branch on minus    ; Verified.
   fxcop c_FxOpb0B

NEWSYM FxOpb0C      ; BCC    branch on carry clear      ; Verified.
   fxcop c_FxOpb0C

NEWSYM FxOpb0D      ; BCS    branch on carry set        ; Verified.
   fxcop c_FxOpb0D

NEWSYM FxOpb0E      ; BVC    branch on overflow clear   ; Verified.
   fxcop c_FxOpb0E

NEWSYM FxOpb0F      ; BVS    branch on overflow set     ; Verified.
   fxcop c_FxOpb0F

NEWSYM FxOpb10      ; TO RN  set register n as destination register
   fxcop c_FxOpb10

NEWSYM FxOpb11      ; TO RN  set register n as destination register
   fxcop c_FxOpb11

NEWSYM FxOpb12      ; TO RN  set register n as destination register
   fxcop c_FxOpb12

NEWSYM FxOpb13      ; TO RN  set register n as destination register
   fxcop c_FxOpb13

NEWSYM FxOpb14      ; TO RN  set register n as destination register
   fxcop c_FxOpb14

NEWSYM FxOpb15      ; TO RN  set register n as destination register
   fxcop c_FxOpb15

NEWSYM FxOpb16      ; TO RN  set register n as destination register
   fxcop c_FxOpb16

NEWSYM FxOpb17      ; TO RN  set register n as destination register
   fxcop c_FxOpb17

NEWSYM FxOpb18      ; TO RN  set register n as destination register
   fxcop c_FxOpb18

NEWSYM FxOpb19      ; TO RN  set register n as destination register
   fxcop c_FxOpb19

NEWSYM FxOpb1A      ; TO RN  set register n as destination register
   fxcop c_FxOpb1A

NEWSYM FxOpb1B      ; TO RN  set register n as destination register
   fxcop c_FxOpb1B

NEWSYM FxOpb1C      ; TO RN  set register n as destination register
   fxcop c_FxOpb1C

NEWSYM FxOpb1D      ; TO RN  set register n as destination register
   fxcop c_FxOpb1D

NEWSYM FxOpb1E      ; TO RN  set register n as destination register
   fxcop c_FxOpb1E

NEWSYM FxOpb1F      ; TO RN  set register n as destination register
   fxcop c_FxOpb1F

NEWSYM FxOpb3D      ; ALT1   set alt1 mode      ; Verified.
   fxcop c_FxOpb3D

NEWSYM FxOpb3E      ; ALT2   set alt1 mode      ; Verified.
   fxcop c_FxOpb3E

NEWSYM FxOpb3F      ; ALT3   set alt3 mode      ; Verified.
   fxcop c_FxOpb3F

NEWSYM FxOpbB0      ; FROM rn   set source register
   fxcop c_FxOpbB0

NEWSYM FxOpbB1      ; FROM rn   set source register
   fxcop c_FxOpbB1

NEWSYM FxOpbB2      ; FROM rn   set source register
   fxcop c_FxOpbB2

NEWSYM FxOpbB3      ; FROM rn   set source register
   fxcop c_FxOpbB3

NEWSYM FxOpbB4      ; FROM rn   set source register
   fxcop c_FxOpbB4

NEWSYM FxOpbB5      ; FROM rn   set source register
   fxcop c_FxOpbB5

NEWSYM FxOpbB6      ; FROM rn   set source register
   fxcop c_FxOpbB6

NEWSYM FxOpbB7      ; FROM rn   set source register
   fxcop c_FxOpbB7

NEWSYM FxOpbB8      ; FROM rn   set source register
   fxcop c_FxOpbB8

NEWSYM FxOpbB9      ; FROM rn   set source register
   fxcop c_FxOpbB9

NEWSYM FxOpbBA      ; FROM rn   set source register
   fxcop c_FxOpbBA

NEWSYM FxOpbBB      ; FROM rn   set source register
   fxcop c_FxOpbBB

NEWSYM FxOpbBC      ; FROM rn   set source register
   fxcop c_FxOpbBC

NEWSYM FxOpbBD      ; FROM rn   set source register
   fxcop c_FxOpbBD

NEWSYM FxOpbBE      ; FROM rn   set source register
   fxcop c_FxOpbBE

NEWSYM FxOpbBF      ; FROM rn   set source register
   fxcop c_FxOpbBF

NEWSYM FxOpc05      ; BRA    branch always      ; Verified.
   fxcop c_FxOpc05

NEWSYM FxOpc06      ; BGE    branch on greater or equals        ; Verified.
   fxcop c_FxOpc06

NEWSYM FxOpc07      ; BLT    branch on lesss than       ; Verified.
   fxcop c_FxOpc07

NEWSYM FxOpc08      ; BNE    branch on not equal        ; Verified.
   fxcop c_FxOpc08

NEWSYM FxOpc09      ; BEQ    branch on equal (z=1)      ; Verified.
   fxcop c_FxOpc09

NEWSYM FxOpc0A      ; BPL    branch on plus     ; Verified.
   fxcop c_FxOpc0A

NEWSYM FxOpc0B      ; BMI    branch on minus    ; Verified.
   fxcop c_FxOpc0B

NEWSYM FxOpc0C      ; BCC    branch on carry clear      ; Verified.
   fxcop c_FxOpc0C

NEWSYM FxOpc0D      ; BCS    branch on carry set        ; Verified.
   fxcop c_FxOpc0D

NEWSYM FxOpc0E      ; BVC    branch on overflow clear   ; Verified.
   fxcop c_FxOpc0E

NEWSYM FxOpc0F      ; BVS    branch on overflow set     ; Verified.
   fxcop c_FxOpc0F

NEWSYM FxOpc10      ; TO RN  set register n as destination register
   fxcop c_FxOpc10

NEWSYM FxOpc11      ; TO RN  set register n as destination register
   fxcop c_FxOpc11

NEWSYM FxOpc12      ; TO RN  set register n as destination register
   fxcop c_FxOpc12

NEWSYM FxOpc13      ; TO RN  set register n as destination register
   fxcop c_FxOpc13

NEWSYM FxOpc14      ; TO RN  set register n as destination register
   fxcop c_FxOpc14

NEWSYM FxOpc15      ; TO RN  set register n as destination register
   fxcop c_FxOpc15

NEWSYM FxOpc16      ; TO RN  set register n as destination register
   fxcop c_FxOpc16

NEWSYM FxOpc17      ; TO RN  set register n as destination register
   fxcop c_FxOpc17

NEWSYM FxOpc18      ; TO RN  set register n as destination register
   fxcop c_FxOpc18

NEWSYM FxOpc19      ; TO RN  set register n as destination register
   fxcop c_FxOpc19

NEWSYM FxOpc1A      ; TO RN  set register n as destination register
   fxcop c_FxOpc1A

NEWSYM FxOpc1B      ; TO RN  set register n as destination register
   fxcop c_FxOpc1B

NEWSYM FxOpc1C      ; TO RN  set register n as destination register
   fxcop c_FxOpc1C

NEWSYM FxOpc1D      ; TO RN  set register n as destination register
   fxcop c_FxOpc1D

NEWSYM FxOpc1E      ; TO RN  set register n as destination register
   fxcop c_FxOpc1E

NEWSYM FxOpc1F      ; TO RN  set register n as destination register
   fxcop c_FxOpc1F

NEWSYM FxOpc3D      ; ALT1   set alt1 mode      ; Verified.
   fxcop c_FxOpc3D

NEWSYM FxOpc3E      ; ALT2   set alt1 mode      ; Verified.
   fxcop c_FxOpc3E

NEWSYM FxOpc3F      ; ALT3   set alt3 mode      ; Verified.
   fxcop c_FxOpc3F

NEWSYM FxOpcB0      ; FROM rn   set source register
   fxcop c_FxOpcB0

NEWSYM FxOpcB1      ; FROM rn   set source register
   fxcop c_FxOpcB1

NEWSYM FxOpcB2      ; FROM rn   set source register
   fxcop c_FxOpcB2

NEWSYM FxOpcB3      ; FROM rn   set source register
   fxcop c_FxOpcB3

NEWSYM FxOpcB4      ; FROM rn   set source register
   fxcop c_FxOpcB4

NEWSYM FxOpcB5      ; FROM rn   set source register
   fxcop c_FxOpcB5

NEWSYM FxOpcB6      ; FROM rn   set source register
   fxcop c_FxOpcB6

NEWSYM FxOpcB7      ; FROM rn   set source register
   fxcop c_FxOpcB7

NEWSYM FxOpcB8      ; FROM rn   set source register
   fxcop c_FxOpcB8

NEWSYM FxOpcB9      ; FROM rn   set source register
   fxcop c_FxOpcB9

NEWSYM FxOpcBA      ; FROM rn   set source register
   fxcop c_FxOpcBA

NEWSYM FxOpcBB      ; FROM rn   set source register
   fxcop c_FxOpcBB

NEWSYM FxOpcBC      ; FROM rn   set source register
   fxcop c_FxOpcBC

NEWSYM FxOpcBD      ; FROM rn   set source register
   fxcop c_FxOpcBD

NEWSYM FxOpcBE      ; FROM rn   set source register
   fxcop c_FxOpcBE

NEWSYM FxOpcBF      ; FROM rn   set source register
   fxcop c_FxOpcBF
