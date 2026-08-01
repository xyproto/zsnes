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



EXTSYM SPC7110Enable,curypos,cycpl
EXTSYM pdh,vram,romispal,reg1read,spcnumread,spcon,reg2read,reg3read
EXTSYM reg4read,JoyEOrig,JoyENow,device2,cycphb,joycontren,totlines
EXTSYM RumbleData

%ifndef NO_DEBUGGER
EXTSYM debuggeron
%endif

; global variables
; The .data and .bss blocks that were here are in cpu/c_regsdata.c.
SECTION .text

NEWSYM ClearRegs
    mov word[VIRQLoc],0
    mov word[bg1ptr],0
    mov word[bg2ptr],0
    mov word[bg3ptr],0
    mov word[bg4ptr],0
    mov word[bg1ptrb],0
    mov word[bg2ptrb],0
    mov word[bg3ptrb],0
    mov word[bg4ptrb],0
    mov word[bg1ptrc],0
    mov word[bg2ptrc],0
    mov word[bg3ptrc],0
    mov word[bg4ptrc],0
    mov word[bg1ptrd],0
    mov word[bg2ptrd],0
    mov word[bg3ptrd],0
    mov word[bg4ptrd],0
    mov word[bg1objptr],0
    mov word[bg2objptr],0
    mov word[bg3objptr],0
    mov word[bg4objptr],0
    mov byte[ioportval],0FFh
    mov byte[hdmastartsc],0
    mov byte[hdmarestart],0
    mov byte[nohdmaframe],0
    mov byte[hdmadelay],0
    ret





; Software latch for horizontal/vertical counter.
; zsnes lacks an accurate dot-level H counter, so we synthesize latchx
; by advancing a 0-339 dot value on each $2137 read. This lets games
; that poll $213C for a specific hdot range (e.g. Super FX titles like
; Star Fox) make progress instead of looping forever.
NEWSYM reg2137r
    inc dword[h_dot_counter]
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je .debugger
%endif
    cmp byte[iohvlatch],1
    je .dolatch
    test byte[ioportval],80h
    jz .nolatch
.dolatch
    push edx
    push eax
    push ebx
    mov eax,[h_dot_counter]
    xor edx,edx
    mov ebx,340
    div ebx
    mov [latchx],dx
    mov bx,[curypos]
    mov [latchy],bx
    pop ebx
    pop eax
    pop edx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq
    cmp word[HIRQLoc],0F0h
    ja .incry
    cmp dh,30
    jae .noincly
.incry
    inc word[latchy]
.novhirq
.noincly
.nolatch
    xor al,al
    mov byte[extlatch],0
    ret
.debugger
debblah:
    cmp byte[iohvlatch],1
    je .dolatch2
    test byte[ioportval],80h
    jz near .nolatch2
.dolatch2
    xor ebx,ebx
    mov bl,[cycpl]
    sub bl,[pdh]
    add bx,bx
    mov [latchx],bx
    mov bx,[curypos]
    mov [latchy],bx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq
    cmp word[HIRQLoc],0F0h
;    ja .incry
    cmp word[latchx],80
;    jb .noincly
.incry
    inc word[latchy]
.novhirq
.noincly
    xor al,al
    mov byte[extlatch],0
    ret

    push eax
    push edx
    xor eax,eax
    xor ebx,ebx
    mov al,[cycpl]
    sub al,[pdh]
    mov bx,339
    mul bx
    xor ebx,ebx
    mov bl,[cycpl]
    div bx
    mov [latchx],ax
    pop edx
    pop eax
    mov bx,[curypos]
    mov [latchy],bx
    mov bl,[INTEnab]
    and bl,30h
    cmp bl,30h
    jne .novhirq2
    cmp word[HIRQLoc],0F0h
    ja .incry2
    cmp word[latchx],80
    jb .noincly2
.incry2
    inc word[latchy]
.novhirq2
.noincly2
.nolatch2
    xor al,al
    mov byte[extlatch],0
    ret

; Read OAM Data (Low, High)
NEWSYM reg2140r
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg1read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    cmp word[ebx],0FB10h
    jne .noret
    mov word[ebx],0EAEAh
.noret
    mov al,5
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    inc byte[sndrot2]
    cmp byte[sndrot2],3
    jne .a
    mov byte[sndrot2],0
