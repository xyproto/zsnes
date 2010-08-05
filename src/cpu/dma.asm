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

%include "cpu/regs.mac"
%include "cpu/regsw.mac"
%include "macros.mac"

EXTSYM memtabler8,regptwa,memtabler16
EXTSYM dmadata,hdmatype,nexthdma,resolutn,curhdma,curypos,hdmadata
EXTSYM hdmadelay,hdmarestart,nohdmaframe,INTEnab,HIRQLoc
EXTSYM transdma

;*******************************************************
; Transfer DMA                     Inits & Transfers DMA
;*******************************************************
; DMA transfer register

section .bss
NEWSYM AddrNoIncr, resb 1
section .text

%macro TestDMA 0
%endmacro

; DMA enable register
; use dmadata for input on dma
NEWSYM reg420Bw
    push eax
    push esi
    push edi
    push ecx
    push edx
    mov esi,dmadata
    test al,01h
    jz .notransa
    TestDMA
    ccallv transdma, esi
.notransa
    add esi,16
    test al,02h
    jz .notransb
    TestDMA
    ccallv transdma, esi
.notransb
    add esi,16
    test al,04h
    jz .notransc
    TestDMA
    ccallv transdma, esi
.notransc
    add esi,16
    test al,08h
    jz .notransd
    TestDMA
    ccallv transdma, esi
.notransd
    add esi,16
    test al,10h
    jz .notranse
    TestDMA
    ccallv transdma, esi
.notranse
    add esi,16
    test al,20h
    jz .notransf
    TestDMA
    ccallv transdma, esi
.notransf
    add esi,16
    test al,40h
    jz .notransg
    TestDMA
    ccallv transdma, esi
.notransg
    add esi,16
    test al,80h
    jz .notransh
    TestDMA
    ccallv transdma, esi
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop eax
    ret

;*******************************************************
; HDMA Settings
;*******************************************************
NEWSYM setuphdma
    push eax

    ; transfer old address to new address
    mov ax,[esi+2]
    mov [esi+8],ax
    mov [edx+17],ax
    ; get address order to be written
    xor ebx,ebx
    xor ecx,ecx
    movzx eax,byte[esi]
    and al,00000111b
    cmp al,5
    jb .notmode567dma
    sub al,4
.notmode567dma
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi]
    cmp bx,2118h
    je .notnormalhdma1
    cmp bx,2119h
    je .notnormalhdma1
    jmp .normalhdma1
.notnormalhdma1
    mov bx,2200h        ; bad hack _Demo_
.normalhdma1
    mov eax,regptw(ebx)
    mov [edx],eax

    ; get pointer #2
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+2]
    cmp bx,2118h
    je .notnormalhdma2
    cmp bx,2119h
    je .notnormalhdma2
    jmp .normalhdma2
.notnormalhdma2
    mov bx,2200h        ; bad hack _Demo_
.normalhdma2
    mov eax,regptw(ebx)
    mov [edx+4],eax

    ; get pointer #3
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+4]
    cmp bx,2118h
    je .notnormalhdma3
    cmp bx,2119h
    je .notnormalhdma3
    jmp .normalhdma3
.notnormalhdma3
    mov bx,2200h        ; bad hack _Demo_
.normalhdma3
    mov eax,regptw(ebx)
    mov [edx+8],eax

    ; get pointer #4
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+6]
    cmp bx,2118h
    je .notnormalhdma4
    cmp bx,2119h
    je .notnormalhdma4
    jmp .normalhdma4
.notnormalhdma4
    mov bx,2200h        ; bad hack _Demo_
.normalhdma4
    mov eax,regptw(ebx)
    mov [edx+12],eax

    xor ebx,ebx
    mov byte[esi+10],0
    pop eax
    or [hdmatype],ah
    ret

section .data
.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4
section .text

NEWSYM setuphdmars
    push eax

    ; get address order to be written
    xor ebx,ebx
    xor ecx,ecx
    movzx eax,byte[esi]
    and al,00000111b
    cmp al,5
    jb .notmode567dma
    sub al,4
.notmode567dma
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi]
    cmp bx,2118h
    je .notnormalhdma1
    cmp bx,2119h
    je .notnormalhdma1
    jmp .normalhdma1
.notnormalhdma1
    mov bx,2200h        ; bad hack _Demo_
