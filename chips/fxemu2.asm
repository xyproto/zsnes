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

%macro ALIGN16 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro
EXTSYM FxTable,FxTableb,FxTablec,FxTabled,SfxMemTable,flagnz,fxbit01,fxbit23
EXTSYM fxxand,sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc
EXTSYM sfxramdata,fxbit45,fxbit67,SFXProc,ChangeOps,PLOTJmpa,PLOTJmpb

; Seam to the handlers ported to C (chips/c_fxops.c, chips/fx_ops.h). See
; chips/fxemu2b.asm for the other half, FxDispatch.
EXTSYM FxSeamPC,FxSeamSrc,FxSeamDst,FxSeamCX
EXTSYM c_FxOp50,c_FxOp51,c_FxOp52,c_FxOp53,c_FxOp54,c_FxOp55
EXTSYM c_FxOp56,c_FxOp57,c_FxOp58,c_FxOp59,c_FxOp5A,c_FxOp5B
EXTSYM c_FxOp5C,c_FxOp5D,c_FxOp5E,c_FxOp50A1,c_FxOp51A1,c_FxOp52A1
EXTSYM c_FxOp53A1,c_FxOp54A1,c_FxOp55A1,c_FxOp56A1,c_FxOp57A1,c_FxOp58A1
EXTSYM c_FxOp59A1,c_FxOp5AA1,c_FxOp5BA1,c_FxOp5CA1,c_FxOp5DA1,c_FxOp5EA1
EXTSYM c_FxOp50A2,c_FxOp51A2,c_FxOp52A2,c_FxOp53A2,c_FxOp54A2,c_FxOp55A2
EXTSYM c_FxOp56A2,c_FxOp57A2,c_FxOp58A2,c_FxOp59A2,c_FxOp5AA2,c_FxOp5BA2
EXTSYM c_FxOp5CA2,c_FxOp5DA2,c_FxOp5EA2,c_FxOp5FA2,c_FxOp50A3,c_FxOp51A3
EXTSYM c_FxOp52A3,c_FxOp53A3,c_FxOp54A3,c_FxOp55A3,c_FxOp56A3,c_FxOp57A3
EXTSYM c_FxOp58A3,c_FxOp59A3,c_FxOp5AA3,c_FxOp5BA3,c_FxOp5CA3,c_FxOp5DA3
EXTSYM c_FxOp5EA3,c_FxOp5FA3,c_FxOp60,c_FxOp61,c_FxOp62,c_FxOp63
EXTSYM c_FxOp64,c_FxOp65,c_FxOp66,c_FxOp67,c_FxOp68,c_FxOp69
EXTSYM c_FxOp6A,c_FxOp6B,c_FxOp6C,c_FxOp6D,c_FxOp6E,c_FxOp60A1
EXTSYM c_FxOp61A1,c_FxOp62A1,c_FxOp63A1,c_FxOp64A1,c_FxOp65A1,c_FxOp66A1
EXTSYM c_FxOp67A1,c_FxOp68A1,c_FxOp69A1,c_FxOp6AA1,c_FxOp6BA1,c_FxOp6CA1
EXTSYM c_FxOp6DA1,c_FxOp6EA1,c_FxOp60A2,c_FxOp61A2,c_FxOp62A2,c_FxOp63A2
EXTSYM c_FxOp64A2,c_FxOp65A2,c_FxOp66A2,c_FxOp67A2,c_FxOp68A2,c_FxOp69A2
EXTSYM c_FxOp6AA2,c_FxOp6BA2,c_FxOp6CA2,c_FxOp6DA2,c_FxOp6EA2,c_FxOp6FA2
EXTSYM c_FxOp60A3,c_FxOp61A3,c_FxOp62A3,c_FxOp63A3,c_FxOp64A3,c_FxOp65A3
EXTSYM c_FxOp66A3,c_FxOp67A3,c_FxOp68A3,c_FxOp69A3,c_FxOp6AA3,c_FxOp6BA3
EXTSYM c_FxOp6CA3,c_FxOp6DA3,c_FxOp6EA3,c_FxOp71,c_FxOp72,c_FxOp73
EXTSYM c_FxOp74,c_FxOp75,c_FxOp76,c_FxOp77,c_FxOp78,c_FxOp79
EXTSYM c_FxOp7A,c_FxOp7B,c_FxOp7C,c_FxOp7D,c_FxOp7E,c_FxOp71A1
EXTSYM c_FxOp72A1,c_FxOp73A1,c_FxOp74A1,c_FxOp75A1,c_FxOp76A1,c_FxOp77A1
EXTSYM c_FxOp78A1,c_FxOp79A1,c_FxOp7AA1,c_FxOp7BA1,c_FxOp7CA1,c_FxOp7DA1
EXTSYM c_FxOp7EA1,c_FxOp71A2,c_FxOp72A2,c_FxOp73A2,c_FxOp74A2,c_FxOp75A2
EXTSYM c_FxOp76A2,c_FxOp77A2,c_FxOp78A2,c_FxOp79A2,c_FxOp7AA2,c_FxOp7BA2
EXTSYM c_FxOp7CA2,c_FxOp7DA2,c_FxOp7EA2,c_FxOp7FA2,c_FxOp71A3,c_FxOp72A3
EXTSYM c_FxOp73A3,c_FxOp74A3,c_FxOp75A3,c_FxOp76A3,c_FxOp77A3,c_FxOp78A3
EXTSYM c_FxOp79A3,c_FxOp7AA3,c_FxOp7BA3,c_FxOp7CA3,c_FxOp7DA3,c_FxOp7EA3
EXTSYM c_FxOp7FA3
EXTSYM c_FxOpC1,c_FxOpC2,c_FxOpC3,c_FxOpC4,c_FxOpC5,c_FxOpC6
EXTSYM c_FxOpC7,c_FxOpC8,c_FxOpC9,c_FxOpCA,c_FxOpCB,c_FxOpCC
EXTSYM c_FxOpCD,c_FxOpCE,c_FxOpC1A1,c_FxOpC2A1,c_FxOpC3A1,c_FxOpC4A1
EXTSYM c_FxOpC5A1,c_FxOpC6A1,c_FxOpC7A1,c_FxOpC8A1,c_FxOpC9A1,c_FxOpCAA1
EXTSYM c_FxOpCBA1,c_FxOpCCA1,c_FxOpCDA1,c_FxOpCEA1,c_FxOpC1A2,c_FxOpC2A2
EXTSYM c_FxOpC3A2,c_FxOpC4A2,c_FxOpC5A2,c_FxOpC6A2,c_FxOpC7A2,c_FxOpC8A2
EXTSYM c_FxOpC9A2,c_FxOpCAA2,c_FxOpCBA2,c_FxOpCCA2,c_FxOpCDA2,c_FxOpCEA2
EXTSYM c_FxOpCFA2,c_FxOpC1A3,c_FxOpC2A3,c_FxOpC3A3,c_FxOpC4A3,c_FxOpC5A3
EXTSYM c_FxOpC6A3,c_FxOpC7A3,c_FxOpC8A3,c_FxOpC9A3,c_FxOpCAA3,c_FxOpCBA3
EXTSYM c_FxOpCCA3,c_FxOpCDA3,c_FxOpCEA3,c_FxOpCFA3,c_FxOpD0,c_FxOpD1
EXTSYM c_FxOpD2,c_FxOpD3,c_FxOpD4,c_FxOpD5,c_FxOpD6,c_FxOpD7
EXTSYM c_FxOpD8,c_FxOpD9,c_FxOpDA,c_FxOpDB,c_FxOpDC,c_FxOpDD
EXTSYM c_FxOpE0,c_FxOpE1,c_FxOpE2,c_FxOpE3,c_FxOpE4,c_FxOpE5
EXTSYM c_FxOpE6,c_FxOpE7,c_FxOpE8,c_FxOpE9,c_FxOpEA,c_FxOpEB
EXTSYM c_FxOpEC,c_FxOpED
EXTSYM c_FxOp80,c_FxOp81,c_FxOp82,c_FxOp83,c_FxOp84,c_FxOp85
EXTSYM c_FxOp86,c_FxOp87,c_FxOp88,c_FxOp89,c_FxOp8A,c_FxOp8B
EXTSYM c_FxOp8C,c_FxOp8D,c_FxOp8E,c_FxOp80A1,c_FxOp81A1,c_FxOp82A1
EXTSYM c_FxOp83A1,c_FxOp84A1,c_FxOp85A1,c_FxOp86A1,c_FxOp87A1,c_FxOp88A1
EXTSYM c_FxOp89A1,c_FxOp8AA1,c_FxOp8BA1,c_FxOp8CA1,c_FxOp8DA1,c_FxOp8EA1
EXTSYM c_FxOp80A2,c_FxOp81A2,c_FxOp82A2,c_FxOp83A2,c_FxOp84A2,c_FxOp85A2
EXTSYM c_FxOp86A2,c_FxOp87A2,c_FxOp88A2,c_FxOp89A2,c_FxOp8AA2,c_FxOp8BA2
EXTSYM c_FxOp8CA2,c_FxOp8DA2,c_FxOp8EA2,c_FxOp8FA2,c_FxOp80A3,c_FxOp81A3
EXTSYM c_FxOp82A3,c_FxOp83A3,c_FxOp84A3,c_FxOp85A3,c_FxOp86A3,c_FxOp87A3
EXTSYM c_FxOp88A3,c_FxOp89A3,c_FxOp8AA3,c_FxOp8BA3,c_FxOp8CA3,c_FxOp8DA3
EXTSYM c_FxOp8EA3,c_FxOp8FA3
EXTSYM c_FxOp10,c_FxOp11,c_FxOp12,c_FxOp13,c_FxOp14,c_FxOp15
EXTSYM c_FxOp16,c_FxOp17,c_FxOp18,c_FxOp19,c_FxOp1A,c_FxOp1B
EXTSYM c_FxOp1C,c_FxOp1D,c_FxOp20,c_FxOp21,c_FxOp22,c_FxOp23
EXTSYM c_FxOp24,c_FxOp25,c_FxOp26,c_FxOp27,c_FxOp28,c_FxOp29
EXTSYM c_FxOp2A,c_FxOp2B,c_FxOp2C,c_FxOp2D,c_FxOpB0,c_FxOpB1
EXTSYM c_FxOpB2,c_FxOpB3,c_FxOpB4,c_FxOpB5,c_FxOpB6,c_FxOpB7
EXTSYM c_FxOpB8,c_FxOpB9,c_FxOpBA,c_FxOpBB,c_FxOpBC,c_FxOpBD
EXTSYM c_FxOpBE
EXTSYM c_FxOp30,c_FxOp31,c_FxOp32,c_FxOp33,c_FxOp34,c_FxOp35
EXTSYM c_FxOp36,c_FxOp37,c_FxOp38,c_FxOp39,c_FxOp3A,c_FxOp3B
EXTSYM c_FxOp30A1,c_FxOp31A1,c_FxOp32A1,c_FxOp33A1,c_FxOp34A1,c_FxOp35A1
EXTSYM c_FxOp36A1,c_FxOp37A1,c_FxOp38A1,c_FxOp39A1,c_FxOp3AA1,c_FxOp3BA1
EXTSYM c_FxOp40,c_FxOp41,c_FxOp42,c_FxOp43,c_FxOp44,c_FxOp45
EXTSYM c_FxOp46,c_FxOp47,c_FxOp48,c_FxOp49,c_FxOp4A,c_FxOp4B
EXTSYM c_FxOp40A1,c_FxOp41A1,c_FxOp42A1,c_FxOp43A1,c_FxOp44A1,c_FxOp45A1
EXTSYM c_FxOp46A1,c_FxOp47A1,c_FxOp48A1,c_FxOp49A1,c_FxOp4AA1,c_FxOp4BA1
EXTSYM c_FxOpA0,c_FxOpA1,c_FxOpA2,c_FxOpA3,c_FxOpA4,c_FxOpA5
EXTSYM c_FxOpA6,c_FxOpA7,c_FxOpA8,c_FxOpA9,c_FxOpAA,c_FxOpAB
EXTSYM c_FxOpAC,c_FxOpAD,c_FxOpA0A1,c_FxOpA1A1,c_FxOpA2A1,c_FxOpA3A1
EXTSYM c_FxOpA4A1,c_FxOpA5A1,c_FxOpA6A1,c_FxOpA7A1,c_FxOpA8A1,c_FxOpA9A1
EXTSYM c_FxOpAAA1,c_FxOpABA1,c_FxOpACA1,c_FxOpADA1,c_FxOpA0A2,c_FxOpA1A2
EXTSYM c_FxOpA2A2,c_FxOpA3A2,c_FxOpA4A2,c_FxOpA5A2,c_FxOpA6A2,c_FxOpA7A2
EXTSYM c_FxOpA8A2,c_FxOpA9A2,c_FxOpAAA2,c_FxOpABA2,c_FxOpACA2,c_FxOpADA2
EXTSYM c_FxOpAEA2,c_FxOpF0,c_FxOpF1,c_FxOpF2,c_FxOpF3,c_FxOpF4
EXTSYM c_FxOpF5,c_FxOpF6,c_FxOpF7,c_FxOpF8,c_FxOpF9,c_FxOpFA
EXTSYM c_FxOpFB,c_FxOpFC,c_FxOpFD,c_FxOpF0A1,c_FxOpF1A1,c_FxOpF2A1
EXTSYM c_FxOpF3A1,c_FxOpF4A1,c_FxOpF5A1,c_FxOpF6A1,c_FxOpF7A1,c_FxOpF8A1
EXTSYM c_FxOpF9A1,c_FxOpFAA1,c_FxOpFBA1,c_FxOpFCA1,c_FxOpFDA1,c_FxOpF0A2
EXTSYM c_FxOpF1A2,c_FxOpF2A2,c_FxOpF3A2,c_FxOpF4A2,c_FxOpF5A2,c_FxOpF6A2
EXTSYM c_FxOpF7A2,c_FxOpF8A2,c_FxOpF9A2,c_FxOpFAA2,c_FxOpFBA2,c_FxOpFCA2
EXTSYM c_FxOpFDA2,c_FxOpFEA2
EXTSYM c_FxOp91,c_FxOp92,c_FxOp93,c_FxOp94,c_FxOp98,c_FxOp99
EXTSYM c_FxOp9A,c_FxOp9B,c_FxOp9C,c_FxOp9D,c_FxOp98A1,c_FxOp99A1
EXTSYM c_FxOp9AA1,c_FxOp9BA1,c_FxOp9CA1,c_FxOp9DA1,c_FxOp02
EXTSYM c_FxOp01,c_FxOp4D,c_FxOp4F,c_FxOp95,c_FxOp96,c_FxOp96A1
EXTSYM c_FxOp97,c_FxOp9E,c_FxOpC0
EXTSYM c_FxOp03,c_FxOp04,c_FxOp3C,c_FxOp9F,c_FxOp9FA1,c_FxOpAE
EXTSYM c_FxOpAF,c_FxOpDE,c_FxOpEE
EXTSYM c_FxOp05,c_FxOp06,c_FxOp07,c_FxOp08,c_FxOp09,c_FxOp0A
EXTSYM c_FxOp0B,c_FxOp0C,c_FxOp0D,c_FxOp0E,c_FxOp0F,c_FxOp1E
EXTSYM c_FxOp1F,c_FxOp2E,c_FxOp2F,c_FxOp3D,c_FxOp3E,c_FxOp3F
EXTSYM c_FxOpBF
EXTSYM c_FxOp5F,c_FxOp5FA1,c_FxOp6F,c_FxOp6FA1,c_FxOp6FA3,c_FxOp7F
EXTSYM c_FxOp7FA1,c_FxOp8F,c_FxOp8FA1,c_FxOp90
EXTSYM c_FxOpEF,c_FxOpEFA1,c_FxOpEFA2,c_FxOpEFA3,c_FxOpDFA2,c_FxOpDFA3
EXTSYM c_FxOpCF,c_FxOpCFA1,c_FxOpAEA1,c_FxOpAFA1,c_FxOpAFA2,c_FxOpFE
EXTSYM c_FxOpFF
EXTSYM c_FxOp4E,c_FxOpDF,c_FxOp4EA1,c_FxOp70,c_FxOpFEA1,c_FxOpFFA1
EXTSYM c_FxOpFFA2
EXTSYM c_FxOp00
EXTSYM c_FxOp4C1284b,c_FxOp4C1284bz,c_FxOp4C1284bd,c_FxOp4C1284bzd
EXTSYM c_FxOp4C1282b,c_FxOp4C1282bz,c_FxOp4C1282bd,c_FxOp4C1282bzd
EXTSYM c_FxOp4C1288b,c_FxOp4C1288bz,c_FxOp4C1288bd,c_FxOp4C1288bzd
EXTSYM c_FxOp4C1288bl,c_FxOp4C1288bzl,c_FxOp4C1288bdl,c_FxOp4C1288bzdl
EXTSYM c_FxOp4C,c_FxOp4CA1

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
; register pointers, ecx the opcode byte (cl) plus the ALT mode (ch).
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

