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
EXTSYM FXEndLoop,FlushCache,FxOp02,FxTable,FxTableb,FxTablec,FxTabled
EXTSYM NumberOfOpcodes,SfxB,SfxCBR,SfxCFGR,SfxCOLR,SfxCPB,SfxCROM
EXTSYM SfxCacheActive,SfxCarry,SfxLastRamAdr,SfxMemTable,SfxOverflow
EXTSYM SfxPBR,SfxPIPE,SfxPOR,SfxR0,SfxR1,SfxR11,SfxR12,SfxR13,SfxR14
EXTSYM SfxR15,SfxR2,SfxR4,SfxR6,SfxR7,SfxR8,SfxRAMBR,SfxRAMMem,SfxROMBR
EXTSYM SfxRomBuffer,SfxSCBR,SfxSCMR,SfxSFR,SfxSignZero,SfxnRamBanks,flagnz
EXTSYM sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc,sfxramdata
EXTSYM withr15sk,sfxclineloc,SCBRrel,ChangeOps
EXTSYM fxbit01pcal,fxbit23pcal,fxbit45pcal,fxbit67pcal
EXTSYM fxbit01,fxbit23,fxbit45,fxbit67,fxxand,PLOTJmpa,PLOTJmpb

; Seam to the handlers ported to C (chips/c_fxops.c, chips/fx_ops.h). The d
; table is threaded rather than called: a handler tail-jumps to the next one
; through FXReturn instead of returning, so the thunk runs the C body and then
; performs that tail-chain itself.
EXTSYM FxSeamPC,FxSeamSrc,FxSeamDst,FxSeamCX
EXTSYM c_FxOp01,c_FxOp02,c_FxOp03,c_FxOp04,c_FxOp05,c_FxOp06
EXTSYM c_FxOp07,c_FxOp08,c_FxOp09,c_FxOp0A,c_FxOp0B,c_FxOp0C
EXTSYM c_FxOp0D,c_FxOp0E,c_FxOp0F,c_FxOp10,c_FxOp11,c_FxOp12
EXTSYM c_FxOp13,c_FxOp14,c_FxOp15,c_FxOp16,c_FxOp17,c_FxOp18
EXTSYM c_FxOp19,c_FxOp1A,c_FxOp1B,c_FxOp1C,c_FxOp1D,c_FxOp1E
EXTSYM c_FxOp1F,c_FxOp20,c_FxOp21,c_FxOp22,c_FxOp23,c_FxOp24
EXTSYM c_FxOp25,c_FxOp26,c_FxOp27,c_FxOp28,c_FxOp29,c_FxOp2A
EXTSYM c_FxOp2B,c_FxOp2C,c_FxOp2D,c_FxOp2E,c_FxOp2F,c_FxOp30
EXTSYM c_FxOp30A1,c_FxOp31,c_FxOp31A1,c_FxOp32,c_FxOp32A1,c_FxOp33
EXTSYM c_FxOp33A1,c_FxOp34,c_FxOp34A1,c_FxOp35,c_FxOp35A1,c_FxOp36
EXTSYM c_FxOp36A1,c_FxOp37,c_FxOp37A1,c_FxOp38,c_FxOp38A1,c_FxOp39
EXTSYM c_FxOp39A1,c_FxOp3A,c_FxOp3AA1,c_FxOp3B,c_FxOp3BA1,c_FxOp3C
EXTSYM c_FxOp3D,c_FxOp3E,c_FxOp3F,c_FxOp40,c_FxOp40A1,c_FxOp41
EXTSYM c_FxOp41A1,c_FxOp42,c_FxOp42A1,c_FxOp43,c_FxOp43A1,c_FxOp44
EXTSYM c_FxOp44A1,c_FxOp45,c_FxOp45A1,c_FxOp46,c_FxOp46A1,c_FxOp47
EXTSYM c_FxOp47A1,c_FxOp48,c_FxOp48A1,c_FxOp49,c_FxOp49A1,c_FxOp4A
EXTSYM c_FxOp4AA1,c_FxOp4B,c_FxOp4BA1,c_FxOp4C1282b,c_FxOp4C1282bd,c_FxOp4C1282bz
EXTSYM c_FxOp4C1282bzd,c_FxOp4C1284b,c_FxOp4C1284bd,c_FxOp4C1284bz,c_FxOp4C1284bzd,c_FxOp4C1288b
EXTSYM c_FxOp4C1288bl,c_FxOp4C1288bz,c_FxOp4C1288bzl,c_FxOp4CA1,c_FxOp4D,c_FxOp4E
EXTSYM c_FxOp4EA1,c_FxOp4F,c_FxOp50,c_FxOp50A1,c_FxOp50A2,c_FxOp50A3
EXTSYM c_FxOp51,c_FxOp51A1,c_FxOp51A2,c_FxOp51A3,c_FxOp52,c_FxOp52A1
EXTSYM c_FxOp52A2,c_FxOp52A3,c_FxOp53,c_FxOp53A1,c_FxOp53A2,c_FxOp53A3
EXTSYM c_FxOp54,c_FxOp54A1,c_FxOp54A2,c_FxOp54A3,c_FxOp55,c_FxOp55A1
EXTSYM c_FxOp55A2,c_FxOp55A3,c_FxOp56,c_FxOp56A1,c_FxOp56A2,c_FxOp56A3
EXTSYM c_FxOp57,c_FxOp57A1,c_FxOp57A2,c_FxOp57A3,c_FxOp58,c_FxOp58A1
EXTSYM c_FxOp58A2,c_FxOp58A3,c_FxOp59,c_FxOp59A1,c_FxOp59A2,c_FxOp59A3
EXTSYM c_FxOp5A,c_FxOp5AA1,c_FxOp5AA2,c_FxOp5AA3,c_FxOp5B,c_FxOp5BA1
EXTSYM c_FxOp5BA2,c_FxOp5BA3,c_FxOp5C,c_FxOp5CA1,c_FxOp5CA2,c_FxOp5CA3
EXTSYM c_FxOp5D,c_FxOp5DA1,c_FxOp5DA2,c_FxOp5DA3,c_FxOp5E,c_FxOp5EA1
EXTSYM c_FxOp5EA2,c_FxOp5EA3,c_FxOp5F,c_FxOp5FA1,c_FxOp5FA2,c_FxOp5FA3
EXTSYM c_FxOp60,c_FxOp60A1,c_FxOp60A2,c_FxOp60A3,c_FxOp61,c_FxOp61A1
EXTSYM c_FxOp61A2,c_FxOp61A3,c_FxOp62,c_FxOp62A1,c_FxOp62A2,c_FxOp62A3
EXTSYM c_FxOp63,c_FxOp63A1,c_FxOp63A2,c_FxOp63A3,c_FxOp64,c_FxOp64A1
EXTSYM c_FxOp64A2,c_FxOp64A3,c_FxOp65,c_FxOp65A1,c_FxOp65A2,c_FxOp65A3
EXTSYM c_FxOp66,c_FxOp66A1,c_FxOp66A2,c_FxOp66A3,c_FxOp67,c_FxOp67A1
EXTSYM c_FxOp67A2,c_FxOp67A3,c_FxOp68,c_FxOp68A1,c_FxOp68A2,c_FxOp68A3
EXTSYM c_FxOp69,c_FxOp69A1,c_FxOp69A2,c_FxOp69A3,c_FxOp6A,c_FxOp6AA1
EXTSYM c_FxOp6AA2,c_FxOp6AA3,c_FxOp6B,c_FxOp6BA1,c_FxOp6BA2,c_FxOp6BA3
EXTSYM c_FxOp6C,c_FxOp6CA1,c_FxOp6CA2,c_FxOp6CA3,c_FxOp6D,c_FxOp6DA1
EXTSYM c_FxOp6DA2,c_FxOp6DA3,c_FxOp6E,c_FxOp6EA1,c_FxOp6EA2,c_FxOp6EA3
EXTSYM c_FxOp6F,c_FxOp6FA1,c_FxOp6FA2,c_FxOp6FA3,c_FxOp70,c_FxOp71
EXTSYM c_FxOp71A1,c_FxOp71A2,c_FxOp71A3,c_FxOp72,c_FxOp72A1,c_FxOp72A2
EXTSYM c_FxOp72A3,c_FxOp73,c_FxOp73A1,c_FxOp73A2,c_FxOp73A3,c_FxOp74
EXTSYM c_FxOp74A1,c_FxOp74A2,c_FxOp74A3,c_FxOp75,c_FxOp75A1,c_FxOp75A2
EXTSYM c_FxOp75A3,c_FxOp76,c_FxOp76A1,c_FxOp76A2,c_FxOp76A3,c_FxOp77
EXTSYM c_FxOp77A1,c_FxOp77A2,c_FxOp77A3,c_FxOp78,c_FxOp78A1,c_FxOp78A2
EXTSYM c_FxOp78A3,c_FxOp79,c_FxOp79A1,c_FxOp79A2,c_FxOp79A3,c_FxOp7A
EXTSYM c_FxOp7AA1,c_FxOp7AA2,c_FxOp7AA3,c_FxOp7B,c_FxOp7BA1,c_FxOp7BA2
EXTSYM c_FxOp7BA3,c_FxOp7C,c_FxOp7CA1,c_FxOp7CA2,c_FxOp7CA3,c_FxOp7D
EXTSYM c_FxOp7DA1,c_FxOp7DA2,c_FxOp7DA3,c_FxOp7E,c_FxOp7EA1,c_FxOp7EA2
EXTSYM c_FxOp7EA3,c_FxOp7F,c_FxOp7FA1,c_FxOp7FA2,c_FxOp7FA3,c_FxOp80
EXTSYM c_FxOp80A1,c_FxOp80A2,c_FxOp80A3,c_FxOp81,c_FxOp81A1,c_FxOp81A2
EXTSYM c_FxOp81A3,c_FxOp82,c_FxOp82A1,c_FxOp82A2,c_FxOp82A3,c_FxOp83
EXTSYM c_FxOp83A1,c_FxOp83A2,c_FxOp83A3,c_FxOp84,c_FxOp84A1,c_FxOp84A2
EXTSYM c_FxOp84A3,c_FxOp85,c_FxOp85A1,c_FxOp85A2,c_FxOp85A3,c_FxOp86
EXTSYM c_FxOp86A1,c_FxOp86A2,c_FxOp86A3,c_FxOp87,c_FxOp87A1,c_FxOp87A2
EXTSYM c_FxOp87A3,c_FxOp88,c_FxOp88A1,c_FxOp88A2,c_FxOp88A3,c_FxOp89
EXTSYM c_FxOp89A1,c_FxOp89A2,c_FxOp89A3,c_FxOp8A,c_FxOp8AA1,c_FxOp8AA2
EXTSYM c_FxOp8AA3,c_FxOp8B,c_FxOp8BA1,c_FxOp8BA2,c_FxOp8BA3,c_FxOp8C
EXTSYM c_FxOp8CA1,c_FxOp8CA2,c_FxOp8CA3,c_FxOp8D,c_FxOp8DA1,c_FxOp8DA2
EXTSYM c_FxOp8DA3,c_FxOp8E,c_FxOp8EA1,c_FxOp8EA2,c_FxOp8EA3,c_FxOp8F
EXTSYM c_FxOp8FA1,c_FxOp8FA2,c_FxOp8FA3,c_FxOp90,c_FxOp91,c_FxOp92
EXTSYM c_FxOp93,c_FxOp94,c_FxOp95,c_FxOp96,c_FxOp96A1,c_FxOp97
EXTSYM c_FxOp98,c_FxOp98A1,c_FxOp99,c_FxOp99A1,c_FxOp9A,c_FxOp9AA1
EXTSYM c_FxOp9B,c_FxOp9BA1,c_FxOp9C,c_FxOp9CA1,c_FxOp9D,c_FxOp9DA1
EXTSYM c_FxOp9E,c_FxOp9F,c_FxOp9FA1,c_FxOpA0,c_FxOpA0A1,c_FxOpA0A2
EXTSYM c_FxOpA1,c_FxOpA1A1,c_FxOpA1A2,c_FxOpA2,c_FxOpA2A1,c_FxOpA2A2
EXTSYM c_FxOpA3,c_FxOpA3A1,c_FxOpA3A2,c_FxOpA4,c_FxOpA4A1,c_FxOpA4A2
EXTSYM c_FxOpA5,c_FxOpA5A1,c_FxOpA5A2,c_FxOpA6,c_FxOpA6A1,c_FxOpA6A2
EXTSYM c_FxOpA7,c_FxOpA7A1,c_FxOpA7A2,c_FxOpA8,c_FxOpA8A1,c_FxOpA8A2
EXTSYM c_FxOpA9,c_FxOpA9A1,c_FxOpA9A2,c_FxOpAA,c_FxOpAAA1,c_FxOpAAA2
EXTSYM c_FxOpAB,c_FxOpABA1,c_FxOpABA2,c_FxOpAC,c_FxOpACA1,c_FxOpACA2
EXTSYM c_FxOpAD,c_FxOpADA1,c_FxOpADA2,c_FxOpAE,c_FxOpAEA1,c_FxOpAEA2
EXTSYM c_FxOpAF,c_FxOpAFA1,c_FxOpAFA2,c_FxOpB0,c_FxOpB1,c_FxOpB2
EXTSYM c_FxOpB3,c_FxOpB4,c_FxOpB5,c_FxOpB6,c_FxOpB7,c_FxOpB8
EXTSYM c_FxOpB9,c_FxOpBA,c_FxOpBB,c_FxOpBC,c_FxOpBD,c_FxOpBE
EXTSYM c_FxOpBF,c_FxOpC0,c_FxOpC1,c_FxOpC1A1,c_FxOpC1A2,c_FxOpC1A3
EXTSYM c_FxOpC2,c_FxOpC2A1,c_FxOpC2A2,c_FxOpC2A3,c_FxOpC3,c_FxOpC3A1
EXTSYM c_FxOpC3A2,c_FxOpC3A3,c_FxOpC4,c_FxOpC4A1,c_FxOpC4A2,c_FxOpC4A3
EXTSYM c_FxOpC5,c_FxOpC5A1,c_FxOpC5A2,c_FxOpC5A3,c_FxOpC6,c_FxOpC6A1
EXTSYM c_FxOpC6A2,c_FxOpC6A3,c_FxOpC7,c_FxOpC7A1,c_FxOpC7A2,c_FxOpC7A3
EXTSYM c_FxOpC8,c_FxOpC8A1,c_FxOpC8A2,c_FxOpC8A3,c_FxOpC9,c_FxOpC9A1
EXTSYM c_FxOpC9A2,c_FxOpC9A3,c_FxOpCA,c_FxOpCAA1,c_FxOpCAA2,c_FxOpCAA3
EXTSYM c_FxOpCB,c_FxOpCBA1,c_FxOpCBA2,c_FxOpCBA3,c_FxOpCC,c_FxOpCCA1
EXTSYM c_FxOpCCA2,c_FxOpCCA3,c_FxOpCD,c_FxOpCDA1,c_FxOpCDA2,c_FxOpCDA3
EXTSYM c_FxOpCE,c_FxOpCEA1,c_FxOpCEA2,c_FxOpCEA3,c_FxOpCF,c_FxOpCFA1
EXTSYM c_FxOpCFA2,c_FxOpCFA3,c_FxOpD0,c_FxOpD1,c_FxOpD2,c_FxOpD3
EXTSYM c_FxOpD4,c_FxOpD5,c_FxOpD6,c_FxOpD7,c_FxOpD8,c_FxOpD9
EXTSYM c_FxOpDA,c_FxOpDB,c_FxOpDC,c_FxOpDD,c_FxOpDE,c_FxOpDF
EXTSYM c_FxOpDFA2,c_FxOpDFA3,c_FxOpE0,c_FxOpE1,c_FxOpE2,c_FxOpE3
EXTSYM c_FxOpE4,c_FxOpE5,c_FxOpE6,c_FxOpE7,c_FxOpE8,c_FxOpE9
EXTSYM c_FxOpEA,c_FxOpEB,c_FxOpEC,c_FxOpED,c_FxOpEE,c_FxOpEF
EXTSYM c_FxOpEFA1,c_FxOpEFA2,c_FxOpEFA3,c_FxOpF0,c_FxOpF0A1,c_FxOpF0A2
EXTSYM c_FxOpF1,c_FxOpF1A1,c_FxOpF1A2,c_FxOpF2,c_FxOpF2A1,c_FxOpF2A2
EXTSYM c_FxOpF3,c_FxOpF3A1,c_FxOpF3A2,c_FxOpF4,c_FxOpF4A1,c_FxOpF4A2
EXTSYM c_FxOpF5,c_FxOpF5A1,c_FxOpF5A2,c_FxOpF6,c_FxOpF6A1,c_FxOpF6A2
EXTSYM c_FxOpF7,c_FxOpF7A1,c_FxOpF7A2,c_FxOpF8,c_FxOpF8A1,c_FxOpF8A2
EXTSYM c_FxOpF9,c_FxOpF9A1,c_FxOpF9A2,c_FxOpFA,c_FxOpFAA1,c_FxOpFAA2
EXTSYM c_FxOpFB,c_FxOpFBA1,c_FxOpFBA2,c_FxOpFC,c_FxOpFCA1,c_FxOpFCA2
EXTSYM c_FxOpFD,c_FxOpFDA1,c_FxOpFDA2,c_FxOpFE,c_FxOpFEA1,c_FxOpFEA2
EXTSYM c_FxOpFF,c_FxOpFFA1,c_FxOpFFA2,c_FxOpd00

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