.normalhdma1
    mov eax,regptw(ebx)
    mov [edx],eax

    ; get pointer #2
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+2]
    cmp bx,2118h
    je .notnormalhdma2
    cmp bx,2119h
    je .notnormalhdma2
    jmp .normalhdma2
.notnormalhdma2
    mov bx,2200h        ; bad hack _Demo_
.normalhdma2
    mov eax,regptw(ebx)
    mov [edx+4],eax

    ; get pointer #3
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+4]
    cmp bx,2118h
    je .notnormalhdma3
    cmp bx,2119h
    je .notnormalhdma3
    jmp .normalhdma3
.notnormalhdma3
    mov bx,2200h        ; bad hack _Demo_
.normalhdma3
    mov eax,regptw(ebx)
    mov [edx+8],eax

    ; get pointer #4
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+6]
    cmp bx,2118h
    je .notnormalhdma4
    cmp bx,2119h
    je .notnormalhdma4
    jmp .normalhdma4
.notnormalhdma4
    mov bx,2200h        ; bad hack _Demo_
.normalhdma4
    mov eax,regptw(ebx)
    mov [edx+12],eax

    xor ebx,ebx
    pop eax
    ret

section .data
.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4
section .text

NEWSYM setuphdma2
    push eax

    cmp byte[esi+10],0
    je near .nohdma

    ; transfer old address to new address
    mov ax,[esi+8]
    mov [edx+17],ax
    ; get address order to be written
    xor ebx,ebx
    xor ecx,ecx
    movzx eax,byte[esi]
    and al,00000111b
    cmp al,5
    jb .notmode567dma
    sub al,4
.notmode567dma
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi]
    cmp bx,2118h
    je .notnormalhdma1
    cmp bx,2119h
    je .notnormalhdma1
    jmp .normalhdma1
.notnormalhdma1
    mov bx,2200h        ; bad hack _Demo_
.normalhdma1
    mov eax,regptw(ebx)
    mov [edx],eax

    ; get pointer #2
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+2]
    cmp bx,2118h
    je .notnormalhdma2
    cmp bx,2119h
    je .notnormalhdma2
    jmp .normalhdma2
.notnormalhdma2
    mov bx,2200h        ; bad hack _Demo_
.normalhdma2
    mov eax,regptw(ebx)
    mov [edx+4],eax

    ; get pointer #3
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+4]
    cmp bx,2118h
    je .notnormalhdma3
    cmp bx,2119h
    je .notnormalhdma3
    jmp .normalhdma3
.notnormalhdma3
    mov bx,2200h        ; bad hack _Demo_
.normalhdma3
    mov eax,regptw(ebx)
    mov [edx+8],eax

    ; get pointer #4
    movzx ebx,byte[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+6]
    cmp bx,2118h
    je .notnormalhdma4
    cmp bx,2119h
    je .notnormalhdma4
    jmp .normalhdma4
.notnormalhdma4
    mov bx,2200h        ; bad hack _Demo_
.normalhdma4
    mov eax,regptw(ebx)
    mov [edx+12],eax

    xor ebx,ebx
    pop eax
    and [hdmatype],ah
    ret
.nohdma
    pop eax
    and [nexthdma],ah
    ret

section .data
.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4

section .text

NEWSYM reg420Cw
    mov [curhdma],al
    mov bx,[resolutn]
    cmp word[curypos],bx
    jae near .nohdma
    mov al,[curhdma]
    mov bx,[HIRQLoc]
    test byte[INTEnab],10h
    jz .skipcheck
    cmp bx,80
    jb near .nohdma
    cmp bx,176
    ja near .nohdma
.skipcheck
    mov bl,[nexthdma]
    and bl,al
    jnz near .nohdma
    mov [nexthdma],al
    push ebx
    push esi
    push edi
    push ecx
    push edx
    mov esi,dmadata
    mov edx,hdmadata
    mov ah,01h
    test al,01h
    jz .notransa
    call setuphdma
.notransa
    add esi,16
    add edx,19
    mov ah,02h
    test al,02h
    jz .notransb
    call setuphdma
.notransb
    add esi,16
    add edx,19
    mov ah,04h
    test al,04h
    jz .notransc
    call setuphdma
