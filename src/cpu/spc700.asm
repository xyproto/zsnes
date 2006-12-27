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

EXTSYM DSPMem,spcWptr,disablespcclr,SPCSkipXtraROM,cycpbl,spcRptr
EXTSYM spc700read,dspWptr,curexecstate,tableadc

%include "cpu/regsw.mac"
%include "cpu/spcdef.inc"
%include "cpu/spcaddr.inc"

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
NEWSYM SPCRAM,   times 65472 db 0FFh  ; Pointer to the SPC's RAM
; copy #1
; THE SPC ROM :)
   db 0CDh,0EFh,0BDh,0E8h,000h,0C6h,01Dh,0D0h,0FCh,08Fh,0AAh,0F4h,08Fh,0BBh,0F5h,078h
   db 0CCh,0F4h,0D0h,0FBh,02Fh,019h,0EBh,0F4h,0D0h,0FCh,07Eh,0F4h,0D0h,00Bh,0E4h,0F5h
   db 0CBh,0F4h,0D7h,000h,0FCh,0D0h,0F3h,0ABh,001h,010h,0EFh,07Eh,0F4h,010h,0EBh,0BAh
   db 0F6h,0DAh,000h,0BAh,0F4h,0C4h,0F4h,0DDh,05Dh,0D0h,0DBh,01Fh,000h,000h,0C0h,0FFh
   db 0AAh,0BBh,0CCh,0DDh,0EEh,0FFh,000h,011h,022h,033h,044h,055h,066h,077h,088h,099h

NEWSYM spcPCRam,
       dd 0     ; Program Counter (with SPCRAM added)
NEWSYM spcA,
       db 0     ; The A register (general purpose)
       db 0
       db 0
       db 0
NEWSYM spcX,
       db 0     ; The X register (general purpose)
       db 0
       db 0
       db 0
NEWSYM spcY,
       db 0     ; The Y register (general purpose)
       db 0
       db 0
       db 0
NEWSYM spcP,
       db 0     ; The processor status byte (Removed for each flags)
       db 0     ; NZ are not always processed...
       db 0
       db 0
NEWSYM spcNZ,
       db 0     ; The processor NZ flag (little speed up hack :) )
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

NEWSYM spcS,     dd 1FFh  ; The stack pointer (always from 100 to 1FF) (added Ram)
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

%macro WriteByte 0
  cmp ebx,0ffh+SPCRAM
  ja %%extramem
  cmp ebx,0f0h+SPCRAM
  jb %%normalmem
  sub ebx,SPCRAM
  call dword near [spcWptr+ebx*4-0f0h*4]
  jmp %%finished
%%extramem
  cmp ebx,0ffc0h+SPCRAM
  jb %%normalmem
  mov [spcextraram+ebx-0FFC0h-SPCRAM],al
  test byte[SPCRAM+0F1h],80h
  jnz %%finished
;  push ecx
;  mov cl,[DSPMem+06Ch]
;  test cl,20h
;  pop ecx
;  jz .finished
%%normalmem
  mov [ebx],al
%%finished
%endmacro

%macro ReadByte 0
  cmp ebx,0f0h+SPCRAM
  jb %%normalmem2
  cmp ebx,0ffh+SPCRAM
  ja %%normalmem
  sub ebx,SPCRAM
  call dword near [spcRptr+ebx*4-0f0h*4]
  jmp %%finished
%%normalmem
;  cmp ebx,0ffc0h+SPCRAM
;  jb .rnormalmem2
;  test byte[DSPMem+6Ch],10h
;  jz .rnormalmem2
;  mov al,[spcextraram+ebx-0FFC0h-SPCRAM]
;  jmp .rfinished
%%normalmem2
   mov al,[ebx]
%%finished
%endmacro

%macro ReadByte2 0
  cmp ebx,0f0h+SPCRAM
  jb %%normalmem2
  cmp ebx,0ffh+SPCRAM
  ja %%normalmem
  sub ebx,SPCRAM
  call dword near [spcRptr+ebx*4-0f0h*4]
  add ebx,SPCRAM
  jmp %%finished
%%normalmem
;  cmp ebx,0ffc0h+SPCRAM
;  jb .rnormalmem2
;  test byte[DSPMem+6Ch],10h
;  jz .rnormalmem2
;  mov al,[spcextraram+ebx-0FFC0h-SPCRAM]
;  jmp .rfinished
%%normalmem2
   mov al,[ebx]
%%finished
%endmacro

SECTION .data
NEWSYM timer2upd, dd 0
SECTION .text

; This function is called every scanline (262*60 times/sec)
; Make it call 0.9825 times (393/400) (skip when divisible by 64)
; 2 8khz, 1 64khz

NEWSYM updatetimer
;    inc dword[timer2upd]
;    cmp dword[timer2upd],400
;    jne .nowrap
;    mov dword[timer2upd],0
;.nowrap
;.again
;    mov eax,dword[timer2upd]
;    shr eax,6
;    shl eax,6
;    cmp eax,dword[timer2upd]
;    je near .noin2d


.another
    xor byte[timrcall],01h
    test byte[timrcall],01h
    jz near .notimer
    test byte[timeron],1
    jz .noin0
    dec byte[timinl0]
    jnz .noin0
    inc byte[SPCRAM+0FDh]
    mov al,[timincr0]
    mov [timinl0],al
    cmp byte[SPCRAM+0FDh],1
    jne .noin0
    reenablespc
    mov dword[cycpbl],0
.noin0
    test byte[timeron],2
    jz .noin1
    dec byte[timinl1]
    jnz .noin1
    inc byte[SPCRAM+0FEh]
    mov al,[timincr1]
    mov [timinl1],al
    cmp byte[SPCRAM+0FEh],1
    jne .noin1
    reenablespc
    mov dword[cycpbl],0
.noin1
.notimer
    test byte[timeron],4
    jz near .noin2d2
    dec byte[timinl2]
    jnz .noin2
    inc byte[SPCRAM+0FFh]
    mov al,[timincr2]
    mov [timinl2],al
    cmp byte[SPCRAM+0FFh],1
    jne .noin2
    reenablespc
    mov dword[cycpbl],0
.noin2
    dec byte[timinl2]
    jnz .noin2b
    inc byte[SPCRAM+0FFh]
    mov al,[timincr2]
    mov [timinl2],al
    cmp byte[SPCRAM+0FFh],1
    jne .noin2b
    reenablespc
    mov dword[cycpbl],0
.noin2b
    dec byte[timinl2]
    jnz .noin2c
    inc byte[SPCRAM+0FFh]
    mov al,[timincr2]
    mov [timinl2],al
    cmp byte[SPCRAM+0FFh],1
    jne .noin2c
    reenablespc
    mov dword[cycpbl],0
.noin2c
    dec byte[timinl2]
    jnz .noin2d
    inc byte[SPCRAM+0FFh]
    mov al,[timincr2]
    mov [timinl2],al
    cmp byte[SPCRAM+0FFh],1
    jne .noin2d
    reenablespc
    mov dword[cycpbl],0
.noin2d
.noin2d2
;    inc dword[timer2upd]
;    cmp dword[timer2upd],31
;    jne .nowrap
;    mov dword[timer2upd],0
;    jmp .again
;.nowrap

    inc dword[timer2upd]
    cmp dword[timer2upd],60
    jne .noanother
    mov dword[timer2upd],0
    jmp .another
.noanother

    ret


; SPC Write Registers
; DO NOT MODIFY DX OR ECX!
NEWSYM SPCRegF0
    mov [SPCRAM+0F0h],al
    ret
NEWSYM SPCRegF1
    cmp byte[disablespcclr],1
    je .No23Clear
    test al,10h
    jz .No01Clear
    mov byte[SPCRAM+0F4h],0
    mov byte[SPCRAM+0F5h],0
.No01Clear
    test al,20h
    jz .No23Clear
    mov byte[SPCRAM+0F6h],0
    mov byte[SPCRAM+0F7h],0
.No23Clear
    cmp byte[SPCSkipXtraROM],1
    je near .AfterNoROM
    test al,80h
    jz .NoROM
    push eax
    push ebx
    xor eax,eax
.loopa
    mov bl,[SPCROM+eax]
    mov [SPCRAM+0FFC0h+eax],bl
    inc eax
    cmp eax,040h
    jne .loopa
    pop ebx
    pop eax
    jmp .AfterNoROM
.NoROM
    push eax
    push ebx
    xor eax,eax
.loopb
    mov bl,[spcextraram+eax]
    mov [SPCRAM+0FFC0h+eax],bl
    inc eax
    cmp eax,040h
    jne .loopb
    pop ebx
    pop eax
.AfterNoROM
    mov [SPCRAM+0F1h],al
    and al,0Fh
    mov [timeron],al
    ret
NEWSYM SPCRegF2
    mov [SPCRAM+0F2h],al
    push eax
    push ebx
    xor eax,eax
    mov al,[SPCRAM+0F2h]
    mov bl,[DSPMem+eax]
    mov [SPCRAM+0F3h],bl
    pop ebx
    pop eax
    ret