%macro fxdop 1
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    ccall %1
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    FXReturn
%endmacro

; STOP leaves the loop outright rather than chaining.
%macro fxdopend 1
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    ccall %1
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    jmp FXEndLoop
%endmacro

%include "chips/fxemu2.mac"
%include "chips/fxemu2c.mac"



SECTION .text

ALIGN32
NEWSYM FxOpd00      ; STOP   stop GSU execution (and maybe generate an IRQ)     ; Verified.
   fxdopend c_FxOpd00
NEWSYM FxOpd01      ; NOP    no operation       ; Verified.
   fxdop c_FxOp01
NEWSYM FxOpd02      ; CACHE  reintialize GSU cache
   fxdop c_FxOp02
NEWSYM FxOpd03      ; LSR    logic shift right  ; Verified.
   fxdop c_FxOp03
NEWSYM FxOpd04      ; ROL    rotate left (RCL?) ; V
   fxdop c_FxOp04
NEWSYM FxOpd05      ; BRA    branch always      ; Verified.
   fxdop c_FxOp05
NEWSYM FxOpd06      ; BGE    branch on greater or equals        ; Verified.
   fxdop c_FxOp06
NEWSYM FxOpd07      ; BLT    branch on lesss than       ; Verified.
   fxdop c_FxOp07
