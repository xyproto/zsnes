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


; Body of an opcode handler that has been ported to C (cpu/spc_ops.h). ebp is
; the SPC program counter; the C function takes it and returns the updated one.
%macro spccop 1
    ccall %1, ebp
    mov ebp, eax
    ret
%endmacro

EXTSYM DSPMem,cycpbl,SPCWriteReg,SPCReadReg,UpdateTimer,SpcOpInvalid
EXTSYM SpcOp00,SpcOp02,SpcOp22,SpcOp42,SpcOp62,SpcOp82,SpcOpA2,SpcOpC2
EXTSYM SpcOpE2,SpcOp12,SpcOp32,SpcOp52,SpcOp72,SpcOp92,SpcOpB2,SpcOpD2
EXTSYM SpcOpF2,SpcOp03,SpcOp23,SpcOp43,SpcOp63,SpcOp83,SpcOpA3,SpcOpC3
EXTSYM SpcOpE3,SpcOp13,SpcOp33,SpcOp53,SpcOp73,SpcOp93,SpcOpB3,SpcOpD3
EXTSYM SpcOpF3
EXTSYM SpcOp10,SpcOp30,SpcOp50,SpcOp70,SpcOp90,SpcOpB0,SpcOpD0,SpcOpF0
EXTSYM SpcOp2F,SpcOp01,SpcOp11,SpcOp21,SpcOp31,SpcOp41,SpcOp51,SpcOp61
EXTSYM SpcOp71,SpcOp81,SpcOp91,SpcOpA1,SpcOpB1,SpcOpC1,SpcOpD1,SpcOpE1
EXTSYM SpcOpF1
EXTSYM SpcOp04,SpcOp14,SpcOp05,SpcOp15,SpcOp06,SpcOp16,SpcOp07,SpcOp17
EXTSYM SpcOp08,SpcOp24,SpcOp34,SpcOp25,SpcOp35,SpcOp26,SpcOp36,SpcOp27
EXTSYM SpcOp37,SpcOp28,SpcOp44,SpcOp54,SpcOp45,SpcOp55,SpcOp46,SpcOp56
EXTSYM SpcOp47,SpcOp57,SpcOp48,SpcOp64,SpcOp74,SpcOp65,SpcOp75,SpcOp66
EXTSYM SpcOp76,SpcOp67,SpcOp77,SpcOp68,SpcOpE4,SpcOpF4,SpcOpE5,SpcOpF5
EXTSYM SpcOpE6,SpcOpF6,SpcOpE7,SpcOpF7,SpcOpE8,SpcOp20,SpcOp40,SpcOp60
EXTSYM SpcOp80,SpcOpA0,SpcOpC0,SpcOpE0,SpcOpED,SpcOpBD
EXTSYM SpcOp84,SpcOp94,SpcOp85,SpcOp95,SpcOp86,SpcOp96,SpcOp87,SpcOp97
EXTSYM SpcOp88,SpcOpA4,SpcOpB4,SpcOpA5,SpcOpB5,SpcOpA6,SpcOpB6,SpcOpA7
EXTSYM SpcOpB7,SpcOpA8,SpcOp18,SpcOp38,SpcOp58,SpcOp98,SpcOpB8,SpcOp09
EXTSYM SpcOp29,SpcOp49,SpcOp89,SpcOpA9,SpcOp19,SpcOp39,SpcOp59,SpcOp99
EXTSYM SpcOpB9,SpcOp78,SpcOp69,SpcOp79,SpcOpFA,SpcOpC4,SpcOpD4,SpcOpC5
EXTSYM SpcOpD5,SpcOpD6,SpcOpC6,SpcOpD8,SpcOpD9,SpcOpC9,SpcOpCB,SpcOpDB
EXTSYM SpcOpCC,SpcOpC7,SpcOpD7,SpcOpF8,SpcOpF9,SpcOpE9,SpcOpEB,SpcOpFB
EXTSYM SpcOpEC,SpcOpDD,SpcOpFD,SpcOpAF,SpcOpBF,SpcOpFE,SpcOp9F,SpcOpEF
EXTSYM SpcOpFF,SpcOp0F
EXTSYM SpcOp5D,SpcOp7D,SpcOp9D,SpcOp8D,SpcOpCD,SpcOp9C,SpcOpBC,SpcOpDC
EXTSYM SpcOpFC,SpcOp1D,SpcOp3D,SpcOp8F,SpcOpC8,SpcOpAD,SpcOp3E,SpcOp7E
EXTSYM SpcOp1E,SpcOp5E,SpcOp8B,SpcOpAB,SpcOp9B,SpcOpBB,SpcOp8C,SpcOpAC
EXTSYM SpcOp0B,SpcOp4B,SpcOp1B,SpcOp5B,SpcOp0C,SpcOp4C,SpcOp1C,SpcOp5C
EXTSYM SpcOp0E,SpcOp4E,SpcOp2D,SpcOp4D,SpcOp6D,SpcOpAE,SpcOpCE,SpcOpEE
EXTSYM SpcOp0D,SpcOp5F,SpcOp1F,SpcOp3F,SpcOp4F,SpcOp6F,SpcOp7F,SpcOp2E
EXTSYM SpcOpDE,SpcOp6E,SpcOpCF
EXTSYM SpcOp1A,SpcOp3A,SpcOp5A,SpcOp7A,SpcOp9A,SpcOpBA,SpcOpDA,SpcOp8E
EXTSYM SpcOp0A,SpcOp2A,SpcOp4A,SpcOp6A,SpcOp8A,SpcOpAA,SpcOpCA,SpcOpEA
EXTSYM SpcOp2B,SpcOp3B,SpcOp2C,SpcOp6B,SpcOp7B,SpcOp6C,SpcOp3C,SpcOp7C
EXTSYM SpcOp9E,SpcOpBE,SpcOpDF
EXTSYM spc700read,curexecstate,tableadc