NEWSYM SPCRegF3
    push ebx
    xor ebx,ebx
    mov bl,[SPCRAM+0F2h]
    and bl,07fh
    call dword near [dspWptr+ebx*4]
    pop ebx
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegF4
    mov [reg1read],al
    inc dword[spc700read]
    ret
NEWSYM SPCRegF5
    mov [reg2read],al
    inc dword[spc700read]
    ret
NEWSYM SPCRegF6
    mov [reg3read],al
    inc dword[spc700read]
    ret
NEWSYM SPCRegF7
    mov [reg4read],al
    inc dword[spc700read]
    ret
NEWSYM SPCRegF8
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegF9
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegFA
    mov [timincr0],al
    test byte[timinl0],0FFh
    jne .nowrite
    mov [timinl0],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegFB
    mov [timincr1],al
    test byte[timinl1],0FFh
    jne .nowrite
    mov [timinl1],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegFC
    mov [timincr2],al
    test byte[timinl2],0FFh
    jne .nowrite
    mov [timinl2],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM SPCRegFD
    ret
NEWSYM SPCRegFE
    ret
NEWSYM SPCRegFF
    ret

; SPC Read Registers
; DO NOT MODIFY ANY REG!
; return data true al
NEWSYM RSPCRegF0
    mov al,[SPCRAM+0f0h]
    ret
NEWSYM RSPCRegF1
    mov al,[SPCRAM+0f1h]
    ret
NEWSYM RSPCRegF2
    mov al,[SPCRAM+0f2h]
    ret
NEWSYM RSPCRegF3
    mov al,[SPCRAM+0f3h]
    ret
NEWSYM RSPCRegF4
    mov al,[SPCRAM+0f4h]
    ret
NEWSYM RSPCRegF5
    mov al,[SPCRAM+0f5h]
    ret
NEWSYM RSPCRegF6
    mov al,[SPCRAM+0f6h]
    ret
NEWSYM RSPCRegF7
    mov al,[SPCRAM+0f7h]
    ret
NEWSYM RSPCRegF8
    mov al,0 ;[SPCRAM+0f8h]
    ret
NEWSYM RSPCRegF9
    mov al,0 ;[SPCRAM+0f9h]
    ret
NEWSYM RSPCRegFA
    mov al,[SPCRAM+0fah]
    ret
NEWSYM RSPCRegFB
    mov al,[SPCRAM+0fbh]
    ret
NEWSYM RSPCRegFC
    mov al,[SPCRAM+0fch]
    ret

NEWSYM RSPCRegFD
    mov al,[SPCRAM+0fdh]
    and al,0Fh
    mov byte[SPCRAM+0fdh],0
    mov byte[spcnumread],0
    ret

NEWSYM RSPCRegFE
    mov al,[SPCRAM+0feh]
    and al,0Fh
    mov byte[SPCRAM+0feh],0
    mov byte[spcnumread],0
    ret

NEWSYM RSPCRegFF
    mov al,[SPCRAM+0ffh]
    and al,0Fh
    mov byte[SPCRAM+0ffh],0
    mov byte[spcnumread],0
    ret

SECTION .data
NEWSYM spcnumread, db 0
SECTION .text

%macro SPCSetFlagnzc 0
  js .setsignflag
  jz .setzeroflag
  mov byte[spcNZ],1
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setsignflag
  mov byte[spcNZ],80h
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setzeroflag
  mov byte[spcNZ],0
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setcarryflag
  or byte[spcP],1
  ret
%endmacro

%macro SPCSetFlagnzcnoret 0
  js .setsignflag
  jz .setzeroflag
  mov byte[spcNZ],1
  jc .setcarryflag
  and byte[spcP],0FEh
  jmp .skipflags
.setsignflag
  mov byte[spcNZ],80h
  jc .setcarryflag
  and byte[spcP],0FEh
  jmp .skipflags
.setzeroflag
  mov byte[spcNZ],0
  jc .setcarryflag
  and byte[spcP],0FEh
  jmp .skipflags
.setcarryflag
  or byte[spcP],1
.skipflags
%endmacro

%macro SPCSetFlagnvhzc 0
  lahf
  js .setsignflag
  jz .setzeroflag
  mov byte[spcNZ],1
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setsignflag
  mov byte[spcNZ],80h
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setzeroflag
  mov byte[spcNZ],0
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setoverflowflag
  or byte[spcP],40h
.skipflags
  and byte[spcP],0F6h
  test ah,01h
  jz .noCarry
  or byte[spcP],1
.noCarry
  test ah,10h
  jz .nohf
  or byte[spcP],8
.nohf
  ret
%endmacro

%macro SPCSetFlagnvhzcnoret 0
  lahf
  js .setsignflag
  jz .setzeroflag
  mov byte[spcNZ],1
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setsignflag
  mov byte[spcNZ],80h
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setzeroflag
  mov byte[spcNZ],0
  jo .setoverflowflag
  and byte[spcP],0BFh
  jmp .skipflags
.setoverflowflag
  or byte[spcP],40h
.skipflags
  and byte[spcP],0F6h
  test ah,01h
  jz .noCarry
  or byte[spcP],1
.noCarry
  test ah,10h
  jz .nohf
  or byte[spcP],8
.nohf
%endmacro

;************************************************
; Misc Opcodes
;************************************************
NEWSYM Op00     ; NOP
    ret
NEWSYM OpEF     ; SLEEP      standby SLEEP mode    .........
    dec ebp
    ret
NEWSYM OpFF     ; STOP       standby STOP mode     .........
    inc dword[spc700read]
    dec ebp
    ret
NEWSYM Op9F     ; XCN A     A(7-4) <-> A(3-0)     N......Z.
    ror byte[spcA],4
    mov al,[spcA]
    mov [spcNZ],al
    ret

;************************************************
; Branch Stuff
;************************************************
NEWSYM Op10     ; BPL Branch on N=0
    test byte[spcNZ],128
    jz .branch
    spcbrancher
NEWSYM Op30     ; BMI Branch on N=1
    test byte[spcNZ],128
    jnz .branch
    spcbrancher
NEWSYM Op50     ; BVC Branch on V=0
    test byte[spcP],64
    jz .branch
    spcbrancher
NEWSYM Op70     ; BVS Branch on V=1
    test byte[spcP],64
    jnz .branch
    spcbrancher
NEWSYM Op90     ; BCC Branc on c=0
    test byte[spcP],1
    jz .branch
    spcbrancher
NEWSYM OpB0     ; BCS Branch on C=1
    test byte[spcP],1
    jnz .branch
    spcbrancher
NEWSYM OpD0     ; BNE branch on Z=0
    test byte[spcNZ],255
    jnz .branch
    spcbrancher
NEWSYM OpF0     ; BEQ Branch on Z=1
    test byte[spcNZ],0FFh
    jz .branch
    spcbrancher
NEWSYM Op2F     ; BRA rel    branch always            ...
    movsx ebx,byte[ebp]
    inc ebp
    add ebp,ebx
    ret


;************************************************
; Clear/Set Flag bits
;************************************************
;  CLRP           20    1     2   clear direct page flag    ..0.....
NEWSYM Op20     ; CLRP Clear direct page flag
    and byte[spcP],11011111b
    mov dword[spcRamDP],SPCRAM
    ret
;  SETP           40    1     2   set dorect page flag    ..1..0..
NEWSYM Op40     ; SETP Set Direct Page Flag  (Also clear interupt flag?)
    or byte[spcP],00100000b
    and byte[spcP],11111011b
    mov dword[spcRamDP],SPCRAM
    add dword[spcRamDP],100h
    ret
;  CLRC           60    1     2   clear carry flag        .......0
NEWSYM Op60     ; CLRC Clear carry flag
    and byte[spcP],11111110b
    ret
;  SETC           80    1     2   set carry flag        .......1
NEWSYM Op80     ; SETC Set carry flag
    or byte[spcP],00000001b
    ret
;  EI             A0    1     3  set interrup enable flag   .....1..
NEWSYM OpA0     ; EI set interrupt flag
    or byte[spcP],00000100b
    ret
;  DI             C0    1     3  clear interrup enable flag .....0..
NEWSYM OpC0     ; DI clear interrupt flag
    and byte[spcP],11111011b
    ret
;  CLRV           E0    1     2   clear V and H         .0..0...
NEWSYM OpE0     ; CLRV clear V and H
    and byte[spcP],10110111b
    ret
;  NOTC           ED    1     3   complement carry flag     .......C
NEWSYM OpED     ; NOTC       complement carry flag     .......C
    xor byte[spcP],00000001b
    ret

;************************************************
; TCALL instructions (Verified)
;************************************************
NEWSYM Op01     ; TCALL 0
    spctcall 30
NEWSYM Op11     ; TCALL 1
    spctcall 28
NEWSYM Op21     ; TCALL 2
    spctcall 26