NEWSYM FxOpd08      ; BNE    branch on not equal        ; Verified.
   fxdop c_FxOp08
NEWSYM FxOpd09      ; BEQ    branch on equal (z=1)      ; Verified.
   fxdop c_FxOp09
NEWSYM FxOpd0A      ; BPL    branch on plus     ; Verified.
   fxdop c_FxOp0A
NEWSYM FxOpd0B      ; BMI    branch on minus    ; Verified.
   fxdop c_FxOp0B
NEWSYM FxOpd0C      ; BCC    branch on carry clear      ; Verified.
   fxdop c_FxOp0C
NEWSYM FxOpd0D      ; BCS    branch on carry set        ; Verified.
   fxdop c_FxOp0D
NEWSYM FxOpd0E      ; BVC    branch on overflow clear   ; Verified.
   fxdop c_FxOp0E
NEWSYM FxOpd0F      ; BVS    branch on overflow set     ; Verified.
   fxdop c_FxOp0F
NEWSYM FxOpd10      ; TO RN  set register n as destination register
   fxdop c_FxOp10
NEWSYM FxOpd11      ; TO RN  set register n as destination register
   fxdop c_FxOp11
NEWSYM FxOpd12      ; TO RN  set register n as destination register
   fxdop c_FxOp12
NEWSYM FxOpd13      ; TO RN  set register n as destination register
   fxdop c_FxOp13
NEWSYM FxOpd14      ; TO RN  set register n as destination register
   fxdop c_FxOp14
NEWSYM FxOpd15      ; TO RN  set register n as destination register
   fxdop c_FxOp15
NEWSYM FxOpd16      ; TO RN  set register n as destination register
   fxdop c_FxOp16
NEWSYM FxOpd17      ; TO RN  set register n as destination register
   fxdop c_FxOp17
NEWSYM FxOpd18      ; TO RN  set register n as destination register
   fxdop c_FxOp18
NEWSYM FxOpd19      ; TO RN  set register n as destination register
   fxdop c_FxOp19
NEWSYM FxOpd1A      ; TO RN  set register n as destination register
   fxdop c_FxOp1A
NEWSYM FxOpd1B      ; TO RN  set register n as destination register
   fxdop c_FxOp1B
NEWSYM FxOpd1C      ; TO RN  set register n as destination register
   fxdop c_FxOp1C
NEWSYM FxOpd1D      ; TO RN  set register n as destination register
   fxdop c_FxOp1D
NEWSYM FxOpd1E      ; TO RN  set register n as destination register
   fxdop c_FxOp1E
NEWSYM FxOpd1F      ; TO RN  set register n as destination register
   fxdop c_FxOp1F
NEWSYM FxOpd20      ; WITH  set register n as source and destination register
   fxdop c_FxOp20
NEWSYM FxOpd21      ; WITH  set register n as source and destination register
   fxdop c_FxOp21
NEWSYM FxOpd22      ; WITH  set register n as source and destination register
   fxdop c_FxOp22
NEWSYM FxOpd23      ; WITH  set register n as source and destination register
   fxdop c_FxOp23
NEWSYM FxOpd24      ; WITH  set register n as source and destination register
   fxdop c_FxOp24
NEWSYM FxOpd25      ; WITH  set register n as source and destination register
   fxdop c_FxOp25
NEWSYM FxOpd26      ; WITH  set register n as source and destination register
   fxdop c_FxOp26
NEWSYM FxOpd27      ; WITH  set register n as source and destination register
   fxdop c_FxOp27
NEWSYM FxOpd28      ; WITH  set register n as source and destination register
   fxdop c_FxOp28
NEWSYM FxOpd29      ; WITH  set register n as source and destination register
   fxdop c_FxOp29
NEWSYM FxOpd2A      ; WITH  set register n as source and destination register
   fxdop c_FxOp2A
NEWSYM FxOpd2B      ; WITH  set register n as source and destination register
   fxdop c_FxOp2B
NEWSYM FxOpd2C      ; WITH  set register n as source and destination register
   fxdop c_FxOp2C
NEWSYM FxOpd2D      ; WITH  set register n as source and destination register
   fxdop c_FxOp2D
NEWSYM FxOpd2E      ; WITH  set register n as source and destination register
   fxdop c_FxOp2E
NEWSYM FxOpd2F      ; WITH  set register n as source and destination register
   fxdop c_FxOp2F
NEWSYM FxOpd30      ; STW RN store word
   fxdop c_FxOp30
NEWSYM FxOpd31      ; STW RN store word
   fxdop c_FxOp31
NEWSYM FxOpd32      ; STW RN store word
   fxdop c_FxOp32
NEWSYM FxOpd33      ; STW RN store word
   fxdop c_FxOp33
NEWSYM FxOpd34      ; STW RN store word
   fxdop c_FxOp34
NEWSYM FxOpd35      ; STW RN store word
   fxdop c_FxOp35
NEWSYM FxOpd36      ; STW RN store word
   fxdop c_FxOp36
NEWSYM FxOpd37      ; STW RN store word
   fxdop c_FxOp37
NEWSYM FxOpd38      ; STW RN store word
   fxdop c_FxOp38
NEWSYM FxOpd39      ; STW RN store word
   fxdop c_FxOp39
NEWSYM FxOpd3A      ; STW RN store word
   fxdop c_FxOp3A
NEWSYM FxOpd3B      ; STW RN store word
   fxdop c_FxOp3B
NEWSYM FxOpd30A1    ; STB RN store byte
   fxdop c_FxOp30A1
NEWSYM FxOpd31A1    ; STB RN store byte
   fxdop c_FxOp31A1
NEWSYM FxOpd32A1    ; STB RN store byte
   fxdop c_FxOp32A1
NEWSYM FxOpd33A1    ; STB RN store byte
   fxdop c_FxOp33A1
NEWSYM FxOpd34A1    ; STB RN store byte
   fxdop c_FxOp34A1
NEWSYM FxOpd35A1    ; STB RN store byte
   fxdop c_FxOp35A1
NEWSYM FxOpd36A1    ; STB RN store byte
   fxdop c_FxOp36A1
NEWSYM FxOpd37A1    ; STB RN store byte
   fxdop c_FxOp37A1
NEWSYM FxOpd38A1    ; STB RN store byte
   fxdop c_FxOp38A1
NEWSYM FxOpd39A1    ; STB RN store byte
   fxdop c_FxOp39A1
NEWSYM FxOpd3AA1    ; STB RN store byte
   fxdop c_FxOp3AA1
NEWSYM FxOpd3BA1    ; STB RN store byte
   fxdop c_FxOp3BA1
NEWSYM FxOpd3C      ; LOOP   decrement loop counter, and branch on not zero ; V
   fxdop c_FxOp3C
NEWSYM FxOpd3D      ; ALT1   set alt1 mode      ; Verified.
   fxdop c_FxOp3D
