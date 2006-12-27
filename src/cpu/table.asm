;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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

%include "macros.mac"

EXTSYM mosjmptab,mosdraw10,mosdraw11,mosdraw12,mosdraw13
EXTSYM mosdraw14,mosdraw15,mosdraw16,mosdraw2,mosdraw3
EXTSYM mosdraw4,mosdraw5,mosdraw6,mosdraw7,mosdraw8
EXTSYM mosdraw9,ngpalcon2b,ngpalcon4b
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
NEWSYM addrmdef, times 27 dd 0      ; Address modes
;tablead times 256 dd 0             ; Table address location according to P
;memtabler8 times 256 dd 0          ; Memory Bank Locations for reading 8-bit
;memtablew8 times 256 dd 0          ; Memory Bank Locations for writing 8-bit
;memtabler16 times 256 dd 0         ; Memory Bank Locations for reading 16-bit
;memtablew16 times 256 dd 0         ; Memory Bank Locations for reading 16-bit

section .text

;*******************************************************
; Generate OpCode Table
;*******************************************************

NEWSYM inittable
    ; set up mosaic
    mov dword[mosjmptab],mosdraw2
    mov dword[mosjmptab+4],mosdraw3
    mov dword[mosjmptab+8],mosdraw4
    mov dword[mosjmptab+12],mosdraw5
    mov dword[mosjmptab+16],mosdraw6
    mov dword[mosjmptab+20],mosdraw7
    mov dword[mosjmptab+24],mosdraw8
    mov dword[mosjmptab+28],mosdraw9
    mov dword[mosjmptab+32],mosdraw10
    mov dword[mosjmptab+36],mosdraw11
    mov dword[mosjmptab+40],mosdraw12
    mov dword[mosjmptab+44],mosdraw13
    mov dword[mosjmptab+48],mosdraw14
    mov dword[mosjmptab+52],mosdraw15
    mov dword[mosjmptab+56],mosdraw16
    mov dword[mosjmptab16b],mosdraw216b
    mov dword[mosjmptab16b+4],mosdraw316b
    mov dword[mosjmptab16b+8],mosdraw416b
    mov dword[mosjmptab16b+12],mosdraw516b
    mov dword[mosjmptab16b+16],mosdraw616b
    mov dword[mosjmptab16b+20],mosdraw716b
    mov dword[mosjmptab16b+24],mosdraw816b
    mov dword[mosjmptab16b+28],mosdraw916b
    mov dword[mosjmptab16b+32],mosdraw1016b
    mov dword[mosjmptab16b+36],mosdraw1116b
    mov dword[mosjmptab16b+40],mosdraw1216b
    mov dword[mosjmptab16b+44],mosdraw1316b
    mov dword[mosjmptab16b+48],mosdraw1416b
    mov dword[mosjmptab16b+52],mosdraw1516b
    mov dword[mosjmptab16b+56],mosdraw1616b
    mov dword[mosjmptab16bt],mosdraw216bt
    mov dword[mosjmptab16bt+4],mosdraw316bt
    mov dword[mosjmptab16bt+8],mosdraw416bt
    mov dword[mosjmptab16bt+12],mosdraw516bt
    mov dword[mosjmptab16bt+16],mosdraw616bt
    mov dword[mosjmptab16bt+20],mosdraw716bt
    mov dword[mosjmptab16bt+24],mosdraw816bt
    mov dword[mosjmptab16bt+28],mosdraw916bt
    mov dword[mosjmptab16bt+32],mosdraw1016bt
    mov dword[mosjmptab16bt+36],mosdraw1116bt
    mov dword[mosjmptab16bt+40],mosdraw1216bt
    mov dword[mosjmptab16bt+44],mosdraw1316bt
    mov dword[mosjmptab16bt+48],mosdraw1416bt
    mov dword[mosjmptab16bt+52],mosdraw1516bt
    mov dword[mosjmptab16bt+56],mosdraw1616bt
    mov dword[mosjmptab16btms],mosdraw216btms
    mov dword[mosjmptab16btms+4],mosdraw316btms
    mov dword[mosjmptab16btms+8],mosdraw416btms
    mov dword[mosjmptab16btms+12],mosdraw516btms
    mov dword[mosjmptab16btms+16],mosdraw616btms
    mov dword[mosjmptab16btms+20],mosdraw716btms
    mov dword[mosjmptab16btms+24],mosdraw816btms
    mov dword[mosjmptab16btms+28],mosdraw916btms
    mov dword[mosjmptab16btms+32],mosdraw1016btms
    mov dword[mosjmptab16btms+36],mosdraw1116btms
    mov dword[mosjmptab16btms+40],mosdraw1216btms
    mov dword[mosjmptab16btms+44],mosdraw1316btms
    mov dword[mosjmptab16btms+48],mosdraw1416btms
    mov dword[mosjmptab16btms+52],mosdraw1516btms
    mov dword[mosjmptab16btms+56],mosdraw1616btms
    mov dword[mosjmptab16bntms],mosdraw216bntms
    mov dword[mosjmptab16bntms+4],mosdraw316bntms
    mov dword[mosjmptab16bntms+8],mosdraw416bntms
    mov dword[mosjmptab16bntms+12],mosdraw516bntms
    mov dword[mosjmptab16bntms+16],mosdraw616bntms
    mov dword[mosjmptab16bntms+20],mosdraw716bntms
    mov dword[mosjmptab16bntms+24],mosdraw816bntms
    mov dword[mosjmptab16bntms+28],mosdraw916bntms
    mov dword[mosjmptab16bntms+32],mosdraw1016bntms
    mov dword[mosjmptab16bntms+36],mosdraw1116bntms
    mov dword[mosjmptab16bntms+40],mosdraw1216bntms
    mov dword[mosjmptab16bntms+44],mosdraw1316bntms
    mov dword[mosjmptab16bntms+48],mosdraw1416bntms
    mov dword[mosjmptab16bntms+52],mosdraw1516bntms
    mov dword[mosjmptab16bntms+56],mosdraw1616bntms
    ; set ngpalcon4b
    mov ecx,20h
    xor eax,eax