NEWSYM Op31     ; TCALL 3
    spctcall 24
NEWSYM Op41     ; TCALL 4
    spctcall 22
NEWSYM Op51     ; TCALL 5
    spctcall 20
NEWSYM Op61     ; TCALL 6
    spctcall 18
NEWSYM Op71     ; TCALL 7
    spctcall 16
NEWSYM Op81     ; TCALL 8
    spctcall 14
NEWSYM Op91     ; TCALL 9
    spctcall 12
NEWSYM OpA1     ; TCALL A
    spctcall 10
NEWSYM OpB1     ; TCALL B
    spctcall 08
NEWSYM OpC1     ; TCALL C
    spctcall 06
NEWSYM OpD1     ; TCALL D
    spctcall 04
NEWSYM OpE1     ; TCALL E
    spctcall 02
NEWSYM OpF1     ; TCALL F
    spctcall 00

;************************************************
; SET1 instructions (Verified)
;************************************************
NEWSYM Op02     ; SET1 direct page bit 0
    set1 1
NEWSYM Op22     ; SET1 direct page bit 1
    set1 2
NEWSYM Op42     ; SET1 direct page bit 2
    set1 4
NEWSYM Op62     ; SET1 direct page bit 3
    set1 8
NEWSYM Op82     ; SET1 direct page bit 4
    set1 16
NEWSYM OpA2     ; SET1 direct page bit 5
    set1 32
NEWSYM OpC2     ; SET1 direct page bit 6
    set1 64
NEWSYM OpE2     ; SET1 direct page bit 7
    set1 128

;************************************************
; CLR1 instructions (Verified)
;************************************************
NEWSYM Op12     ; CLR1 direct page bit 0
    clr1 255-1
NEWSYM Op32     ; CLR1 direct page bit 1
    clr1 255-2
NEWSYM Op52     ; CLR1 direct page bit 2
    clr1 255-4
NEWSYM Op72     ; CLR1 direct page bit 3
    clr1 255-8
NEWSYM Op92     ; CLR1 direct page bit 4
    clr1 255-16
NEWSYM OpB2     ; CLR1 direct page bit 5
    clr1 255-32
NEWSYM OpD2     ; CLR1 direct page bit 6
    clr1 255-64
NEWSYM OpF2     ; CLR1 direct page bit 7
    clr1 255-128

;************************************************
; BBS instructions (Verified)
;************************************************
NEWSYM Op03     ; BBS direct page bit 0
    bbs 1
NEWSYM Op23     ; BBS direct page bit 1
    bbs 2
NEWSYM Op43     ; BBS direct page bit 2
    bbs 4
NEWSYM Op63     ; BBS direct page bit 3
    bbs 8
NEWSYM Op83     ; BBS direct page bit 4
    bbs 16
NEWSYM OpA3     ; BBS direct page bit 5
    bbs 32
NEWSYM OpC3     ; BBS direct page bit 6
    bbs 64
NEWSYM OpE3     ; BBS direct page bit 7
    bbs 128

;************************************************
; BBC instructions (Verified)
;************************************************
NEWSYM Op13     ; BBC direct page bit 0
    bbc 1
NEWSYM Op33     ; BBC direct page bit 1
    bbc 2
NEWSYM Op53     ; BBC direct page bit 2
    bbc 4
NEWSYM Op73     ; BBC direct page bit 3
    bbc 8
NEWSYM Op93     ; BBC direct page bit 4
    bbc 16
NEWSYM OpB3     ; BBC direct page bit 5
    bbc 32
NEWSYM OpD3     ; BBC direct page bit 6
    bbc 64
NEWSYM OpF3     ; BBC direct page bit 7
    bbc 128

;************************************************
; OR A,instructions
;************************************************
NEWSYM Op04     ; OR A,dp   A <- A OR (dp)    N.....Z.
    SPCaddr_DP
    SPC_OR_A
NEWSYM Op14     ; OR A,dp+X    A <- A OR (dp+X)     N.....Z.
    SPCaddr_DP_X
    SPC_OR_A
NEWSYM Op05     ; OR A,labs   A <- A OR (abs)     N.....Z.
    SPCaddr_LABS
    SPC_OR_A
NEWSYM Op15     ; OR A,labs+x  A <- A OR (abs+X)    N.....Z.
    SPCaddr_LABS_X
    SPC_OR_A
NEWSYM Op06     ; OR A,(X)     A <- A OR (X)      N.....Z.
    SPCaddr__X_
    SPC_OR_A
NEWSYM Op16     ; OR A,labs+Y  A <- A OR (abs+Y)    N......Z.
    SPCaddr_LABS_Y
    SPC_OR_A
NEWSYM Op07     ; OR A,(dp+X)  A <- A OR ((dp+X+1)(dp+X))  N......Z.
    SPCaddr_bDP_Xb
    SPC_OR_A
NEWSYM Op17     ; OR A,(dp)+Y  A <- A OR ((dp+1)(dp)+Y)   N......Z.
    SPCaddr_bDPb_Y
    SPC_OR_A
NEWSYM Op08     ; OR A,#inm    A <- A OR inm        N......Z.
    mov al,[ebp]
    inc ebp
    SPC_OR_A

;************************************************
; AND A, instructions
;************************************************
NEWSYM Op24     ; AND A,dp     A <- A AND (dp)    N.....Z.
    SPCaddr_DP
    SPC_AND_A
NEWSYM Op34     ; AND A,dp+x   A <- A AND (dp+X)    N.....Z.
    SPCaddr_DP_X
    SPC_AND_A
NEWSYM Op25     ; AND A,labs   A <- A AND (abs)     N.....Z.
    SPCaddr_LABS
    SPC_AND_A
NEWSYM Op35     ; AND A,labs+X A <- A AND (abs+X)   N.....Z.
    SPCaddr_LABS_X
    SPC_AND_A
NEWSYM Op26     ; AND A,(X)    A <- A AND (X)     N......Z.
    SPCaddr__X_
    SPC_AND_A
NEWSYM Op36     ; AND A,labs+Y A <- A AND (abs+Y)   N......Z.
    SPCaddr_LABS_Y
    SPC_AND_A
NEWSYM Op27     ; AND A,(dp+X) A <- A AND ((dp+X+1)(dp+X)) N......Z.
    SPCaddr_bDP_Xb
    SPC_AND_A
NEWSYM Op37     ; AND A,(dp)+Y A <- A AND ((dp+1)(dp)+Y)  N......Z.
    SPCaddr_bDPb_Y
    SPC_AND_A
NEWSYM Op28     ; AND A,#inm   A <- A AND inm         N......Z.
    mov al,[ebp]
    inc ebp
    SPC_AND_A

;************************************************
; EOR A, instructions
;************************************************
NEWSYM Op44     ; EOR A,dp     A <- A EOR (dp)    N.....Z.
    SPCaddr_DP
    SPC_EOR_A
NEWSYM Op54     ; EOR A,dp+x   A <- A EOR (dp+X)    N.....Z.
    SPCaddr_DP_X
    SPC_EOR_A
NEWSYM Op45     ; EOR A,labs   A <- A EOR (abs)     N.....Z.
    SPCaddr_LABS
    SPC_EOR_A
NEWSYM Op55     ; EOR A,labs+X A <- A EOR (abs+X)   N.....Z.
    SPCaddr_LABS_X
    SPC_EOR_A
NEWSYM Op46     ; EOR A,(X)    A <- A EOR (X)     N......Z.
    SPCaddr__X_
    SPC_EOR_A
NEWSYM Op56     ; EOR A,labs+Y A <- A EOR (abs+Y)   N......Z.
    SPCaddr_LABS_Y
    SPC_EOR_A
NEWSYM Op47     ; EOR A,(dp+X) A <- A EOR ((dp+X+1)(dp+X)) N......Z.
    SPCaddr_bDP_Xb
    SPC_EOR_A
NEWSYM Op57     ; EOR A,(dp)+Y A <- A EOR ((dp+1)(dp)+Y)  N......Z.
    SPCaddr_bDPb_Y
    SPC_EOR_A
NEWSYM Op48     ; EOR A,#inm   A <- A EOR inm         N......Z.
    mov al,[ebp]
    inc ebp
    SPC_EOR_A

;************************************************
; CMP A, instructions
;************************************************
NEWSYM Op64     ; CMP A,dp     A-(dp)           N.....ZC
    SPCaddr_DP
    SPC_CMP_A
NEWSYM Op74     ; CMP A,dp+x   A-(dp+X)         N.....ZC
    SPCaddr_DP_X
    SPC_CMP_A
NEWSYM Op65     ; CMP A,labs   A-(abs)          N.....ZC
    SPCaddr_LABS
    SPC_CMP_A
NEWSYM Op75     ; CMP A,labs+X A-(abs+X)        N.....ZC
    SPCaddr_LABS_X
    SPC_CMP_A
