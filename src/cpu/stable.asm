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

EXTSYM SA1tableA,SA1tableB,SA1tableC,SA1tableD,SA1tableE
EXTSYM SA1tableF,SA1tableG,SA1tableH,SA1tablead
EXTSYM cpucycle,SA1UpdateDPage,intrset

%include "cpu/s65816d.inc"
%include "cpu/saddress.inc"
%include "cpu/saddrni.inc"
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

;*******************************************************
; Generate OpCode Table
;*******************************************************

SECTION .text

NEWSYM SA1inittable
    ; set tablead  (NVMXDIZC) (  MXD   )
    push es
    xor ecx,ecx
    xor al,al
    mov cx,256
    mov edi,SA1tablead
.loopa
    test al,08h ; D flag
    jnz .decon
    test al,10h ; X flag
    jnz .xon
    test al,20h ; M flag
    jnz .mon
    mov esi,SA1tableA
    jmp .done
.mon
    mov esi,SA1tableB
    jmp .done

.xon
    test al,20h ; M flag
    jnz .mon2
    mov esi,SA1tableC
    jmp .done
.mon2
    mov esi,SA1tableD
    jmp .done

.decon
    test al,10h ; X flag
    jnz .xon3
    test al,20h ; M flag
    jnz .mon3
    mov esi,SA1tableE
    jmp .done
.mon3
    mov esi,SA1tableF
    jmp .done

.xon3
    test al,20h
    jnz .mon4
    mov esi,SA1tableG
    jmp .done
.mon4
    mov esi,SA1tableH
.done
    inc al
    push eax
    mov eax,esi
    stosd
    pop eax
    dec ecx
    jnz .loopa

    ; Set CPU addresses
    mov edi,SA1tableA
    call SA1settables
    mov edi,SA1tableB
    call SA1settables
    mov edi,SA1tableC
    call SA1settables
    mov edi,SA1tableD
    call SA1settables
    mov edi,SA1tableE
    call SA1settables
    mov edi,SA1tableF
    call SA1settables
    mov edi,SA1tableG
    call SA1settables
    mov edi,SA1tableH
    call SA1settables

    ; set proper functions
    mov edi,SA1tableA              ; Table addresses (M:0,X:0,D:0)
    call SA1settablem16
    mov edi,SA1tableA
    call SA1settablex16

    mov edi,SA1tableB              ; Table addresses (M:1,X:0,D:0)
    call SA1settablex16

    mov edi,SA1tableC              ; Table addresses (M:0,X:1,D:0)
    call SA1settablem16

    mov edi,SA1tableE              ; Table addresses (M:0,X:0,D:1)
    call SA1settablem16
    mov edi,SA1tableE
    call SA1settableDm16
    mov edi,SA1tableE
    call SA1settablex16

    mov edi,SA1tableF              ; Table addresses (M:1,X:0,D:1)
    call SA1settablex16
    mov edi,SA1tableF
    call SA1settableDm8

    mov edi,SA1tableG              ; Table addresses (M:0,X:1,D:1)
    call SA1settablem16
    mov edi,SA1tableG
    call SA1settableDm16

    mov edi,SA1tableH              ; Table addresses (M:1,X:1,D:1)
    call SA1settableDm8
    pop es
    ret

;*******************************************************
; Set Tables     Sets the opcode tables according to EDI
;*******************************************************
; This function sets all the non-multiple entries