%include "cpu/regsw.mac"

; SPC 700 Emulation by _Demo_
; Version 2.0

; Little info on functions :
; Write byte : write al at [ebx]
; Read byte : read al from [ebx]
; update timer : update the timers, called every scanline

SECTION .data

ALIGN32

;spcBuffer times 65536*4 db 0    ; The buffer of brr blocks... 4 bits -> 16 bits
;spcPrevbf times 65536   db 0    ; SPC PrevX compare buffer
NEWSYM SPCRAM,   times 65472 db 0FFh
; copy #1
; THE SPC ROM :)
   db 0CDh,0EFh,0BDh,0E8h,000h,0C6h,01Dh,0D0h,0FCh,08Fh,0AAh,0F4h,08Fh,0BBh,0F5h,078h
   db 0CCh,0F4h,0D0h,0FBh,02Fh,019h,0EBh,0F4h,0D0h,0FCh,07Eh,0F4h,0D0h,00Bh,0E4h,0F5h
   db 0CBh,0F4h,0D7h,000h,0FCh,0D0h,0F3h,0ABh,001h,010h,0EFh,07Eh,0F4h,010h,0EBh,0BAh
   db 0F6h,0DAh,000h,0BAh,0F4h,0C4h,0F4h,0DDh,05Dh,0D0h,0DBh,01Fh,000h,000h,0C0h,0FFh
   db 0AAh,0BBh,0CCh,0DDh,0EEh,0FFh,000h,011h,022h,033h,044h,055h,066h,077h,088h,099h

NEWSYM spcPCRam,
       dd 0
NEWSYM spcA,
       db 0
       db 0
       db 0
       db 0
NEWSYM spcX,
       db 0
       db 0
       db 0
       db 0
NEWSYM spcY,
       db 0
       db 0
       db 0
       db 0
NEWSYM spcP,
       db 0
       db 0
       db 0
       db 0
NEWSYM spcNZ,
       db 0
       db 0
       db 0
       db 0


;spcNF    db 0     ; The Negative Flag  128 or 127
;spcOF    db 0     ; The Overflow Flag   64 or 191
;spcDPF   db 0     ; Direct Page Flag    32 or 223
;spcUF    db 0     ; The Unused Flag ?   16 or 239
;spcHCF   db 0     ; The Half Carry Flag  8 or 247
;spcIF    db 0     ; The interrupt flag   4 or 251
;spcZF    db 0     ; The Zero Flag      2 or 253
;spcCF    db 0     ; The Carry Flag     1 or 254

NEWSYM spcS,     dd 1FFh
NEWSYM spcRamDP, dd 0     ; The direct page pointer
NEWSYM spcCycle, dd 0     ; The Cycle Counter
NEWSYM reg1read, db 0     ; read from 65816
NEWSYM reg2read, db 0     ; read from 65816
NEWSYM reg3read, db 0     ; read from 65816
NEWSYM reg4read, db 0     ; read from 65816
NEWSYM timeron,  db 0     ; timer0 on
NEWSYM timincr0, db 0     ; # of ticks before incrementing
NEWSYM timincr1, db 0     ; # of ticks before incrementing
NEWSYM timincr2, db 0     ; # of ticks before incrementing
NEWSYM timinl0,  db 0     ; ticks left before incrementing
NEWSYM timinl1,  db 0     ; ticks left before incrementing
NEWSYM timinl2,  db 0     ; ticks left before incrementing
NEWSYM timrcall, db 0     ; alternating bit 0 to correctly timer timer1 & 2 to 8000hz

NEWSYM spcextraram, times 64 db 0 ; extra ram, used for tcall

NEWSYM FutureExpandS,  times 256-64 db 0

spcsave equ $-SPCRAM
; pharos equ hack *sigh*
NEWSYM PHspcsave, dd spcsave

; copy #2
NEWSYM SPCROM
   db 0CDh,0EFh,0BDh,0E8h,000h,0C6h,01Dh,0D0h,0FCh,08Fh,0AAh,0F4h,08Fh,0BBh,0F5h,078h
   db 0CCh,0F4h,0D0h,0FBh,02Fh,019h,0EBh,0F4h,0D0h,0FCh,07Eh,0F4h,0D0h,00Bh,0E4h,0F5h
   db 0CBh,0F4h,0D7h,000h,0FCh,0D0h,0F3h,0ABh,001h,010h,0EFh,07Eh,0F4h,010h,0EBh,0BAh
   db 0F6h,0DAh,000h,0BAh,0F4h,0C4h,0F4h,0DDh,05Dh,0D0h,0DBh,01Fh,000h,000h,0C0h,0FFh

SECTION .text




SECTION .data
NEWSYM timer2upd, dd 0
SECTION .text

; This function is called every scanline (262*60 times/sec)
; Make it call 0.9825 times (393/400) (skip when divisible by 64)
; 2 8khz, 1 64khz