NEWSYM FxOpd3E      ; ALT2   set alt1 mode      ; Verified.
   fxdop c_FxOp3E
NEWSYM FxOpd3F      ; ALT3   set alt3 mode      ; Verified.
   fxdop c_FxOp3F
NEWSYM FxOpd40      ; LDW RN load word from RAM
   fxdop c_FxOp40
NEWSYM FxOpd41      ; LDW RN load word from RAM
   fxdop c_FxOp41
NEWSYM FxOpd42      ; LDW RN load word from RAM
   fxdop c_FxOp42
NEWSYM FxOpd43      ; LDW RN load word from RAM
   fxdop c_FxOp43
NEWSYM FxOpd44      ; LDW RN load word from RAM
   fxdop c_FxOp44
NEWSYM FxOpd45      ; LDW RN load word from RAM
   fxdop c_FxOp45
NEWSYM FxOpd46      ; LDW RN load word from RAM
   fxdop c_FxOp46
NEWSYM FxOpd47      ; LDW RN load word from RAM
   fxdop c_FxOp47
NEWSYM FxOpd48      ; LDW RN load word from RAM
   fxdop c_FxOp48
NEWSYM FxOpd49      ; LDW RN load word from RAM
   fxdop c_FxOp49
NEWSYM FxOpd4A      ; LDW RN load word from RAM
   fxdop c_FxOp4A
NEWSYM FxOpd4B      ; LDW RN load word from RAM
   fxdop c_FxOp4B
NEWSYM FxOpd40A1    ; LDB RN load byte from RAM
   fxdop c_FxOp40A1
NEWSYM FxOpd41A1    ; LDB RN load byte from RAM
   fxdop c_FxOp41A1
NEWSYM FxOpd42A1    ; LDB RN load byte from RAM
   fxdop c_FxOp42A1
NEWSYM FxOpd43A1    ; LDB RN load byte from RAM
   fxdop c_FxOp43A1
NEWSYM FxOpd44A1    ; LDB RN load byte from RAM
   fxdop c_FxOp44A1
NEWSYM FxOpd45A1    ; LDB RN load byte from RAM
   fxdop c_FxOp45A1
NEWSYM FxOpd46A1    ; LDB RN load byte from RAM
   fxdop c_FxOp46A1
NEWSYM FxOpd47A1    ; LDB RN load byte from RAM
   fxdop c_FxOp47A1
NEWSYM FxOpd48A1    ; LDB RN load byte from RAM
   fxdop c_FxOp48A1
NEWSYM FxOpd49A1    ; LDB RN load byte from RAM
   fxdop c_FxOp49A1
NEWSYM FxOpd4AA1    ; LDB RN load byte from RAM
   fxdop c_FxOp4AA1
NEWSYM FxOpd4BA1    ; LDB RN load byte from RAM
   fxdop c_FxOp4BA1
NEWSYM FxOpd4C1284b       ; PLOT 4bit
   fxdop c_FxOp4C1284b
NEWSYM FxOpd4C1284bz      ; PLOT 4bit, zero check
   fxdop c_FxOp4C1284bz
NEWSYM FxOpd4C1284bd      ; PLOT 4bit, dither
   fxdop c_FxOp4C1284bd
NEWSYM FxOpd4C1284bzd     ; PLOT 4bit, zero check + dither
   fxdop c_FxOp4C1284bzd
NEWSYM FxOpd4C1282b       ; PLOT 2bit
   fxdop c_FxOp4C1282b
NEWSYM FxOpd4C1282bz      ; PLOT 2bit, zero check
   fxdop c_FxOp4C1282bz
NEWSYM FxOpd4C1282bd      ; PLOT 2bit, dither
   fxdop c_FxOp4C1282bd
NEWSYM FxOpd4C1282bzd     ; PLOT 2bit, zero check + dither
   fxdop c_FxOp4C1282bzd
NEWSYM FxOpd4C1288b       ; PLOT 8bit
   fxdop c_FxOp4C1288b
NEWSYM FxOpd4C1288bz      ; PLOT 8bit, zero check
   fxdop c_FxOp4C1288bz
NEWSYM FxOpd4C1288bd      ; PLOT 8bit, dither
   fxdop c_FxOp4C1288b
NEWSYM FxOpd4C1288bzd     ; PLOT 8bit, zero check + dither
   fxdop c_FxOp4C1288bz
NEWSYM FxOpd4C1288bl       ; PLOT 8bit
   fxdop c_FxOp4C1288bl
NEWSYM FxOpd4C1288bzl      ; PLOT 8bit, zero check
   fxdop c_FxOp4C1288bzl
NEWSYM FxOpd4C1288bdl      ; PLOT 8bit, dither
   fxdop c_FxOp4C1288bl
NEWSYM FxOpd4C1288bzdl     ; PLOT 8bit, zero check + dither
   fxdop c_FxOp4C1288bzl
NEWSYM FxOpd4C      ; PLOT   plot pixel with R1,R2 as x,y and the color register as the color
   fxdop c_FxOp4C1284b
NEWSYM FxOpd4CA1    ; RPIX   read color of the pixel with R1,R2 as x,y
   fxdop c_FxOp4CA1
NEWSYM FxOpd4D      ; SWAP   swap upper and lower byte of a register    ; V
   fxdop c_FxOp4D
NEWSYM FxOpd4E      ; COLOR  copy source register to color register     ; V
   fxdop c_FxOp4E
NEWSYM FxOpd4EA1    ; CMODE  set plot option register ; V
   fxdop c_FxOp4EA1
NEWSYM FxOpd4F      ; NOT    perform exclusive exor with 1 on all bits  ; V
   fxdop c_FxOp4F
NEWSYM FxOpd50      ; ADD RN add, register + register
   fxdop c_FxOp50
NEWSYM FxOpd51      ; ADD RN add, register + register
   fxdop c_FxOp51
NEWSYM FxOpd52      ; ADD RN add, register + register
   fxdop c_FxOp52
NEWSYM FxOpd53      ; ADD RN add, register + register
   fxdop c_FxOp53
NEWSYM FxOpd54      ; ADD RN add, register + register
   fxdop c_FxOp54
NEWSYM FxOpd55      ; ADD RN add, register + register
   fxdop c_FxOp55
NEWSYM FxOpd56      ; ADD RN add, register + register
   fxdop c_FxOp56
NEWSYM FxOpd57      ; ADD RN add, register + register
   fxdop c_FxOp57
NEWSYM FxOpd58      ; ADD RN add, register + register
   fxdop c_FxOp58
NEWSYM FxOpd59      ; ADD RN add, register + register
   fxdop c_FxOp59
NEWSYM FxOpd5A      ; ADD RN add, register + register
   fxdop c_FxOp5A
NEWSYM FxOpd5B      ; ADD RN add, register + register
   fxdop c_FxOp5B
NEWSYM FxOpd5C      ; ADD RN add, register + register
   fxdop c_FxOp5C
NEWSYM FxOpd5D      ; ADD RN add, register + register
   fxdop c_FxOp5D
NEWSYM FxOpd5E      ; ADD RN add, register + register
   fxdop c_FxOp5E
NEWSYM FxOpd5F      ; ADD RN add, register + register
   fxdop c_FxOp5F
NEWSYM FxOpd50A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp50A1
NEWSYM FxOpd51A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp51A1
NEWSYM FxOpd52A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp52A1
NEWSYM FxOpd53A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp53A1
NEWSYM FxOpd54A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp54A1
NEWSYM FxOpd55A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp55A1
NEWSYM FxOpd56A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp56A1
NEWSYM FxOpd57A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp57A1
NEWSYM FxOpd58A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp58A1
NEWSYM FxOpd59A1    ; ADC RN add with carry, register + register
   fxdop c_FxOp59A1
NEWSYM FxOpd5AA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5AA1
NEWSYM FxOpd5BA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5BA1
NEWSYM FxOpd5CA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5CA1
NEWSYM FxOpd5DA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5DA1
NEWSYM FxOpd5EA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5EA1
NEWSYM FxOpd5FA1    ; ADC RN add with carry, register + register
   fxdop c_FxOp5FA1
NEWSYM FxOpd50A2    ; ADI RN add, register + immediate
   fxdop c_FxOp50A2
NEWSYM FxOpd51A2    ; ADI RN add, register + immediate
   fxdop c_FxOp51A2
NEWSYM FxOpd52A2    ; ADI RN add, register + immediate
   fxdop c_FxOp52A2
NEWSYM FxOpd53A2    ; ADI RN add, register + immediate
   fxdop c_FxOp53A2