NEWSYM SA1settables
    ;row 0
    mov dword[edi+00h*4],SA1COp00
    mov dword[edi+01h*4],SA1COp01m8
    mov dword[edi+02h*4],SA1COp02
    mov dword[edi+03h*4],SA1COp03m8
    mov dword[edi+04h*4],SA1COp04m8
    mov dword[edi+05h*4],SA1COp05m8
    mov dword[edi+06h*4],SA1COp06m8
    mov dword[edi+07h*4],SA1COp07m8
    mov dword[edi+08h*4],SA1COp08
    mov dword[edi+09h*4],SA1COp09m8
    mov dword[edi+0Ah*4],SA1COp0Am8
    mov dword[edi+0Bh*4],SA1COp0B
    mov dword[edi+0Ch*4],SA1COp0Cm8
    mov dword[edi+0Dh*4],SA1COp0Dm8
    mov dword[edi+0Eh*4],SA1COp0Em8
    mov dword[edi+0Fh*4],SA1COp0Fm8
    mov dword[edi+10h*4],SA1COp10
    mov dword[edi+11h*4],SA1COp11m8
    mov dword[edi+12h*4],SA1COp12m8
    mov dword[edi+13h*4],SA1COp13m8
    mov dword[edi+14h*4],SA1COp14m8
    mov dword[edi+15h*4],SA1COp15m8
    mov dword[edi+16h*4],SA1COp16m8
    mov dword[edi+17h*4],SA1COp17m8
    mov dword[edi+18h*4],SA1COp18
    mov dword[edi+19h*4],SA1COp19m8
    mov dword[edi+1Ah*4],SA1COp1Am8
    mov dword[edi+1Bh*4],SA1COp1B
    mov dword[edi+1Ch*4],SA1COp1Cm8
    mov dword[edi+1Dh*4],SA1COp1Dm8
    mov dword[edi+1Eh*4],SA1COp1Em8
    mov dword[edi+1Fh*4],SA1COp1Fm8
    mov dword[edi+20h*4],SA1COp20
    mov dword[edi+21h*4],SA1COp21m8
    mov dword[edi+22h*4],SA1COp22
    mov dword[edi+23h*4],SA1COp23m8
    mov dword[edi+24h*4],SA1COp24m8
    mov dword[edi+25h*4],SA1COp25m8
    mov dword[edi+26h*4],SA1COp26m8
    mov dword[edi+27h*4],SA1COp27m8
    mov dword[edi+28h*4],SA1COp28
    mov dword[edi+29h*4],SA1COp29m8
    mov dword[edi+2Ah*4],SA1COp2Am8
    mov dword[edi+2Bh*4],SA1COp2B
    mov dword[edi+2Ch*4],SA1COp2Cm8
    mov dword[edi+2Dh*4],SA1COp2Dm8
    mov dword[edi+2Eh*4],SA1COp2Em8
    mov dword[edi+2Fh*4],SA1COp2Fm8
    mov dword[edi+30h*4],SA1COp30
    mov dword[edi+31h*4],SA1COp31m8
    mov dword[edi+32h*4],SA1COp32m8
    mov dword[edi+33h*4],SA1COp33m8
    mov dword[edi+34h*4],SA1COp34m8
    mov dword[edi+35h*4],SA1COp35m8
    mov dword[edi+36h*4],SA1COp36m8
    mov dword[edi+37h*4],SA1COp37m8
    mov dword[edi+38h*4],SA1COp38
    mov dword[edi+39h*4],SA1COp39m8
    mov dword[edi+3Ah*4],SA1COp3Am8
    mov dword[edi+3Bh*4],SA1COp3B
    mov dword[edi+3Ch*4],SA1COp3Cm8
    mov dword[edi+3Dh*4],SA1COp3Dm8
    mov dword[edi+3Eh*4],SA1COp3Em8
    mov dword[edi+3Fh*4],SA1COp3Fm8
    mov dword[edi+40h*4],SA1COp40
    mov dword[edi+41h*4],SA1COp41m8
    mov dword[edi+42h*4],SA1COp42
    mov dword[edi+43h*4],SA1COp43m8
    mov dword[edi+44h*4],SA1COp44
    mov dword[edi+45h*4],SA1COp45m8
    mov dword[edi+46h*4],SA1COp46m8
    mov dword[edi+47h*4],SA1COp47m8
    mov dword[edi+48h*4],SA1COp48m8
    mov dword[edi+49h*4],SA1COp49m8
    mov dword[edi+4Ah*4],SA1COp4Am8
    mov dword[edi+4Bh*4],SA1COp4B
    mov dword[edi+4Ch*4],SA1COp4C
    mov dword[edi+4Dh*4],SA1COp4Dm8
    mov dword[edi+4Eh*4],SA1COp4Em8
    mov dword[edi+4Fh*4],SA1COp4Fm8
    mov dword[edi+50h*4],SA1COp50
    mov dword[edi+51h*4],SA1COp51m8
    mov dword[edi+52h*4],SA1COp52m8
    mov dword[edi+53h*4],SA1COp53m8
    mov dword[edi+54h*4],SA1COp54
    mov dword[edi+55h*4],SA1COp55m8
    mov dword[edi+56h*4],SA1COp56m8
    mov dword[edi+57h*4],SA1COp57m8
    mov dword[edi+58h*4],SA1COp58
    mov dword[edi+59h*4],SA1COp59m8
    mov dword[edi+5Ah*4],SA1COp5Ax8
    mov dword[edi+5Bh*4],SA1COp5B
    mov dword[edi+5Ch*4],SA1COp5C
    mov dword[edi+5Dh*4],SA1COp5Dm8
    mov dword[edi+5Eh*4],SA1COp5Em8
    mov dword[edi+5Fh*4],SA1COp5Fm8
    mov dword[edi+60h*4],SA1COp60
    mov dword[edi+61h*4],SA1COp61m8nd
    mov dword[edi+62h*4],SA1COp62
    mov dword[edi+63h*4],SA1COp63m8nd
    mov dword[edi+64h*4],SA1COp64m8
    mov dword[edi+65h*4],SA1COp65m8nd
    mov dword[edi+66h*4],SA1COp66m8
    mov dword[edi+67h*4],SA1COp67m8nd
    mov dword[edi+68h*4],SA1COp68m8
    mov dword[edi+69h*4],SA1COp69m8nd
    mov dword[edi+6Ah*4],SA1COp6Am8
    mov dword[edi+6Bh*4],SA1COp6B
    mov dword[edi+6Ch*4],SA1COp6C
    mov dword[edi+6Dh*4],SA1COp6Dm8nd
    mov dword[edi+6Eh*4],SA1COp6Em8
    mov dword[edi+6Fh*4],SA1COp6Fm8nd
    mov dword[edi+70h*4],SA1COp70
    mov dword[edi+71h*4],SA1COp71m8nd
    mov dword[edi+72h*4],SA1COp72m8nd
    mov dword[edi+73h*4],SA1COp73m8nd
    mov dword[edi+74h*4],SA1COp74m8
    mov dword[edi+75h*4],SA1COp75m8nd
    mov dword[edi+76h*4],SA1COp76m8
    mov dword[edi+77h*4],SA1COp77m8nd
    mov dword[edi+78h*4],SA1COp78
    mov dword[edi+79h*4],SA1COp79m8nd
    mov dword[edi+7Ah*4],SA1COp7Ax8
    mov dword[edi+7Bh*4],SA1COp7B
    mov dword[edi+7Ch*4],SA1COp7C
    mov dword[edi+7Dh*4],SA1COp7Dm8nd
    mov dword[edi+7Eh*4],SA1COp7Em8
    mov dword[edi+7Fh*4],SA1COp7Fm8nd
    mov dword[edi+80h*4],SA1COp80
    mov dword[edi+81h*4],SA1COp81m8
    mov dword[edi+82h*4],SA1COp82
    mov dword[edi+83h*4],SA1COp83m8
    mov dword[edi+84h*4],SA1COp84x8
    mov dword[edi+85h*4],SA1COp85m8
    mov dword[edi+86h*4],SA1COp86x8
    mov dword[edi+87h*4],SA1COp87m8
    mov dword[edi+88h*4],SA1COp88x8
    mov dword[edi+89h*4],SA1COp89m8
    mov dword[edi+8Ah*4],SA1COp8Am8
    mov dword[edi+8Bh*4],SA1COp8B
    mov dword[edi+8Ch*4],SA1COp8Cx8
    mov dword[edi+8Dh*4],SA1COp8Dm8
    mov dword[edi+8Eh*4],SA1COp8Ex8
    mov dword[edi+8Fh*4],SA1COp8Fm8
    mov dword[edi+90h*4],SA1COp90
    mov dword[edi+91h*4],SA1COp91m8
    mov dword[edi+92h*4],SA1COp92m8
    mov dword[edi+93h*4],SA1COp93m8
    mov dword[edi+94h*4],SA1COp94x8
    mov dword[edi+95h*4],SA1COp95m8
    mov dword[edi+96h*4],SA1COp96x8
    mov dword[edi+97h*4],SA1COp97m8
    mov dword[edi+98h*4],SA1COp98m8
    mov dword[edi+99h*4],SA1COp99m8
    mov dword[edi+9Ah*4],SA1COp9A
    mov dword[edi+9Bh*4],SA1COp9Bx8
    mov dword[edi+9Ch*4],SA1COp9Cm8
    mov dword[edi+9Dh*4],SA1COp9Dm8
    mov dword[edi+9Eh*4],SA1COp9Em8
    mov dword[edi+9Fh*4],SA1COp9Fm8
    mov dword[edi+0A0h*4],SA1COpA0x8
    mov dword[edi+0A1h*4],SA1COpA1m8
    mov dword[edi+0A2h*4],SA1COpA2x8
    mov dword[edi+0A3h*4],SA1COpA3m8
    mov dword[edi+0A4h*4],SA1COpA4x8
    mov dword[edi+0A5h*4],SA1COpA5m8
    mov dword[edi+0A6h*4],SA1COpA6x8
    mov dword[edi+0A7h*4],SA1COpA7m8
    mov dword[edi+0A8h*4],SA1COpA8x8
    mov dword[edi+0A9h*4],SA1COpA9m8
    mov dword[edi+0AAh*4],SA1COpAAx8
    mov dword[edi+0ABh*4],SA1COpAB
    mov dword[edi+0ACh*4],SA1COpACx8
    mov dword[edi+0ADh*4],SA1COpADm8
    mov dword[edi+0AEh*4],SA1COpAEx8
    mov dword[edi+0AFh*4],SA1COpAFm8
    mov dword[edi+0B0h*4],SA1COpB0
    mov dword[edi+0B1h*4],SA1COpB1m8
    mov dword[edi+0B2h*4],SA1COpB2m8
    mov dword[edi+0B3h*4],SA1COpB3m8
    mov dword[edi+0B4h*4],SA1COpB4x8
    mov dword[edi+0B5h*4],SA1COpB5m8
    mov dword[edi+0B6h*4],SA1COpB6x8
    mov dword[edi+0B7h*4],SA1COpB7m8
    mov dword[edi+0B8h*4],SA1COpB8
    mov dword[edi+0B9h*4],SA1COpB9m8
    mov dword[edi+0BAh*4],SA1COpBAx8
    mov dword[edi+0BBh*4],SA1COpBBx8
    mov dword[edi+0BCh*4],SA1COpBCx8
    mov dword[edi+0BDh*4],SA1COpBDm8
    mov dword[edi+0BEh*4],SA1COpBEx8
    mov dword[edi+0BFh*4],SA1COpBFm8
    mov dword[edi+0C0h*4],SA1COpC0x8
    mov dword[edi+0C1h*4],SA1COpC1m8
    mov dword[edi+0C2h*4],SA1COpC2
    mov dword[edi+0C3h*4],SA1COpC3m8
    mov dword[edi+0C4h*4],SA1COpC4x8
    mov dword[edi+0C5h*4],SA1COpC5m8
    mov dword[edi+0C6h*4],SA1COpC6m8
    mov dword[edi+0C7h*4],SA1COpC7m8
    mov dword[edi+0C8h*4],SA1COpC8x8
    mov dword[edi+0C9h*4],SA1COpC9m8
    mov dword[edi+0CAh*4],SA1COpCAx8
    mov dword[edi+0CBh*4],SA1COpCB
    mov dword[edi+0CCh*4],SA1COpCCx8
    mov dword[edi+0CDh*4],SA1COpCDm8
    mov dword[edi+0CEh*4],SA1COpCEm8
    mov dword[edi+0CFh*4],SA1COpCFm8
    mov dword[edi+0D0h*4],SA1COpD0
    mov dword[edi+0D1h*4],SA1COpD1m8
    mov dword[edi+0D2h*4],SA1COpD2m8
    mov dword[edi+0D3h*4],SA1COpD3m8
    mov dword[edi+0D4h*4],SA1COpD4
    mov dword[edi+0D5h*4],SA1COpD5m8
    mov dword[edi+0D6h*4],SA1COpD6m8
    mov dword[edi+0D7h*4],SA1COpD7m8
    mov dword[edi+0D8h*4],SA1COpD8
    mov dword[edi+0D9h*4],SA1COpD9m8
    mov dword[edi+0DAh*4],SA1COpDAx8
    mov dword[edi+0DBh*4],SA1COpDB
    mov dword[edi+0DCh*4],SA1COpDC
    mov dword[edi+0DDh*4],SA1COpDDm8
    mov dword[edi+0DEh*4],SA1COpDEm8
    mov dword[edi+0DFh*4],SA1COpDFm8
    mov dword[edi+0E0h*4],SA1COpE0x8
    mov dword[edi+0E1h*4],SA1COpE1m8nd
    mov dword[edi+0E2h*4],SA1COpE2
    mov dword[edi+0E3h*4],SA1COpE3m8nd
    mov dword[edi+0E4h*4],SA1COpE4x8
    mov dword[edi+0E5h*4],SA1COpE5m8nd
    mov dword[edi+0E6h*4],SA1COpE6m8
    mov dword[edi+0E7h*4],SA1COpE7m8nd
    mov dword[edi+0E8h*4],SA1COpE8x8
    mov dword[edi+0E9h*4],SA1COpE9m8nd
    mov dword[edi+0EAh*4],SA1COpEA
    mov dword[edi+0EBh*4],SA1COpEB
    mov dword[edi+0ECh*4],SA1COpECx8
    mov dword[edi+0EDh*4],SA1COpEDm8nd
    mov dword[edi+0EEh*4],SA1COpEEm8
    mov dword[edi+0EFh*4],SA1COpEFm8nd
    mov dword[edi+0F0h*4],SA1COpF0
    mov dword[edi+0F1h*4],SA1COpF1m8nd
    mov dword[edi+0F2h*4],SA1COpF2m8nd
    mov dword[edi+0F3h*4],SA1COpF3m8nd
    mov dword[edi+0F4h*4],SA1COpF4
    mov dword[edi+0F5h*4],SA1COpF5m8nd
    mov dword[edi+0F6h*4],SA1COpF6m8
    mov dword[edi+0F7h*4],SA1COpF7m8nd
    mov dword[edi+0F8h*4],SA1COpF8
    mov dword[edi+0F9h*4],SA1COpF9m8nd
    mov dword[edi+0FAh*4],SA1COpFAx8
    mov dword[edi+0FBh*4],SA1COpFB
    mov dword[edi+0FCh*4],SA1COpFC
    mov dword[edi+0FDh*4],SA1COpFDm8nd
    mov dword[edi+0FEh*4],SA1COpFEm8
    mov dword[edi+0FFh*4],SA1COpFFm8nd
    ret