NEWSYM updatetimer
    push edi                  ; reenablespc may retarget the 65816 opcode table
    mov eax, esp
    ccall UpdateTimer, edx, eax
    pop edi
    ret

SECTION .data
NEWSYM spcnumread, db 0
SECTION .text





;************************************************
; Misc Opcodes
;************************************************
NEWSYM Op00     ; NOP
    spccop SpcOp00
NEWSYM OpEF     ; SLEEP      standby SLEEP mode    .........
    spccop SpcOpEF
NEWSYM OpFF     ; STOP       standby STOP mode     .........
    spccop SpcOpFF
NEWSYM Op9F     ; XCN A     A(7-4) <-> A(3-0)     N......Z.
    spccop SpcOp9F

;************************************************
; Branch Stuff
;************************************************
NEWSYM Op10     ; BPL Branch on N=0
    spccop SpcOp10
NEWSYM Op30     ; BMI Branch on N=1
    spccop SpcOp30
NEWSYM Op50     ; BVC Branch on V=0
    spccop SpcOp50
NEWSYM Op70     ; BVS Branch on V=1
    spccop SpcOp70
NEWSYM Op90     ; BCC Branc on c=0
    spccop SpcOp90
NEWSYM OpB0     ; BCS Branch on C=1
    spccop SpcOpB0
NEWSYM OpD0     ; BNE branch on Z=0
    spccop SpcOpD0
NEWSYM OpF0     ; BEQ Branch on Z=1
    spccop SpcOpF0
NEWSYM Op2F     ; BRA rel    branch always            ...
    spccop SpcOp2F


;************************************************
; Clear/Set Flag bits
;************************************************
;  CLRP           20    1     2   clear direct page flag    ..0.....
NEWSYM Op20     ; CLRP Clear direct page flag
    spccop SpcOp20
;  SETP           40    1     2   set dorect page flag    ..1..0..
NEWSYM Op40     ; SETP Set Direct Page Flag  (Also clear interupt flag?)
    spccop SpcOp40
;  CLRC           60    1     2   clear carry flag        .......0
NEWSYM Op60     ; CLRC Clear carry flag
    spccop SpcOp60
;  SETC           80    1     2   set carry flag        .......1
NEWSYM Op80     ; SETC Set carry flag
    spccop SpcOp80
;  EI             A0    1     3  set interrup enable flag   .....1..
NEWSYM OpA0     ; EI set interrupt flag
    spccop SpcOpA0
;  DI             C0    1     3  clear interrup enable flag .....0..
NEWSYM OpC0     ; DI clear interrupt flag
    spccop SpcOpC0
;  CLRV           E0    1     2   clear V and H         .0..0...
NEWSYM OpE0     ; CLRV clear V and H
    spccop SpcOpE0
;  NOTC           ED    1     3   complement carry flag     .......C
NEWSYM OpED     ; NOTC       complement carry flag     .......C
    spccop SpcOpED

;************************************************
; TCALL instructions (Verified)
;************************************************
NEWSYM Op01     ; TCALL 0
    spccop SpcOp01
NEWSYM Op11     ; TCALL 1
    spccop SpcOp11
NEWSYM Op21     ; TCALL 2
    spccop SpcOp21
NEWSYM Op31     ; TCALL 3
    spccop SpcOp31
NEWSYM Op41     ; TCALL 4
    spccop SpcOp41
NEWSYM Op51     ; TCALL 5
    spccop SpcOp51
NEWSYM Op61     ; TCALL 6
    spccop SpcOp61
NEWSYM Op71     ; TCALL 7
    spccop SpcOp71
NEWSYM Op81     ; TCALL 8
    spccop SpcOp81
NEWSYM Op91     ; TCALL 9
    spccop SpcOp91
NEWSYM OpA1     ; TCALL A
    spccop SpcOpA1
NEWSYM OpB1     ; TCALL B
    spccop SpcOpB1
NEWSYM OpC1     ; TCALL C
    spccop SpcOpC1
NEWSYM OpD1     ; TCALL D
    spccop SpcOpD1
NEWSYM OpE1     ; TCALL E
    spccop SpcOpE1
NEWSYM OpF1     ; TCALL F
    spccop SpcOpF1

;************************************************
; SET1 instructions (Verified)
;************************************************
NEWSYM Op02     ; SET1 direct page bit 0
    spccop SpcOp02
NEWSYM Op22     ; SET1 direct page bit 1
    spccop SpcOp22
NEWSYM Op42     ; SET1 direct page bit 2
    spccop SpcOp42
NEWSYM Op62     ; SET1 direct page bit 3
    spccop SpcOp62
NEWSYM Op82     ; SET1 direct page bit 4
    spccop SpcOp82
NEWSYM OpA2     ; SET1 direct page bit 5
    spccop SpcOpA2
NEWSYM OpC2     ; SET1 direct page bit 6
    spccop SpcOpC2
NEWSYM OpE2     ; SET1 direct page bit 7
    spccop SpcOpE2

;************************************************
; CLR1 instructions (Verified)
;************************************************
NEWSYM Op12     ; CLR1 direct page bit 0
    spccop SpcOp12
NEWSYM Op32     ; CLR1 direct page bit 1
    spccop SpcOp32
NEWSYM Op52     ; CLR1 direct page bit 2
    spccop SpcOp52
NEWSYM Op72     ; CLR1 direct page bit 3
    spccop SpcOp72
NEWSYM Op92     ; CLR1 direct page bit 4
    spccop SpcOp92
