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