NEWSYM Op66     ; CMP A,(X)    A-(X)            N......ZC
    SPCaddr__X_
    SPC_CMP_A
NEWSYM Op76     ; CMP A,labs+Y A-(abs+Y)        N......ZC
    SPCaddr_LABS_Y
    SPC_CMP_A
NEWSYM Op67     ; CMP A,(dp+X) A-((dp+X+1)(dp+X))    N......ZC
    SPCaddr_bDP_Xb
    SPC_CMP_A
NEWSYM Op77     ; CMP A,(dp)+Y A-((dp+1)(dp)+Y)      N......ZC
    SPCaddr_bDPb_Y
    SPC_CMP_A
NEWSYM Op68     ; CMP A,#inm   A-inm             N......ZC
    mov al,[ebp]
    inc ebp
    SPC_CMP_A

;************************************************
; ADC A, instructions
;************************************************
NEWSYM Op84     ; ADC A,dp     A <- A+(dp)+C      NV..H.ZC
    SPCaddr_DP
    SPC_ADC_A
NEWSYM Op94     ; ADC A,dp+x   A <- A+(dp+X)+C    NV..H.ZC
    SPCaddr_DP_X
    SPC_ADC_A
NEWSYM Op85     ; ADC A,labs   A <- A+(abs)+C     NV..H.ZC
    SPCaddr_LABS
    SPC_ADC_A
NEWSYM Op95     ; ADC A,labs+X A <- A+(abs+X)+C     NV..H.ZC
    SPCaddr_LABS_X
    SPC_ADC_A
NEWSYM Op86     ; ADC A,(X)    A <- A+(X)+C       NV..H..ZC
    SPCaddr__X_
    SPC_ADC_A
NEWSYM Op96     ; ADC A,labs+Y A <- A+(abs+Y)+C     NV..H..ZC
    SPCaddr_LABS_Y
    SPC_ADC_A
NEWSYM Op87     ; ADC A,(dp+X) A <- A+((dp+X+1)(dp+X)) NV..H..ZC
    SPCaddr_bDP_Xb
    SPC_ADC_A
NEWSYM Op97     ; ADC A,(dp)+Y A <- A+((dp+1)(dp)+Y)   NV..H..ZC
    SPCaddr_bDPb_Y
    SPC_ADC_A
NEWSYM Op88     ; ADC A,#inm   A <- A+inm+C        NV..H..ZC
    mov al,[ebp]
    inc ebp
    SPC_ADC_A

;************************************************
; SBC A, instructions
;************************************************
NEWSYM OpA4     ; SBC A,dp     A <- A-(dp)-!C     NV..H.ZC
    SPCaddr_DP
    SPC_SBC_A
NEWSYM OpB4     ; SBC A,dp+x   A <- A-(dp+X)-!C     NV..H.ZC
    SPCaddr_DP_X
    SPC_SBC_A
NEWSYM OpA5     ; SBC A,labs   A <- A-(abs)-!C    NV..H.ZC
    SPCaddr_LABS
    SPC_SBC_A
NEWSYM OpB5     ; SBC A,labs+x A <- A-(abs+X)-!C    NV..H.ZC
    SPCaddr_LABS_X
    SPC_SBC_A
NEWSYM OpA6     ; SBC A,(X)    A <- A-(X)-!C      NV..H..ZC
    SPCaddr__X_
    SPC_SBC_A
NEWSYM OpB6     ; SBC A,labs+Y A <- A-(abs+Y)-!C    NV..H..ZC
    SPCaddr_LABS_Y
    SPC_SBC_A
NEWSYM OpA7     ; SBC A,(dp+X) A <- A-((dp+X+1)(dp+X))-!C NV..H..ZC
    SPCaddr_bDP_Xb
    SPC_SBC_A
NEWSYM OpB7     ; SBC A,(dp)+Y A <- A-((dp+1)(dp)+Y)-!C   NV..H..ZC
    SPCaddr_bDPb_Y
    SPC_SBC_A
NEWSYM OpA8     ; SBC A,#inm   A <- A-inm-!C         NV..H..ZC
    mov al,[ebp]
    inc ebp
    SPC_SBC_A

;************************************************
; MOV A, instructions
;************************************************
NEWSYM OpE4     ; MOV A,dp     A <- (dp)        N......Z
    SPCaddr_DP
    SPC_MOV_A
NEWSYM OpF4     ; MOV A,dp+x   A <- (dp+X)        N......Z
    SPCaddr_DP_X
    SPC_MOV_A
NEWSYM OpE5     ; MOV A,labs   A <- (abs)         N......Z
    SPCaddr_LABS
    SPC_MOV_A
NEWSYM OpF5     ; MOV A,labs+X A <- (abs+X)       N......Z
    SPCaddr_LABS_X
    SPC_MOV_A
NEWSYM OpE6     ; MOV A,(X)    A <- (X)         N......Z
    SPCaddr__X_
    SPC_MOV_A
NEWSYM OpF6     ; MOV A,labs+Y A <- (abs+Y)       N......Z
    SPCaddr_LABS_Y
    SPC_MOV_A
NEWSYM OpE7     ; MOV A,(dp+X) A <- ((dp+X+1)(dp+X))     N......Z
    SPCaddr_bDP_Xb
    SPC_MOV_A
NEWSYM OpF7     ; MOV A,(dp)+Y A <- ((dp+1)(dp)+Y)     N......Z
    SPCaddr_bDPb_Y
    SPC_MOV_A
NEWSYM OpE8     ;  MOV A,#inm  A <- inm            N......Z
    mov al,[ebp]
    inc ebp
    SPC_MOV_A

;************************************************
; DP,#imm instructions
;************************************************

%macro spcgetdp_imm 0
    mov bl,[ebp+1]
    mov ah,[ebp]
    add ebx,[spcRamDP]
    ReadByte2
    add ebp,2
%endmacro

NEWSYM OpB8     ; SBC dp,#inm  (dp) <- (dp)-inm-!C      NV..H..ZC
    spcgetdp_imm ; bl<-[ebp+1], ah<-[ebp], ebx+[spcRamDP],Readbyte,ebp+2
    mov cl,[spcP]
    xor cl,1
    shr cl,1
    sbb al,ah
    cmc
    SPCSetFlagnvhzcnoret
    WriteByte
    ret

NEWSYM Op98     ; ADC dp,#inm  (dp) <- (dp)+inm+C       NV..H..ZC
    spcgetdp_imm ; bl<-[ebp+1], ah<-[ebp], ebx+[spcRamDP],Readbyte,ebp+2
    mov cl,[spcP]
    shr cl,1
    adc al,ah
    SPCSetFlagnvhzcnoret
    WriteByte
    ret

NEWSYM Op78     ; CMP dp,#inm  (dp)-inm            N......ZC
    mov bl,[ebp+1]
    mov ah,[ebp]
    add ebx,[spcRamDP]
    ReadByte
    add ebp,2
    cmp al,ah
    cmc
    SPCSetFlagnzcnoret
    ret

NEWSYM Op58    ; EOR dp,#inm  (dp) <- (dp) EOR inm      N......Z.
    spcgetdp_imm ; bl<-[ebp+1], ah<-[ebp], ebx+[spcRamDP],Readbyte,ebp+2
    xor al,ah
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM Op38     ; AND dp,#inm  (dp) <- (dp) AND inm      N......Z.
    spcgetdp_imm ; bl<-[ebp+1], ah<-[ebp], ebx+[spcRamDP],Readbyte,ebp+2
    and al,ah
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM Op18     ; OR dp,#inm   (dp) <- (dp) OR inm       N......Z.
    spcgetdp_imm ; bl<-[ebp+1], ah<-[ebp], ebx+[spcRamDP],Readbyte,ebp+2
    or al,ah
    mov [spcNZ],al
    WriteByte
    ret

;************************************************
; DP(D),DP(S) instructions
;************************************************
%macro spcaddrDPbDb_DPbSb 1
    xor ecx,ecx
    mov bl,[ebp+1]
    mov cl,[ebp]
    add ebx,[spcRamDP]
    add ebp,2
    add ecx,[spcRamDP]
    push ebx
    ReadByte
    mov ebx,ecx
    mov cl,al
%1
    ReadByte
    mov ah,al
    mov al,cl
    pop ebx
%endmacro

NEWSYM Op09     ; OR dp(d),dp(s)  (dp(d))<-(dp(d)) OR (dp(s))  N......Z.
    spcaddrDPbDb_DPbSb Op09b:
    or al,ah
    mov [spcNZ], al
    WriteByte
    xor ecx,ecx
    ret

NEWSYM Op29     ; AND dp(d),dp(s) (dp(d))<-(dp(d)) AND (dp(s)) N......Z.
    spcaddrDPbDb_DPbSb Op29b:
    and al,ah
    mov [spcNZ], al
    WriteByte
    xor ecx,ecx
    ret