NEWSYM OpB2     ; CLR1 direct page bit 5
    spccop SpcOpB2
NEWSYM OpD2     ; CLR1 direct page bit 6
    spccop SpcOpD2
NEWSYM OpF2     ; CLR1 direct page bit 7
    spccop SpcOpF2

;************************************************
; BBS instructions (Verified)
;************************************************
NEWSYM Op03     ; BBS direct page bit 0
    spccop SpcOp03
NEWSYM Op23     ; BBS direct page bit 1
    spccop SpcOp23
NEWSYM Op43     ; BBS direct page bit 2
    spccop SpcOp43
NEWSYM Op63     ; BBS direct page bit 3
    spccop SpcOp63
NEWSYM Op83     ; BBS direct page bit 4
    spccop SpcOp83
NEWSYM OpA3     ; BBS direct page bit 5
    spccop SpcOpA3
NEWSYM OpC3     ; BBS direct page bit 6
    spccop SpcOpC3
NEWSYM OpE3     ; BBS direct page bit 7
    spccop SpcOpE3

;************************************************
; BBC instructions (Verified)
;************************************************
NEWSYM Op13     ; BBC direct page bit 0
    spccop SpcOp13
NEWSYM Op33     ; BBC direct page bit 1
    spccop SpcOp33
NEWSYM Op53     ; BBC direct page bit 2
    spccop SpcOp53
NEWSYM Op73     ; BBC direct page bit 3
    spccop SpcOp73
NEWSYM Op93     ; BBC direct page bit 4
    spccop SpcOp93
NEWSYM OpB3     ; BBC direct page bit 5
    spccop SpcOpB3
NEWSYM OpD3     ; BBC direct page bit 6
    spccop SpcOpD3
NEWSYM OpF3     ; BBC direct page bit 7
    spccop SpcOpF3

;************************************************
; OR A,instructions
;************************************************
NEWSYM Op04     ; OR A,dp   A <- A OR (dp)    N.....Z.
    spccop SpcOp04
NEWSYM Op14     ; OR A,dp+X    A <- A OR (dp+X)     N.....Z.
    spccop SpcOp14
NEWSYM Op05     ; OR A,labs   A <- A OR (abs)     N.....Z.
    spccop SpcOp05
NEWSYM Op15     ; OR A,labs+x  A <- A OR (abs+X)    N.....Z.
    spccop SpcOp15
NEWSYM Op06     ; OR A,(X)     A <- A OR (X)      N.....Z.
    spccop SpcOp06
NEWSYM Op16     ; OR A,labs+Y  A <- A OR (abs+Y)    N......Z.
    spccop SpcOp16
NEWSYM Op07     ; OR A,(dp+X)  A <- A OR ((dp+X+1)(dp+X))  N......Z.
    spccop SpcOp07
NEWSYM Op17     ; OR A,(dp)+Y  A <- A OR ((dp+1)(dp)+Y)   N......Z.
    spccop SpcOp17
NEWSYM Op08     ; OR A,#inm    A <- A OR inm        N......Z.
    spccop SpcOp08

;************************************************
; AND A, instructions
;************************************************
NEWSYM Op24     ; AND A,dp     A <- A AND (dp)    N.....Z.
    spccop SpcOp24
NEWSYM Op34     ; AND A,dp+x   A <- A AND (dp+X)    N.....Z.
    spccop SpcOp34
NEWSYM Op25     ; AND A,labs   A <- A AND (abs)     N.....Z.
    spccop SpcOp25
NEWSYM Op35     ; AND A,labs+X A <- A AND (abs+X)   N.....Z.
    spccop SpcOp35
NEWSYM Op26     ; AND A,(X)    A <- A AND (X)     N......Z.
    spccop SpcOp26
NEWSYM Op36     ; AND A,labs+Y A <- A AND (abs+Y)   N......Z.
    spccop SpcOp36
NEWSYM Op27     ; AND A,(dp+X) A <- A AND ((dp+X+1)(dp+X)) N......Z.
    spccop SpcOp27
NEWSYM Op37     ; AND A,(dp)+Y A <- A AND ((dp+1)(dp)+Y)  N......Z.
    spccop SpcOp37
NEWSYM Op28     ; AND A,#inm   A <- A AND inm         N......Z.
    spccop SpcOp28

;************************************************
; EOR A, instructions
;************************************************
NEWSYM Op44     ; EOR A,dp     A <- A EOR (dp)    N.....Z.
    spccop SpcOp44
NEWSYM Op54     ; EOR A,dp+x   A <- A EOR (dp+X)    N.....Z.
    spccop SpcOp54
NEWSYM Op45     ; EOR A,labs   A <- A EOR (abs)     N.....Z.
    spccop SpcOp45
NEWSYM Op55     ; EOR A,labs+X A <- A EOR (abs+X)   N.....Z.
    spccop SpcOp55
NEWSYM Op46     ; EOR A,(X)    A <- A EOR (X)     N......Z.
    spccop SpcOp46
NEWSYM Op56     ; EOR A,labs+Y A <- A EOR (abs+Y)   N......Z.
    spccop SpcOp56
NEWSYM Op47     ; EOR A,(dp+X) A <- A EOR ((dp+X+1)(dp+X)) N......Z.
    spccop SpcOp47
NEWSYM Op57     ; EOR A,(dp)+Y A <- A EOR ((dp+1)(dp)+Y)  N......Z.
    spccop SpcOp57
