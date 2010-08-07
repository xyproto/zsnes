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

EXTSYM dmadata,nexthdma,resolutn,curhdma,curypos,hdmadata
EXTSYM hdmadelay,hdmarestart,nohdmaframe,INTEnab,HIRQLoc
EXTSYM transdma
EXTSYM setuphdma
EXTSYM setuphdmars
EXTSYM dohdma

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
    test al,01h
    jz .notransa
    ccallv setuphdma, 0x01, edx, esi
.notransa
    add esi,16
    add edx,19
    mov ah,02h
    jz .notransb
    ccallv setuphdma, 0x02, edx, esi
.notransb
    add esi,16
    add edx,19
    test al,04h
    jz .notransc
    ccallv setuphdma, 0x04, edx, esi
.notransc
    add esi,16
    add edx,19
    test al,08h
    jz .notransd
    ccallv setuphdma, 0x08, edx, esi
.notransd
    add esi,16
    add edx,19
    test al,10h
    jz .notranse
    ccallv setuphdma, 0x10, edx, esi
.notranse
    add esi,16
    add edx,19
    test al,20h
    jz .notransf
    ccallv setuphdma, 0x20, edx, esi
.notransf
    add esi,16
    add edx,19
    test al,40h
    jz .notransg
    ccallv setuphdma, 0x40, edx, esi
.notransg
    add esi,16
    add edx,19
    test al,80h
    jz .notransh
    ccallv setuphdma, 0x80, edx, esi
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
.nohdma
    cmp byte[nohdmaframe],1
    jne .notframe
    inc byte[hdmadelay]
.notframe
    mov byte[hdmarestart],0
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
    test al,01h
    jz .notransa
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x01, edx, esi
.notransa
    add esi,16
    add edx,19
    test al,02h
    jz .notransb
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x02, edx, esi
.notransb
    add esi,16
    add edx,19
    test al,04h
    jz .notransc
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x04, edx, esi
.notransc
    add esi,16
    add edx,19
    test al,08h
    jz .notransd
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x08, edx, esi
.notransd
    add esi,16
    add edx,19
    test al,10h
    jz .notranse
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x10, edx, esi
.notranse
    add esi,16
    add edx,19
    test al,20h
    jz .notransf
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x20, edx, esi
.notransf
    add esi,16
    add edx,19
    test al,40h
    jz .notransg
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x40, edx, esi
.notransg
    add esi,16
    add edx,19
    test al,80h
    jz .notransh
    ccallv setuphdmars, edx, esi
    ccallv dohdma, 0x80, edx, esi
.notransh
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
.nohdma
    mov byte[hdmarestart],0
    ret