%include "chips/fxemu2.mac"

; * Optimise PLOT, COLOR!
SECTION .text
NEWSYM FlushCache
   ; Copy 512 bytes from pb:eax to SfxCACHERAM
   ret

SECTION .bss
NEWSYM tempsfx, resb 3

SECTION .data

; FxChip emulation by _Demo_
; Optimised by zsKnight
; based on fxemu by lestat

NEWSYM SfxR0,    dd 0     ; default source/destination register
NEWSYM SfxR1,    dd 0     ; pixel plot X position register
NEWSYM SfxR2,    dd 0     ; pixel plot Y position register
NEWSYM SfxR3,    dd 0     ;
NEWSYM SfxR4,    dd 0     ; lower 16 bit result of lmult
NEWSYM SfxR5,    dd 0     ;
NEWSYM SfxR6,    dd 0     ; multiplier for fmult and lmult
NEWSYM SfxR7,    dd 0     ; fixed point texel X position for merge
NEWSYM SfxR8,    dd 0     ; fixed point texel Y position for merge
NEWSYM SfxR9,    dd 0     ;
NEWSYM SfxR10,   dd 0     ;
NEWSYM SfxR11,   dd 0     ; return address set by link
NEWSYM SfxR12,   dd 0     ; loop counter
NEWSYM SfxR13,   dd 0     ; loop point address
NEWSYM SfxR14,   dd 0     ; rom address for getb, getbh, getbl, getbs
NEWSYM SfxR15,   dd 0     ; program counter