NEWSYM Op48     ; EOR A,#inm   A <- A EOR inm         N......Z.
    spccop SpcOp48

;************************************************
; CMP A, instructions
;************************************************
NEWSYM Op64     ; CMP A,dp     A-(dp)           N.....ZC
    spccop SpcOp64
NEWSYM Op74     ; CMP A,dp+x   A-(dp+X)         N.....ZC
    spccop SpcOp74
NEWSYM Op65     ; CMP A,labs   A-(abs)          N.....ZC
    spccop SpcOp65
NEWSYM Op75     ; CMP A,labs+X A-(abs+X)        N.....ZC
    spccop SpcOp75
NEWSYM Op66     ; CMP A,(X)    A-(X)            N......ZC
    spccop SpcOp66
NEWSYM Op76     ; CMP A,labs+Y A-(abs+Y)        N......ZC
    spccop SpcOp76
NEWSYM Op67     ; CMP A,(dp+X) A-((dp+X+1)(dp+X))    N......ZC
    spccop SpcOp67
NEWSYM Op77     ; CMP A,(dp)+Y A-((dp+1)(dp)+Y)      N......ZC
    spccop SpcOp77
NEWSYM Op68     ; CMP A,#inm   A-inm             N......ZC
    spccop SpcOp68

;************************************************
; ADC A, instructions
;************************************************
NEWSYM Op84     ; ADC A,dp     A <- A+(dp)+C      NV..H.ZC
    spccop SpcOp84
NEWSYM Op94     ; ADC A,dp+x   A <- A+(dp+X)+C    NV..H.ZC
    spccop SpcOp94
NEWSYM Op85     ; ADC A,labs   A <- A+(abs)+C     NV..H.ZC
    spccop SpcOp85
NEWSYM Op95     ; ADC A,labs+X A <- A+(abs+X)+C     NV..H.ZC
    spccop SpcOp95
NEWSYM Op86     ; ADC A,(X)    A <- A+(X)+C       NV..H..ZC
    spccop SpcOp86
NEWSYM Op96     ; ADC A,labs+Y A <- A+(abs+Y)+C     NV..H..ZC
    spccop SpcOp96
NEWSYM Op87     ; ADC A,(dp+X) A <- A+((dp+X+1)(dp+X)) NV..H..ZC
    spccop SpcOp87
NEWSYM Op97     ; ADC A,(dp)+Y A <- A+((dp+1)(dp)+Y)   NV..H..ZC
    spccop SpcOp97
NEWSYM Op88     ; ADC A,#inm   A <- A+inm+C        NV..H..ZC
    spccop SpcOp88

;************************************************
; SBC A, instructions
;************************************************
NEWSYM OpA4     ; SBC A,dp     A <- A-(dp)-!C     NV..H.ZC
    spccop SpcOpA4
NEWSYM OpB4     ; SBC A,dp+x   A <- A-(dp+X)-!C     NV..H.ZC
    spccop SpcOpB4
NEWSYM OpA5     ; SBC A,labs   A <- A-(abs)-!C    NV..H.ZC
    spccop SpcOpA5
NEWSYM OpB5     ; SBC A,labs+x A <- A-(abs+X)-!C    NV..H.ZC
    spccop SpcOpB5
NEWSYM OpA6     ; SBC A,(X)    A <- A-(X)-!C      NV..H..ZC
    spccop SpcOpA6
NEWSYM OpB6     ; SBC A,labs+Y A <- A-(abs+Y)-!C    NV..H..ZC
    spccop SpcOpB6
NEWSYM OpA7     ; SBC A,(dp+X) A <- A-((dp+X+1)(dp+X))-!C NV..H..ZC
    spccop SpcOpA7
NEWSYM OpB7     ; SBC A,(dp)+Y A <- A-((dp+1)(dp)+Y)-!C   NV..H..ZC
    spccop SpcOpB7
NEWSYM OpA8     ; SBC A,#inm   A <- A-inm-!C         NV..H..ZC
    spccop SpcOpA8

;************************************************
; MOV A, instructions
;************************************************
NEWSYM OpE4     ; MOV A,dp     A <- (dp)        N......Z
    spccop SpcOpE4
NEWSYM OpF4     ; MOV A,dp+x   A <- (dp+X)        N......Z
    spccop SpcOpF4
NEWSYM OpE5     ; MOV A,labs   A <- (abs)         N......Z
    spccop SpcOpE5
NEWSYM OpF5     ; MOV A,labs+X A <- (abs+X)       N......Z
    spccop SpcOpF5
NEWSYM OpE6     ; MOV A,(X)    A <- (X)         N......Z
    spccop SpcOpE6
NEWSYM OpF6     ; MOV A,labs+Y A <- (abs+Y)       N......Z
    spccop SpcOpF6
NEWSYM OpE7     ; MOV A,(dp+X) A <- ((dp+X+1)(dp+X))     N......Z
    spccop SpcOpE7
NEWSYM OpF7     ; MOV A,(dp)+Y A <- ((dp+1)(dp)+Y)     N......Z
    spccop SpcOpF7
NEWSYM OpE8     ;  MOV A,#inm  A <- inm            N......Z
    spccop SpcOpE8

;************************************************
; DP,#imm instructions
;************************************************


NEWSYM OpB8     ; SBC dp,#inm  (dp) <- (dp)-inm-!C      NV..H..ZC
    spccop SpcOpB8