.notransc
    add esi,16
    add edx,19
    mov ah,08h
    test al,08h
    jz .notransd
    call setuphdma
.notransd
    add esi,16
    add edx,19
    mov ah,10h
    test al,10h
    jz .notranse
    call setuphdma
.notranse
    add esi,16
    add edx,19
    mov ah,20h
    test al,20h
    jz .notransf
    call setuphdma
.notransf
    add esi,16
    add edx,19
    mov ah,40h
    test al,40h
    jz .notransg
    call setuphdma
.notransg
    add esi,16
    add edx,19
    mov ah,80h
    test al,80h
    jz .notransh
    call setuphdma
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
;    call exechdma
;    call exechdma
.nohdma
    cmp byte[nohdmaframe],1
    jne .notframe
    inc byte[hdmadelay]
.notframe
    mov byte[hdmarestart],0
    ret

; HDMA enable register
NEWSYM starthdma
    mov al,[curhdma]
NEWSYM startnexthdma
    mov [nexthdma],al
    cmp al,0
    je near .nohdma
    push ebx
    push esi
    push edi
    push ecx
    push edx
    mov esi,dmadata
    mov edx,hdmadata
    mov ah,01h
    test al,01h
    jz .notransa
    call setuphdma
.notransa
    add esi,16
    add edx,19
    mov ah,02h
    test al,02h
    jz .notransb
    call setuphdma
.notransb
    add esi,16
    add edx,19
    mov ah,04h
    test al,04h
    jz .notransc
    call setuphdma
.notransc
    add esi,16
    add edx,19
    mov ah,08h
    test al,08h
    jz .notransd
    call setuphdma
.notransd
    add esi,16
    add edx,19
    mov ah,10h
    test al,10h
    jz .notranse
    call setuphdma
.notranse
    add esi,16
    add edx,19
    mov ah,20h
    test al,20h
    jz .notransf
    call setuphdma
.notransf
    add esi,16
    add edx,19
    mov ah,40h
    test al,40h
    jz .notransg
    call setuphdma
.notransg
    add esi,16
    add edx,19
    mov ah,80h
    test al,80h
    jz .notransh
    call setuphdma
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
.nohdma
    ret

NEWSYM dohdma
    xor ebx,ebx
    test byte[esi],40h
    jnz near indirectaddr
    push eax
    test byte[esi+10],07Fh
    jnz near .nozero
    test byte[esi+10],80h
    jnz near .noincr
    test [hdmatype],ah
    jnz .noincr
    mov bl,[edx+16]
    add word[edx+17],bx
.noincr
    mov bl,ah
    not bl
    and [hdmatype],bl
    mov bl,[esi+4]
    mov cx,[edx+17]
    call dword near [memtabler8+ebx*4]
    inc word[edx+17]
    mov [esi+10],al
    test al,0FFh
    jnz .yeszero
    xor [nexthdma],ah
    jmp .finhdma2
.yeszero
    cmp byte[esi+10],80h
    ja near hdmatype2
    mov al,[edx+16]
    mov [.tempdecr],al
    movzx ebx,byte[esi+4]
    movzx ecx,word[edx+17]  ; increment/decrement/keep pointer location
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc cx
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    add cx,2
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    add cx,3
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
    jmp .finhdma
.nozero
    test byte[esi+10],80h
    jnz near hdmatype2
.finhdma
    mov ax,[edx+17]
    mov [esi+8],ax
    pop eax
    dec byte[esi+10]
    ret
.finhdma2
    mov ax,[edx+17]
    mov [esi+8],ax
    pop eax
    ret

section .bss
.tempdecr resd 1
section .text

NEWSYM hdmatype2
    mov al,[edx+16]
    mov [.tempdecr],al
    movzx ebx,byte[esi+4]
    movzx ecx,word[edx+17] ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
.finhdma
    mov ax,[edx+17]
    mov [esi+8],ax
    pop eax
    dec byte[esi+10]
    ret

section .bss
.tempdecr resd 1
section .text

NEWSYM indirectaddr
    push eax
    test byte[esi+10],07Fh
    jnz near .nozero
    test [hdmatype],ah
    jnz .noincr
    add word[edx+17],2