.loopng
    mov bl,al
    shr bl,2
    shl bl,4
    mov dl,bl
    mov dh,bl
    shl edx,16
    mov dl,bl
    mov dh,bl
    mov [ngpalcon4b+eax*4],edx
    inc eax
    dec ecx
    jnz .loopng
    ; set ngpalcon2b
    mov ecx,20h
    xor eax,eax
.loopngb
    mov bl,al
    shr bl,2
    shl bl,2
    mov dl,bl
    mov dh,bl
    shl edx,16
    mov dl,bl
    mov dh,bl
    mov [ngpalcon2b+eax*4],edx
    inc eax
    dec ecx
    jnz .loopngb
    ; set tablead  (NVMXDIZC) (  MXD   )
    push es
    xor ecx,ecx
    xor al,al
    mov cx,256
    mov edi,tablead
.loopa
    test al,08h ; D flag
    jnz .decon
    test al,10h ; X flag
    jnz .xon
    test al,20h ; M flag
    jnz .mon
    mov esi,tableA
    jmp .done
.mon
    mov esi,tableB
    jmp .done

.xon
    test al,20h ; M flag
    jnz .mon2
    mov esi,tableC
    jmp .done
.mon2
    mov esi,tableD
    jmp .done

.decon
    test al,10h ; X flag
    jnz .xon3
    test al,20h ; M flag
    jnz .mon3
    mov esi,tableE
    jmp .done
.mon3
    mov esi,tableF
    jmp .done

.xon3
    test al,20h
    jnz .mon4
    mov esi,tableG
    jmp .done
.mon4
    mov esi,tableH
.done
    inc al
    push eax
    mov eax,esi
    stosd
    pop eax
    dec ecx
    jnz .loopa

    ; Set CPU addresses
    ; First, set all addresses to invalid
    mov eax,eopINVALID
    mov edi,tableA
    mov ecx,256
    rep stosd
    mov edi,tableB
    mov ecx,256
    rep stosd
    mov edi,tableC
    mov ecx,256
    rep stosd
    mov edi,tableD
    mov ecx,256
    rep stosd
    mov edi,tableE
    mov ecx,256
    rep stosd
    mov edi,tableF
    mov ecx,256
    rep stosd
    mov edi,tableG
    mov ecx,256
    rep stosd
    mov edi,tableH
    mov ecx,256
    rep stosd
    mov edi,tableA
    call settables
    mov edi,tableB
    call settables
    mov edi,tableC
    call settables
    mov edi,tableD
    call settables
    mov edi,tableE
    call settables
    mov edi,tableF
    call settables
    mov edi,tableG
    call settables
    mov edi,tableH
    call settables

    ; set proper functions
    mov edi,tableA              ; Table addresses (M:0,X:0,D:0)
    call settablem16
    mov edi,tableA
    call settablex16

    mov edi,tableB              ; Table addresses (M:1,X:0,D:0)
    call settablex16

    mov edi,tableC              ; Table addresses (M:0,X:1,D:0)
    call settablem16

    mov edi,tableE              ; Table addresses (M:0,X:0,D:1)
    call settablem16
    mov edi,tableE
    call settableDm16
    mov edi,tableE
    call settablex16

    mov edi,tableF              ; Table addresses (M:1,X:0,D:1)
    call settablex16
    mov edi,tableF
    call settableDm8

    mov edi,tableG              ; Table addresses (M:0,X:1,D:1)
    call settablem16
    mov edi,tableG
    call settableDm16

    mov edi,tableH              ; Table addresses (M:1,X:1,D:1)
    call settableDm8
    pop es
    ret