NEWSYM Op49     ; EOR dp(d),dp(s) (dp(d))<-(dp(d)) EOR (dp(s)) N......Z.
    spcaddrDPbDb_DPbSb Op49b:
    xor al,ah
    mov [spcNZ], al
    WriteByte
    xor ecx,ecx
    ret

NEWSYM Op69     ; CMP dp(d),dp(s) (dp(d))-(dp(s))       N......ZC
    spcaddrDPbDb_DPbSb Op69b:
    cmp al,ah
    cmc
    SPCSetFlagnzcnoret
    xor ecx,ecx
    ret

NEWSYM Op89     ; ADC dp(d),dp(s) (dp(d))<-(dp(d))+(dp(s))+C  NV..H..ZC
    spcaddrDPbDb_DPbSb Op89b
    mov cl,[spcP]
    shr cl,1
    adc al,ah
    SPCSetFlagnvhzcnoret
    WriteByte
    xor ecx,ecx
    ret

NEWSYM OpA9     ; SBC dp(d),dp(s) (dp(d))<-(dp(d))-(dp(s))-!C NV..H..ZC
    spcaddrDPbDb_DPbSb OpA9b
    mov cl,[spcP]
    xor cl,1
    shr cl,1
    sbb al,ah
    cmc
    SPCSetFlagnvhzcnoret
    WriteByte
    xor ecx,ecx
    ret

NEWSYM OpFA     ; MOV dp(d),dp(s) (dp(d)) <- (dp(s))      ........
    xor ecx,ecx
    mov bl,[ebp+1]
    mov cl,[ebp]
    add ebx,[spcRamDP]
    add ecx,[spcRamDP]
    add ebp,2
    push ebx
    mov ebx,ecx
    ReadByte
    pop ebx
    WriteByte
    xor ecx,ecx
    ret

;************************************************
; (X),(Y) instructions
;************************************************
%macro spcaddrDPbXb_bYb 1
    mov bl,[spcY]
    add ebx,[spcRamDP]
    ReadByte
    xor ebx,ebx
    mov ah,al
    mov bl,[spcX]
    add ebx,[spcRamDP]
%1
    ReadByte
%endmacro

NEWSYM Op19     ; OR (X),(Y)   (X) <- (X) OR (Y)        N......Z.
    spcaddrDPbXb_bYb Op19b:
    or al, ah
    mov [spcNZ],al
    WriteByte
    ret

NEWSYM Op39     ; AND (X),(Y)  (X) <- (X) AND (Y)       N......Z.
    spcaddrDPbXb_bYb Op39b:
    and al, ah
    mov [spcNZ],al
    WriteByte
    ret


NEWSYM Op59     ; EOR (X),(Y)  (X) <- (X) EOR (Y)       N......Z.
    spcaddrDPbXb_bYb Op59b:
    xor al, ah
    mov [spcNZ],al
    WriteByte
    ret

NEWSYM Op79     ; CMP (X),(Y)  (X)-(Y)             N......ZC
    spcaddrDPbXb_bYb Op79b:
    cmp al, ah
    cmc
    SPCSetFlagnzc

NEWSYM Op99     ; ADC (X),(Y)  (X) <- (X)+(Y)+C        NV..H..ZC
    spcaddrDPbXb_bYb Op99b:
    mov cl,[spcP]
    shr cl,1
    adc al,ah
    SPCSetFlagnvhzcnoret
    WriteByte
    ret

NEWSYM OpB9     ; SBC (X),(Y)  (X) <- (X)-(Y)-!C       NV..H..ZC
    spcaddrDPbXb_bYb OpB9b:
    mov cl,[spcP]
    xor cl,1
    shr cl,1
    sbb al,ah
    cmc
    SPCSetFlagnvhzcnoret
    WriteByte
    ret

;************************************************
; MOV ,A instructions (Verified)
;************************************************

NEWSYM OpC4     ; MOV dp,A     A -> (dp)        ........
    mov bl,[ebp]
    mov al, [spcA]
    add ebx,[spcRamDP]
    inc ebp
    WriteByte
    ret

NEWSYM OpD4     ; MOV dp+x,A   A -> (dp+X)        ........
    mov bl,[ebp]
    add bl,[spcX]
    mov al, [spcA]
    add ebx,[spcRamDP]
    inc ebp
    WriteByte
    ret

NEWSYM OpC5     ; MOV labs,A   A -> (abs)         ........
    mov bx,[ebp]
    mov al, [spcA]
    add ebp,2
    add ebx,SPCRAM
    WriteByte
    ret

NEWSYM OpD5     ; MOV labs+X,A A -> (abs+X)       ........
    mov bl,[spcX]
    add bx,[ebp]
    mov al, [spcA]
    add ebp,2
    add ebx,SPCRAM
    WriteByte
    ret

NEWSYM OpC6     ; MOV (X),A    A -> (X)         ........
    mov bl,[spcX]
    add ebx,[spcRamDP]
    mov al, [spcA]
    WriteByte
    ret

NEWSYM OpD6     ; MOV labs+Y,A A -> (abs+Y)       ........
    mov bl,[spcY]
    mov al, [spcA]
    add bx,[ebp]
    add ebp,2
    add ebx,SPCRAM
    WriteByte
    ret

NEWSYM OpC7     ; MOV (dp+X),A A -> ((dp+X+1)(dp+X))     ........
    mov bl,[ebp]
    add bl,[spcX]
    xor eax,eax
    add ebx,[spcRamDP]
    inc ebp
    mov ax, [ebx]
    mov ebx,eax
    add ebx,SPCRAM
    mov al, [spcA]
    WriteByte
    ret

NEWSYM OpD7     ; MOV (dp)+Y,A A -> ((dp+1)(dp)+Y)     ........
    mov bl,[ebp]
    xor eax,eax
    add ebx,[spcRamDP]
    inc ebp
    mov ax, [ebx]
    add ax,[spcY]
    mov ebx,eax
    add ebx,SPCRAM
    mov al, [spcA]
    WriteByte
    ret

;************************************************
; MOV instructions (Verified)
;************************************************

NEWSYM OpD8     ; MOV dp,X     X -> (dp)             ........
    mov bl,[ebp]
    mov al, [spcX]
    add ebx,[spcRamDP]
    inc ebp
    WriteByte
    ret

NEWSYM OpF8     ;  MOV X,dp    X <- (dp)             N......Z
    mov bl,[ebp]
    inc ebp
    add ebx,[spcRamDP]
    ReadByte
    mov [spcX], al
    mov [spcNZ],al
    ret

NEWSYM OpC9     ; MOV labs,X   X -> (abs)            ........
    mov bx,[ebp]
    mov al, [spcX]
    add ebp,2
    add ebx,SPCRAM
    WriteByte
    ret

NEWSYM OpE9     ; MOV X,labs   X <- (abs)            N......Z
    mov bx,[ebp]
    add ebx,SPCRAM
    ReadByte
    add ebp,2
    mov [spcX], al
    mov [spcNZ],al
    ret

NEWSYM OpD9     ; MOV dp+Y,X   X -> (dp+Y)           ........
    mov bl,[ebp]
    mov al, [spcX]
    add bl,[spcY]
    inc ebp
    add ebx,[spcRamDP]
    WriteByte
    ret

NEWSYM OpF9     ; MOV X,dp+Y   X <- (dp+Y)           N......Z
    mov bl,[ebp]
    add bl,[spcY]
    inc ebp
    add ebx,[spcRamDP]
    ReadByte
    mov [spcX], al
    mov [spcNZ],al
    ret

NEWSYM OpCB     ; MOV dp,Y  Y -> (dp)             ........
    mov bl,[ebp]
    mov al, [spcY]
    add ebx,[spcRamDP]
    inc ebp
    WriteByte
    ret

NEWSYM OpEB     ; MOV Y,dp  Y <- (dp)             N......Z
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte
    mov [spcY], al
    mov [spcNZ],al
    ret

NEWSYM OpDB     ; MOV dp+X,Y   X -> (dp+X)           ........
    mov bl,[ebp]
    add bl,[spcX]
    mov al, [spcY]
    add ebx,[spcRamDP]
    inc ebp
    WriteByte
    ret

NEWSYM OpFB     ; MOV Y,dp+X   Y <- (dp+X)           N......Z
    mov bl,[ebp]
    add bl,[spcX]
    inc ebp
    add ebx,[spcRamDP]
    ReadByte
    mov [spcY], al
    mov [spcNZ],al
    ret

NEWSYM OpCC     ; MOV labs,Y   Y -> (abs)            ........
    mov bx,[ebp]
    mov al, [spcY]
    add ebp,2
    add ebx,SPCRAM
    WriteByte
    ret

NEWSYM OpEC     ; MOV Y,labs   Y <- (abs)            N......Z
    mov bx,[ebp]
    add ebx,SPCRAM
    ReadByte
    add ebp,2
    mov [spcY],al
    mov [spcNZ],al
    ret

NEWSYM Op5D     ; MOV X,A    X <- A             N......Z
    mov al,[spcA]
    mov [spcX],al
    mov [spcNZ],al
    ret