NEWSYM FxOpd54A2    ; ADI RN add, register + immediate
   fxdop c_FxOp54A2
NEWSYM FxOpd55A2    ; ADI RN add, register + immediate
   fxdop c_FxOp55A2
NEWSYM FxOpd56A2    ; ADI RN add, register + immediate
   fxdop c_FxOp56A2
NEWSYM FxOpd57A2    ; ADI RN add, register + immediate
   fxdop c_FxOp57A2
NEWSYM FxOpd58A2    ; ADI RN add, register + immediate
   fxdop c_FxOp58A2
NEWSYM FxOpd59A2    ; ADI RN add, register + immediate
   fxdop c_FxOp59A2
NEWSYM FxOpd5AA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5AA2
NEWSYM FxOpd5BA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5BA2
NEWSYM FxOpd5CA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5CA2
NEWSYM FxOpd5DA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5DA2
NEWSYM FxOpd5EA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5EA2
NEWSYM FxOpd5FA2    ; ADI RN add, register + immediate
   fxdop c_FxOp5FA2
NEWSYM FxOpd50A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp50A3
NEWSYM FxOpd51A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp51A3
NEWSYM FxOpd52A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp52A3
NEWSYM FxOpd53A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp53A3
NEWSYM FxOpd54A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp54A3
NEWSYM FxOpd55A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp55A3
NEWSYM FxOpd56A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp56A3
NEWSYM FxOpd57A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp57A3
NEWSYM FxOpd58A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp58A3
NEWSYM FxOpd59A3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp59A3
NEWSYM FxOpd5AA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5AA3
NEWSYM FxOpd5BA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5BA3
NEWSYM FxOpd5CA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5CA3
NEWSYM FxOpd5DA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5DA3
NEWSYM FxOpd5EA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5EA3
NEWSYM FxOpd5FA3    ; ADCIRN add with carry, register + immediate
   fxdop c_FxOp5FA3
NEWSYM FxOpd60      ; SUBRN  subtract, register - register
   fxdop c_FxOp60
NEWSYM FxOpd61      ; SUBRN  subtract, register - register
   fxdop c_FxOp61
NEWSYM FxOpd62      ; SUBRN  subtract, register - register
   fxdop c_FxOp62
NEWSYM FxOpd63      ; SUBRN  subtract, register - register
   fxdop c_FxOp63
NEWSYM FxOpd64      ; SUBRN  subtract, register - register
   fxdop c_FxOp64
NEWSYM FxOpd65      ; SUBRN  subtract, register - register
   fxdop c_FxOp65
NEWSYM FxOpd66      ; SUBRN  subtract, register - register
   fxdop c_FxOp66
NEWSYM FxOpd67      ; SUBRN  subtract, register - register
   fxdop c_FxOp67
NEWSYM FxOpd68      ; SUBRN  subtract, register - register
   fxdop c_FxOp68
NEWSYM FxOpd69      ; SUBRN  subtract, register - register
   fxdop c_FxOp69
NEWSYM FxOpd6A      ; SUBRN  subtract, register - register
   fxdop c_FxOp6A
NEWSYM FxOpd6B      ; SUBRN  subtract, register - register
   fxdop c_FxOp6B
NEWSYM FxOpd6C      ; SUBRN  subtract, register - register
   fxdop c_FxOp6C
NEWSYM FxOpd6D      ; SUBRN  subtract, register - register
   fxdop c_FxOp6D
NEWSYM FxOpd6E      ; SUBRN  subtract, register - register
   fxdop c_FxOp6E
NEWSYM FxOpd6F      ; SUBRN  subtract, register - register
   fxdop c_FxOp6F
NEWSYM FxOpd60A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp60A1
NEWSYM FxOpd61A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp61A1
NEWSYM FxOpd62A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp62A1
NEWSYM FxOpd63A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp63A1
NEWSYM FxOpd64A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp64A1
NEWSYM FxOpd65A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp65A1
NEWSYM FxOpd66A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp66A1
NEWSYM FxOpd67A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp67A1
NEWSYM FxOpd68A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp68A1
NEWSYM FxOpd69A1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp69A1
NEWSYM FxOpd6AA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6AA1
NEWSYM FxOpd6BA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6BA1
NEWSYM FxOpd6CA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6CA1
NEWSYM FxOpd6DA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6DA1
NEWSYM FxOpd6EA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6EA1
NEWSYM FxOpd6FA1    ; SBCRN  subtract with carry, register - register
   fxdop c_FxOp6FA1
NEWSYM FxOpd60A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp60A2
NEWSYM FxOpd61A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp61A2
NEWSYM FxOpd62A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp62A2
NEWSYM FxOpd63A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp63A2
NEWSYM FxOpd64A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp64A2
NEWSYM FxOpd65A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp65A2
NEWSYM FxOpd66A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp66A2
NEWSYM FxOpd67A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp67A2
NEWSYM FxOpd68A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp68A2
NEWSYM FxOpd69A2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp69A2
NEWSYM FxOpd6AA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6AA2
NEWSYM FxOpd6BA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6BA2
NEWSYM FxOpd6CA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6CA2
NEWSYM FxOpd6DA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6DA2
NEWSYM FxOpd6EA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6EA2
NEWSYM FxOpd6FA2    ; SUBIRN subtract, register - immediate
   fxdop c_FxOp6FA2
NEWSYM FxOpd60A3    ; CMPRN  compare, register, register
   fxdop c_FxOp60A3
NEWSYM FxOpd61A3    ; CMPRN  compare, register, register
   fxdop c_FxOp61A3
NEWSYM FxOpd62A3    ; CMPRN  compare, register, register
   fxdop c_FxOp62A3
NEWSYM FxOpd63A3    ; CMPRN  compare, register, register
   fxdop c_FxOp63A3
NEWSYM FxOpd64A3    ; CMPRN  compare, register, register
   fxdop c_FxOp64A3
NEWSYM FxOpd65A3    ; CMPRN  compare, register, register
   fxdop c_FxOp65A3
NEWSYM FxOpd66A3    ; CMPRN  compare, register, register
   fxdop c_FxOp66A3
NEWSYM FxOpd67A3    ; CMPRN  compare, register, register
   fxdop c_FxOp67A3
NEWSYM FxOpd68A3    ; CMPRN  compare, register, register
   fxdop c_FxOp68A3
NEWSYM FxOpd69A3    ; CMPRN  compare, register, register
   fxdop c_FxOp69A3
NEWSYM FxOpd6AA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6AA3
NEWSYM FxOpd6BA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6BA3
NEWSYM FxOpd6CA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6CA3
NEWSYM FxOpd6DA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6DA3
NEWSYM FxOpd6EA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6EA3
NEWSYM FxOpd6FA3    ; CMPRN  compare, register, register
   fxdop c_FxOp6FA3
NEWSYM FxOpd70      ; MERGE  R7 as upper byte, R8 as lower byte (used for texture-mapping) */
   fxdop c_FxOp70
NEWSYM FxOpd71      ; AND RN register & register
   fxdop c_FxOp71
NEWSYM FxOpd72      ; AND RN register & register
   fxdop c_FxOp72
NEWSYM FxOpd73      ; AND RN register & register
   fxdop c_FxOp73
NEWSYM FxOpd74      ; AND RN register & register
   fxdop c_FxOp74
NEWSYM FxOpd75      ; AND RN register & register
   fxdop c_FxOp75
NEWSYM FxOpd76      ; AND RN register & register
   fxdop c_FxOp76
NEWSYM FxOpd77      ; AND RN register & register
   fxdop c_FxOp77
NEWSYM FxOpd78      ; AND RN register & register
   fxdop c_FxOp78
NEWSYM FxOpd79      ; AND RN register & register
   fxdop c_FxOp79
NEWSYM FxOpd7A      ; AND RN register & register
   fxdop c_FxOp7A
NEWSYM FxOpd7B      ; AND RN register & register
   fxdop c_FxOp7B
NEWSYM FxOpd7C      ; AND RN register & register
   fxdop c_FxOp7C
NEWSYM FxOpd7D      ; AND RN register & register
   fxdop c_FxOp7D
NEWSYM FxOpd7E      ; AND RN register & register
   fxdop c_FxOp7E
NEWSYM FxOpd7F      ; AND RN register & register
   fxdop c_FxOp7F
NEWSYM FxOpd71A1    ; BIC RN register & ~register
   fxdop c_FxOp71A1
NEWSYM FxOpd72A1    ; BIC RN register & ~register
   fxdop c_FxOp72A1
NEWSYM FxOpd73A1    ; BIC RN register & ~register
   fxdop c_FxOp73A1