eopINVALID
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

section .text

;*******************************************************
; Set Tables     Sets the opcode tables according to EDI
;*******************************************************
; This function sets all the non-multiple entries

NEWSYM settables
    ;row 0
    mov dword[edi+00h*4],COp00
    mov dword[edi+01h*4],COp01m8
    mov dword[edi+02h*4],COp02
    mov dword[edi+03h*4],COp03m8
    mov dword[edi+04h*4],COp04m8
    mov dword[edi+05h*4],COp05m8
    mov dword[edi+06h*4],COp06m8
    mov dword[edi+07h*4],COp07m8
    mov dword[edi+08h*4],COp08
    mov dword[edi+09h*4],COp09m8
    mov dword[edi+0Ah*4],COp0Am8
    mov dword[edi+0Bh*4],COp0B
    mov dword[edi+0Ch*4],COp0Cm8
    mov dword[edi+0Dh*4],COp0Dm8
    mov dword[edi+0Eh*4],COp0Em8
    mov dword[edi+0Fh*4],COp0Fm8
    mov dword[edi+10h*4],COp10
    mov dword[edi+11h*4],COp11m8
    mov dword[edi+12h*4],COp12m8
    mov dword[edi+13h*4],COp13m8
    mov dword[edi+14h*4],COp14m8
    mov dword[edi+15h*4],COp15m8
    mov dword[edi+16h*4],COp16m8
    mov dword[edi+17h*4],COp17m8
    mov dword[edi+18h*4],COp18
    mov dword[edi+19h*4],COp19m8
    mov dword[edi+1Ah*4],COp1Am8
    mov dword[edi+1Bh*4],COp1B
    mov dword[edi+1Ch*4],COp1Cm8
    mov dword[edi+1Dh*4],COp1Dm8
    mov dword[edi+1Eh*4],COp1Em8
    mov dword[edi+1Fh*4],COp1Fm8
    mov dword[edi+20h*4],COp20
    mov dword[edi+21h*4],COp21m8
    mov dword[edi+22h*4],COp22
    mov dword[edi+23h*4],COp23m8
    mov dword[edi+24h*4],COp24m8
    mov dword[edi+25h*4],COp25m8
    mov dword[edi+26h*4],COp26m8
    mov dword[edi+27h*4],COp27m8
    mov dword[edi+28h*4],COp28
    mov dword[edi+29h*4],COp29m8
    mov dword[edi+2Ah*4],COp2Am8
    mov dword[edi+2Bh*4],COp2B
    mov dword[edi+2Ch*4],COp2Cm8
    mov dword[edi+2Dh*4],COp2Dm8
    mov dword[edi+2Eh*4],COp2Em8
    mov dword[edi+2Fh*4],COp2Fm8
    mov dword[edi+30h*4],COp30
    mov dword[edi+31h*4],COp31m8
    mov dword[edi+32h*4],COp32m8
    mov dword[edi+33h*4],COp33m8
    mov dword[edi+34h*4],COp34m8
    mov dword[edi+35h*4],COp35m8
    mov dword[edi+36h*4],COp36m8
    mov dword[edi+37h*4],COp37m8
    mov dword[edi+38h*4],COp38
    mov dword[edi+39h*4],COp39m8
    mov dword[edi+3Ah*4],COp3Am8
    mov dword[edi+3Bh*4],COp3B
    mov dword[edi+3Ch*4],COp3Cm8
    mov dword[edi+3Dh*4],COp3Dm8
    mov dword[edi+3Eh*4],COp3Em8
    mov dword[edi+3Fh*4],COp3Fm8
    mov dword[edi+40h*4],COp40
    mov dword[edi+41h*4],COp41m8
    mov dword[edi+42h*4],COp42
    mov dword[edi+43h*4],COp43m8
    mov dword[edi+44h*4],COp44
    mov dword[edi+45h*4],COp45m8
    mov dword[edi+46h*4],COp46m8
    mov dword[edi+47h*4],COp47m8
    mov dword[edi+48h*4],COp48m8
    mov dword[edi+49h*4],COp49m8
    mov dword[edi+4Ah*4],COp4Am8
    mov dword[edi+4Bh*4],COp4B
    mov dword[edi+4Ch*4],COp4C
    mov dword[edi+4Dh*4],COp4Dm8
    mov dword[edi+4Eh*4],COp4Em8
    mov dword[edi+4Fh*4],COp4Fm8
    mov dword[edi+50h*4],COp50
    mov dword[edi+51h*4],COp51m8
    mov dword[edi+52h*4],COp52m8
    mov dword[edi+53h*4],COp53m8
    mov dword[edi+54h*4],COp54
    mov dword[edi+55h*4],COp55m8
    mov dword[edi+56h*4],COp56m8
    mov dword[edi+57h*4],COp57m8
    mov dword[edi+58h*4],COp58
    mov dword[edi+59h*4],COp59m8
    mov dword[edi+5Ah*4],COp5Ax8
    mov dword[edi+5Bh*4],COp5B
    mov dword[edi+5Ch*4],COp5C
    mov dword[edi+5Dh*4],COp5Dm8
    mov dword[edi+5Eh*4],COp5Em8
    mov dword[edi+5Fh*4],COp5Fm8
    mov dword[edi+60h*4],COp60
    mov dword[edi+61h*4],COp61m8nd
    mov dword[edi+62h*4],COp62
    mov dword[edi+63h*4],COp63m8nd
    mov dword[edi+64h*4],COp64m8
    mov dword[edi+65h*4],COp65m8nd
    mov dword[edi+66h*4],COp66m8
    mov dword[edi+67h*4],COp67m8nd
    mov dword[edi+68h*4],COp68m8
    mov dword[edi+69h*4],COp69m8nd
    mov dword[edi+6Ah*4],COp6Am8
    mov dword[edi+6Bh*4],COp6B
    mov dword[edi+6Ch*4],COp6C
    mov dword[edi+6Dh*4],COp6Dm8nd
    mov dword[edi+6Eh*4],COp6Em8
    mov dword[edi+6Fh*4],COp6Fm8nd
    mov dword[edi+70h*4],COp70
    mov dword[edi+71h*4],COp71m8nd
    mov dword[edi+72h*4],COp72m8nd
    mov dword[edi+73h*4],COp73m8nd
    mov dword[edi+74h*4],COp74m8
    mov dword[edi+75h*4],COp75m8nd
    mov dword[edi+76h*4],COp76m8
    mov dword[edi+77h*4],COp77m8nd
    mov dword[edi+78h*4],COp78
    mov dword[edi+79h*4],COp79m8nd
    mov dword[edi+7Ah*4],COp7Ax8
    mov dword[edi+7Bh*4],COp7B
    mov dword[edi+7Ch*4],COp7C
    mov dword[edi+7Dh*4],COp7Dm8nd
    mov dword[edi+7Eh*4],COp7Em8
    mov dword[edi+7Fh*4],COp7Fm8nd
    mov dword[edi+80h*4],COp80
    mov dword[edi+81h*4],COp81m8
    mov dword[edi+82h*4],COp82
    mov dword[edi+83h*4],COp83m8
    mov dword[edi+84h*4],COp84x8
    mov dword[edi+85h*4],COp85m8
    mov dword[edi+86h*4],COp86x8
    mov dword[edi+87h*4],COp87m8
    mov dword[edi+88h*4],COp88x8
    mov dword[edi+89h*4],COp89m8
    mov dword[edi+8Ah*4],COp8Am8
    mov dword[edi+8Bh*4],COp8B
    mov dword[edi+8Ch*4],COp8Cx8
    mov dword[edi+8Dh*4],COp8Dm8
    mov dword[edi+8Eh*4],COp8Ex8
    mov dword[edi+8Fh*4],COp8Fm8
    mov dword[edi+90h*4],COp90
    mov dword[edi+91h*4],COp91m8
    mov dword[edi+92h*4],COp92m8
    mov dword[edi+93h*4],COp93m8
    mov dword[edi+94h*4],COp94x8
    mov dword[edi+95h*4],COp95m8
    mov dword[edi+96h*4],COp96x8
    mov dword[edi+97h*4],COp97m8
    mov dword[edi+98h*4],COp98m8
    mov dword[edi+99h*4],COp99m8
    mov dword[edi+9Ah*4],COp9A
    mov dword[edi+9Bh*4],COp9Bx8
    mov dword[edi+9Ch*4],COp9Cm8
    mov dword[edi+9Dh*4],COp9Dm8
    mov dword[edi+9Eh*4],COp9Em8
    mov dword[edi+9Fh*4],COp9Fm8
    mov dword[edi+0A0h*4],COpA0x8
    mov dword[edi+0A1h*4],COpA1m8
    mov dword[edi+0A2h*4],COpA2x8
    mov dword[edi+0A3h*4],COpA3m8
    mov dword[edi+0A4h*4],COpA4x8
    mov dword[edi+0A5h*4],COpA5m8
    mov dword[edi+0A6h*4],COpA6x8
    mov dword[edi+0A7h*4],COpA7m8
    mov dword[edi+0A8h*4],COpA8x8
    mov dword[edi+0A9h*4],COpA9m8
    mov dword[edi+0AAh*4],COpAAx8
    mov dword[edi+0ABh*4],COpAB
    mov dword[edi+0ACh*4],COpACx8
    mov dword[edi+0ADh*4],COpADm8
    mov dword[edi+0AEh*4],COpAEx8
    mov dword[edi+0AFh*4],COpAFm8
    mov dword[edi+0B0h*4],COpB0
    mov dword[edi+0B1h*4],COpB1m8
    mov dword[edi+0B2h*4],COpB2m8
    mov dword[edi+0B3h*4],COpB3m8
    mov dword[edi+0B4h*4],COpB4x8
    mov dword[edi+0B5h*4],COpB5m8
    mov dword[edi+0B6h*4],COpB6x8
    mov dword[edi+0B7h*4],COpB7m8
    mov dword[edi+0B8h*4],COpB8
    mov dword[edi+0B9h*4],COpB9m8
    mov dword[edi+0BAh*4],COpBAx8
    mov dword[edi+0BBh*4],COpBBx8
    mov dword[edi+0BCh*4],COpBCx8
    mov dword[edi+0BDh*4],COpBDm8
    mov dword[edi+0BEh*4],COpBEx8
    mov dword[edi+0BFh*4],COpBFm8
    mov dword[edi+0C0h*4],COpC0x8
    mov dword[edi+0C1h*4],COpC1m8
    mov dword[edi+0C2h*4],COpC2
    mov dword[edi+0C3h*4],COpC3m8
    mov dword[edi+0C4h*4],COpC4x8
    mov dword[edi+0C5h*4],COpC5m8
    mov dword[edi+0C6h*4],COpC6m8
    mov dword[edi+0C7h*4],COpC7m8
    mov dword[edi+0C8h*4],COpC8x8
    mov dword[edi+0C9h*4],COpC9m8
    mov dword[edi+0CAh*4],COpCAx8
    mov dword[edi+0CBh*4],COpCB
    mov dword[edi+0CCh*4],COpCCx8
    mov dword[edi+0CDh*4],COpCDm8
    mov dword[edi+0CEh*4],COpCEm8
    mov dword[edi+0CFh*4],COpCFm8
    mov dword[edi+0D0h*4],COpD0
    mov dword[edi+0D1h*4],COpD1m8
    mov dword[edi+0D2h*4],COpD2m8
    mov dword[edi+0D3h*4],COpD3m8
    mov dword[edi+0D4h*4],COpD4
    mov dword[edi+0D5h*4],COpD5m8
    mov dword[edi+0D6h*4],COpD6m8
    mov dword[edi+0D7h*4],COpD7m8
    mov dword[edi+0D8h*4],COpD8
    mov dword[edi+0D9h*4],COpD9m8
    mov dword[edi+0DAh*4],COpDAx8
    mov dword[edi+0DBh*4],COpDB
    mov dword[edi+0DCh*4],COpDC
    mov dword[edi+0DDh*4],COpDDm8
    mov dword[edi+0DEh*4],COpDEm8
    mov dword[edi+0DFh*4],COpDFm8
    mov dword[edi+0E0h*4],COpE0x8
    mov dword[edi+0E1h*4],COpE1m8nd
    mov dword[edi+0E2h*4],COpE2
    mov dword[edi+0E3h*4],COpE3m8nd
    mov dword[edi+0E4h*4],COpE4x8
    mov dword[edi+0E5h*4],COpE5m8nd
    mov dword[edi+0E6h*4],COpE6m8
    mov dword[edi+0E7h*4],COpE7m8nd
    mov dword[edi+0E8h*4],COpE8x8
    mov dword[edi+0E9h*4],COpE9m8nd
    mov dword[edi+0EAh*4],COpEA
    mov dword[edi+0EBh*4],COpEB
    mov dword[edi+0ECh*4],COpECx8
    mov dword[edi+0EDh*4],COpEDm8nd
    mov dword[edi+0EEh*4],COpEEm8
    mov dword[edi+0EFh*4],COpEFm8nd
    mov dword[edi+0F0h*4],COpF0
    mov dword[edi+0F1h*4],COpF1m8nd
    mov dword[edi+0F2h*4],COpF2m8nd
    mov dword[edi+0F3h*4],COpF3m8nd
    mov dword[edi+0F4h*4],COpF4
    mov dword[edi+0F5h*4],COpF5m8nd
    mov dword[edi+0F6h*4],COpF6m8
    mov dword[edi+0F7h*4],COpF7m8nd
    mov dword[edi+0F8h*4],COpF8
    mov dword[edi+0F9h*4],COpF9m8nd
    mov dword[edi+0FAh*4],COpFAx8
    mov dword[edi+0FBh*4],COpFB
    mov dword[edi+0FCh*4],COpFC
    mov dword[edi+0FDh*4],COpFDm8nd
    mov dword[edi+0FEh*4],COpFEm8
    mov dword[edi+0FFh*4],COpFFm8nd
    ret

