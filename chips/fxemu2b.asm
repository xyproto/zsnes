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

%include "chips/fxemu2.mac"
%include "chips/fxemu2b.mac"

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
   TORNb 0
NEWSYM FxOpb11      ; TO RN  set register n as destination register
   TORNb 1
NEWSYM FxOpb12      ; TO RN  set register n as destination register
   TORNb 2
NEWSYM FxOpb13      ; TO RN  set register n as destination register
   TORNb 3
NEWSYM FxOpb14      ; TO RN  set register n as destination register
   TORNb 4
NEWSYM FxOpb15      ; TO RN  set register n as destination register
   TORNb 5
NEWSYM FxOpb16      ; TO RN  set register n as destination register
   TORNb 6
NEWSYM FxOpb17      ; TO RN  set register n as destination register
   TORNb 7
NEWSYM FxOpb18      ; TO RN  set register n as destination register
   TORNb 8
NEWSYM FxOpb19      ; TO RN  set register n as destination register
   TORNb 9
NEWSYM FxOpb1A      ; TO RN  set register n as destination register
   TORNb 10
NEWSYM FxOpb1B      ; TO RN  set register n as destination register
   TORNb 11
NEWSYM FxOpb1C      ; TO RN  set register n as destination register
   TORNb 12
NEWSYM FxOpb1D      ; TO RN  set register n as destination register
   TORNb 13
NEWSYM FxOpb1E      ; TO RN  set register n as destination register
   FETCHPIPE
   test dword[SfxB],1
   jnz .VersionB
   mov edi,SfxR0+14*4
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov dword[withr15sk],1
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov edi,SfxR0
   UpdateR14
   ret
.VersionB
   mov eax,[esi]            ; Read Source
   mov dword[withr15sk],1
   mov [SfxR0+14*4],eax             ; Write
   CLRFLAGS
   UpdateR14
   inc ebp                ; Increase program counter
   ret
NEWSYM FxOpb1F      ; TO RN  set register n as destination register
   FETCHPIPE
   test dword[SfxB],1
   jnz .VersionB
   mov edi,SfxR0+15*4
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov ebp,[SfxCPB]
   mov dword[withr15sk],1
   add ebp,[SfxR15]
   mov edi,SfxR0
   ret
.VersionB
   mov eax,[esi]            ; Read Source
   mov ebp,[SfxCPB]
   mov dword[withr15sk],1
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpb3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,01h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpb3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,02h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpb3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,03h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpbB0      ; FROM rn   set source register
   FROMRNb 0
NEWSYM FxOpbB1      ; FROM rn   set source register
   FROMRNb 1
NEWSYM FxOpbB2      ; FROM rn   set source register
   FROMRNb 2
NEWSYM FxOpbB3      ; FROM rn   set source register
   FROMRNb 3
NEWSYM FxOpbB4      ; FROM rn   set source register
   FROMRNb 4
NEWSYM FxOpbB5      ; FROM rn   set source register
   FROMRNb 5
NEWSYM FxOpbB6      ; FROM rn   set source register
   FROMRNb 6
NEWSYM FxOpbB7      ; FROM rn   set source register
   FROMRNb 7
NEWSYM FxOpbB8      ; FROM rn   set source register
   FROMRNb 8
NEWSYM FxOpbB9      ; FROM rn   set source register
   FROMRNb 9
NEWSYM FxOpbBA      ; FROM rn   set source register
   FROMRNb 10
NEWSYM FxOpbBB      ; FROM rn   set source register
   FROMRNb 11
NEWSYM FxOpbBC      ; FROM rn   set source register
   FROMRNb 12
NEWSYM FxOpbBD      ; FROM rn   set source register
   FROMRNb 13
NEWSYM FxOpbBE      ; FROM rn   set source register
   FROMRNb 14
NEWSYM FxOpbBF      ; FROM rn   set source register
   test dword[SfxB],1
   jnz .VersionB
   mov esi,SfxR0+15*4
   inc ebp                ; Increase program counter
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov esi,SfxR0
   ret
.VersionB
   FETCHPIPE
   mov eax,ebp
   sub eax,[SfxCPB]
   inc ebp
   mov [edi],eax        ; Write Destination
   mov [SfxSignZero],eax
   shr al,7
   mov [SfxOverflow],al
   CLRFLAGS
   ret

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
   TORNc 0
NEWSYM FxOpc11      ; TO RN  set register n as destination register
   TORNc 1
NEWSYM FxOpc12      ; TO RN  set register n as destination register
   TORNc 2
NEWSYM FxOpc13      ; TO RN  set register n as destination register
   TORNc 3
NEWSYM FxOpc14      ; TO RN  set register n as destination register
   TORNc 4
NEWSYM FxOpc15      ; TO RN  set register n as destination register
   TORNc 5
NEWSYM FxOpc16      ; TO RN  set register n as destination register
   TORNc 6
NEWSYM FxOpc17      ; TO RN  set register n as destination register
   TORNc 7
NEWSYM FxOpc18      ; TO RN  set register n as destination register
   TORNc 8
NEWSYM FxOpc19      ; TO RN  set register n as destination register
   TORNc 9
NEWSYM FxOpc1A      ; TO RN  set register n as destination register
   TORNc 10
NEWSYM FxOpc1B      ; TO RN  set register n as destination register
   TORNc 11
NEWSYM FxOpc1C      ; TO RN  set register n as destination register
   TORNc 12
NEWSYM FxOpc1D      ; TO RN  set register n as destination register
   TORNc 13
NEWSYM FxOpc1E      ; TO RN  set register n as destination register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov [SfxR0+14*4],eax             ; Write
   CLRFLAGS
   UpdateR14
   inc ebp                ; Increase program counter
   ret
NEWSYM FxOpc1F      ; TO RN  set register n as destination register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebp,[SfxCPB]
   mov [SfxR15],eax
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpc3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,01h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpc3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,02h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpc3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword[SfxB],0
   or ch,03h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpcB0      ; FROM rn   set source register
   FROMRNc 0
NEWSYM FxOpcB1      ; FROM rn   set source register
   FROMRNc 1
NEWSYM FxOpcB2      ; FROM rn   set source register
   FROMRNc 2
NEWSYM FxOpcB3      ; FROM rn   set source register
   FROMRNc 3
NEWSYM FxOpcB4      ; FROM rn   set source register
   FROMRNc 4
NEWSYM FxOpcB5      ; FROM rn   set source register
   FROMRNc 5
NEWSYM FxOpcB6      ; FROM rn   set source register
   FROMRNc 6
NEWSYM FxOpcB7      ; FROM rn   set source register
   FROMRNc 7
NEWSYM FxOpcB8      ; FROM rn   set source register
   FROMRNc 8
NEWSYM FxOpcB9      ; FROM rn   set source register
   FROMRNc 9
NEWSYM FxOpcBA      ; FROM rn   set source register
   FROMRNc 10
NEWSYM FxOpcBB      ; FROM rn   set source register
   FROMRNc 11
NEWSYM FxOpcBC      ; FROM rn   set source register
   FROMRNc 12
NEWSYM FxOpcBD      ; FROM rn   set source register
   FROMRNc 13
NEWSYM FxOpcBE      ; FROM rn   set source register
   FROMRNc 14
NEWSYM FxOpcBF      ; FROM rn   set source register
   FETCHPIPE
   mov eax,ebp
   sub eax,[SfxCPB]
   inc ebp
   mov [edi],eax        ; Write Destination
   mov [SfxSignZero],eax
   shr al,7
   mov [SfxOverflow],al
   CLRFLAGS
   ret