.a
    xor al,al
    test byte[sndrot2],01h
    jz .n
    mov al,[xa]
.n
    ret

; Sound Reg #2
NEWSYM reg2141r
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg2read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    xor byte[sndrot],01h
    mov al,[xa+1]
    test byte[sndrot],01h
    jz .n
    mov al,[xa]
.n
    ret

; Sound Reg #3
NEWSYM reg2142r
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg3read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    mov al,[xa]
    test byte[sndrot],01h
    jz .n
    mov al,[xa+1]
.n
    ret

; Sound Reg #4
NEWSYM reg2143r
    cmp byte[spcon],0
    je .nosound
    inc dword[SPC700read]
    mov al,[reg4read]
    mov byte[spcnumread],0
    ret
.nosound
    ; Find for D0
    mov ebx,esi
    mov al,3
.tryagain
    cmp byte[ebx],0D0h
    je .foundit
    inc ebx
    dec al
    jnz .tryagain
    jmp .notfound
.foundit
    mov byte[ebx],0EAh
    mov byte[ebx+1],0EAh
.notfound
    mov al,[xa+1]
    ret

; WRAM Read

; Unknown register, used by test cart


; Joystick Data for controller 1 and 2

SECTION .text

; Joystick Data for controller 2 and 4
SECTION .text

; H/V Blank Flag & Joystick Controller Enable Flag
; bit 7 = vblank, 0=out,1=in, bit 6 = hblank, 0=out,1=in, bit 0 = auto-joy busy
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je .debugger
%endif
    ; bit 0: auto-joypad poll busy. Active for ~3 scanlines starting at the
    ; first vblank line when auto-poll is enabled via $4200 bit 0.
    test byte[INTEnab],01h
    jz .nojoyb
    mov bx,[resolutn]
    inc bx
    cmp word[curypos],bx
    jb .nojoyb
    add bx,3
    cmp word[curypos],bx
    jae .nojoyb
    or al,01h
.nojoyb
    mov bx,[resolutn]
    cmp word[curypos],bx
    jne .notres
    cmp byte[nmistatus],2
    je .vblank
.notres
    inc bx
    cmp word[curypos],bx
    jb .novbl
    mov bx,[totlines]
    dec bx
    cmp word[curypos],bx
    jae .novbl
.vblank
    or al,80h
.novbl
    mov byte[hblank],0
    cmp dh,[cycphb]
    jae .nohblank
    mov byte[hblank],1
    or al,40h
.nohblank
    ret
.debugger
debblah2:
    push edx
    mov dh,[pdh]
    test byte[INTEnab],01h
    jz .dnojoyb
    mov bx,[resolutn]
    inc bx
    cmp word[curypos],bx
    jb .dnojoyb
    add bx,3
    cmp word[curypos],bx
    jae .dnojoyb
    or al,01h
.dnojoyb
    mov bx,[resolutn]
    cmp word[curypos],bx
    jne .dnotres
    cmp byte[nmistatus],2
    je .dvblank
.dnotres
    inc bx
    cmp word[curypos],bx
    jb .dnovbl
    mov bx,[totlines]
    dec bx
    cmp word[curypos],bx
    jae .dnovbl
.dvblank
    or al,80h
.dnovbl
    cmp dh,[cycphb]
    jae .dnohblank
    or al,40h
.dnohblank
    pop edx
    ret

; Programmable I/O port

; Quotent of Divide Result (Low)

; Quotent of Divide Result (High)

; Product of Multiplication Result or Remainder of Divide Result (Low)

; Product of Multiplication Result or Remainder of Divide Result (High)

; Joypad snapshots latched at start of vblank by exechdma init in execute.asm.
; The $4218-$421F regs return these latched values (not live input).
SECTION .text

; Joystick 1 Low
; bit7=X,bit6=Y,bit5=L,bit4=R

; Joystick 1 High
; bit7=A,bit6=B,bit5=Sel,bit4=Start,bit3=up,bit2=down,bit1=left,bit0=right

; Joystick 2 Low

; Joystick 2 High

; Joystick 3 Low

; Joystick 3 High

; Joystick 4 Low

; Joystick 4 High

; DMA Reader