NEWSYM Op98     ; ADC dp,#inm  (dp) <- (dp)+inm+C       NV..H..ZC
    spccop SpcOp98

NEWSYM Op78     ; CMP dp,#inm  (dp)-inm            N......ZC
    spccop SpcOp78

NEWSYM Op58    ; EOR dp,#inm  (dp) <- (dp) EOR inm      N......Z.
    spccop SpcOp58

NEWSYM Op38     ; AND dp,#inm  (dp) <- (dp) AND inm      N......Z.
    spccop SpcOp38

NEWSYM Op18     ; OR dp,#inm   (dp) <- (dp) OR inm       N......Z.
    spccop SpcOp18

;************************************************
; DP(D),DP(S) instructions
;************************************************

NEWSYM Op09     ; OR dp(d),dp(s)  (dp(d))<-(dp(d)) OR (dp(s))  N......Z.
    spccop SpcOp09

NEWSYM Op29     ; AND dp(d),dp(s) (dp(d))<-(dp(d)) AND (dp(s)) N......Z.
    spccop SpcOp29

NEWSYM Op49     ; EOR dp(d),dp(s) (dp(d))<-(dp(d)) EOR (dp(s)) N......Z.
    spccop SpcOp49

NEWSYM Op69     ; CMP dp(d),dp(s) (dp(d))-(dp(s))       N......ZC
    spccop SpcOp69

NEWSYM Op89     ; ADC dp(d),dp(s) (dp(d))<-(dp(d))+(dp(s))+C  NV..H..ZC
    spccop SpcOp89

NEWSYM OpA9     ; SBC dp(d),dp(s) (dp(d))<-(dp(d))-(dp(s))-!C NV..H..ZC
    spccop SpcOpA9

NEWSYM OpFA     ; MOV dp(d),dp(s) (dp(d)) <- (dp(s))      ........
    spccop SpcOpFA

;************************************************
; (X),(Y) instructions
;************************************************

NEWSYM Op19     ; OR (X),(Y)   (X) <- (X) OR (Y)        N......Z.
    spccop SpcOp19

NEWSYM Op39     ; AND (X),(Y)  (X) <- (X) AND (Y)       N......Z.
    spccop SpcOp39


NEWSYM Op59     ; EOR (X),(Y)  (X) <- (X) EOR (Y)       N......Z.
    spccop SpcOp59

NEWSYM Op79     ; CMP (X),(Y)  (X)-(Y)             N......ZC
    spccop SpcOp79

NEWSYM Op99     ; ADC (X),(Y)  (X) <- (X)+(Y)+C        NV..H..ZC
    spccop SpcOp99

NEWSYM OpB9     ; SBC (X),(Y)  (X) <- (X)-(Y)-!C       NV..H..ZC
    spccop SpcOpB9

;************************************************
; MOV ,A instructions (Verified)
;************************************************

NEWSYM OpC4     ; MOV dp,A     A -> (dp)        ........
    spccop SpcOpC4

NEWSYM OpD4     ; MOV dp+x,A   A -> (dp+X)        ........
    spccop SpcOpD4

NEWSYM OpC5     ; MOV labs,A   A -> (abs)         ........
    spccop SpcOpC5

NEWSYM OpD5     ; MOV labs+X,A A -> (abs+X)       ........
    spccop SpcOpD5

NEWSYM OpC6     ; MOV (X),A    A -> (X)         ........
    spccop SpcOpC6

NEWSYM OpD6     ; MOV labs+Y,A A -> (abs+Y)       ........
    spccop SpcOpD6

NEWSYM OpC7     ; MOV (dp+X),A A -> ((dp+X+1)(dp+X))     ........
    spccop SpcOpC7

NEWSYM OpD7     ; MOV (dp)+Y,A A -> ((dp+1)(dp)+Y)     ........
    spccop SpcOpD7

;************************************************
; MOV instructions (Verified)
;************************************************

NEWSYM OpD8     ; MOV dp,X     X -> (dp)             ........
    spccop SpcOpD8

NEWSYM OpF8     ;  MOV X,dp    X <- (dp)             N......Z
    spccop SpcOpF8

NEWSYM OpC9     ; MOV labs,X   X -> (abs)            ........
    spccop SpcOpC9

NEWSYM OpE9     ; MOV X,labs   X <- (abs)            N......Z
    spccop SpcOpE9

NEWSYM OpD9     ; MOV dp+Y,X   X -> (dp+Y)           ........
    spccop SpcOpD9

NEWSYM OpF9     ; MOV X,dp+Y   X <- (dp+Y)           N......Z
    spccop SpcOpF9

NEWSYM OpCB     ; MOV dp,Y  Y -> (dp)             ........
    spccop SpcOpCB

NEWSYM OpEB     ; MOV Y,dp  Y <- (dp)             N......Z
    spccop SpcOpEB

NEWSYM OpDB     ; MOV dp+X,Y   X -> (dp+X)           ........
    spccop SpcOpDB

NEWSYM OpFB     ; MOV Y,dp+X   Y <- (dp+X)           N......Z
    spccop SpcOpFB

NEWSYM OpCC     ; MOV labs,Y   Y -> (abs)            ........
    spccop SpcOpCC

NEWSYM OpEC     ; MOV Y,labs   Y <- (abs)            N......Z
    spccop SpcOpEC

NEWSYM Op5D     ; MOV X,A    X <- A             N......Z
    spccop SpcOp5D

NEWSYM Op7D     ; MOV A,X    A <- X             N......Z
    spccop SpcOp7D