NEWSYM Op7D     ; MOV A,X    A <- X             N......Z
    mov al,[spcX]
    mov [spcA],al
    mov [spcNZ],al
    ret

NEWSYM Op8D     ; MOV Y,#inm   Y <- inm            N......Z
    mov bl,[ebp]
    mov [spcY],bl
    inc ebp
    mov [spcNZ],bl
    ret

NEWSYM OpCD     ; MOV X,#inm   X <- inm            N......Z
    mov bl,[ebp]
    mov [spcX],bl
    inc ebp
    mov [spcNZ],bl
    ret

NEWSYM Op8F     ; MOV dp,#inm  (dp) <- inm           ........
    mov bl,[ebp+1]
    mov al,[ebp]
    add ebx,[spcRamDP]
    add ebp,2
    WriteByte
    ret

NEWSYM Op9D     ; MOV X,SP     X <- SP            N......Z
    mov al,[spcS]
    mov [spcX],al
    mov [spcNZ],al
    ret

NEWSYM OpBD     ; MOV SP,X     SP <- X             ........
    mov al,[spcX]
    mov [spcS],al
    ret

NEWSYM OpDD     ; MOV A,Y    A <- Y             N......Z
    mov al,[spcY]
    mov [spcA],al
    mov [spcNZ],al
    ret


NEWSYM OpFD     ; MOV Y,A    Y <- A             N......Z
    mov al,[spcA]
    mov [spcY],al
    mov [spcNZ],al
    ret

NEWSYM OpAF     ; MOV (X)+,A   A -> (X) with auto inc    ........
    mov bl,[spcX]
    add ebx,[spcRamDP]
    mov al, [spcA]
    inc byte[spcX]
    WriteByte
    ret

NEWSYM OpBF     ; MOV A,(X)+  A <- (X) with auto inc    N......Z
    mov bl,[spcX]
    add ebx,[spcRamDP]
    ReadByte
    inc byte[spcX]
    mov [spcA],al
    mov [spcNZ],al
    ret


;************************************************
; CMP instructions (Verified)
;************************************************

NEWSYM OpC8     ; CMP X,#inm   X-inm             N......ZC
    mov bl,[ebp]
    inc ebp
    cmp [spcX],bl
    cmc
    SPCSetFlagnzc

NEWSYM OpAD     ; CMP Y,#inm   Y-inm             N......ZC
    mov bl,[ebp]
    inc ebp
    cmp [spcY],bl
    cmc
    SPCSetFlagnzc

NEWSYM Op1E     ; CMP X,labs   X-(abs)             N......ZC
    mov bx,[ebp]
    add ebp,2
    add ebx,SPCRAM
    ReadByte
    cmp byte[spcX], al
    cmc
    SPCSetFlagnzc

NEWSYM Op3E     ; CMP X,dp     X-(dp)            N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte
    cmp byte[spcX], al
    cmc
    SPCSetFlagnzc

NEWSYM Op5E     ; CMP Y,labs   Y-(abs)             N......ZC
    mov bx,[ebp]
    add ebx,SPCRAM
    ReadByte
    add ebp,2
    cmp byte[spcY], al
    cmc
    SPCSetFlagnzc

NEWSYM Op7E     ; CMP Y,dp     Y-(dp)            N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte
    cmp byte[spcY], al
    cmc
    SPCSetFlagnzc

;************************************************
; Word Instructions (Verified)
;************************************************

NEWSYM Op1A     ; DECW dp   Decrement dp memory pair  N......Z.
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM Op1AB
    ReadByte2
    dec ax
    test ax,8000h
    jnz .YesNeg
    cmp ax,0000h
    je .YesZero
    mov byte[spcNZ],1
    jmp .SkipFlag
.YesNeg
    mov byte[spcNZ],80h
    jmp .SkipFlag
.YesZero
    mov byte[spcNZ],0
.SkipFlag
    push ebx
    WriteByte
    pop ebx
NEWSYM Op1Ab
    inc ebx
    mov al,ah
    WriteByte
    ret

NEWSYM Op3A     ; INCW dp   Increment dp memory pair  N......Z.
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM Op3AB
    ReadByte2
    inc ax
    test ax,8000h
    jnz .YesNeg
    cmp ax,0000h
    je .YesZero
    mov byte[spcNZ],1
    jmp .SkipFlag
.YesNeg
    mov byte[spcNZ],80h
    jmp .SkipFlag
.YesZero
    mov byte[spcNZ],0
.SkipFlag
    push ebx
    WriteByte
    pop ebx
NEWSYM Op3Ab
    inc ebx
    mov al,ah
    WriteByte
    ret

; looks like there is the Carry flag checked in op5a..

NEWSYM Op5A     ; CMPW YA,dp   YA - (dp+1)(dp)      N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM Op5AB
    ReadByte
    mov bl,[spcA]
    mov bh,[spcY]
    cmp bx,ax
    cmc
    SPCSetFlagnzc

NEWSYM Op7A     ; ADDW YA,dp   YA  <- YA + (dp+1)(dp)   NV..H..ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM Op7AB
    ReadByte
    mov bl,[spcA]
    mov bh,[spcY]
    add bx,ax
    mov [spcA],bl
    mov [spcY],bh
    SPCSetFlagnvhzc

NEWSYM Op9A     ; SUBW YA,dp   YA  <- YA - (dp+1)(dp)   NV..H..ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM Op9AB
    ReadByte
    mov bl,[spcA]
    mov bh,[spcY]
    sub bx,ax
    cmc
    mov [spcA],bl
    mov [spcY],bh
    SPCSetFlagnvhzc

NEWSYM OpBA     ; MOVW YA,dp   YA  - (dp+1)(dp)     N......Z.
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    inc ebx
    ReadByte2
    mov ah,al
    dec ebx
NEWSYM OpBAb
    ReadByte
    mov [spcA],al
    mov [spcY],ah
    test ax,8000h
    jnz .YesNeg
    cmp ax,0000h
    je .YesZero
    mov byte[spcNZ],1
    ret
.YesNeg
    mov byte[spcNZ],80h
    ret
.YesZero
    mov byte[spcNZ],0
    ret

NEWSYM OpDA     ; MOVW dp,YA   (dp+1)(dp) - YA       .........
    mov bl,[ebp]
    mov al,[spcA]
    add ebx,[spcRamDP]
    inc ebp
    push ebx
    WriteByte
    pop ebx
    inc ebx
    mov al,[spcY]
NEWSYM OpDAb
    WriteByte
    ret

;************************************************
; mem.bit instructions (Verified)
;************************************************

%macro spcaddrmembit 0
    mov bx,[ebp]

    mov cl,bh
    add ebp,2
    shr cl,5
    and bx,1FFFh

;    mov cl,bl
;    add ebp,2
;    shr bx,3
;    and cl,00000111b

    add ebx,SPCRAM
    ReadByte
    shr al,cl
    and al,01h
%endmacro

NEWSYM Op0A     ; OR1 C,mem.bit   C <- C OR  (mem.bit)    ........C
    spcaddrmembit
    or [spcP],al
    ret

NEWSYM Op2A     ; OR1 C,/mem.bit  C <- C OR  !(mem.bit)     ........C
    spcaddrmembit
    xor al,01h
    or [spcP],al
    ret

NEWSYM Op4A     ; AND1 C,mem.bit  C <- C AND (mem.bit)    ........C
    mov bx,[ebp]

    mov cl,bh
    add ebp,2
    shr cl,5
    and bx,1FFFh

;    mov cl,bl
;    add ebp,2
;    shr bx,3
;    and cl,00000111b

    add ebx,SPCRAM
    ReadByte
    shr al,cl
    or al,0FEh
    and [spcP],al
    ret

NEWSYM Op6A     ; AND1 C,/mem.bit C <- C AND !(mem.bit)     ........C
    mov bx,[ebp]

    mov cl,bh
    add ebp,2
    shr cl,5
    and bx,1FFFh

;    mov cl,bl
;    add ebp,2
;    shr bx,3
;    and cl,00000111b

    add ebx,SPCRAM
    ReadByte
    shr al,cl
    or al,0FEh
    xor al,01h
    and [spcP],al
    ret

NEWSYM Op8A     ; EOR1 C,mem.bit  C <- C EOR (mem.bit)    ........C
    spcaddrmembit
    xor [spcP],al
    ret

NEWSYM OpAA     ; MOV1 C,mem.bit  C <- (mem.bit)
    spcaddrmembit
    and byte[spcP],0FEh
    or [spcP],al
    ret

NEWSYM OpCA     ; MOV1 mem.bit,C  C -> (mem.bit)        .........
    mov bx,[ebp]
    mov al,[spcP]

    mov cl,bh
    mov ah,01h
    shr cl,5
    and bx,1FFFh

