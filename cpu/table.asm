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

; Body of a 65816 opcode ported to C (cpu/ops65816.h). The core runs with its
; state in registers - esi is the program counter, dl the flags, dh the cycle
; count, edi the opcode table and ebp the SPC program counter - so pushad hands
; the whole file to the C, which reads and writes it in place.
%macro cop 1
    pushad
    mov eax, esp
    ccall %1, eax
    popad
    endloop
%endmacro

EXTSYM ngpalcon2b,ngpalcon4b
EXTSYM mosjmptab16b
EXTSYM mosjmptab16bt
EXTSYM mosjmptab16btms
EXTSYM mosjmptab16bntms
EXTSYM tableA,tableB,tableC,tableD,tableE,tableF,tableG,tableH
EXTSYM DPageR8,DPageW8,DPageR16,DPageW16
EXTSYM cpucycle,eopINVALID
; The register file moved to cpu/c_regsdata.c. These have to be declared
; here, above the %includes: on PE/COFF EXTSYM also emits a %define, which
; only applies below the line it appears on, and cpu/e65816.inc references
; some of them.
EXTSYM invreg,sndrot,sndrot2,INTEnab
EXTSYM NMIEnab,VIRQLoc,vidbright,previdbr
EXTSYM forceblnk,objptr,objptrn,objsize1
EXTSYM objsize2,objmovs1,objadds1,objmovs2
EXTSYM objadds2,oamaddrt,oamaddrs,objhipr
EXTSYM bgmode,bg3highst,bgtilesz,mosaicon
EXTSYM mosaicsz,bg1ptr,bg2ptr,bg3ptr
EXTSYM bg4ptr,bg1ptrb,bg2ptrb,bg3ptrb
EXTSYM bg4ptrb,bg1ptrc,bg2ptrc,bg3ptrc
EXTSYM bg4ptrc,bg1ptrd,bg2ptrd,bg3ptrd
EXTSYM bg4ptrd,bg1scsize,bg2scsize,bg3scsize
EXTSYM bg4scsize,bg1objptr,bg2objptr,bg3objptr
EXTSYM bg4objptr,bg1scrolx,bg2scrolx,bg3scrolx
EXTSYM bg4scrolx,bg1sx,bg1scroly,bg2scroly
EXTSYM bg3scroly,bg4scroly,addrincr,vramincr
EXTSYM vramread,vramaddr,cgaddr,cgmod
EXTSYM scrnon,scrndist,resolutn,multa
EXTSYM diva,divres,multres,latchx
EXTSYM latchy,latchxr,latchyr,frskipper
EXTSYM winl1,winr1,winl2,winr2
EXTSYM winen,winbg1en,winbg2en,winbg3en
EXTSYM winbg4en,winobjen,wincolen,winlogica
EXTSYM winlogicb,winenabm,winenabs,mode7set
EXTSYM mode7A,mode7B,mode7C,mode7D
EXTSYM mode7X0,mode7Y0,JoyAPos,JoyBPos
EXTSYM compmult,joyalt,wramrwadr,dmadata
EXTSYM irqon,nexthdma,curhdma,hdmadata
EXTSYM hdmatype,coladdr,coladdg,coladdb
EXTSYM colnull,scaddset,scaddtype,Voice0Disabl2
EXTSYM Voice1Disabl2,Voice2Disabl2,Voice3Disabl2,Voice4Disabl2
EXTSYM Voice5Disabl2,Voice6Disabl2,Voice7Disabl2,oamram
EXTSYM cgram,pcgram,vraminctype,vramincby8on
EXTSYM vramincby8left,vramincby8totl,vramincby8rowl,vramincby8ptri
EXTSYM nexthprior,doirqnext,vramincby8var,screstype
EXTSYM extlatch,cfield,interlval,HIRQLoc
EXTSYM KeyOnStA,KeyOnStB,SDD1BankA,SDD1BankB
EXTSYM SDD1BankC,SDD1BankD,vramread2,nosprincr
EXTSYM poamaddrs,ioportval,iohvlatch,ppustatus
EXTSYM hdmastartsc,hdmarestart,hdmadelay,nohdmaframe
EXTSYM rtoflags,h_dot_counter,tempdat,PHnum2writeppureg
EXTSYM scrndis,oamaddr,bg1ptrx,bg2ptrx
EXTSYM bg3ptrx,bg4ptrx,bg1ptry,bg2ptry
EXTSYM bg3ptry,bg4ptry,Voice0Disable,Voice1Disable
EXTSYM Voice2Disable,Voice3Disable,Voice4Disable,Voice5Disable
EXTSYM Voice6Disable,Voice7Disable,BG116x16t,BG216x16t
EXTSYM BG316x16t,BG416x16t,SPC700read,SPC700write
EXTSYM JoyCRead,nssdip1,nssdip2,nssdip3
EXTSYM nssdip4,nssdip5,nssdip6,MultiTap
EXTSYM hblank,JoyARead,JoyBRead,JoyCRead2
EXTSYM JoyDRead,JoyERead,cpu_mdr,ppu2_mdr
EXTSYM SDD1Enable
EXTSYM JoyAOrig,JoyANow,JoyBOrig,JoyBNow,JoyCOrig,JoyCNow,JoyDOrig,JoyDNow
EXTSYM reg2101w_objsize1,reg2101w_objsize2,reg2101w_objmovs1,reg2101w_objmovs2
EXTSYM reg2101w_objadds1,reg2101w_objadds2,bgscrolPrev,bg1scrolx_m7
EXTSYM bg1scroly_m7,multchange,m7byte,prevoamptr
EXTSYM oamlow,MultiTapStat
; 65816 opcodes ported to cpu/ops65816.h.
EXTSYM c_COp80,c_COp18,c_COpD8,c_COpB8
EXTSYM c_COpCAx8,c_COpCAx16,c_COp88x8,c_COp88x16
EXTSYM c_COpE8x8,c_COpE8x16,c_COpC8x8,c_COpC8x16
EXTSYM c_COpEA,c_COp38,c_COpF8,c_COp78
EXTSYM c_COpDB,c_COpAAx8,c_COpAAx16,c_COpA8x8
EXTSYM c_COpA8x16,c_COp1B,c_COp7B,c_COp3B
EXTSYM c_COpBAx8,c_COpBAx16,c_COp8Am8,c_COp8Am16
EXTSYM c_COp9A,c_COp9Bx8,c_COp9Bx16,c_COp98m8
EXTSYM c_COp98m16,c_COpBBx8,c_COpBBx16,c_COpEB
EXTSYM c_COp42
EXTSYM c_COp90,c_COpB0,c_COpF0,c_COpD0
EXTSYM c_COp30,c_COp10,c_COp50,c_COp70
EXTSYM c_COp1Am8,c_COp1Am16,c_COp3Am8,c_COp3Am16
EXTSYM c_COp5B,c_COpC2,c_COpE2,c_COpFB
EXTSYM c_COp48m8,c_COp48m16,c_COp8B,c_COp0B
EXTSYM c_COp4B,c_COpDAx8,c_COpDAx16,c_COp5Ax8
EXTSYM c_COp5Ax16,c_COp08,c_COp68m8,c_COp68m16
EXTSYM c_COpAB,c_COpFAx8,c_COpFAx16,c_COp7Ax8
EXTSYM c_COp7Ax16,c_COp2B,c_COp28,c_COpF4
EXTSYM c_COpD4,c_COp62
EXTSYM c_COpA9m8,c_COpA9m16,c_COpADm8,c_COpADm16
EXTSYM c_COpBDm8,c_COpBDm16,c_COpB9m8,c_COpB9m16
EXTSYM c_COpAFm8,c_COpAFm16,c_COpBFm8,c_COpBFm16
EXTSYM c_COpA5m8,c_COpA5m16,c_COpB5m8,c_COpB5m16
EXTSYM c_COpA3m8,c_COpA3m16,c_COpB2m8,c_COpB2m16
EXTSYM c_COpB1m8,c_COpB1m16,c_COpA1m8,c_COpA1m16
EXTSYM c_COpB3m8,c_COpB3m16,c_COpA7m8,c_COpA7m16
EXTSYM c_COpB7m8,c_COpB7m16
EXTSYM c_COp21m8,c_COp21m16,c_COp23m8,c_COp23m16
EXTSYM c_COp25m8,c_COp25m16,c_COp27m8,c_COp27m16
EXTSYM c_COp29m8,c_COp29m16,c_COp2Dm8,c_COp2Dm16
EXTSYM c_COp2Fm8,c_COp2Fm16,c_COp31m8,c_COp31m16
EXTSYM c_COp32m8,c_COp32m16,c_COp33m8,c_COp33m16
EXTSYM c_COp35m8,c_COp35m16,c_COp37m8,c_COp37m16
EXTSYM c_COp39m8,c_COp39m16,c_COp3Dm8,c_COp3Dm16
EXTSYM c_COp3Fm8,c_COp3Fm16,c_COp24m8,c_COp24m16
EXTSYM c_COp2Cm8,c_COp2Cm16,c_COp34m8,c_COp34m16
EXTSYM c_COp3Cm8,c_COp3Cm16,c_COpC1m8,c_COpC1m16
EXTSYM c_COpC3m8,c_COpC3m16,c_COpC5m8,c_COpC5m16
EXTSYM c_COpC7m8,c_COpC7m16,c_COpC9m8,c_COpC9m16
EXTSYM c_COpCDm8,c_COpCDm16,c_COpCFm8,c_COpCFm16
EXTSYM c_COpD1m8,c_COpD1m16,c_COpD2m8,c_COpD2m16
EXTSYM c_COpD3m8,c_COpD3m16,c_COpD5m8,c_COpD5m16
EXTSYM c_COpD7m8,c_COpD7m16,c_COpD9m8,c_COpD9m16
EXTSYM c_COpDDm8,c_COpDDm16,c_COpDFm8,c_COpDFm16
EXTSYM c_COpE0x8,c_COpE0x16,c_COpE4x8,c_COpE4x16
EXTSYM c_COpECx8,c_COpECx16,c_COpC0x8,c_COpC0x16
EXTSYM c_COpC4x8,c_COpC4x16,c_COpCCx8,c_COpCCx16
EXTSYM c_COp41m8,c_COp41m16,c_COp43m8,c_COp43m16
EXTSYM c_COp45m8,c_COp45m16,c_COp47m8,c_COp47m16
EXTSYM c_COp49m8,c_COp49m16,c_COp4Dm8,c_COp4Dm16
EXTSYM c_COp4Fm8,c_COp4Fm16,c_COp51m8,c_COp51m16
EXTSYM c_COp52m8,c_COp52m16,c_COp53m8,c_COp53m16
EXTSYM c_COp55m8,c_COp55m16,c_COp57m8,c_COp57m16
EXTSYM c_COp59m8,c_COp59m16,c_COp5Dm8,c_COp5Dm16
EXTSYM c_COp5Fm8,c_COp5Fm16,c_COpA2x8,c_COpA2x16
EXTSYM c_COpA6x8,c_COpA6x16,c_COpAEx8,c_COpAEx16
EXTSYM c_COpB6x8,c_COpB6x16,c_COpBEx8,c_COpBEx16
EXTSYM c_COpA0x8,c_COpA0x16,c_COpA4x8,c_COpA4x16
EXTSYM c_COpACx8,c_COpACx16,c_COpB4x8,c_COpB4x16
EXTSYM c_COpBCx8,c_COpBCx16,c_COp01m8,c_COp01m16
EXTSYM c_COp03m8,c_COp03m16,c_COp05m8,c_COp05m16
EXTSYM c_COp07m8,c_COp07m16,c_COp09m8,c_COp09m16
EXTSYM c_COp0Dm8,c_COp0Dm16,c_COp0Fm8,c_COp0Fm16
EXTSYM c_COp11m8,c_COp11m16,c_COp12m8,c_COp12m16
EXTSYM c_COp13m8,c_COp13m16,c_COp15m8,c_COp15m16
EXTSYM c_COp17m8,c_COp17m16,c_COp19m8,c_COp19m16
EXTSYM c_COp1Dm8,c_COp1Dm16,c_COp1Fm8,c_COp1Fm16

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