.noincr
    mov bl,ah
    not bl
    and [hdmatype],bl
    mov bl,[esi+4]
    mov cx,[edx+17]
    call dword near [memtabler8+ebx*4]
    inc word[edx+17]
    mov [esi+10],al
    push eax
    mov bl,[esi+4]
    mov cx,[edx+17]
    call dword near [memtabler16+ebx*4]
    mov [esi+5],ax
    pop eax
    test al,0FFh
    jnz .yeszero
    xor [nexthdma],ah
    jmp .finhdma2
.yeszero
    cmp byte[esi+10],80h
    ja near hdmatype2indirect
    mov al,[edx+16]
    mov [.tempdecr],al
    movzx ebx,byte[esi+7]
    movzx ecx,word[esi+5]  ; increment/decrement/keep pointer location
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc cx
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    add cx,2
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    add cx,3
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
    jmp .finhdma
.nozero
    test byte[esi+10],80h
    jnz near hdmatype2indirect
.finhdma
    mov ax,[edx+17]
    mov [esi+8],ax
    pop eax
    dec byte[esi+10]
    ret
.finhdma2
    mov ax,[edx+17]
    mov [esi+8],ax
    pop eax
    ret

section .bss
.tempdecr resd 1

section .data
.fname2 db 9,'vram2.dat',0
section .text

NEWSYM hdmatype2indirect
    mov al,[edx+16]
    mov [.tempdecr],al
    movzx ebx,byte[esi+7]
    movzx ecx,word[esi+5]  ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    movzx ebx,byte[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
.finhdma
    pop eax
    dec byte[esi+10]
    ret

section .bss
.tempdecr resd 1
.dest resd 1
section .text

NEWSYM exechdma
    cmp byte[hdmarestart],1
    je near exechdmars
    mov al,[nexthdma]
    cmp al,0
    je near .nohdma
    push ebx
    push esi
    push edi
    push ecx
    push edx
    mov esi,dmadata
    mov edx,hdmadata
    mov ah,01h
    test al,01h
    jz .notransa
    call dohdma
.notransa
    add esi,16
    add edx,19
    mov ah,02h
    test al,02h
    jz .notransb
    call dohdma
.notransb
    add esi,16
    add edx,19
    mov ah,04h
    test al,04h
    jz .notransc
    call dohdma
.notransc
    add esi,16
    add edx,19
    mov ah,08h
    test al,08h
    jz .notransd
    call dohdma
.notransd
    add esi,16
    add edx,19
    mov ah,10h
    test al,10h
    jz .notranse
    call dohdma
.notranse
    add esi,16
    add edx,19
    mov ah,20h
    test al,20h
    jz .notransf
    call dohdma
.notransf
    add esi,16
    add edx,19
    mov ah,40h
    test al,40h
    jz .notransg
    call dohdma
.notransg
    add esi,16
    add edx,19
    mov ah,80h
    test al,80h
    jz .notransh
    call dohdma
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
.nohdma
    ret

NEWSYM exechdmars
    mov al,[nexthdma]
    cmp al,0
    je near .nohdma
    push ebx
    push esi
    push edi
    push ecx
    push edx
    mov esi,dmadata
    mov edx,hdmadata
    mov ah,01h
    test al,01h
    jz .notransa
    call setuphdmars
    call dohdma
.notransa
    add esi,16
    add edx,19
    mov ah,02h
    test al,02h
    jz .notransb
    call setuphdmars
    call dohdma
.notransb
    add esi,16
    add edx,19
    mov ah,04h
    test al,04h
    jz .notransc
    call setuphdmars
    call dohdma
.notransc
    add esi,16
    add edx,19
    mov ah,08h
    test al,08h
    jz .notransd
    call setuphdmars
    call dohdma
.notransd
    add esi,16
    add edx,19
    mov ah,10h
    test al,10h
    jz .notranse
    call setuphdmars
    call dohdma
.notranse
    add esi,16
    add edx,19
    mov ah,20h
    test al,20h
    jz .notransf
    call setuphdmars
    call dohdma
.notransf
    add esi,16
    add edx,19
    mov ah,40h
    test al,40h
    jz .notransg
    call setuphdmars
    call dohdma
.notransg
    add esi,16
    add edx,19
    mov ah,80h
    test al,80h
    jz .notransh
    call setuphdmars
    call dohdma
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
.nohdma
    mov byte[hdmarestart],0
    ret