NEWSYM FxOpd74A1    ; BIC RN register & ~register
   fxdop c_FxOp74A1
NEWSYM FxOpd75A1    ; BIC RN register & ~register
   fxdop c_FxOp75A1
NEWSYM FxOpd76A1    ; BIC RN register & ~register
   fxdop c_FxOp76A1
NEWSYM FxOpd77A1    ; BIC RN register & ~register
   fxdop c_FxOp77A1
NEWSYM FxOpd78A1    ; BIC RN register & ~register
   fxdop c_FxOp78A1
NEWSYM FxOpd79A1    ; BIC RN register & ~register
   fxdop c_FxOp79A1
NEWSYM FxOpd7AA1    ; BIC RN register & ~register
   fxdop c_FxOp7AA1
NEWSYM FxOpd7BA1    ; BIC RN register & ~register
   fxdop c_FxOp7BA1
NEWSYM FxOpd7CA1    ; BIC RN register & ~register
   fxdop c_FxOp7CA1
NEWSYM FxOpd7DA1    ; BIC RN register & ~register
   fxdop c_FxOp7DA1
NEWSYM FxOpd7EA1    ; BIC RN register & ~register
   fxdop c_FxOp7EA1
NEWSYM FxOpd7FA1    ; BIC RN register & ~register
   fxdop c_FxOp7FA1
NEWSYM FxOpd71A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp71A2
NEWSYM FxOpd72A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp72A2
NEWSYM FxOpd73A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp73A2
NEWSYM FxOpd74A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp74A2
NEWSYM FxOpd75A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp75A2
NEWSYM FxOpd76A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp76A2
NEWSYM FxOpd77A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp77A2
NEWSYM FxOpd78A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp78A2
NEWSYM FxOpd79A2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp79A2
NEWSYM FxOpd7AA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7AA2
NEWSYM FxOpd7BA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7BA2
NEWSYM FxOpd7CA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7CA2
NEWSYM FxOpd7DA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7DA2
NEWSYM FxOpd7EA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7EA2
NEWSYM FxOpd7FA2    ; ANDIRNc and #n - register & immediate
   fxdop c_FxOp7FA2
NEWSYM FxOpd71A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp71A3
NEWSYM FxOpd72A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp72A3
NEWSYM FxOpd73A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp73A3
NEWSYM FxOpd74A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp74A3
NEWSYM FxOpd75A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp75A3
NEWSYM FxOpd76A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp76A3
NEWSYM FxOpd77A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp77A3
NEWSYM FxOpd78A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp78A3
NEWSYM FxOpd79A3    ; BICIRNc register & ~immediate
   fxdop c_FxOp79A3
NEWSYM FxOpd7AA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7AA3
NEWSYM FxOpd7BA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7BA3
NEWSYM FxOpd7CA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7CA3
NEWSYM FxOpd7DA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7DA3
NEWSYM FxOpd7EA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7EA3
NEWSYM FxOpd7FA3    ; BICIRNc register & ~immediate
   fxdop c_FxOp7FA3
NEWSYM FxOpd80      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp80
NEWSYM FxOpd81      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp81
NEWSYM FxOpd82      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp82
NEWSYM FxOpd83      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp83
NEWSYM FxOpd84      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp84
NEWSYM FxOpd85      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp85
NEWSYM FxOpd86      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp86
NEWSYM FxOpd87      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp87
NEWSYM FxOpd88      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp88
NEWSYM FxOpd89      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp89
NEWSYM FxOpd8A      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8A
NEWSYM FxOpd8B      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8B
NEWSYM FxOpd8C      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8C
NEWSYM FxOpd8D      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8D
NEWSYM FxOpd8E      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8E
NEWSYM FxOpd8F      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   fxdop c_FxOp8F
NEWSYM FxOpd80A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp80A1
NEWSYM FxOpd81A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp81A1
NEWSYM FxOpd82A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp82A1
NEWSYM FxOpd83A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp83A1
NEWSYM FxOpd84A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp84A1
NEWSYM FxOpd85A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp85A1
NEWSYM FxOpd86A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp86A1
NEWSYM FxOpd87A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp87A1
NEWSYM FxOpd88A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp88A1
NEWSYM FxOpd89A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp89A1
NEWSYM FxOpd8AA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8AA1
NEWSYM FxOpd8BA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8BA1
NEWSYM FxOpd8CA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8CA1
NEWSYM FxOpd8DA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8DA1
NEWSYM FxOpd8EA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8EA1
NEWSYM FxOpd8FA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   fxdop c_FxOp8FA1
NEWSYM FxOpd80A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp80A2
NEWSYM FxOpd81A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp81A2
NEWSYM FxOpd82A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp82A2
NEWSYM FxOpd83A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp83A2
NEWSYM FxOpd84A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp84A2
NEWSYM FxOpd85A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp85A2
NEWSYM FxOpd86A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp86A2
NEWSYM FxOpd87A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp87A2
NEWSYM FxOpd88A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp88A2
NEWSYM FxOpd89A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp89A2
NEWSYM FxOpd8AA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8AA2
NEWSYM FxOpd8BA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8BA2
NEWSYM FxOpd8CA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8CA2
NEWSYM FxOpd8DA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8DA2
NEWSYM FxOpd8EA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8EA2
NEWSYM FxOpd8FA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   fxdop c_FxOp8FA2
NEWSYM FxOpd80A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp80A3
NEWSYM FxOpd81A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp81A3
NEWSYM FxOpd82A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp82A3
NEWSYM FxOpd83A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp83A3
NEWSYM FxOpd84A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp84A3
NEWSYM FxOpd85A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp85A3
NEWSYM FxOpd86A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp86A3
NEWSYM FxOpd87A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp87A3
NEWSYM FxOpd88A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp88A3
NEWSYM FxOpd89A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp89A3
NEWSYM FxOpd8AA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8AA3
NEWSYM FxOpd8BA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8BA3
NEWSYM FxOpd8CA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8CA3
NEWSYM FxOpd8DA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8DA3
NEWSYM FxOpd8EA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8EA3
NEWSYM FxOpd8FA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   fxdop c_FxOp8FA3
NEWSYM FxOpd90      ; SBK    store word to last accessed RAM address    ; V
   fxdop c_FxOp90
NEWSYM FxOpd91      ; LINKc#n R11 = R15 + immediate
   fxdop c_FxOp91
NEWSYM FxOpd92      ; LINKc#n R11 = R15 + immediate
   fxdop c_FxOp92
NEWSYM FxOpd93      ; LINKc#n R11 = R15 + immediate
   fxdop c_FxOp93
NEWSYM FxOpd94      ; LINKc#n R11 = R15 + immediate
   fxdop c_FxOp94
NEWSYM FxOpd95      ; SEX    sign extend 8 bit to 16 bit        ; V
   fxdop c_FxOp95
NEWSYM FxOpd96      ; ASR    aritmethic shift right by one      ; V
   fxdop c_FxOp96
NEWSYM FxOpd96A1    ; DIV2   aritmethic shift right by one      ; V
   fxdop c_FxOp96A1
NEWSYM FxOpd97      ; ROR    rotate right by one        ; V
   fxdop c_FxOp97
NEWSYM FxOpd98      ; JMPRNc  jump to address of register
   fxdop c_FxOp98
NEWSYM FxOpd99      ; JMPRNc  jump to address of register
   fxdop c_FxOp99
NEWSYM FxOpd9A      ; JMPRNc  jump to address of register
   fxdop c_FxOp9A
NEWSYM FxOpd9B      ; JMPRNc  jump to address of register
   fxdop c_FxOp9B
NEWSYM FxOpd9C      ; JMPRNc  jump to address of register
   fxdop c_FxOp9C
NEWSYM FxOpd9D      ; JMPRNc  jump to address of register
   fxdop c_FxOp9D
NEWSYM FxOpd98A1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp98A1
NEWSYM FxOpd99A1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp99A1
NEWSYM FxOpd9AA1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp9AA1
NEWSYM FxOpd9BA1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp9BA1
NEWSYM FxOpd9CA1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp9CA1
NEWSYM FxOpd9DA1    ; LJMPRNc set program bank to source register and jump to address of register
   fxdop c_FxOp9DA1
NEWSYM FxOpd9E      ; LOB    set upper byte to zero (keep low byte) ; V
   fxdop c_FxOp9E
NEWSYM FxOpd9F      ; FMULT  16 bit to 32 bit signed multiplication, upper 16 bits only
   fxdop c_FxOp9F
NEWSYM FxOpd9FA1    ; LMULT  16 bit to 32 bit signed multiplication     ; V
   fxdop c_FxOp9FA1