NEWSYM SfxSFR,   dd 0     ; status flag register (16bit)
;SFR status flag register bits:
; 0   -
; 1   Z   Zero flag
; 2   CY  Carry flag
; 3   S   Sign flag
; 4   OV  Overflow flag
; 5   G   Go flag (set to 1 when the GSU is running)
; 6   R   Set to 1 when reading ROM using R14 address
; 7   -
; 8   ALT1   Mode set-up flag for the next instruction
; 9   ALT2   Mode set-up flag for the next instruction
;10   IL  Immediate lower 8-bit flag
;11   IH  Immediate higher 8-bit flag
;12   B   Set to 1 when the WITH instruction is executed
;13   -
;14   -
;15   IRQ Set to 1 when GSU caused an interrupt
;                Set to 0 when read by 658c16

NEWSYM SfxBRAMR,    dd 0  ; backup ram read only on/off (8bit)
NEWSYM SfxPBR,      dd 0
NEWSYM SfxROMBR,    dd 0  ; rom bank register (8bit)
NEWSYM SfxCFGR,     dd 0  ; control flags register (8bit)
NEWSYM SfxSCBR,     dd 0
NEWSYM SfxCLSR,     dd 0
NEWSYM SfxSCMR,     dd 0
NEWSYM SfxVCR,      dd 0  ; version code register (8bit)
NEWSYM SfxRAMBR,    dd 0  ; ram bank register (8bit)
NEWSYM SfxCBR,      dd 0  ; cache base register (16bit)

NEWSYM SfxCOLR,     dd 0
NEWSYM SfxPOR,      dd 0

NEWSYM SfxCacheFlags,  dd 0  ; Saying what parts of the cache was written to
NEWSYM SfxLastRamAdr,  dd 0  ; Last RAM address accessed
NEWSYM SfxDREG,        dd 0  ; Current destination register index
NEWSYM SfxSREG,        dd 0  ; Current source register index
NEWSYM SfxRomBuffer,   dd 0  ; Current byte read by R14
NEWSYM SfxPIPE,        dd 0  ; Instructionset pipe
NEWSYM SfxPipeAdr,     dd 0  ; The address of where the pipe was read from