NEWSYM SA1settablem16
    mov dword[edi+01h*4],SA1COp01m16
    mov dword[edi+03h*4],SA1COp03m16
    mov dword[edi+04h*4],SA1COp04m16
    mov dword[edi+05h*4],SA1COp05m16
    mov dword[edi+06h*4],SA1COp06m16
    mov dword[edi+07h*4],SA1COp07m16
    mov dword[edi+09h*4],SA1COp09m16
    mov dword[edi+0Ah*4],SA1COp0Am16
    mov dword[edi+0Ch*4],SA1COp0Cm16
    mov dword[edi+0Dh*4],SA1COp0Dm16
    mov dword[edi+0Eh*4],SA1COp0Em16
    mov dword[edi+0Fh*4],SA1COp0Fm16
    mov dword[edi+11h*4],SA1COp11m16
    mov dword[edi+12h*4],SA1COp12m16
    mov dword[edi+13h*4],SA1COp13m16
    mov dword[edi+14h*4],SA1COp14m16
    mov dword[edi+15h*4],SA1COp15m16
    mov dword[edi+16h*4],SA1COp16m16
    mov dword[edi+17h*4],SA1COp17m16
    mov dword[edi+19h*4],SA1COp19m16
    mov dword[edi+1Ah*4],SA1COp1Am16
    mov dword[edi+1Ch*4],SA1COp1Cm16
    mov dword[edi+1Dh*4],SA1COp1Dm16
    mov dword[edi+1Eh*4],SA1COp1Em16
    mov dword[edi+1Fh*4],SA1COp1Fm16
    mov dword[edi+21h*4],SA1COp21m16
    mov dword[edi+23h*4],SA1COp23m16
    mov dword[edi+24h*4],SA1COp24m16
    mov dword[edi+25h*4],SA1COp25m16
    mov dword[edi+26h*4],SA1COp26m16
    mov dword[edi+27h*4],SA1COp27m16
    mov dword[edi+29h*4],SA1COp29m16
    mov dword[edi+2Ah*4],SA1COp2Am16
    mov dword[edi+2Ch*4],SA1COp2Cm16
    mov dword[edi+2Dh*4],SA1COp2Dm16
    mov dword[edi+2Eh*4],SA1COp2Em16
    mov dword[edi+2Fh*4],SA1COp2Fm16
    mov dword[edi+31h*4],SA1COp31m16
    mov dword[edi+32h*4],SA1COp32m16
    mov dword[edi+33h*4],SA1COp33m16
    mov dword[edi+34h*4],SA1COp34m16
    mov dword[edi+35h*4],SA1COp35m16
    mov dword[edi+36h*4],SA1COp36m16
    mov dword[edi+37h*4],SA1COp37m16
    mov dword[edi+39h*4],SA1COp39m16
    mov dword[edi+3Ah*4],SA1COp3Am16
    mov dword[edi+3Ch*4],SA1COp3Cm16
    mov dword[edi+3Dh*4],SA1COp3Dm16
    mov dword[edi+3Eh*4],SA1COp3Em16
    mov dword[edi+3Fh*4],SA1COp3Fm16
    mov dword[edi+41h*4],SA1COp41m16
    mov dword[edi+43h*4],SA1COp43m16
    mov dword[edi+45h*4],SA1COp45m16
    mov dword[edi+46h*4],SA1COp46m16
    mov dword[edi+47h*4],SA1COp47m16
    mov dword[edi+48h*4],SA1COp48m16
    mov dword[edi+49h*4],SA1COp49m16
    mov dword[edi+4Ah*4],SA1COp4Am16
    mov dword[edi+4Dh*4],SA1COp4Dm16
    mov dword[edi+4Eh*4],SA1COp4Em16
    mov dword[edi+4Fh*4],SA1COp4Fm16
    mov dword[edi+51h*4],SA1COp51m16
    mov dword[edi+52h*4],SA1COp52m16
    mov dword[edi+53h*4],SA1COp53m16
    mov dword[edi+55h*4],SA1COp55m16
    mov dword[edi+56h*4],SA1COp56m16
    mov dword[edi+57h*4],SA1COp57m16
    mov dword[edi+59h*4],SA1COp59m16
    mov dword[edi+5Dh*4],SA1COp5Dm16
    mov dword[edi+5Eh*4],SA1COp5Em16
    mov dword[edi+5Fh*4],SA1COp5Fm16
    mov dword[edi+61h*4],SA1COp61m16nd
    mov dword[edi+63h*4],SA1COp63m16nd
    mov dword[edi+64h*4],SA1COp64m16
    mov dword[edi+65h*4],SA1COp65m16nd
    mov dword[edi+66h*4],SA1COp66m16
    mov dword[edi+67h*4],SA1COp67m16nd
    mov dword[edi+68h*4],SA1COp68m16
    mov dword[edi+69h*4],SA1COp69m16nd
    mov dword[edi+6Ah*4],SA1COp6Am16
    mov dword[edi+6Dh*4],SA1COp6Dm16nd
    mov dword[edi+6Eh*4],SA1COp6Em16
    mov dword[edi+6Fh*4],SA1COp6Fm16nd
    mov dword[edi+71h*4],SA1COp71m16nd
    mov dword[edi+72h*4],SA1COp72m16nd
    mov dword[edi+73h*4],SA1COp73m16nd
    mov dword[edi+74h*4],SA1COp74m16
    mov dword[edi+75h*4],SA1COp75m16nd
    mov dword[edi+76h*4],SA1COp76m16
    mov dword[edi+77h*4],SA1COp77m16nd
    mov dword[edi+79h*4],SA1COp79m16nd
    mov dword[edi+7Dh*4],SA1COp7Dm16nd
    mov dword[edi+7Eh*4],SA1COp7Em16
    mov dword[edi+7Fh*4],SA1COp7Fm16nd
    mov dword[edi+81h*4],SA1COp81m16
    mov dword[edi+83h*4],SA1COp83m16
    mov dword[edi+85h*4],SA1COp85m16
    mov dword[edi+87h*4],SA1COp87m16
    mov dword[edi+89h*4],SA1COp89m16
    mov dword[edi+8Ah*4],SA1COp8Am16
    mov dword[edi+8Dh*4],SA1COp8Dm16
    mov dword[edi+8Fh*4],SA1COp8Fm16
    mov dword[edi+91h*4],SA1COp91m16
    mov dword[edi+92h*4],SA1COp92m16
    mov dword[edi+93h*4],SA1COp93m16
    mov dword[edi+95h*4],SA1COp95m16
    mov dword[edi+97h*4],SA1COp97m16
    mov dword[edi+98h*4],SA1COp98m16
    mov dword[edi+99h*4],SA1COp99m16
    mov dword[edi+9Ch*4],SA1COp9Cm16
    mov dword[edi+9Dh*4],SA1COp9Dm16
    mov dword[edi+9Eh*4],SA1COp9Em16
    mov dword[edi+9Fh*4],SA1COp9Fm16
    mov dword[edi+0A1h*4],SA1COpA1m16
    mov dword[edi+0A3h*4],SA1COpA3m16
    mov dword[edi+0A5h*4],SA1COpA5m16
    mov dword[edi+0A7h*4],SA1COpA7m16
    mov dword[edi+0A9h*4],SA1COpA9m16
    mov dword[edi+0ADh*4],SA1COpADm16
    mov dword[edi+0AFh*4],SA1COpAFm16
    mov dword[edi+0B1h*4],SA1COpB1m16
    mov dword[edi+0B2h*4],SA1COpB2m16
    mov dword[edi+0B3h*4],SA1COpB3m16
    mov dword[edi+0B5h*4],SA1COpB5m16
    mov dword[edi+0B7h*4],SA1COpB7m16
    mov dword[edi+0B9h*4],SA1COpB9m16
    mov dword[edi+0BDh*4],SA1COpBDm16
    mov dword[edi+0BFh*4],SA1COpBFm16
    mov dword[edi+0C1h*4],SA1COpC1m16
    mov dword[edi+0C3h*4],SA1COpC3m16
    mov dword[edi+0C5h*4],SA1COpC5m16
    mov dword[edi+0C6h*4],SA1COpC6m16
    mov dword[edi+0C7h*4],SA1COpC7m16
    mov dword[edi+0C9h*4],SA1COpC9m16
    mov dword[edi+0CDh*4],SA1COpCDm16
    mov dword[edi+0CEh*4],SA1COpCEm16
    mov dword[edi+0CFh*4],SA1COpCFm16
    mov dword[edi+0D1h*4],SA1COpD1m16
    mov dword[edi+0D2h*4],SA1COpD2m16
    mov dword[edi+0D3h*4],SA1COpD3m16
    mov dword[edi+0D5h*4],SA1COpD5m16
    mov dword[edi+0D6h*4],SA1COpD6m16
    mov dword[edi+0D7h*4],SA1COpD7m16
    mov dword[edi+0D9h*4],SA1COpD9m16
    mov dword[edi+0DDh*4],SA1COpDDm16
    mov dword[edi+0DEh*4],SA1COpDEm16
    mov dword[edi+0DFh*4],SA1COpDFm16
    mov dword[edi+0E1h*4],SA1COpE1m16nd
    mov dword[edi+0E3h*4],SA1COpE3m16nd
    mov dword[edi+0E5h*4],SA1COpE5m16nd
    mov dword[edi+0E6h*4],SA1COpE6m16
    mov dword[edi+0E7h*4],SA1COpE7m16nd
    mov dword[edi+0E9h*4],SA1COpE9m16nd
    mov dword[edi+0EDh*4],SA1COpEDm16nd
    mov dword[edi+0EEh*4],SA1COpEEm16
    mov dword[edi+0EFh*4],SA1COpEFm16nd
    mov dword[edi+0F1h*4],SA1COpF1m16nd
    mov dword[edi+0F2h*4],SA1COpF2m16nd
    mov dword[edi+0F3h*4],SA1COpF3m16nd
    mov dword[edi+0F5h*4],SA1COpF5m16nd
    mov dword[edi+0F6h*4],SA1COpF6m16
    mov dword[edi+0F7h*4],SA1COpF7m16nd
    mov dword[edi+0F9h*4],SA1COpF9m16nd
    mov dword[edi+0FDh*4],SA1COpFDm16nd
    mov dword[edi+0FEh*4],SA1COpFEm16
    mov dword[edi+0FFh*4],SA1COpFFm16nd
    ret