NEWSYM FxOpdA0      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA0
NEWSYM FxOpdA1      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA1
NEWSYM FxOpdA2      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA2
NEWSYM FxOpdA3      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA3
NEWSYM FxOpdA4      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA4
NEWSYM FxOpdA5      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA5
NEWSYM FxOpdA6      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA6
NEWSYM FxOpdA7      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA7
NEWSYM FxOpdA8      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA8
NEWSYM FxOpdA9      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpA9
NEWSYM FxOpdAA      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAA
NEWSYM FxOpdAB      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAB
NEWSYM FxOpdAC      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAC
NEWSYM FxOpdAD      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAD
NEWSYM FxOpdAE      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAE
NEWSYM FxOpdAF      ; IBTRNc,#PP immediate byte transfer
   fxdop c_FxOpAF
NEWSYM FxOpdA0A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA0A1
NEWSYM FxOpdA1A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA1A1
NEWSYM FxOpdA2A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA2A1
NEWSYM FxOpdA3A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA3A1
NEWSYM FxOpdA4A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA4A1
NEWSYM FxOpdA5A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA5A1
NEWSYM FxOpdA6A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA6A1
NEWSYM FxOpdA7A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA7A1
NEWSYM FxOpdA8A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA8A1
NEWSYM FxOpdA9A1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpA9A1
NEWSYM FxOpdAAA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpAAA1
NEWSYM FxOpdABA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpABA1
NEWSYM FxOpdACA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpACA1
NEWSYM FxOpdADA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpADA1
NEWSYM FxOpdAEA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpAEA1
NEWSYM FxOpdAFA1    ; LMS rn,(yy)  load word from RAM (short address)
   fxdop c_FxOpAFA1
NEWSYM FxOpdA0A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA0A2
NEWSYM FxOpdA1A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA1A2
NEWSYM FxOpdA2A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA2A2
NEWSYM FxOpdA3A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA3A2
NEWSYM FxOpdA4A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA4A2
NEWSYM FxOpdA5A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA5A2
NEWSYM FxOpdA6A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA6A2
NEWSYM FxOpdA7A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA7A2
NEWSYM FxOpdA8A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA8A2
NEWSYM FxOpdA9A2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpA9A2
NEWSYM FxOpdAAA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpAAA2
NEWSYM FxOpdABA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpABA2
NEWSYM FxOpdACA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpACA2
NEWSYM FxOpdADA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpADA2
NEWSYM FxOpdAEA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpAEA2
NEWSYM FxOpdAFA2    ; SMS (yy),rn  store word in RAM (short address)
   fxdop c_FxOpAFA2
NEWSYM FxOpdB0      ; FROM rn   set source register
   fxdop c_FxOpB0
NEWSYM FxOpdB1      ; FROM rn   set source register
   fxdop c_FxOpB1
NEWSYM FxOpdB2      ; FROM rn   set source register
   fxdop c_FxOpB2
NEWSYM FxOpdB3      ; FROM rn   set source register
   fxdop c_FxOpB3
NEWSYM FxOpdB4      ; FROM rn   set source register
   fxdop c_FxOpB4
NEWSYM FxOpdB5      ; FROM rn   set source register
   fxdop c_FxOpB5
NEWSYM FxOpdB6      ; FROM rn   set source register
   fxdop c_FxOpB6
NEWSYM FxOpdB7      ; FROM rn   set source register
   fxdop c_FxOpB7
NEWSYM FxOpdB8      ; FROM rn   set source register
   fxdop c_FxOpB8
NEWSYM FxOpdB9      ; FROM rn   set source register
   fxdop c_FxOpB9
NEWSYM FxOpdBA      ; FROM rn   set source register
   fxdop c_FxOpBA
NEWSYM FxOpdBB      ; FROM rn   set source register
   fxdop c_FxOpBB
NEWSYM FxOpdBC      ; FROM rn   set source register
   fxdop c_FxOpBC
NEWSYM FxOpdBD      ; FROM rn   set source register
   fxdop c_FxOpBD
NEWSYM FxOpdBE      ; FROM rn   set source register
   fxdop c_FxOpBE
NEWSYM FxOpdBF      ; FROM rn   set source register
   fxdop c_FxOpBF
NEWSYM FxOpdC0      ; HIB       move high-byte to low-byte      ; V
   fxdop c_FxOpC0
NEWSYM FxOpdC1      ; OR rn     or rn
   fxdop c_FxOpC1
NEWSYM FxOpdC2      ; OR rn     or rn
   fxdop c_FxOpC2
NEWSYM FxOpdC3      ; OR rn     or rn
   fxdop c_FxOpC3
NEWSYM FxOpdC4      ; OR rn     or rn
   fxdop c_FxOpC4
NEWSYM FxOpdC5      ; OR rn     or rn
   fxdop c_FxOpC5
NEWSYM FxOpdC6      ; OR rn     or rn
   fxdop c_FxOpC6
NEWSYM FxOpdC7      ; OR rn     or rn
   fxdop c_FxOpC7
NEWSYM FxOpdC8      ; OR rn     or rn
   fxdop c_FxOpC8
NEWSYM FxOpdC9      ; OR rn     or rn
   fxdop c_FxOpC9
NEWSYM FxOpdCA      ; OR rn     or rn
   fxdop c_FxOpCA
NEWSYM FxOpdCB      ; OR rn     or rn
   fxdop c_FxOpCB
NEWSYM FxOpdCC      ; OR rn     or rn
   fxdop c_FxOpCC
NEWSYM FxOpdCD      ; OR rn     or rn
   fxdop c_FxOpCD
NEWSYM FxOpdCE      ; OR rn     or rn
   fxdop c_FxOpCE
NEWSYM FxOpdCF      ; OR rn     or rn
   fxdop c_FxOpCF
NEWSYM FxOpdC1A1    ; XOR rn    xor rn
   fxdop c_FxOpC1A1
NEWSYM FxOpdC2A1    ; XOR rn    xor rn
   fxdop c_FxOpC2A1
NEWSYM FxOpdC3A1    ; XOR rn    xor rn
   fxdop c_FxOpC3A1
NEWSYM FxOpdC4A1    ; XOR rn    xor rn
   fxdop c_FxOpC4A1
NEWSYM FxOpdC5A1    ; XOR rn    xor rn
   fxdop c_FxOpC5A1
NEWSYM FxOpdC6A1    ; XOR rn    xor rn
   fxdop c_FxOpC6A1
NEWSYM FxOpdC7A1    ; XOR rn    xor rn
   fxdop c_FxOpC7A1
NEWSYM FxOpdC8A1    ; XOR rn    xor rn
   fxdop c_FxOpC8A1
NEWSYM FxOpdC9A1    ; XOR rn    xor rn
   fxdop c_FxOpC9A1
NEWSYM FxOpdCAA1    ; XOR rn    xor rn
   fxdop c_FxOpCAA1
NEWSYM FxOpdCBA1    ; XOR rn    xor rn
   fxdop c_FxOpCBA1
NEWSYM FxOpdCCA1    ; XOR rn    xor rn
   fxdop c_FxOpCCA1
NEWSYM FxOpdCDA1    ; XOR rn    xor rn
   fxdop c_FxOpCDA1
NEWSYM FxOpdCEA1    ; XOR rn    xor rn
   fxdop c_FxOpCEA1
NEWSYM FxOpdCFA1    ; XOR rn    xor rn
   fxdop c_FxOpCFA1
NEWSYM FxOpdC1A2    ; OR #n     OR #n
   fxdop c_FxOpC1A2
NEWSYM FxOpdC2A2    ; OR #n     OR #n
   fxdop c_FxOpC2A2
NEWSYM FxOpdC3A2    ; OR #n     OR #n
   fxdop c_FxOpC3A2
NEWSYM FxOpdC4A2    ; OR #n     OR #n
   fxdop c_FxOpC4A2
NEWSYM FxOpdC5A2    ; OR #n     OR #n
   fxdop c_FxOpC5A2
NEWSYM FxOpdC6A2    ; OR #n     OR #n
   fxdop c_FxOpC6A2
NEWSYM FxOpdC7A2    ; OR #n     OR #n
   fxdop c_FxOpC7A2
NEWSYM FxOpdC8A2    ; OR #n     OR #n
   fxdop c_FxOpC8A2
NEWSYM FxOpdC9A2    ; OR #n     OR #n
   fxdop c_FxOpC9A2
NEWSYM FxOpdCAA2    ; OR #n     OR #n
   fxdop c_FxOpCAA2
NEWSYM FxOpdCBA2    ; OR #n     OR #n
   fxdop c_FxOpCBA2