;    mov cl,bl
;    mov ah,01h
;    and cl,00000111b
;    shr bx,3

    shl ah,cl
    and al,01h
    add ebp,2
    shl al,cl
    add ebx,SPCRAM
    ; al = carry flag positioned in correct location, ah = 1 positioned
    mov cl,al
    xor ah,0FFh
    ReadByte2
    and al,ah
    or al,cl
    WriteByte
    ret

NEWSYM OpEA     ; NOT1 mem.bit    complement (mem.bit)    .........
    mov bx,[ebp]

    mov cl,bh
    mov ah,01h
    shr cl,5
    and bx,1FFFh

;    mov cl,bl
;    mov ah,01h
;    and cl,00000111b
;    shr bx,3

    shl ah,cl
    add ebp,2
    add ebx,SPCRAM
    ReadByte2
    xor al,ah
    WriteByte
    ret

;************************************************
; Shift Instructions (Verified)
;************************************************

NEWSYM Op0B     ; ASL dp    C << (dp)   <<0     N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    shl al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op4B     ; LSR dp    0 >> (dp)   <<C     N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    shr al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op1B     ; ASL dp+X  C << (dp+X) <<0     N......ZC
    mov bl,[ebp]
    add bl,[spcX]
    inc ebp
    add ebx,[spcRamDP]
    ReadByte2
    shl al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op5B     ; LSR dp+X  0 >> (dp+X) <<C     N......ZC
    mov bl,[ebp]
    add bl,[spcX]
    inc ebp
    add ebx,[spcRamDP]
    ReadByte2
    shr al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op0C     ; ASL labs  C << (abs)  <<0     N......ZC
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    ReadByte2
    shl al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op4C     ; LSR labs  0 >> (abs)  <<C     N......ZC
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    ReadByte2
    shr al,1
    SPCSetFlagnzcnoret
    WriteByte
    ret

NEWSYM Op1C     ; ASL A  C << A    <<0     N......ZC
    shl byte[spcA],1
    SPCSetFlagnzc

NEWSYM Op5C     ; LSR A  0 >> A    <<C     N......ZC
    shr byte[spcA],1
    SPCSetFlagnzc

%macro spcROLstuff 0
    rcl al,1
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    jmp .skipflags
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
.skipflags
%endmacro

%macro spcRORstuff 0
    rcr al,1
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    jmp .skipflags
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
.skipflags
%endmacro

NEWSYM Op2B     ; ROL dp    C << (dp)   <<C     N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    test byte[spcP],01h
    jnz near Op2Bb
    ReadByte2
    clc
    spcROLstuff
    WriteByte
    ret
NEWSYM Op2Bb
    ReadByte2
    stc
    spcROLstuff
    WriteByte
    ret

NEWSYM Op6B     ; ROR dp    C >> (dp)   <<C     N......ZC
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    test byte[spcP],01h
    jnz near Op6Bb
    ReadByte2
    clc
    spcRORstuff
    WriteByte
    ret
NEWSYM Op6Bb
    ReadByte2
    stc
    spcRORstuff
    WriteByte
    ret

NEWSYM Op3B     ; ROL dp+X  C << (dp+X) <<C     N......ZC
    mov bl,[ebp]
    add bl,[spcX]
    add ebx,[spcRamDP]
    inc ebp
    test byte[spcP],01h
    jnz near Op3Bb
    ReadByte2
    clc
    spcROLstuff
    WriteByte
    ret
NEWSYM Op3Bb
    ReadByte2
    stc
    spcROLstuff
    WriteByte
    ret

NEWSYM Op7B     ; ROR dp+X  C >> (dp+X) <<C     N......ZC
    mov bl,[ebp]
    add bl,[spcX]
    add ebx,[spcRamDP]
    inc ebp
    test byte[spcP],01h
    jnz near Op7Bb
    ReadByte2
    clc
    spcRORstuff
    WriteByte
    ret
NEWSYM Op7Bb
    ReadByte2
    stc
    spcRORstuff
    WriteByte
    ret

NEWSYM Op2C     ; ROL labs  C << (abs)  <<C     N......ZC
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    test byte[spcP],01h
    jnz near Op2Cb
    ReadByte2
    clc
    spcROLstuff
    WriteByte
    ret
NEWSYM Op2Cb
    ReadByte2
    stc
    spcROLstuff
    WriteByte
    ret

NEWSYM Op6C     ; ROR labs  C >> (abs)  <<C     N......ZC
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    test byte[spcP],01h
    jnz near Op6Cb
    ReadByte2
    clc
    spcRORstuff
    WriteByte
    ret
NEWSYM Op6Cb
    ReadByte2
    stc
    spcRORstuff
    WriteByte
    ret

NEWSYM Op3C     ; ROL A  C << A    <<C     N......ZC
    test byte[spcP],01h
    jnz near Op3Cb
    clc
    rcl byte[spcA],1
    mov al,[spcA]
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    ret
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
    ret
NEWSYM Op3Cb
    stc
    rcl byte[spcA],1
    mov al,[spcA]
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    ret
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
    ret

NEWSYM Op7C     ; ROR A  C >> A    <<C     N......ZC
    test byte[spcP],01h
    jnz near Op7Cb
    clc
    rcr byte[spcA],1
    mov al,[spcA]
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    ret
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
    ret
NEWSYM Op7Cb
    stc
    rcr byte[spcA],1
    mov al,[spcA]
    jc .setcarryflag
    and byte[spcP],0FEh
    mov [spcNZ],al
    ret
.setcarryflag
    or byte[spcP],01h
    mov [spcNZ],al
    ret

;************************************************
; INC/DEC instructions (Verified)
;************************************************

NEWSYM Op8B     ;  DEC dp   -- (dp)           N......Z.
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    dec al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM OpAB     ; INC dp    ++ (dp)           N......Z.
    mov bl,[ebp]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    inc al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM Op9B     ;  DEC dp+X -- (dp+X)         N......Z.
    mov bl,[ebp]
    add bl,[spcX]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    dec al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM OpBB     ; INC dp+X  ++ (dp+X)         N......Z.
    mov bl,[ebp]
    add bl,[spcX]
    add ebx,[spcRamDP]
    inc ebp
    ReadByte2
    inc al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM Op8C     ; DEC labs  -- (abs)          N......Z.
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    ReadByte2
    dec al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM OpAC     ; INC labs  ++ (abs)          N......Z.
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    ReadByte2
    inc al
    mov [spcNZ], al
    WriteByte
    ret

NEWSYM Op9C     ; DEC A  -- A            N......Z.
    dec byte[spcA]
    mov al,[spcA]
    mov [spcNZ],al
    ret

NEWSYM OpBC     ; INC A  ++ A            N......Z.
    inc byte[spcA]
    mov al,[spcA]
    mov [spcNZ],al
    ret

NEWSYM OpDC     ; DEC Y  -- Y            N......Z.
    dec byte[spcY]
    mov al,[spcY]
    mov [spcNZ],al
    ret

NEWSYM OpFC     ; INC Y  ++ Y            N......Z.
    inc byte[spcY]
    mov al,[spcY]
    mov [spcNZ],al
    ret

NEWSYM Op1D     ; DEC X     -- X            N......Z.
    dec byte[spcX]
    mov al,[spcX]
    mov [spcNZ],al
    ret

NEWSYM Op3D     ; INC X     ++ X            N......Z.
    inc byte[spcX]
    mov al,[spcX]
    mov [spcNZ],al
    ret

;************************************************
; PUSH/POP instructions (Verified)
;************************************************

NEWSYM Op0D     ; PUSH PSW     push PSW to stack     .........
    mov eax,[spcS]
    mov bl,[spcP]
    and bl,01111101b
    test byte[spcNZ],80h
    jnz .NegSet
    cmp byte[spcNZ],0
    je .ZeroSet
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret
.NegSet
    or bl,80h
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret
.ZeroSet
    or bl,02h
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret

NEWSYM Op2D     ; PUSH A     push A to stack       .........
    mov eax,[spcS]
    mov bl,[spcA]
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret

NEWSYM Op4D     ; PUSH X     push X to stack       .........
    mov eax,[spcS]
    mov bl,[spcX]
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret

NEWSYM Op6D     ; PUSH Y    push Y to stack       .........
    mov eax,[spcS]
    mov bl,[spcY]
    dec byte[spcS]
    mov [SPCRAM+eax],bl
    ret

NEWSYM Op8E     ; POP PSW   pop PSW from stack     (Restored)
    inc byte[spcS]
    mov eax,[spcS]
    mov byte[spcNZ],0
    mov bl,[SPCRAM+eax]
    mov [spcP],bl
    test byte[spcP],2
    jnz .ZeroYes
    mov byte[spcNZ],1
    test byte[spcP],80h
    jz .NoNeg
    or byte[spcNZ],80h
.NoNeg
.ZeroYes
    mov dword[spcRamDP],SPCRAM
    test byte[spcP],32
    jnz .setpage1
    ret
.setpage1
    add dword[spcRamDP],100h
    ret