NEWSYM settablem16
    mov dword[edi+01h*4],COp01m16
    mov dword[edi+03h*4],COp03m16
    mov dword[edi+04h*4],COp04m16
    mov dword[edi+05h*4],COp05m16
    mov dword[edi+06h*4],COp06m16
    mov dword[edi+07h*4],COp07m16
    mov dword[edi+09h*4],COp09m16
    mov dword[edi+0Ah*4],COp0Am16
    mov dword[edi+0Ch*4],COp0Cm16
    mov dword[edi+0Dh*4],COp0Dm16
    mov dword[edi+0Eh*4],COp0Em16
    mov dword[edi+0Fh*4],COp0Fm16
    mov dword[edi+11h*4],COp11m16
    mov dword[edi+12h*4],COp12m16
    mov dword[edi+13h*4],COp13m16
    mov dword[edi+14h*4],COp14m16
    mov dword[edi+15h*4],COp15m16
    mov dword[edi+16h*4],COp16m16
    mov dword[edi+17h*4],COp17m16
    mov dword[edi+19h*4],COp19m16
    mov dword[edi+1Ah*4],COp1Am16
    mov dword[edi+1Ch*4],COp1Cm16
    mov dword[edi+1Dh*4],COp1Dm16
    mov dword[edi+1Eh*4],COp1Em16
    mov dword[edi+1Fh*4],COp1Fm16
    mov dword[edi+21h*4],COp21m16
    mov dword[edi+23h*4],COp23m16
    mov dword[edi+24h*4],COp24m16
    mov dword[edi+25h*4],COp25m16
    mov dword[edi+26h*4],COp26m16
    mov dword[edi+27h*4],COp27m16
    mov dword[edi+29h*4],COp29m16
    mov dword[edi+2Ah*4],COp2Am16
    mov dword[edi+2Ch*4],COp2Cm16
    mov dword[edi+2Dh*4],COp2Dm16
    mov dword[edi+2Eh*4],COp2Em16
    mov dword[edi+2Fh*4],COp2Fm16
    mov dword[edi+31h*4],COp31m16
    mov dword[edi+32h*4],COp32m16
    mov dword[edi+33h*4],COp33m16
    mov dword[edi+34h*4],COp34m16
    mov dword[edi+35h*4],COp35m16
    mov dword[edi+36h*4],COp36m16
    mov dword[edi+37h*4],COp37m16
    mov dword[edi+39h*4],COp39m16
    mov dword[edi+3Ah*4],COp3Am16
    mov dword[edi+3Ch*4],COp3Cm16
    mov dword[edi+3Dh*4],COp3Dm16
    mov dword[edi+3Eh*4],COp3Em16
    mov dword[edi+3Fh*4],COp3Fm16
    mov dword[edi+41h*4],COp41m16
    mov dword[edi+43h*4],COp43m16
    mov dword[edi+45h*4],COp45m16
    mov dword[edi+46h*4],COp46m16
    mov dword[edi+47h*4],COp47m16
    mov dword[edi+48h*4],COp48m16
    mov dword[edi+49h*4],COp49m16
    mov dword[edi+4Ah*4],COp4Am16
    mov dword[edi+4Dh*4],COp4Dm16
    mov dword[edi+4Eh*4],COp4Em16
    mov dword[edi+4Fh*4],COp4Fm16
    mov dword[edi+51h*4],COp51m16
    mov dword[edi+52h*4],COp52m16
    mov dword[edi+53h*4],COp53m16
    mov dword[edi+55h*4],COp55m16
    mov dword[edi+56h*4],COp56m16
    mov dword[edi+57h*4],COp57m16
    mov dword[edi+59h*4],COp59m16
    mov dword[edi+5Dh*4],COp5Dm16
    mov dword[edi+5Eh*4],COp5Em16
    mov dword[edi+5Fh*4],COp5Fm16
    mov dword[edi+61h*4],COp61m16nd
    mov dword[edi+63h*4],COp63m16nd
    mov dword[edi+64h*4],COp64m16
    mov dword[edi+65h*4],COp65m16nd
    mov dword[edi+66h*4],COp66m16
    mov dword[edi+67h*4],COp67m16nd
    mov dword[edi+68h*4],COp68m16
    mov dword[edi+69h*4],COp69m16nd
    mov dword[edi+6Ah*4],COp6Am16
    mov dword[edi+6Dh*4],COp6Dm16nd
    mov dword[edi+6Eh*4],COp6Em16
    mov dword[edi+6Fh*4],COp6Fm16nd
    mov dword[edi+71h*4],COp71m16nd
    mov dword[edi+72h*4],COp72m16nd
    mov dword[edi+73h*4],COp73m16nd
    mov dword[edi+74h*4],COp74m16
    mov dword[edi+75h*4],COp75m16nd
    mov dword[edi+76h*4],COp76m16
    mov dword[edi+77h*4],COp77m16nd
    mov dword[edi+79h*4],COp79m16nd
    mov dword[edi+7Dh*4],COp7Dm16nd
    mov dword[edi+7Eh*4],COp7Em16
    mov dword[edi+7Fh*4],COp7Fm16nd
    mov dword[edi+81h*4],COp81m16
    mov dword[edi+83h*4],COp83m16
    mov dword[edi+85h*4],COp85m16
    mov dword[edi+87h*4],COp87m16
    mov dword[edi+89h*4],COp89m16
    mov dword[edi+8Ah*4],COp8Am16
    mov dword[edi+8Dh*4],COp8Dm16
    mov dword[edi+8Fh*4],COp8Fm16
    mov dword[edi+91h*4],COp91m16
    mov dword[edi+92h*4],COp92m16
    mov dword[edi+93h*4],COp93m16
    mov dword[edi+95h*4],COp95m16
    mov dword[edi+97h*4],COp97m16
    mov dword[edi+98h*4],COp98m16
    mov dword[edi+99h*4],COp99m16
    mov dword[edi+9Ch*4],COp9Cm16
    mov dword[edi+9Dh*4],COp9Dm16
    mov dword[edi+9Eh*4],COp9Em16
    mov dword[edi+9Fh*4],COp9Fm16
    mov dword[edi+0A1h*4],COpA1m16
    mov dword[edi+0A3h*4],COpA3m16
    mov dword[edi+0A5h*4],COpA5m16
    mov dword[edi+0A7h*4],COpA7m16
    mov dword[edi+0A9h*4],COpA9m16
    mov dword[edi+0ADh*4],COpADm16
    mov dword[edi+0AFh*4],COpAFm16
    mov dword[edi+0B1h*4],COpB1m16
    mov dword[edi+0B2h*4],COpB2m16
    mov dword[edi+0B3h*4],COpB3m16
    mov dword[edi+0B5h*4],COpB5m16
    mov dword[edi+0B7h*4],COpB7m16
    mov dword[edi+0B9h*4],COpB9m16
    mov dword[edi+0BDh*4],COpBDm16
    mov dword[edi+0BFh*4],COpBFm16
    mov dword[edi+0C1h*4],COpC1m16
    mov dword[edi+0C3h*4],COpC3m16
    mov dword[edi+0C5h*4],COpC5m16
    mov dword[edi+0C6h*4],COpC6m16
    mov dword[edi+0C7h*4],COpC7m16
    mov dword[edi+0C9h*4],COpC9m16
    mov dword[edi+0CDh*4],COpCDm16
    mov dword[edi+0CEh*4],COpCEm16
    mov dword[edi+0CFh*4],COpCFm16
    mov dword[edi+0D1h*4],COpD1m16
    mov dword[edi+0D2h*4],COpD2m16
    mov dword[edi+0D3h*4],COpD3m16
    mov dword[edi+0D5h*4],COpD5m16
    mov dword[edi+0D6h*4],COpD6m16
    mov dword[edi+0D7h*4],COpD7m16
    mov dword[edi+0D9h*4],COpD9m16
    mov dword[edi+0DDh*4],COpDDm16
    mov dword[edi+0DEh*4],COpDEm16
    mov dword[edi+0DFh*4],COpDFm16
    mov dword[edi+0E1h*4],COpE1m16nd
    mov dword[edi+0E3h*4],COpE3m16nd
    mov dword[edi+0E5h*4],COpE5m16nd
    mov dword[edi+0E6h*4],COpE6m16
    mov dword[edi+0E7h*4],COpE7m16nd
    mov dword[edi+0E9h*4],COpE9m16nd
    mov dword[edi+0EDh*4],COpEDm16nd
    mov dword[edi+0EEh*4],COpEEm16
    mov dword[edi+0EFh*4],COpEFm16nd
    mov dword[edi+0F1h*4],COpF1m16nd
    mov dword[edi+0F2h*4],COpF2m16nd
    mov dword[edi+0F3h*4],COpF3m16nd
    mov dword[edi+0F5h*4],COpF5m16nd
    mov dword[edi+0F6h*4],COpF6m16
    mov dword[edi+0F7h*4],COpF7m16nd
    mov dword[edi+0F9h*4],COpF9m16nd
    mov dword[edi+0FDh*4],COpFDm16nd
    mov dword[edi+0FEh*4],COpFEm16
    mov dword[edi+0FFh*4],COpFFm16nd
    ret