NEWSYM FxOpdCCA2    ; OR #n     OR #n
   fxdop c_FxOpCCA2
NEWSYM FxOpdCDA2    ; OR #n     OR #n
   fxdop c_FxOpCDA2
NEWSYM FxOpdCEA2    ; OR #n     OR #n
   fxdop c_FxOpCEA2
NEWSYM FxOpdCFA2    ; OR #n     OR #n
   fxdop c_FxOpCFA2
NEWSYM FxOpdC1A3    ; XOR #n    xor #n
   fxdop c_FxOpC1A3
NEWSYM FxOpdC2A3    ; XOR #n    xor #n
   fxdop c_FxOpC2A3
NEWSYM FxOpdC3A3    ; XOR #n    xor #n
   fxdop c_FxOpC3A3
NEWSYM FxOpdC4A3    ; XOR #n    xor #n
   fxdop c_FxOpC4A3
NEWSYM FxOpdC5A3    ; XOR #n    xor #n
   fxdop c_FxOpC5A3
NEWSYM FxOpdC6A3    ; XOR #n    xor #n
   fxdop c_FxOpC6A3
NEWSYM FxOpdC7A3    ; XOR #n    xor #n
   fxdop c_FxOpC7A3
NEWSYM FxOpdC8A3    ; XOR #n    xor #n
   fxdop c_FxOpC8A3
NEWSYM FxOpdC9A3    ; XOR #n    xor #n
   fxdop c_FxOpC9A3
NEWSYM FxOpdCAA3    ; XOR #n    xor #n
   fxdop c_FxOpCAA3
NEWSYM FxOpdCBA3    ; XOR #n    xor #n
   fxdop c_FxOpCBA3
NEWSYM FxOpdCCA3    ; XOR #n    xor #n
   fxdop c_FxOpCCA3
NEWSYM FxOpdCDA3    ; XOR #n    xor #n
   fxdop c_FxOpCDA3
NEWSYM FxOpdCEA3    ; XOR #n    xor #n
   fxdop c_FxOpCEA3
NEWSYM FxOpdCFA3    ; XOR #n    xor #n
   fxdop c_FxOpCFA3
NEWSYM FxOpdD0      ; INC rn    increase by one
   fxdop c_FxOpD0
NEWSYM FxOpdD1      ; INC rn    increase by one
   fxdop c_FxOpD1
NEWSYM FxOpdD2      ; INC rn    increase by one
   fxdop c_FxOpD2
NEWSYM FxOpdD3      ; INC rn    increase by one
   fxdop c_FxOpD3
NEWSYM FxOpdD4      ; INC rn    increase by one
   fxdop c_FxOpD4
NEWSYM FxOpdD5      ; INC rn    increase by one
   fxdop c_FxOpD5
NEWSYM FxOpdD6      ; INC rn    increase by one
   fxdop c_FxOpD6
NEWSYM FxOpdD7      ; INC rn    increase by one
   fxdop c_FxOpD7
NEWSYM FxOpdD8      ; INC rn    increase by one
   fxdop c_FxOpD8
NEWSYM FxOpdD9      ; INC rn    increase by one
   fxdop c_FxOpD9
NEWSYM FxOpdDA      ; INC rn    increase by one
   fxdop c_FxOpDA
NEWSYM FxOpdDB      ; INC rn    increase by one
   fxdop c_FxOpDB
NEWSYM FxOpdDC      ; INC rn    increase by one
   fxdop c_FxOpDC
NEWSYM FxOpdDD      ; INC rn    increase by one
   fxdop c_FxOpDD
NEWSYM FxOpdDE      ; INC rn    increase by one
   fxdop c_FxOpDE
NEWSYM FxOpdDF      ; GETC      transfer ROM buffer to color register
   fxdop c_FxOpDF
NEWSYM FxOpdDFA2    ; RAMB      set current RAM bank    ; Verified
   fxdop c_FxOpDFA2
NEWSYM FxOpdDFA3    ; ROMB      set current ROM bank    ; Verified
   fxdop c_FxOpDFA3
NEWSYM FxOpdE0      ; DEC rn    decrement by one
   fxdop c_FxOpE0
NEWSYM FxOpdE1      ; DEC rn    decrement by one
   fxdop c_FxOpE1
NEWSYM FxOpdE2      ; DEC rn    decrement by one
   fxdop c_FxOpE2
NEWSYM FxOpdE3      ; DEC rn    decrement by one
   fxdop c_FxOpE3
NEWSYM FxOpdE4      ; DEC rn    decrement by one
   fxdop c_FxOpE4
NEWSYM FxOpdE5      ; DEC rn    decrement by one
   fxdop c_FxOpE5
NEWSYM FxOpdE6      ; DEC rn    decrement by one
   fxdop c_FxOpE6
NEWSYM FxOpdE7      ; DEC rn    decrement by one
   fxdop c_FxOpE7
NEWSYM FxOpdE8      ; DEC rn    decrement by one
   fxdop c_FxOpE8
NEWSYM FxOpdE9      ; DEC rn    decrement by one
   fxdop c_FxOpE9
NEWSYM FxOpdEA      ; DEC rn    decrement by one
   fxdop c_FxOpEA
NEWSYM FxOpdEB      ; DEC rn    decrement by one
   fxdop c_FxOpEB
NEWSYM FxOpdEC      ; DEC rn    decrement by one
   fxdop c_FxOpEC
NEWSYM FxOpdED      ; DEC rn    decrement by one
   fxdop c_FxOpED
NEWSYM FxOpdEE      ; DEC rn    decrement by one
   fxdop c_FxOpEE
NEWSYM FxOpdEF      ; getb      get byte from ROM at address R14        ; V
   fxdop c_FxOpEF
NEWSYM FxOpdEFA1    ; getbh     get high-byte from ROM at address R14   ; V
   fxdop c_FxOpEFA1
NEWSYM FxOpdEFA2    ; getbl     get low-byte from ROM at address R14    ; V
   fxdop c_FxOpEFA2
NEWSYM FxOpdEFA3    ; getbs     get sign extended byte from ROM at address R14  ; V
   fxdop c_FxOpEFA3
NEWSYM FxOpdF0      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF0
NEWSYM FxOpdF1      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF1
NEWSYM FxOpdF2      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF2
NEWSYM FxOpdF3      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF3
NEWSYM FxOpdF4      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF4
NEWSYM FxOpdF5      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF5
NEWSYM FxOpdF6      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF6
NEWSYM FxOpdF7      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF7
NEWSYM FxOpdF8      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF8
NEWSYM FxOpdF9      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpF9
NEWSYM FxOpdFA      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFA
NEWSYM FxOpdFB      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFB
NEWSYM FxOpdFC      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFC
NEWSYM FxOpdFD      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFD
NEWSYM FxOpdFE      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFE
NEWSYM FxOpdFF      ; IWT RN,#xx   immediate word transfer to register
   fxdop c_FxOpFF
NEWSYM FxOpdF0A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF0A1
NEWSYM FxOpdF1A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF1A1
NEWSYM FxOpdF2A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF2A1
NEWSYM FxOpdF3A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF3A1
NEWSYM FxOpdF4A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF4A1
NEWSYM FxOpdF5A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF5A1
NEWSYM FxOpdF6A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF6A1
NEWSYM FxOpdF7A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF7A1
NEWSYM FxOpdF8A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF8A1
NEWSYM FxOpdF9A1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpF9A1
NEWSYM FxOpdFAA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFAA1
NEWSYM FxOpdFBA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFBA1
NEWSYM FxOpdFCA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFCA1
NEWSYM FxOpdFDA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFDA1
NEWSYM FxOpdFEA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFEA1
NEWSYM FxOpdFFA1    ; LM RN,(XX)   load word from RAM
   fxdop c_FxOpFFA1
NEWSYM FxOpdF0A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF0A2
NEWSYM FxOpdF1A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF1A2
NEWSYM FxOpdF2A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF2A2
NEWSYM FxOpdF3A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF3A2
NEWSYM FxOpdF4A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF4A2
NEWSYM FxOpdF5A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF5A2
NEWSYM FxOpdF6A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF6A2
NEWSYM FxOpdF7A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF7A2
NEWSYM FxOpdF8A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF8A2
NEWSYM FxOpdF9A2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpF9A2
NEWSYM FxOpdFAA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFAA2
NEWSYM FxOpdFBA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFBA2
NEWSYM FxOpdFCA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFCA2
NEWSYM FxOpdFDA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFDA2
NEWSYM FxOpdFEA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFEA2
NEWSYM FxOpdFFA2    ; SM (XX),RN   store word in RAM
   fxdop c_FxOpFFA2