NEWSYM SA1settablex16
    mov dword[edi+5Ah*4],SA1COp5Ax16
    mov dword[edi+7Ah*4],SA1COp7Ax16
    mov dword[edi+84h*4],SA1COp84x16
    mov dword[edi+86h*4],SA1COp86x16
    mov dword[edi+88h*4],SA1COp88x16
    mov dword[edi+8Ch*4],SA1COp8Cx16
    mov dword[edi+8Eh*4],SA1COp8Ex16
    mov dword[edi+94h*4],SA1COp94x16
    mov dword[edi+96h*4],SA1COp96x16
    mov dword[edi+9Bh*4],SA1COp9Bx16
    mov dword[edi+0A0h*4],SA1COpA0x16
    mov dword[edi+0A2h*4],SA1COpA2x16
    mov dword[edi+0A4h*4],SA1COpA4x16
    mov dword[edi+0A6h*4],SA1COpA6x16
    mov dword[edi+0A8h*4],SA1COpA8x16
    mov dword[edi+0AAh*4],SA1COpAAx16
    mov dword[edi+0ACh*4],SA1COpACx16
    mov dword[edi+0AEh*4],SA1COpAEx16
    mov dword[edi+0B4h*4],SA1COpB4x16
    mov dword[edi+0B6h*4],SA1COpB6x16
    mov dword[edi+0BAh*4],SA1COpBAx16
    mov dword[edi+0BBh*4],SA1COpBBx16
    mov dword[edi+0BCh*4],SA1COpBCx16
    mov dword[edi+0BEh*4],SA1COpBEx16
    mov dword[edi+0C0h*4],SA1COpC0x16
    mov dword[edi+0C4h*4],SA1COpC4x16
    mov dword[edi+0C8h*4],SA1COpC8x16
    mov dword[edi+0CAh*4],SA1COpCAx16
    mov dword[edi+0CCh*4],SA1COpCCx16
    mov dword[edi+0DAh*4],SA1COpDAx16
    mov dword[edi+0E0h*4],SA1COpE0x16
    mov dword[edi+0E4h*4],SA1COpE4x16
    mov dword[edi+0E8h*4],SA1COpE8x16
    mov dword[edi+0ECh*4],SA1COpECx16
    mov dword[edi+0FAh*4],SA1COpFAx16
    ret