NEWSYM settablex16
    mov dword[edi+5Ah*4],COp5Ax16
    mov dword[edi+7Ah*4],COp7Ax16
    mov dword[edi+84h*4],COp84x16
    mov dword[edi+86h*4],COp86x16
    mov dword[edi+88h*4],COp88x16
    mov dword[edi+8Ch*4],COp8Cx16
    mov dword[edi+8Eh*4],COp8Ex16
    mov dword[edi+94h*4],COp94x16
    mov dword[edi+96h*4],COp96x16
    mov dword[edi+9Bh*4],COp9Bx16
    mov dword[edi+0A0h*4],COpA0x16
    mov dword[edi+0A2h*4],COpA2x16
    mov dword[edi+0A4h*4],COpA4x16
    mov dword[edi+0A6h*4],COpA6x16
    mov dword[edi+0A8h*4],COpA8x16
    mov dword[edi+0AAh*4],COpAAx16
    mov dword[edi+0ACh*4],COpACx16
    mov dword[edi+0AEh*4],COpAEx16
    mov dword[edi+0B4h*4],COpB4x16
    mov dword[edi+0B6h*4],COpB6x16
    mov dword[edi+0BAh*4],COpBAx16
    mov dword[edi+0BBh*4],COpBBx16
    mov dword[edi+0BCh*4],COpBCx16
    mov dword[edi+0BEh*4],COpBEx16
    mov dword[edi+0C0h*4],COpC0x16
    mov dword[edi+0C4h*4],COpC4x16
    mov dword[edi+0C8h*4],COpC8x16
    mov dword[edi+0CAh*4],COpCAx16
    mov dword[edi+0CCh*4],COpCCx16
    mov dword[edi+0DAh*4],COpDAx16
    mov dword[edi+0E0h*4],COpE0x16
    mov dword[edi+0E4h*4],COpE4x16
    mov dword[edi+0E8h*4],COpE8x16
    mov dword[edi+0ECh*4],COpECx16
    mov dword[edi+0FAh*4],COpFAx16
    ret