NEWSYM Op8D     ; MOV Y,#inm   Y <- inm            N......Z
    spccop SpcOp8D

NEWSYM OpCD     ; MOV X,#inm   X <- inm            N......Z
    spccop SpcOpCD

NEWSYM Op8F     ; MOV dp,#inm  (dp) <- inm           ........
    spccop SpcOp8F

NEWSYM Op9D     ; MOV X,SP     X <- SP            N......Z
    spccop SpcOp9D

NEWSYM OpBD     ; MOV SP,X     SP <- X             ........
    spccop SpcOpBD

NEWSYM OpDD     ; MOV A,Y    A <- Y             N......Z
    spccop SpcOpDD


NEWSYM OpFD     ; MOV Y,A    Y <- A             N......Z
    spccop SpcOpFD

NEWSYM OpAF     ; MOV (X)+,A   A -> (X) with auto inc    ........
    spccop SpcOpAF

NEWSYM OpBF     ; MOV A,(X)+  A <- (X) with auto inc    N......Z
    spccop SpcOpBF


;************************************************
; CMP instructions (Verified)
;************************************************

NEWSYM OpC8     ; CMP X,#inm   X-inm             N......ZC
    spccop SpcOpC8

NEWSYM OpAD     ; CMP Y,#inm   Y-inm             N......ZC
    spccop SpcOpAD

NEWSYM Op1E     ; CMP X,labs   X-(abs)             N......ZC
    spccop SpcOp1E

NEWSYM Op3E     ; CMP X,dp     X-(dp)            N......ZC
    spccop SpcOp3E

NEWSYM Op5E     ; CMP Y,labs   Y-(abs)             N......ZC
    spccop SpcOp5E

NEWSYM Op7E     ; CMP Y,dp     Y-(dp)            N......ZC
    spccop SpcOp7E

;************************************************
; Word Instructions (Verified)
;************************************************

NEWSYM Op1A     ; DECW dp   Decrement dp memory pair  N......Z.
    spccop SpcOp1A
NEWSYM Op3A     ; INCW dp   Increment dp memory pair  N......Z.
    spccop SpcOp3A
; looks like there is the Carry flag checked in op5a..

NEWSYM Op5A     ; CMPW YA,dp   YA - (dp+1)(dp)      N......ZC
    spccop SpcOp5A
NEWSYM Op7A     ; ADDW YA,dp   YA  <- YA + (dp+1)(dp)   NV..H..ZC
    spccop SpcOp7A
NEWSYM Op9A     ; SUBW YA,dp   YA  <- YA - (dp+1)(dp)   NV..H..ZC
    spccop SpcOp9A
NEWSYM OpBA     ; MOVW YA,dp   YA  - (dp+1)(dp)     N......Z.
    spccop SpcOpBA

NEWSYM OpDA     ; MOVW dp,YA   (dp+1)(dp) - YA       .........
    spccop SpcOpDA

;************************************************
; mem.bit instructions (Verified)
;************************************************


NEWSYM Op0A     ; OR1 C,mem.bit   C <- C OR  (mem.bit)    ........C
    spccop SpcOp0A

NEWSYM Op2A     ; OR1 C,/mem.bit  C <- C OR  !(mem.bit)     ........C
    spccop SpcOp2A

NEWSYM Op4A     ; AND1 C,mem.bit  C <- C AND (mem.bit)    ........C
    spccop SpcOp4A

NEWSYM Op6A     ; AND1 C,/mem.bit C <- C AND !(mem.bit)     ........C
    spccop SpcOp6A

NEWSYM Op8A     ; EOR1 C,mem.bit  C <- C EOR (mem.bit)    ........C
    spccop SpcOp8A

NEWSYM OpAA     ; MOV1 C,mem.bit  C <- (mem.bit)
    spccop SpcOpAA

NEWSYM OpCA     ; MOV1 mem.bit,C  C -> (mem.bit)        .........
    spccop SpcOpCA

NEWSYM OpEA     ; NOT1 mem.bit    complement (mem.bit)    .........
    spccop SpcOpEA

;************************************************
; Shift Instructions (Verified)
;************************************************

NEWSYM Op0B     ; ASL dp    C << (dp)   <<0     N......ZC
    spccop SpcOp0B

NEWSYM Op4B     ; LSR dp    0 >> (dp)   <<C     N......ZC
    spccop SpcOp4B

NEWSYM Op1B     ; ASL dp+X  C << (dp+X) <<0     N......ZC
    spccop SpcOp1B

NEWSYM Op5B     ; LSR dp+X  0 >> (dp+X) <<C     N......ZC
    spccop SpcOp5B

NEWSYM Op0C     ; ASL labs  C << (abs)  <<0     N......ZC
    spccop SpcOp0C

NEWSYM Op4C     ; LSR labs  0 >> (abs)  <<C     N......ZC
    spccop SpcOp4C

NEWSYM Op1C     ; ASL A  C << A    <<0     N......ZC
    spccop SpcOp1C

NEWSYM Op5C     ; LSR A  0 >> A    <<C     N......ZC
    spccop SpcOp5C



NEWSYM Op2B     ; ROL dp    C << (dp)   <<C     N......ZC
    spccop SpcOp2B

NEWSYM Op6B     ; ROR dp    C >> (dp)   <<C     N......ZC
    spccop SpcOp6B