NEWSYM SA1settableDm8
    mov dword[edi+61h*4],SA1COp61m8d
    mov dword[edi+63h*4],SA1COp63m8d
    mov dword[edi+65h*4],SA1COp65m8d
    mov dword[edi+67h*4],SA1COp67m8d
    mov dword[edi+69h*4],SA1COp69m8d
    mov dword[edi+6Dh*4],SA1COp6Dm8d
    mov dword[edi+6Fh*4],SA1COp6Fm8d
    mov dword[edi+71h*4],SA1COp71m8d
    mov dword[edi+72h*4],SA1COp72m8d
    mov dword[edi+73h*4],SA1COp73m8d
    mov dword[edi+75h*4],SA1COp75m8d
    mov dword[edi+77h*4],SA1COp77m8d
    mov dword[edi+79h*4],SA1COp79m8d
    mov dword[edi+7Dh*4],SA1COp7Dm8d
    mov dword[edi+7Fh*4],SA1COp7Fm8d
    mov dword[edi+0E1h*4],SA1COpE1m8d
    mov dword[edi+0E3h*4],SA1COpE3m8d
    mov dword[edi+0E5h*4],SA1COpE5m8d
    mov dword[edi+0E7h*4],SA1COpE7m8d
    mov dword[edi+0E9h*4],SA1COpE9m8d
    mov dword[edi+0EDh*4],SA1COpEDm8d
    mov dword[edi+0EFh*4],SA1COpEFm8d
    mov dword[edi+0F1h*4],SA1COpF1m8d
    mov dword[edi+0F2h*4],SA1COpF2m8d
    mov dword[edi+0F3h*4],SA1COpF3m8d
    mov dword[edi+0F5h*4],SA1COpF5m8d
    mov dword[edi+0F7h*4],SA1COpF7m8d
    mov dword[edi+0F9h*4],SA1COpF9m8d
    mov dword[edi+0FDh*4],SA1COpFDm8d
    mov dword[edi+0FFh*4],SA1COpFFm8d
    ret