NEWSYM settableDm8
    mov dword[edi+61h*4],COp61m8d
    mov dword[edi+63h*4],COp63m8d
    mov dword[edi+65h*4],COp65m8d
    mov dword[edi+67h*4],COp67m8d
    mov dword[edi+69h*4],COp69m8d
    mov dword[edi+6Dh*4],COp6Dm8d
    mov dword[edi+6Fh*4],COp6Fm8d
    mov dword[edi+71h*4],COp71m8d
    mov dword[edi+72h*4],COp72m8d
    mov dword[edi+73h*4],COp73m8d
    mov dword[edi+75h*4],COp75m8d
    mov dword[edi+77h*4],COp77m8d
    mov dword[edi+79h*4],COp79m8d
    mov dword[edi+7Dh*4],COp7Dm8d
    mov dword[edi+7Fh*4],COp7Fm8d
    mov dword[edi+0E1h*4],COpE1m8d
    mov dword[edi+0E3h*4],COpE3m8d
    mov dword[edi+0E5h*4],COpE5m8d
    mov dword[edi+0E7h*4],COpE7m8d
    mov dword[edi+0E9h*4],COpE9m8d
    mov dword[edi+0EDh*4],COpEDm8d
    mov dword[edi+0EFh*4],COpEFm8d
    mov dword[edi+0F1h*4],COpF1m8d
    mov dword[edi+0F2h*4],COpF2m8d
    mov dword[edi+0F3h*4],COpF3m8d
    mov dword[edi+0F5h*4],COpF5m8d
    mov dword[edi+0F7h*4],COpF7m8d
    mov dword[edi+0F9h*4],COpF9m8d
    mov dword[edi+0FDh*4],COpFDm8d
    mov dword[edi+0FFh*4],COpFFm8d
    ret