NEWSYM SfxnRamBanks,   dd 4  ; Number of 64kb-banks in FxRam (Don't confuse it with SNES-Ram!!!)
NEWSYM SfxnRomBanks,   dd 0  ; Number of 32kb-banks in Cart-ROM

NEWSYM SfxvScreenHeight, dd 0 ; 128, 160 or 192
NEWSYM SfxvScreenSize, dd 0

NEWSYM SfxCacheActive, dd 0  ; Cache Active

NEWSYM SfxCarry,       dd 0  ; Carry flag
NEWSYM SfxSignZero,    dd 0  ; Sign and Zero flag
NEWSYM SfxB,           dd 0  ; B flag  (1 when with instruction executed)
NEWSYM SfxOverflow,    dd 0  ; Overflow flag

NEWSYM SfxCACHERAM, times 512 db 0    ; 512 bytes of GSU cache memory
num2writesfxreg  equ $-SfxR0
; pharos equ hack *sigh*
NEWSYM PHnum2writesfxreg, dd num2writesfxreg

NEWSYM SfxCPB,         dd 0
NEWSYM SfxCROM,        dd 0
NEWSYM SfxRAMMem,      dd 0
NEWSYM withr15sk,      dd 0
NEWSYM sfxclineloc,       dd 0
NEWSYM SCBRrel, dd 0
NEWSYM fxbit01pcal, dd 0
NEWSYM fxbit23pcal, dd 0
NEWSYM fxbit45pcal, dd 0
NEWSYM fxbit67pcal, dd 0

;SfxRAM times 256*1024 db 0

; If we need this later...

SECTION .text
NEWSYM FxOp00     ; STOP   stop GSU execution (and maybe generate an IRQ)     ; Verified.
   fxcop c_FxOp00
NEWSYM FxOp01      ; NOP    no operation       ; Verified.
   fxcop c_FxOp01
NEWSYM FxOp02      ; CACHE  reintialize GSU cache
   fxcop c_FxOp02
NEWSYM FxOp03      ; LSR    logic shift right  ; Verified.
   fxcop c_FxOp03
NEWSYM FxOp04      ; ROL    rotate left (RCL?) ; V
   fxcop c_FxOp04
NEWSYM FxOp05      ; BRA    branch always      ; Verified.
   fxcop c_FxOp05
NEWSYM FxOp06      ; BGE    branch on greater or equals        ; Verified.
   fxcop c_FxOp06
NEWSYM FxOp07      ; BLT    branch on lesss than       ; Verified.
   fxcop c_FxOp07
NEWSYM FxOp08      ; BNE    branch on not equal        ; Verified.
   fxcop c_FxOp08
NEWSYM FxOp09      ; BEQ    branch on equal (z=1)      ; Verified.
   fxcop c_FxOp09
NEWSYM FxOp0A      ; BPL    branch on plus     ; Verified.
   fxcop c_FxOp0A
NEWSYM FxOp0B      ; BMI    branch on minus    ; Verified.
   fxcop c_FxOp0B
NEWSYM FxOp0C      ; BCC    branch on carry clear      ; Verified.
   fxcop c_FxOp0C
NEWSYM FxOp0D      ; BCS    branch on carry set        ; Verified.
   fxcop c_FxOp0D
NEWSYM FxOp0E      ; BVC    branch on overflow clear   ; Verified.
   fxcop c_FxOp0E
NEWSYM FxOp0F      ; BVS    branch on overflow set     ; Verified.
   fxcop c_FxOp0F
NEWSYM FxOp10      ; TO RN  set register n as destination register
   fxcop c_FxOp10
NEWSYM FxOp11      ; TO RN  set register n as destination register
   fxcop c_FxOp11
NEWSYM FxOp12      ; TO RN  set register n as destination register
   fxcop c_FxOp12
NEWSYM FxOp13      ; TO RN  set register n as destination register
   fxcop c_FxOp13
NEWSYM FxOp14      ; TO RN  set register n as destination register
   fxcop c_FxOp14
NEWSYM FxOp15      ; TO RN  set register n as destination register
   fxcop c_FxOp15
NEWSYM FxOp16      ; TO RN  set register n as destination register
   fxcop c_FxOp16
NEWSYM FxOp17      ; TO RN  set register n as destination register
   fxcop c_FxOp17
NEWSYM FxOp18      ; TO RN  set register n as destination register
   fxcop c_FxOp18
NEWSYM FxOp19      ; TO RN  set register n as destination register
   fxcop c_FxOp19
NEWSYM FxOp1A      ; TO RN  set register n as destination register
   fxcop c_FxOp1A
NEWSYM FxOp1B      ; TO RN  set register n as destination register
   fxcop c_FxOp1B
NEWSYM FxOp1C      ; TO RN  set register n as destination register
   fxcop c_FxOp1C
NEWSYM FxOp1D      ; TO RN  set register n as destination register
   fxcop c_FxOp1D
NEWSYM FxOp1E      ; TO RN  set register n as destination register
   fxcop c_FxOp1E
NEWSYM FxOp1F      ; TO RN  set register n as destination register
   fxcop c_FxOp1F
NEWSYM FxOp20      ; WITH  set register n as source and destination register
   fxcop c_FxOp20
NEWSYM FxOp21      ; WITH  set register n as source and destination register
   fxcop c_FxOp21
NEWSYM FxOp22      ; WITH  set register n as source and destination register
   fxcop c_FxOp22
NEWSYM FxOp23      ; WITH  set register n as source and destination register
   fxcop c_FxOp23
NEWSYM FxOp24      ; WITH  set register n as source and destination register
   fxcop c_FxOp24
NEWSYM FxOp25      ; WITH  set register n as source and destination register
   fxcop c_FxOp25
NEWSYM FxOp26      ; WITH  set register n as source and destination register
   fxcop c_FxOp26
NEWSYM FxOp27      ; WITH  set register n as source and destination register
   fxcop c_FxOp27
NEWSYM FxOp28      ; WITH  set register n as source and destination register
   fxcop c_FxOp28
NEWSYM FxOp29      ; WITH  set register n as source and destination register
   fxcop c_FxOp29
NEWSYM FxOp2A      ; WITH  set register n as source and destination register
   fxcop c_FxOp2A
NEWSYM FxOp2B      ; WITH  set register n as source and destination register
   fxcop c_FxOp2B
NEWSYM FxOp2C      ; WITH  set register n as source and destination register
   fxcop c_FxOp2C
NEWSYM FxOp2D      ; WITH  set register n as source and destination register
   fxcop c_FxOp2D
NEWSYM FxOp2E      ; WITH  set register n as source and destination register
   fxcop c_FxOp2E
NEWSYM FxOp2F      ; WITH  set register n as source and destination register
   fxcop c_FxOp2F
NEWSYM FxOp30      ; STW RN store word
   fxcop c_FxOp30
NEWSYM FxOp31      ; STW RN store word
   fxcop c_FxOp31
NEWSYM FxOp32      ; STW RN store word
   fxcop c_FxOp32
NEWSYM FxOp33      ; STW RN store word
   fxcop c_FxOp33
NEWSYM FxOp34      ; STW RN store word
   fxcop c_FxOp34
NEWSYM FxOp35      ; STW RN store word
   fxcop c_FxOp35
NEWSYM FxOp36      ; STW RN store word
   fxcop c_FxOp36
NEWSYM FxOp37      ; STW RN store word
   fxcop c_FxOp37
NEWSYM FxOp38      ; STW RN store word
   fxcop c_FxOp38
NEWSYM FxOp39      ; STW RN store word
   fxcop c_FxOp39
NEWSYM FxOp3A      ; STW RN store word
   fxcop c_FxOp3A
NEWSYM FxOp3B      ; STW RN store word
   fxcop c_FxOp3B
NEWSYM FxOp30A1    ; STB RN store byte
   fxcop c_FxOp30A1
NEWSYM FxOp31A1    ; STB RN store byte
   fxcop c_FxOp31A1
NEWSYM FxOp32A1    ; STB RN store byte
   fxcop c_FxOp32A1
NEWSYM FxOp33A1    ; STB RN store byte
   fxcop c_FxOp33A1
NEWSYM FxOp34A1    ; STB RN store byte
   fxcop c_FxOp34A1
NEWSYM FxOp35A1    ; STB RN store byte
   fxcop c_FxOp35A1
NEWSYM FxOp36A1    ; STB RN store byte
   fxcop c_FxOp36A1
NEWSYM FxOp37A1    ; STB RN store byte
   fxcop c_FxOp37A1
NEWSYM FxOp38A1    ; STB RN store byte
   fxcop c_FxOp38A1
NEWSYM FxOp39A1    ; STB RN store byte
   fxcop c_FxOp39A1
NEWSYM FxOp3AA1    ; STB RN store byte
   fxcop c_FxOp3AA1
NEWSYM FxOp3BA1    ; STB RN store byte
   fxcop c_FxOp3BA1
NEWSYM FxOp3C      ; LOOP   decrement loop counter, and branch on not zero ; V
   fxcop c_FxOp3C
NEWSYM FxOp3D      ; ALT1   set alt1 mode      ; Verified.
   fxcop c_FxOp3D
NEWSYM FxOp3E      ; ALT2   set alt1 mode      ; Verified.
   fxcop c_FxOp3E
NEWSYM FxOp3F      ; ALT3   set alt3 mode      ; Verified.
   fxcop c_FxOp3F
NEWSYM FxOp40      ; LDW RN load word from RAM
   fxcop c_FxOp40
NEWSYM FxOp41      ; LDW RN load word from RAM
   fxcop c_FxOp41
NEWSYM FxOp42      ; LDW RN load word from RAM
   fxcop c_FxOp42
NEWSYM FxOp43      ; LDW RN load word from RAM
   fxcop c_FxOp43
NEWSYM FxOp44      ; LDW RN load word from RAM
   fxcop c_FxOp44
NEWSYM FxOp45      ; LDW RN load word from RAM
   fxcop c_FxOp45
NEWSYM FxOp46      ; LDW RN load word from RAM
   fxcop c_FxOp46
NEWSYM FxOp47      ; LDW RN load word from RAM
   fxcop c_FxOp47
NEWSYM FxOp48      ; LDW RN load word from RAM
   fxcop c_FxOp48
NEWSYM FxOp49      ; LDW RN load word from RAM
   fxcop c_FxOp49
NEWSYM FxOp4A      ; LDW RN load word from RAM
   fxcop c_FxOp4A
NEWSYM FxOp4B      ; LDW RN load word from RAM
   fxcop c_FxOp4B
NEWSYM FxOp40A1    ; LDB RN load byte from RAM
   fxcop c_FxOp40A1
NEWSYM FxOp41A1    ; LDB RN load byte from RAM
   fxcop c_FxOp41A1
NEWSYM FxOp42A1    ; LDB RN load byte from RAM
   fxcop c_FxOp42A1
NEWSYM FxOp43A1    ; LDB RN load byte from RAM
   fxcop c_FxOp43A1
NEWSYM FxOp44A1    ; LDB RN load byte from RAM
   fxcop c_FxOp44A1
NEWSYM FxOp45A1    ; LDB RN load byte from RAM
   fxcop c_FxOp45A1
NEWSYM FxOp46A1    ; LDB RN load byte from RAM
   fxcop c_FxOp46A1
NEWSYM FxOp47A1    ; LDB RN load byte from RAM
   fxcop c_FxOp47A1
NEWSYM FxOp48A1    ; LDB RN load byte from RAM
   fxcop c_FxOp48A1
NEWSYM FxOp49A1    ; LDB RN load byte from RAM
   fxcop c_FxOp49A1
NEWSYM FxOp4AA1    ; LDB RN load byte from RAM
   fxcop c_FxOp4AA1
NEWSYM FxOp4BA1    ; LDB RN load byte from RAM
   fxcop c_FxOp4BA1
NEWSYM FxOp4C1284b       ; PLOT 4bit
   fxcop c_FxOp4C1284b
NEWSYM FxOp4C1284bz      ; PLOT 4bit, zero check
   fxcop c_FxOp4C1284bz
NEWSYM FxOp4C1284bd      ; PLOT 4bit, dither
   fxcop c_FxOp4C1284bd
NEWSYM FxOp4C1284bzd     ; PLOT 4bit, zero check + dither
   fxcop c_FxOp4C1284bzd
NEWSYM FxOp4C1282b       ; PLOT 2bit
   fxcop c_FxOp4C1282b
NEWSYM FxOp4C1282bz      ; PLOT 2bit, zero check
   fxcop c_FxOp4C1282bz
NEWSYM FxOp4C1282bd      ; PLOT 2bit, dither
   fxcop c_FxOp4C1282bd
NEWSYM FxOp4C1282bzd     ; PLOT 2bit, zero check + dither
   fxcop c_FxOp4C1282bzd
NEWSYM FxOp4C1288b       ; PLOT 8bit
   fxcop c_FxOp4C1288b
NEWSYM FxOp4C1288bz      ; PLOT 8bit, zero check
   fxcop c_FxOp4C1288bz
NEWSYM FxOp4C1288bd      ; PLOT 8bit, dither
   fxcop c_FxOp4C1288bd
NEWSYM FxOp4C1288bzd     ; PLOT 8bit, zero check + dither
   fxcop c_FxOp4C1288bzd
NEWSYM FxOp4C1288bl       ; PLOT 8bit
   fxcop c_FxOp4C1288bl
NEWSYM FxOp4C1288bzl      ; PLOT 8bit, zero check
   fxcop c_FxOp4C1288bzl
NEWSYM FxOp4C1288bdl      ; PLOT 8bit, dither
   fxcop c_FxOp4C1288bdl
NEWSYM FxOp4C1288bzdl     ; PLOT 8bit, zero check + dither
   fxcop c_FxOp4C1288bzdl
NEWSYM FxOp4C      ; PLOT   plot pixel with R1,R2 as x,y and the color register as the color
   fxcop c_FxOp4C
NEWSYM FxOp4CA1    ; RPIX   read color of the pixel with R1,R2 as x,y
   fxcop c_FxOp4CA1
NEWSYM FxOp4D      ; SWAP   swap upper and lower byte of a register    ; V
   fxcop c_FxOp4D
NEWSYM FxOp4E      ; COLOR  copy source register to color register     ; V
   fxcop c_FxOp4E
NEWSYM FxOp4EA1    ; CMODE  set plot option register ; V
   fxcop c_FxOp4EA1
NEWSYM FxOp4F      ; NOT    perform exclusive exor with 1 on all bits  ; V
   fxcop c_FxOp4F
NEWSYM FxOp50      ; ADD RN add, register + register
   fxcop c_FxOp50
NEWSYM FxOp51      ; ADD RN add, register + register
   fxcop c_FxOp51
NEWSYM FxOp52      ; ADD RN add, register + register
   fxcop c_FxOp52
NEWSYM FxOp53      ; ADD RN add, register + register
   fxcop c_FxOp53
NEWSYM FxOp54      ; ADD RN add, register + register
   fxcop c_FxOp54
NEWSYM FxOp55      ; ADD RN add, register + register
   fxcop c_FxOp55
NEWSYM FxOp56      ; ADD RN add, register + register
   fxcop c_FxOp56
NEWSYM FxOp57      ; ADD RN add, register + register
   fxcop c_FxOp57
NEWSYM FxOp58      ; ADD RN add, register + register
   fxcop c_FxOp58
NEWSYM FxOp59      ; ADD RN add, register + register
   fxcop c_FxOp59
NEWSYM FxOp5A      ; ADD RN add, register + register
   fxcop c_FxOp5A
NEWSYM FxOp5B      ; ADD RN add, register + register
   fxcop c_FxOp5B
NEWSYM FxOp5C      ; ADD RN add, register + register
   fxcop c_FxOp5C
NEWSYM FxOp5D      ; ADD RN add, register + register
   fxcop c_FxOp5D
NEWSYM FxOp5E      ; ADD RN add, register + register
   fxcop c_FxOp5E
NEWSYM FxOp5F      ; ADD RN add, register + register
   fxcop c_FxOp5F
NEWSYM FxOp50A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp50A1
NEWSYM FxOp51A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp51A1
NEWSYM FxOp52A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp52A1
NEWSYM FxOp53A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp53A1
NEWSYM FxOp54A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp54A1
NEWSYM FxOp55A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp55A1
NEWSYM FxOp56A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp56A1
NEWSYM FxOp57A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp57A1
NEWSYM FxOp58A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp58A1
NEWSYM FxOp59A1    ; ADC RN add with carry, register + register
   fxcop c_FxOp59A1
NEWSYM FxOp5AA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5AA1
NEWSYM FxOp5BA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5BA1
NEWSYM FxOp5CA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5CA1
NEWSYM FxOp5DA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5DA1
NEWSYM FxOp5EA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5EA1
NEWSYM FxOp5FA1    ; ADC RN add with carry, register + register
   fxcop c_FxOp5FA1
NEWSYM FxOp50A2    ; ADI RN add, register + immediate
   fxcop c_FxOp50A2
NEWSYM FxOp51A2    ; ADI RN add, register + immediate
   fxcop c_FxOp51A2
NEWSYM FxOp52A2    ; ADI RN add, register + immediate
   fxcop c_FxOp52A2
NEWSYM FxOp53A2    ; ADI RN add, register + immediate
   fxcop c_FxOp53A2
NEWSYM FxOp54A2    ; ADI RN add, register + immediate
   fxcop c_FxOp54A2
NEWSYM FxOp55A2    ; ADI RN add, register + immediate
   fxcop c_FxOp55A2
NEWSYM FxOp56A2    ; ADI RN add, register + immediate
   fxcop c_FxOp56A2
NEWSYM FxOp57A2    ; ADI RN add, register + immediate
   fxcop c_FxOp57A2
NEWSYM FxOp58A2    ; ADI RN add, register + immediate
   fxcop c_FxOp58A2
NEWSYM FxOp59A2    ; ADI RN add, register + immediate
   fxcop c_FxOp59A2
NEWSYM FxOp5AA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5AA2
NEWSYM FxOp5BA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5BA2
NEWSYM FxOp5CA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5CA2
NEWSYM FxOp5DA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5DA2
NEWSYM FxOp5EA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5EA2
NEWSYM FxOp5FA2    ; ADI RN add, register + immediate
   fxcop c_FxOp5FA2
NEWSYM FxOp50A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp50A3
NEWSYM FxOp51A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp51A3
NEWSYM FxOp52A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp52A3
NEWSYM FxOp53A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp53A3
NEWSYM FxOp54A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp54A3
NEWSYM FxOp55A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp55A3
NEWSYM FxOp56A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp56A3
NEWSYM FxOp57A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp57A3
NEWSYM FxOp58A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp58A3
NEWSYM FxOp59A3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp59A3
NEWSYM FxOp5AA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5AA3
NEWSYM FxOp5BA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5BA3
NEWSYM FxOp5CA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5CA3
NEWSYM FxOp5DA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5DA3
NEWSYM FxOp5EA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5EA3
NEWSYM FxOp5FA3    ; ADCIRN add with carry, register + immediate
   fxcop c_FxOp5FA3
NEWSYM FxOp60      ; SUBRN  subtract, register - register
   fxcop c_FxOp60
NEWSYM FxOp61      ; SUBRN  subtract, register - register
   fxcop c_FxOp61
NEWSYM FxOp62      ; SUBRN  subtract, register - register
   fxcop c_FxOp62
NEWSYM FxOp63      ; SUBRN  subtract, register - register
   fxcop c_FxOp63
NEWSYM FxOp64      ; SUBRN  subtract, register - register
   fxcop c_FxOp64
NEWSYM FxOp65      ; SUBRN  subtract, register - register
   fxcop c_FxOp65
NEWSYM FxOp66      ; SUBRN  subtract, register - register
   fxcop c_FxOp66
NEWSYM FxOp67      ; SUBRN  subtract, register - register
   fxcop c_FxOp67
NEWSYM FxOp68      ; SUBRN  subtract, register - register
   fxcop c_FxOp68
NEWSYM FxOp69      ; SUBRN  subtract, register - register
   fxcop c_FxOp69
NEWSYM FxOp6A      ; SUBRN  subtract, register - register
   fxcop c_FxOp6A
NEWSYM FxOp6B      ; SUBRN  subtract, register - register
   fxcop c_FxOp6B
NEWSYM FxOp6C      ; SUBRN  subtract, register - register
   fxcop c_FxOp6C
NEWSYM FxOp6D      ; SUBRN  subtract, register - register
   fxcop c_FxOp6D
NEWSYM FxOp6E      ; SUBRN  subtract, register - register
   fxcop c_FxOp6E
NEWSYM FxOp6F      ; SUBRN  subtract, register - register
   fxcop c_FxOp6F
NEWSYM FxOp60A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp60A1
NEWSYM FxOp61A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp61A1
NEWSYM FxOp62A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp62A1
NEWSYM FxOp63A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp63A1
NEWSYM FxOp64A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp64A1
NEWSYM FxOp65A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp65A1
NEWSYM FxOp66A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp66A1
NEWSYM FxOp67A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp67A1
NEWSYM FxOp68A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp68A1
NEWSYM FxOp69A1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp69A1
NEWSYM FxOp6AA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6AA1
NEWSYM FxOp6BA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6BA1
NEWSYM FxOp6CA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6CA1
NEWSYM FxOp6DA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6DA1
NEWSYM FxOp6EA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6EA1
NEWSYM FxOp6FA1    ; SBCRN  subtract with carry, register - register
   fxcop c_FxOp6FA1
NEWSYM FxOp60A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp60A2
NEWSYM FxOp61A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp61A2
NEWSYM FxOp62A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp62A2
NEWSYM FxOp63A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp63A2
NEWSYM FxOp64A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp64A2
NEWSYM FxOp65A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp65A2
NEWSYM FxOp66A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp66A2
NEWSYM FxOp67A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp67A2
NEWSYM FxOp68A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp68A2
NEWSYM FxOp69A2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp69A2
NEWSYM FxOp6AA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6AA2
NEWSYM FxOp6BA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6BA2
NEWSYM FxOp6CA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6CA2
NEWSYM FxOp6DA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6DA2
NEWSYM FxOp6EA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6EA2
NEWSYM FxOp6FA2    ; SUBIRN subtract, register - immediate
   fxcop c_FxOp6FA2
NEWSYM FxOp60A3    ; CMPRN  compare, register, register
   fxcop c_FxOp60A3
NEWSYM FxOp61A3    ; CMPRN  compare, register, register
   fxcop c_FxOp61A3
NEWSYM FxOp62A3    ; CMPRN  compare, register, register
   fxcop c_FxOp62A3
NEWSYM FxOp63A3    ; CMPRN  compare, register, register
   fxcop c_FxOp63A3
NEWSYM FxOp64A3    ; CMPRN  compare, register, register
   fxcop c_FxOp64A3
NEWSYM FxOp65A3    ; CMPRN  compare, register, register
   fxcop c_FxOp65A3
NEWSYM FxOp66A3    ; CMPRN  compare, register, register
   fxcop c_FxOp66A3
NEWSYM FxOp67A3    ; CMPRN  compare, register, register
   fxcop c_FxOp67A3
NEWSYM FxOp68A3    ; CMPRN  compare, register, register
   fxcop c_FxOp68A3
NEWSYM FxOp69A3    ; CMPRN  compare, register, register
   fxcop c_FxOp69A3
NEWSYM FxOp6AA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6AA3
NEWSYM FxOp6BA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6BA3
NEWSYM FxOp6CA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6CA3
NEWSYM FxOp6DA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6DA3
NEWSYM FxOp6EA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6EA3
NEWSYM FxOp6FA3    ; CMPRN  compare, register, register
   fxcop c_FxOp6FA3
NEWSYM FxOp70      ; MERGE  R7 as upper byte, R8 as lower byte (used for texture-mapping) */
   fxcop c_FxOp70
NEWSYM FxOp71      ; AND RN register & register
   fxcop c_FxOp71
NEWSYM FxOp72      ; AND RN register & register
   fxcop c_FxOp72
NEWSYM FxOp73      ; AND RN register & register
   fxcop c_FxOp73
NEWSYM FxOp74      ; AND RN register & register
   fxcop c_FxOp74
NEWSYM FxOp75      ; AND RN register & register
   fxcop c_FxOp75
NEWSYM FxOp76      ; AND RN register & register
   fxcop c_FxOp76
NEWSYM FxOp77      ; AND RN register & register
   fxcop c_FxOp77
NEWSYM FxOp78      ; AND RN register & register
   fxcop c_FxOp78
NEWSYM FxOp79      ; AND RN register & register
   fxcop c_FxOp79
NEWSYM FxOp7A      ; AND RN register & register
   fxcop c_FxOp7A
NEWSYM FxOp7B      ; AND RN register & register
   fxcop c_FxOp7B
NEWSYM FxOp7C      ; AND RN register & register
   fxcop c_FxOp7C
NEWSYM FxOp7D      ; AND RN register & register
   fxcop c_FxOp7D
NEWSYM FxOp7E      ; AND RN register & register
   fxcop c_FxOp7E
NEWSYM FxOp7F      ; AND RN register & register
   fxcop c_FxOp7F
NEWSYM FxOp71A1    ; BIC RN register & ~register
   fxcop c_FxOp71A1
NEWSYM FxOp72A1    ; BIC RN register & ~register
   fxcop c_FxOp72A1
NEWSYM FxOp73A1    ; BIC RN register & ~register
   fxcop c_FxOp73A1
NEWSYM FxOp74A1    ; BIC RN register & ~register
   fxcop c_FxOp74A1
NEWSYM FxOp75A1    ; BIC RN register & ~register
   fxcop c_FxOp75A1
NEWSYM FxOp76A1    ; BIC RN register & ~register
   fxcop c_FxOp76A1
NEWSYM FxOp77A1    ; BIC RN register & ~register
   fxcop c_FxOp77A1
NEWSYM FxOp78A1    ; BIC RN register & ~register
   fxcop c_FxOp78A1
NEWSYM FxOp79A1    ; BIC RN register & ~register
   fxcop c_FxOp79A1
NEWSYM FxOp7AA1    ; BIC RN register & ~register
   fxcop c_FxOp7AA1
NEWSYM FxOp7BA1    ; BIC RN register & ~register
   fxcop c_FxOp7BA1
NEWSYM FxOp7CA1    ; BIC RN register & ~register
   fxcop c_FxOp7CA1
NEWSYM FxOp7DA1    ; BIC RN register & ~register
   fxcop c_FxOp7DA1
NEWSYM FxOp7EA1    ; BIC RN register & ~register
   fxcop c_FxOp7EA1
NEWSYM FxOp7FA1    ; BIC RN register & ~register
   fxcop c_FxOp7FA1
NEWSYM FxOp71A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp71A2
NEWSYM FxOp72A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp72A2
NEWSYM FxOp73A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp73A2
NEWSYM FxOp74A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp74A2
NEWSYM FxOp75A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp75A2
NEWSYM FxOp76A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp76A2
NEWSYM FxOp77A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp77A2
NEWSYM FxOp78A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp78A2
NEWSYM FxOp79A2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp79A2
NEWSYM FxOp7AA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7AA2
NEWSYM FxOp7BA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7BA2
NEWSYM FxOp7CA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7CA2
NEWSYM FxOp7DA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7DA2
NEWSYM FxOp7EA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7EA2
NEWSYM FxOp7FA2    ; ANDIRN and #n - register & immediate
   fxcop c_FxOp7FA2
NEWSYM FxOp71A3    ; BICIRN register & ~immediate
   fxcop c_FxOp71A3
NEWSYM FxOp72A3    ; BICIRN register & ~immediate
   fxcop c_FxOp72A3
NEWSYM FxOp73A3    ; BICIRN register & ~immediate
   fxcop c_FxOp73A3
NEWSYM FxOp74A3    ; BICIRN register & ~immediate
   fxcop c_FxOp74A3
NEWSYM FxOp75A3    ; BICIRN register & ~immediate
   fxcop c_FxOp75A3
NEWSYM FxOp76A3    ; BICIRN register & ~immediate
   fxcop c_FxOp76A3
NEWSYM FxOp77A3    ; BICIRN register & ~immediate
   fxcop c_FxOp77A3
NEWSYM FxOp78A3    ; BICIRN register & ~immediate
   fxcop c_FxOp78A3
NEWSYM FxOp79A3    ; BICIRN register & ~immediate
   fxcop c_FxOp79A3
NEWSYM FxOp7AA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7AA3
NEWSYM FxOp7BA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7BA3
NEWSYM FxOp7CA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7CA3
NEWSYM FxOp7DA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7DA3
NEWSYM FxOp7EA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7EA3
NEWSYM FxOp7FA3    ; BICIRN register & ~immediate
   fxcop c_FxOp7FA3
NEWSYM FxOp80      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp80
NEWSYM FxOp81      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp81
NEWSYM FxOp82      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp82
NEWSYM FxOp83      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp83
NEWSYM FxOp84      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp84
NEWSYM FxOp85      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp85
NEWSYM FxOp86      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp86
NEWSYM FxOp87      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp87
NEWSYM FxOp88      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp88
NEWSYM FxOp89      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp89
NEWSYM FxOp8A      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8A
NEWSYM FxOp8B      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8B
NEWSYM FxOp8C      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8C
NEWSYM FxOp8D      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8D
NEWSYM FxOp8E      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8E
NEWSYM FxOp8F      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   fxcop c_FxOp8F
NEWSYM FxOp80A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp80A1
NEWSYM FxOp81A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp81A1
NEWSYM FxOp82A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp82A1
NEWSYM FxOp83A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp83A1
NEWSYM FxOp84A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp84A1
NEWSYM FxOp85A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp85A1
NEWSYM FxOp86A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp86A1
NEWSYM FxOp87A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp87A1
NEWSYM FxOp88A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp88A1
NEWSYM FxOp89A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp89A1
NEWSYM FxOp8AA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8AA1
NEWSYM FxOp8BA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8BA1
NEWSYM FxOp8CA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8CA1
NEWSYM FxOp8DA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8DA1
NEWSYM FxOp8EA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8EA1
NEWSYM FxOp8FA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxcop c_FxOp8FA1
NEWSYM FxOp80A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp80A2
NEWSYM FxOp81A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp81A2
NEWSYM FxOp82A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp82A2
NEWSYM FxOp83A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp83A2
NEWSYM FxOp84A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp84A2
NEWSYM FxOp85A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp85A2
NEWSYM FxOp86A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp86A2
NEWSYM FxOp87A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp87A2
NEWSYM FxOp88A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp88A2
NEWSYM FxOp89A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp89A2
NEWSYM FxOp8AA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8AA2
NEWSYM FxOp8BA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8BA2
NEWSYM FxOp8CA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8CA2
NEWSYM FxOp8DA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8DA2
NEWSYM FxOp8EA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8EA2
NEWSYM FxOp8FA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxcop c_FxOp8FA2
NEWSYM FxOp80A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp80A3
NEWSYM FxOp81A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp81A3
NEWSYM FxOp82A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp82A3
NEWSYM FxOp83A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp83A3
NEWSYM FxOp84A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp84A3
NEWSYM FxOp85A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp85A3
NEWSYM FxOp86A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp86A3
NEWSYM FxOp87A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp87A3
NEWSYM FxOp88A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp88A3
NEWSYM FxOp89A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp89A3
NEWSYM FxOp8AA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8AA3
NEWSYM FxOp8BA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8BA3
NEWSYM FxOp8CA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8CA3
NEWSYM FxOp8DA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8DA3
NEWSYM FxOp8EA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8EA3
NEWSYM FxOp8FA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxcop c_FxOp8FA3
NEWSYM FxOp90      ; SBK    store word to last accessed RAM address    ; V
   fxcop c_FxOp90
NEWSYM FxOp91      ; LINK#n R11 = R15 + immediate
   fxcop c_FxOp91
NEWSYM FxOp92      ; LINK#n R11 = R15 + immediate
   fxcop c_FxOp92
NEWSYM FxOp93      ; LINK#n R11 = R15 + immediate
   fxcop c_FxOp93
NEWSYM FxOp94      ; LINK#n R11 = R15 + immediate
   fxcop c_FxOp94
NEWSYM FxOp95      ; SEX    sign extend 8 bit to 16 bit        ; V
   fxcop c_FxOp95
NEWSYM FxOp96      ; ASR    aritmethic shift right by one      ; V
   fxcop c_FxOp96
NEWSYM FxOp96A1    ; DIV2   aritmethic shift right by one      ; V
   fxcop c_FxOp96A1
NEWSYM FxOp97      ; ROR    rotate right by one        ; V
   fxcop c_FxOp97
NEWSYM FxOp98      ; JMPRN  jump to address of register
   fxcop c_FxOp98
NEWSYM FxOp99      ; JMPRN  jump to address of register
   fxcop c_FxOp99
NEWSYM FxOp9A      ; JMPRN  jump to address of register
   fxcop c_FxOp9A
NEWSYM FxOp9B      ; JMPRN  jump to address of register
   fxcop c_FxOp9B
NEWSYM FxOp9C      ; JMPRN  jump to address of register
   fxcop c_FxOp9C
NEWSYM FxOp9D      ; JMPRN  jump to address of register
   fxcop c_FxOp9D
NEWSYM FxOp98A1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp98A1
NEWSYM FxOp99A1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp99A1
NEWSYM FxOp9AA1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp9AA1
NEWSYM FxOp9BA1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp9BA1
NEWSYM FxOp9CA1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp9CA1
NEWSYM FxOp9DA1    ; LJMPRN set program bank to source register and jump to address of register
   fxcop c_FxOp9DA1
NEWSYM FxOp9E      ; LOB    set upper byte to zero (keep low byte) ; V
   fxcop c_FxOp9E
NEWSYM FxOp9F      ; FMULT  16 bit to 32 bit signed multiplication, upper 16 bits only
   fxcop c_FxOp9F
NEWSYM FxOp9FA1    ; LMULT  16 bit to 32 bit signed multiplication     ; V
   fxcop c_FxOp9FA1
NEWSYM FxOpA0      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA0
NEWSYM FxOpA1      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA1
NEWSYM FxOpA2      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA2
NEWSYM FxOpA3      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA3
NEWSYM FxOpA4      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA4
NEWSYM FxOpA5      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA5
NEWSYM FxOpA6      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA6
NEWSYM FxOpA7      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA7
NEWSYM FxOpA8      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA8
NEWSYM FxOpA9      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpA9
NEWSYM FxOpAA      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAA
NEWSYM FxOpAB      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAB
NEWSYM FxOpAC      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAC
NEWSYM FxOpAD      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAD
NEWSYM FxOpAE      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAE
NEWSYM FxOpAF      ; IBTRN,#PP immediate byte transfer
   fxcop c_FxOpAF
NEWSYM FxOpA0A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA0A1
NEWSYM FxOpA1A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA1A1
NEWSYM FxOpA2A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA2A1
NEWSYM FxOpA3A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA3A1
NEWSYM FxOpA4A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA4A1
NEWSYM FxOpA5A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA5A1
NEWSYM FxOpA6A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA6A1
NEWSYM FxOpA7A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA7A1
NEWSYM FxOpA8A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA8A1
NEWSYM FxOpA9A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpA9A1
NEWSYM FxOpAAA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpAAA1
NEWSYM FxOpABA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpABA1
NEWSYM FxOpACA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpACA1
NEWSYM FxOpADA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpADA1
NEWSYM FxOpAEA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpAEA1
NEWSYM FxOpAFA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxcop c_FxOpAFA1
NEWSYM FxOpA0A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA0A2
NEWSYM FxOpA1A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA1A2
NEWSYM FxOpA2A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA2A2
NEWSYM FxOpA3A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA3A2
NEWSYM FxOpA4A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA4A2
NEWSYM FxOpA5A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA5A2
NEWSYM FxOpA6A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA6A2
NEWSYM FxOpA7A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA7A2
NEWSYM FxOpA8A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA8A2
NEWSYM FxOpA9A2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpA9A2
NEWSYM FxOpAAA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpAAA2
NEWSYM FxOpABA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpABA2
NEWSYM FxOpACA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpACA2
NEWSYM FxOpADA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpADA2
NEWSYM FxOpAEA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpAEA2
NEWSYM FxOpAFA2    ; SMS (yy),rn  store word in RAM (short address)
   fxcop c_FxOpAFA2
NEWSYM FxOpB0      ; FROM rn   set source register
   fxcop c_FxOpB0
NEWSYM FxOpB1      ; FROM rn   set source register
   fxcop c_FxOpB1
NEWSYM FxOpB2      ; FROM rn   set source register
   fxcop c_FxOpB2
NEWSYM FxOpB3      ; FROM rn   set source register
   fxcop c_FxOpB3
NEWSYM FxOpB4      ; FROM rn   set source register
   fxcop c_FxOpB4
NEWSYM FxOpB5      ; FROM rn   set source register
   fxcop c_FxOpB5
NEWSYM FxOpB6      ; FROM rn   set source register
   fxcop c_FxOpB6
NEWSYM FxOpB7      ; FROM rn   set source register
   fxcop c_FxOpB7
NEWSYM FxOpB8      ; FROM rn   set source register
   fxcop c_FxOpB8
NEWSYM FxOpB9      ; FROM rn   set source register
   fxcop c_FxOpB9
NEWSYM FxOpBA      ; FROM rn   set source register
   fxcop c_FxOpBA
NEWSYM FxOpBB      ; FROM rn   set source register
   fxcop c_FxOpBB
NEWSYM FxOpBC      ; FROM rn   set source register
   fxcop c_FxOpBC
NEWSYM FxOpBD      ; FROM rn   set source register
   fxcop c_FxOpBD
NEWSYM FxOpBE      ; FROM rn   set source register
   fxcop c_FxOpBE
NEWSYM FxOpBF      ; FROM rn   set source register
   fxcop c_FxOpBF
NEWSYM FxOpC0      ; HIB       move high-byte to low-byte      ; V
   fxcop c_FxOpC0
NEWSYM FxOpC1      ; OR rn     or rn
   fxcop c_FxOpC1
NEWSYM FxOpC2      ; OR rn     or rn
   fxcop c_FxOpC2
NEWSYM FxOpC3      ; OR rn     or rn
   fxcop c_FxOpC3
NEWSYM FxOpC4      ; OR rn     or rn
   fxcop c_FxOpC4
NEWSYM FxOpC5      ; OR rn     or rn
   fxcop c_FxOpC5
NEWSYM FxOpC6      ; OR rn     or rn
   fxcop c_FxOpC6
NEWSYM FxOpC7      ; OR rn     or rn
   fxcop c_FxOpC7
NEWSYM FxOpC8      ; OR rn     or rn
   fxcop c_FxOpC8
NEWSYM FxOpC9      ; OR rn     or rn
   fxcop c_FxOpC9
NEWSYM FxOpCA      ; OR rn     or rn
   fxcop c_FxOpCA
NEWSYM FxOpCB      ; OR rn     or rn
   fxcop c_FxOpCB
NEWSYM FxOpCC      ; OR rn     or rn
   fxcop c_FxOpCC
NEWSYM FxOpCD      ; OR rn     or rn
   fxcop c_FxOpCD
NEWSYM FxOpCE      ; OR rn     or rn
   fxcop c_FxOpCE
NEWSYM FxOpCF      ; OR rn     or rn
   fxcop c_FxOpCF
NEWSYM FxOpC1A1    ; XOR rn    xor rn
   fxcop c_FxOpC1A1
NEWSYM FxOpC2A1    ; XOR rn    xor rn
   fxcop c_FxOpC2A1
NEWSYM FxOpC3A1    ; XOR rn    xor rn
   fxcop c_FxOpC3A1
NEWSYM FxOpC4A1    ; XOR rn    xor rn
   fxcop c_FxOpC4A1
NEWSYM FxOpC5A1    ; XOR rn    xor rn
   fxcop c_FxOpC5A1
NEWSYM FxOpC6A1    ; XOR rn    xor rn
   fxcop c_FxOpC6A1
NEWSYM FxOpC7A1    ; XOR rn    xor rn
   fxcop c_FxOpC7A1
NEWSYM FxOpC8A1    ; XOR rn    xor rn
   fxcop c_FxOpC8A1
NEWSYM FxOpC9A1    ; XOR rn    xor rn
   fxcop c_FxOpC9A1
NEWSYM FxOpCAA1    ; XOR rn    xor rn
   fxcop c_FxOpCAA1
NEWSYM FxOpCBA1    ; XOR rn    xor rn
   fxcop c_FxOpCBA1
NEWSYM FxOpCCA1    ; XOR rn    xor rn
   fxcop c_FxOpCCA1
NEWSYM FxOpCDA1    ; XOR rn    xor rn
   fxcop c_FxOpCDA1
NEWSYM FxOpCEA1    ; XOR rn    xor rn
   fxcop c_FxOpCEA1
NEWSYM FxOpCFA1    ; XOR rn    xor rn
   fxcop c_FxOpCFA1
NEWSYM FxOpC1A2    ; OR #n     OR #n
   fxcop c_FxOpC1A2
NEWSYM FxOpC2A2    ; OR #n     OR #n
   fxcop c_FxOpC2A2
NEWSYM FxOpC3A2    ; OR #n     OR #n
   fxcop c_FxOpC3A2
NEWSYM FxOpC4A2    ; OR #n     OR #n
   fxcop c_FxOpC4A2
NEWSYM FxOpC5A2    ; OR #n     OR #n
   fxcop c_FxOpC5A2
NEWSYM FxOpC6A2    ; OR #n     OR #n
   fxcop c_FxOpC6A2
NEWSYM FxOpC7A2    ; OR #n     OR #n
   fxcop c_FxOpC7A2
NEWSYM FxOpC8A2    ; OR #n     OR #n
   fxcop c_FxOpC8A2
NEWSYM FxOpC9A2    ; OR #n     OR #n
   fxcop c_FxOpC9A2
NEWSYM FxOpCAA2    ; OR #n     OR #n
   fxcop c_FxOpCAA2
NEWSYM FxOpCBA2    ; OR #n     OR #n
   fxcop c_FxOpCBA2
NEWSYM FxOpCCA2    ; OR #n     OR #n
   fxcop c_FxOpCCA2
NEWSYM FxOpCDA2    ; OR #n     OR #n
   fxcop c_FxOpCDA2
NEWSYM FxOpCEA2    ; OR #n     OR #n
   fxcop c_FxOpCEA2
NEWSYM FxOpCFA2    ; OR #n     OR #n
   fxcop c_FxOpCFA2
NEWSYM FxOpC1A3    ; XOR #n    xor #n
   fxcop c_FxOpC1A3
NEWSYM FxOpC2A3    ; XOR #n    xor #n
   fxcop c_FxOpC2A3
NEWSYM FxOpC3A3    ; XOR #n    xor #n
   fxcop c_FxOpC3A3
NEWSYM FxOpC4A3    ; XOR #n    xor #n
   fxcop c_FxOpC4A3
NEWSYM FxOpC5A3    ; XOR #n    xor #n
   fxcop c_FxOpC5A3
NEWSYM FxOpC6A3    ; XOR #n    xor #n
   fxcop c_FxOpC6A3
NEWSYM FxOpC7A3    ; XOR #n    xor #n
   fxcop c_FxOpC7A3
NEWSYM FxOpC8A3    ; XOR #n    xor #n
   fxcop c_FxOpC8A3
NEWSYM FxOpC9A3    ; XOR #n    xor #n
   fxcop c_FxOpC9A3
NEWSYM FxOpCAA3    ; XOR #n    xor #n
   fxcop c_FxOpCAA3
NEWSYM FxOpCBA3    ; XOR #n    xor #n
   fxcop c_FxOpCBA3
NEWSYM FxOpCCA3    ; XOR #n    xor #n
   fxcop c_FxOpCCA3
NEWSYM FxOpCDA3    ; XOR #n    xor #n
   fxcop c_FxOpCDA3
NEWSYM FxOpCEA3    ; XOR #n    xor #n
   fxcop c_FxOpCEA3
NEWSYM FxOpCFA3    ; XOR #n    xor #n
   fxcop c_FxOpCFA3
NEWSYM FxOpD0      ; INC rn    increase by one
   fxcop c_FxOpD0
NEWSYM FxOpD1      ; INC rn    increase by one
   fxcop c_FxOpD1
NEWSYM FxOpD2      ; INC rn    increase by one
   fxcop c_FxOpD2
NEWSYM FxOpD3      ; INC rn    increase by one
   fxcop c_FxOpD3
NEWSYM FxOpD4      ; INC rn    increase by one
   fxcop c_FxOpD4
NEWSYM FxOpD5      ; INC rn    increase by one
   fxcop c_FxOpD5
NEWSYM FxOpD6      ; INC rn    increase by one
   fxcop c_FxOpD6
NEWSYM FxOpD7      ; INC rn    increase by one
   fxcop c_FxOpD7
NEWSYM FxOpD8      ; INC rn    increase by one
   fxcop c_FxOpD8
NEWSYM FxOpD9      ; INC rn    increase by one
   fxcop c_FxOpD9
NEWSYM FxOpDA      ; INC rn    increase by one
   fxcop c_FxOpDA
NEWSYM FxOpDB      ; INC rn    increase by one
   fxcop c_FxOpDB
NEWSYM FxOpDC      ; INC rn    increase by one
   fxcop c_FxOpDC
NEWSYM FxOpDD      ; INC rn    increase by one
   fxcop c_FxOpDD
NEWSYM FxOpDE      ; INC rn    increase by one
   fxcop c_FxOpDE
NEWSYM FxOpDF      ; GETC      transfer ROM buffer to color register
   fxcop c_FxOpDF
NEWSYM FxOpDFA2    ; RAMB      set current RAM bank    ; Verified
   fxcop c_FxOpDFA2
NEWSYM FxOpDFA3    ; ROMB      set current ROM bank    ; Verified
   fxcop c_FxOpDFA3
NEWSYM FxOpE0      ; DEC rn    decrement by one
   fxcop c_FxOpE0
NEWSYM FxOpE1      ; DEC rn    decrement by one
   fxcop c_FxOpE1
NEWSYM FxOpE2      ; DEC rn    decrement by one
   fxcop c_FxOpE2
NEWSYM FxOpE3      ; DEC rn    decrement by one
   fxcop c_FxOpE3
NEWSYM FxOpE4      ; DEC rn    decrement by one
   fxcop c_FxOpE4
NEWSYM FxOpE5      ; DEC rn    decrement by one
   fxcop c_FxOpE5
NEWSYM FxOpE6      ; DEC rn    decrement by one
   fxcop c_FxOpE6
NEWSYM FxOpE7      ; DEC rn    decrement by one
   fxcop c_FxOpE7
NEWSYM FxOpE8      ; DEC rn    decrement by one
   fxcop c_FxOpE8
NEWSYM FxOpE9      ; DEC rn    decrement by one
   fxcop c_FxOpE9
NEWSYM FxOpEA      ; DEC rn    decrement by one
   fxcop c_FxOpEA
NEWSYM FxOpEB      ; DEC rn    decrement by one
   fxcop c_FxOpEB
NEWSYM FxOpEC      ; DEC rn    decrement by one
   fxcop c_FxOpEC
NEWSYM FxOpED      ; DEC rn    decrement by one
   fxcop c_FxOpED
NEWSYM FxOpEE      ; DEC rn    decrement by one
   fxcop c_FxOpEE
NEWSYM FxOpEF      ; getb      get byte from ROM at address R14        ; V
   fxcop c_FxOpEF
NEWSYM FxOpEFA1    ; getbh     get high-byte from ROM at address R14   ; V
   fxcop c_FxOpEFA1
NEWSYM FxOpEFA2    ; getbl     get low-byte from ROM at address R14    ; V
   fxcop c_FxOpEFA2
NEWSYM FxOpEFA3    ; getbs     get sign extended byte from ROM at address R14  ; V
   fxcop c_FxOpEFA3
NEWSYM FxOpF0      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF0
NEWSYM FxOpF1      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF1
NEWSYM FxOpF2      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF2
NEWSYM FxOpF3      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF3
NEWSYM FxOpF4      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF4
NEWSYM FxOpF5      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF5
NEWSYM FxOpF6      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF6
NEWSYM FxOpF7      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF7
NEWSYM FxOpF8      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF8
NEWSYM FxOpF9      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpF9
NEWSYM FxOpFA      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFA
NEWSYM FxOpFB      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFB
NEWSYM FxOpFC      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFC
NEWSYM FxOpFD      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFD
NEWSYM FxOpFE      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFE
NEWSYM FxOpFF      ; IWT RN,#xx   immediate word transfer to register
   fxcop c_FxOpFF
NEWSYM FxOpF0A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF0A1
NEWSYM FxOpF1A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF1A1
NEWSYM FxOpF2A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF2A1
NEWSYM FxOpF3A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF3A1
NEWSYM FxOpF4A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF4A1
NEWSYM FxOpF5A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF5A1
NEWSYM FxOpF6A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF6A1
NEWSYM FxOpF7A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF7A1
NEWSYM FxOpF8A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF8A1
NEWSYM FxOpF9A1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpF9A1
NEWSYM FxOpFAA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFAA1
NEWSYM FxOpFBA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFBA1
NEWSYM FxOpFCA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFCA1
NEWSYM FxOpFDA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFDA1
NEWSYM FxOpFEA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFEA1
NEWSYM FxOpFFA1    ; LM RN,(XX)   load word from RAM
   fxcop c_FxOpFFA1
NEWSYM FxOpF0A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF0A2
NEWSYM FxOpF1A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF1A2
NEWSYM FxOpF2A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF2A2
NEWSYM FxOpF3A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF3A2
NEWSYM FxOpF4A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF4A2
NEWSYM FxOpF5A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF5A2
NEWSYM FxOpF6A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF6A2
NEWSYM FxOpF7A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF7A2
NEWSYM FxOpF8A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF8A2
NEWSYM FxOpF9A2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpF9A2
NEWSYM FxOpFAA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFAA2
NEWSYM FxOpFBA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFBA2
NEWSYM FxOpFCA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFCA2
NEWSYM FxOpFDA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFDA2
NEWSYM FxOpFEA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFEA2
NEWSYM FxOpFFA2    ; SM (XX),RN   store word in RAM
   fxcop c_FxOpFFA2
SECTION .bss

NEWSYM NumberOfOpcodes, resd 1    ; Number of opcodes to execute
NEWSYM NumberOfOpcodesBU, resd 1  ; Number of opcodes to execute backup value
NEWSYM sfxwarningb, resb 1

SECTION .text

NEWSYM MainLoop
   mov eax,[SfxPBR]
   and eax,0FFh
;   mov byte[fxtrace+eax],1
   mov ebp,[SfxCPB]
   add ebp,[SfxR15]
   xor ecx,ecx
   mov cl,[SfxPIPE]
   mov ch,[SfxSFR+1]
   and ch,03h
   ; pack esi/edi
   PackEsiEdi
   jmp [FxTabled+ecx*4]
   jmp .LoopAgain
ALIGN16
.LoopAgain
   call [FxTable+ecx*4]
   dec dword[NumberOfOpcodes]
   jnz .LoopAgain
.EndLoop
NEWSYM FXEndLoop
   sub ebp,[SfxCPB]
   mov [SfxR15],ebp
   mov [SfxPIPE],cl
   and byte[SfxSFR+1],0FFh-03h
   or [SfxSFR+1],ch
   UnPackEsiEdi
   ret

SECTION .data
NEWSYM fxtrace, db 0; times 65536 db 0