NEWSYM SA1settableDm16
    mov dword[edi+61h*4],SA1COp61m16d
    mov dword[edi+63h*4],SA1COp63m16d
    mov dword[edi+65h*4],SA1COp65m16d
    mov dword[edi+67h*4],SA1COp67m16d
    mov dword[edi+69h*4],SA1COp69m16d
    mov dword[edi+6Dh*4],SA1COp6Dm16d
    mov dword[edi+6Fh*4],SA1COp6Fm16d
    mov dword[edi+71h*4],SA1COp71m16d
    mov dword[edi+72h*4],SA1COp72m16d
    mov dword[edi+73h*4],SA1COp73m16d
    mov dword[edi+75h*4],SA1COp75m16d
    mov dword[edi+77h*4],SA1COp77m16d
    mov dword[edi+79h*4],SA1COp79m16d
    mov dword[edi+7Dh*4],SA1COp7Dm16d
    mov dword[edi+7Fh*4],SA1COp7Fm16d
    mov dword[edi+0E1h*4],SA1COpE1m16d
    mov dword[edi+0E3h*4],SA1COpE3m16d
    mov dword[edi+0E5h*4],SA1COpE5m16d
    mov dword[edi+0E7h*4],SA1COpE7m16d
    mov dword[edi+0E9h*4],SA1COpE9m16d
    mov dword[edi+0EDh*4],SA1COpEDm16d
    mov dword[edi+0EFh*4],SA1COpEFm16d
    mov dword[edi+0F1h*4],SA1COpF1m16d
    mov dword[edi+0F2h*4],SA1COpF2m16d
    mov dword[edi+0F3h*4],SA1COpF3m16d
    mov dword[edi+0F5h*4],SA1COpF5m16d
    mov dword[edi+0F7h*4],SA1COpF7m16d
    mov dword[edi+0F9h*4],SA1COpF9m16d
    mov dword[edi+0FDh*4],SA1COpFDm16d
    mov dword[edi+0FFh*4],SA1COpFFm16d
    ret