NEWSYM settableDm16
    mov dword[edi+61h*4],COp61m16d
    mov dword[edi+63h*4],COp63m16d
    mov dword[edi+65h*4],COp65m16d
    mov dword[edi+67h*4],COp67m16d
    mov dword[edi+69h*4],COp69m16d
    mov dword[edi+6Dh*4],COp6Dm16d
    mov dword[edi+6Fh*4],COp6Fm16d
    mov dword[edi+71h*4],COp71m16d
    mov dword[edi+72h*4],COp72m16d
    mov dword[edi+73h*4],COp73m16d
    mov dword[edi+75h*4],COp75m16d
    mov dword[edi+77h*4],COp77m16d
    mov dword[edi+79h*4],COp79m16d
    mov dword[edi+7Dh*4],COp7Dm16d
    mov dword[edi+7Fh*4],COp7Fm16d
    mov dword[edi+0E1h*4],COpE1m16d
    mov dword[edi+0E3h*4],COpE3m16d
    mov dword[edi+0E5h*4],COpE5m16d
    mov dword[edi+0E7h*4],COpE7m16d
    mov dword[edi+0E9h*4],COpE9m16d
    mov dword[edi+0EDh*4],COpEDm16d
    mov dword[edi+0EFh*4],COpEFm16d
    mov dword[edi+0F1h*4],COpF1m16d
    mov dword[edi+0F2h*4],COpF2m16d
    mov dword[edi+0F3h*4],COpF3m16d
    mov dword[edi+0F5h*4],COpF5m16d
    mov dword[edi+0F7h*4],COpF7m16d
    mov dword[edi+0F9h*4],COpF9m16d
    mov dword[edi+0FDh*4],COpFDm16d
    mov dword[edi+0FFh*4],COpFFm16d
    ret