NEWSYM OpAE     ; POP A     pop A from stack      .........
    inc byte[spcS]
    mov eax,[spcS]
    mov bl,[SPCRAM+eax]
    mov [spcA],bl
    ret

NEWSYM OpCE     ; POP X     pop X from stack      .........
    inc byte[spcS]
    mov eax,[spcS]
    mov bl,[SPCRAM+eax]
    mov [spcX],bl
    ret

NEWSYM OpEE     ; POP Y     pop Y from stack      .........
    inc byte[spcS]
    mov eax,[spcS]
    mov bl,[SPCRAM+eax]
    mov [spcY],bl
    ret

;************************************************
; Test & set bits Instructions (Verified?)
;************************************************

NEWSYM Op0E     ; TSET1 labs   test and set bits with A   N......Z.
    mov bx,[ebp]
    add ebx,SPCRAM
    add ebp,2
    ReadByte2
    mov ah,al
    and ah,[spcA]
    mov [spcNZ],ah
    or al,[spcA]
    WriteByte
    ret

NEWSYM Op4E     ; TCLR1     test and clear bits with A N......Z.
    mov bx,[ebp]
    add ebx,[spcRamDP]
    add ebp,2
    ReadByte2
    mov ah,al
    and ah,[spcA]
    mov [spcNZ],ah
    mov ah,[spcA]
    not ah
    and al,ah
    WriteByte
    ret

;************************************************
; Compare/Decrement & Branch Instructions (Verified)
;************************************************

NEWSYM Op2E     ; CBNE dp,rel  compare A with (dp) then BNE   ...
    mov bl,[ebp]
    add ebx,[spcRamDP]
    ReadByte2
    cmp byte[spcA], al
    jne .Jump
    add ebp,2
    ret
.Jump
    movsx ebx,byte[ebp+1]
    add ebp,ebx
    add ebp,2
    ret

NEWSYM OpDE     ; CBNE dp+X,rel   compare A with (dp+X) then BNE ...
    mov bl,[ebp]
    add bl,[spcX]
    add ebx,[spcRamDP]
    ReadByte2
    cmp byte[spcA], al
    jne .Jump
    add ebp,2
    ret
.Jump
    movsx ebx,byte[ebp+1]
    add ebp,ebx
    add ebp,2
    ret

NEWSYM Op6E     ; DBNZ   decrement memory (dp) then JNZ ...
    mov bl,[ebp]
    add ebx,[spcRamDP]
    ReadByte2
    dec al
    jnz .Jump
    add ebp,2
    WriteByte
    ret
.Jump
NEWSYM Op6Eb
    push ebx
    movsx ebx,byte[ebp+1]
    add ebp,ebx
    add ebp,2
    pop ebx
    WriteByte
    ret

NEWSYM OpFE     ; DBNZ Y,rel   decrement Y then JNZ         ...
    dec byte[spcY]
    jnz .Jump
    inc ebp
    ret
.Jump
    movsx ebx,byte[ebp]
    add ebp,ebx
    inc ebp
    ret

;************************************************
; Jump/Subroutine Instructions
;************************************************

NEWSYM Op0F     ; BRK     software interrupt     ...1.0..
    inc dword[spc700read]
    dec ebp
    ret

NEWSYM Op1F     ; JMP (labs+X)    PC <- (abs+X+1)(abs+X)       ...
    mov bx,[ebp]
    add bx,[spcX]
    xor eax,eax
    add ebp,2
    mov ax,[SPCRAM+ebx]
    mov ebp,SPCRAM
    add ebp,eax
    ret

NEWSYM Op3F     ; CALL labs    subroutine call        ........
    ; calculate PC
    mov ecx,ebp
    add ecx,2
    sub ecx,SPCRAM
    mov eax,[spcS]
    mov [SPCRAM+eax],ch
    dec byte[spcS]
    mov eax,[spcS]
    mov [SPCRAM+eax],cl
    dec byte[spcS]
    ; set new PC
    mov cx,[ebp]
    add ecx,SPCRAM
    mov ebp,ecx
    xor ecx,ecx
    ret

NEWSYM Op4F     ; PCALL upage  upage call           ........
    ; calculate PC
    mov ecx,ebp
    inc ecx
    sub ecx,SPCRAM
    mov eax,[spcS]
    mov [SPCRAM+eax],ch
    dec byte[spcS]
    mov eax,[spcS]
    mov [SPCRAM+eax],cl
    dec byte[spcS]
    ; set new PC
    xor ecx,ecx
    mov cl,[ebp]
    add ecx,SPCRAM
    add ecx,0ff00h
    mov ebp,ecx
    xor ecx,ecx
    ret

; I'm not sure about this one and JMP labs+X...

NEWSYM Op5F     ; JMP labs     jump to new location         ...
    mov bx,[ebp]
    add ebp,2
    mov ebp,SPCRAM
    add ebp,ebx
    ret

NEWSYM Op6F     ; ret        ret from subroutine   ........
    xor ecx,ecx
    inc byte[spcS]
    mov eax,[spcS]
    mov cl,[SPCRAM+eax]
    inc byte[spcS]
    mov eax,[spcS]
    mov ch,[SPCRAM+eax]
    add ecx,SPCRAM
    mov ebp,ecx
    xor ecx,ecx
    ret

NEWSYM Op7F     ; ret1       return from interrupt   (Restored)
    dec ebp
    ret
    xor ecx,ecx
    inc byte[spcS]
    mov eax,[spcS]
    mov cl,[SPCRAM+eax]
    mov [spcP],cl
    test byte[spcP],80h
    jz .NoNeg
    or byte[spcNZ],80h
.NoNeg
    test byte[spcP],2
    jz .NoZero
    mov byte[spcNZ],0
    jmp .YesZero
.NoZero
    or byte[spcNZ],1
.YesZero
    inc byte[spcS]
    mov eax,[spcS]
    mov cl,[SPCRAM+eax]
    inc byte[spcS]
    mov eax,[spcS]
    mov ch,[SPCRAM+eax]
    add ecx,SPCRAM
    mov ebp,ecx
    ; set direct page
    mov dword[spcRamDP],SPCRAM
    test byte[spcP],32
    jz .nodp
    add dword[spcRamDP],100h
.nodp
    xor ecx,ecx
    ret

;************************************************
; Divide/Multiply Instructions
;************************************************

NEWSYM Op9E     ; DIV YA,X     Q:A B:Y <- YA / X     NV..H..Z.
   push edx
   mov ah,[spcY]
   mov al,[spcA]
   xor bh,bh
   xor dx,dx
   mov bl,[spcX]
   cmp bl,0
   je NoDiv
   div bx
   mov [spcA],al
   mov [spcY],dl
   cmp ah,0
   jne Over
   and byte[spcP],191-16
   pop edx
   mov [spcNZ],al
   ret

NEWSYM NoDiv
   mov byte[spcA],0ffh
   mov byte[spcY],0ffh
   or byte[spcP],16
   and byte[spcP],255-64
   pop edx
   ret
NEWSYM Over
   or byte[spcP],64
   and byte[spcP],255-16
   pop edx
   mov [spcNZ],al
   ret

NEWSYM OpCF     ; MUL YA     YA(16 bits) <- Y * A    N......Z.
    mov al,[spcA]
    mov bl,[spcY]
    mul bl
    mov [spcA],al
    mov [spcY],ah
    ; ??? (Is the n flag set on YA or A?)
    test ax,8000h
    jnz .YesNeg
    cmp ax,0000h
    je .YesZero
    mov byte[spcNZ],1
    ret
.YesNeg
    mov byte[spcNZ],80h
    ret
.YesZero
    mov byte[spcNZ],0
    ret

;************************************************
; Decimal Operations
;************************************************

NEWSYM OpBE     ; DAS A     decimal adjust for sub  N......ZC
    ; copy al flags into AH
    xor ah,ah
    test byte[spcNZ],80h
    jz .noneg
    or ah,10000000b
.noneg
    test byte[spcP],01h
    jz .nocarry
    or ah,00000001b
.nocarry
    test byte[spcNZ],0FFh
    jnz .nozero
    or ah,01000000b
.nozero
    test byte[spcP],08h
    jz .nohcarry
    or ah,00010000b
.nohcarry
    mov al,[spcA]
    sahf
    das
    mov [spcA],al
    SPCSetFlagnzc

NEWSYM OpDF     ; DAA A      decimal adjust for add  N......ZC
    ; copy al flags into AH
    xor ah,ah
    test byte[spcNZ],80h
    jz .noneg
    or ah,10000000b
.noneg
    test byte[spcP],01h
    jz .nocarry
    or ah,00000001b
.nocarry
    test byte[spcNZ],0FFh
    jnz .nozero
    or ah,01000000b
.nozero
    test byte[spcP],08h
    jz .nohcarry
    or ah,00010000b
.nohcarry
    mov al,[spcA]
    sahf
    daa
    mov [spcA],al
    SPCSetFlagnzc

NEWSYM Invalidopcode ; Invalid Opcode
    dec ebp
    ret