NEWSYM Op3B     ; ROL dp+X  C << (dp+X) <<C     N......ZC
    spccop SpcOp3B

NEWSYM Op7B     ; ROR dp+X  C >> (dp+X) <<C     N......ZC
    spccop SpcOp7B

NEWSYM Op2C     ; ROL labs  C << (abs)  <<C     N......ZC
    spccop SpcOp2C

NEWSYM Op6C     ; ROR labs  C >> (abs)  <<C     N......ZC
    spccop SpcOp6C

NEWSYM Op3C     ; ROL A  C << A    <<C     N......ZC
    spccop SpcOp3C

NEWSYM Op7C     ; ROR A  C >> A    <<C     N......ZC
    spccop SpcOp7C

;************************************************
; INC/DEC instructions (Verified)
;************************************************

NEWSYM Op8B     ;  DEC dp   -- (dp)           N......Z.
    spccop SpcOp8B

NEWSYM OpAB     ; INC dp    ++ (dp)           N......Z.
    spccop SpcOpAB

NEWSYM Op9B     ;  DEC dp+X -- (dp+X)         N......Z.
    spccop SpcOp9B

NEWSYM OpBB     ; INC dp+X  ++ (dp+X)         N......Z.
    spccop SpcOpBB

NEWSYM Op8C     ; DEC labs  -- (abs)          N......Z.
    spccop SpcOp8C

NEWSYM OpAC     ; INC labs  ++ (abs)          N......Z.
    spccop SpcOpAC

NEWSYM Op9C     ; DEC A  -- A            N......Z.
    spccop SpcOp9C

NEWSYM OpBC     ; INC A  ++ A            N......Z.
    spccop SpcOpBC

NEWSYM OpDC     ; DEC Y  -- Y            N......Z.
    spccop SpcOpDC

NEWSYM OpFC     ; INC Y  ++ Y            N......Z.
    spccop SpcOpFC

NEWSYM Op1D     ; DEC X     -- X            N......Z.
    spccop SpcOp1D

NEWSYM Op3D     ; INC X     ++ X            N......Z.
    spccop SpcOp3D

;************************************************
; PUSH/POP instructions (Verified)
;************************************************

NEWSYM Op0D     ; PUSH PSW     push PSW to stack     .........
    spccop SpcOp0D

NEWSYM Op2D     ; PUSH A     push A to stack       .........
    spccop SpcOp2D

NEWSYM Op4D     ; PUSH X     push X to stack       .........
    spccop SpcOp4D

NEWSYM Op6D     ; PUSH Y    push Y to stack       .........
    spccop SpcOp6D

NEWSYM Op8E     ; POP PSW   pop PSW from stack     (Restored)
    spccop SpcOp8E

NEWSYM OpAE     ; POP A     pop A from stack      .........
    spccop SpcOpAE

NEWSYM OpCE     ; POP X     pop X from stack      .........
    spccop SpcOpCE

NEWSYM OpEE     ; POP Y     pop Y from stack      .........
    spccop SpcOpEE

;************************************************
; Test & set bits Instructions (Verified?)
;************************************************

NEWSYM Op0E     ; TSET1 labs   test and set bits with A   N......Z.
    spccop SpcOp0E

NEWSYM Op4E     ; TCLR1     test and clear bits with A N......Z.
    spccop SpcOp4E

;************************************************
; Compare/Decrement & Branch Instructions (Verified)
;************************************************

NEWSYM Op2E     ; CBNE dp,rel  compare A with (dp) then BNE   ...
    spccop SpcOp2E

NEWSYM OpDE     ; CBNE dp+X,rel   compare A with (dp+X) then BNE ...
    spccop SpcOpDE

NEWSYM Op6E     ; DBNZ   decrement memory (dp) then JNZ ...
    spccop SpcOp6E

NEWSYM OpFE     ; DBNZ Y,rel   decrement Y then JNZ         ...
    spccop SpcOpFE

;************************************************
; Jump/Subroutine Instructions
;************************************************

NEWSYM Op0F     ; BRK     software interrupt     ...1.0..
    spccop SpcOp0F

NEWSYM Op1F     ; JMP (labs+X)    PC <- (abs+X+1)(abs+X)       ...
    spccop SpcOp1F

NEWSYM Op3F     ; CALL labs    subroutine call        ........
    spccop SpcOp3F

NEWSYM Op4F     ; PCALL upage  upage call           ........
    spccop SpcOp4F

; I'm not sure about this one and JMP labs+X...

NEWSYM Op5F     ; JMP labs     jump to new location         ...
    spccop SpcOp5F

NEWSYM Op6F     ; ret        ret from subroutine   ........
    spccop SpcOp6F

NEWSYM Op7F     ; ret1       return from interrupt   (Restored)
    spccop SpcOp7F

;************************************************
; Divide/Multiply Instructions
;************************************************

NEWSYM Op9E     ; DIV YA,X     Q:A B:Y <- YA / X     NV..H..Z.
    spccop SpcOp9E

NEWSYM OpCF     ; MUL YA     YA(16 bits) <- Y * A    N......Z.
    spccop SpcOpCF

;************************************************
; Decimal Operations
;************************************************

NEWSYM OpBE     ; DAS A     decimal adjust for sub  N......ZC
    spccop SpcOpBE
NEWSYM OpDF     ; DAA A      decimal adjust for add  N......ZC
    spccop SpcOpDF
NEWSYM Invalidopcode ; Invalid Opcode
    spccop SpcOpInvalid
