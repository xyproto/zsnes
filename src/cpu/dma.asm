;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
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

EXTSYM memtabler8,regptw,snesmap2,snesmmap,debstop3
;EXTSYM soundcycleft,pexecs2
EXTSYM memtablew8,regptr
EXTSYM dmadata
EXTSYM hdmatype
EXTSYM nexthdma
EXTSYM curhdma,curypos,disablehdma,hdmadata,hdmadelay,hdmaearlstart
EXTSYM resolutn
EXTSYM memtabler16

NEWSYM DmaAsmStart




;*******************************************************
; Transfer DMA                     Inits & Transfers DMA
;*******************************************************
; DMA transfer register

NEWSYM AddrNoIncr, db 0

%macro ExecSPCCycles 0
    xor ebx,ebx
    mov bx,[esi+5]
    inc bx
    inc ebx
    shr ebx,2
    mov [soundcycleft],ebx
    or ebx,ebx
    jz .nocycles
    xor ebx,ebx
    xor ecx,ecx
    call pexecs2
.nocycles
%endmacro

NEWSYM transdma
    push eax
    cmp word[esi+5],480h
    jne .no
;    mov byte[debstop3],1
.no
;   ExecSPCCycles

    mov al,[esi]
    test al,80h
    jnz near transdmappu2cpu

    ; set address increment value
    mov dword[.addrincr],0
    test al,00001000b
    jnz .skipaddrincr
    test al,00010000b
    jnz .autodec
    mov dword[.addrincr],1
    jmp .skipaddrincr
.autodec
    mov dword[.addrincr],0FFFFFFFFh
.skipaddrincr

    mov byte[AddrNoIncr],0
    cmp dword[.addrincr],0
    jne .notzero
    mov byte[AddrNoIncr],1
.notzero
    ; get address order to be written
    xor ebx,ebx
    and al,00000111b
    cmp al,5
    jne .notmode5dma
    mov al,1
.notmode5dma
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi]
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [.regptra],eax

    ; get pointer #2
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+2]
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [.regptrb],eax

    ; get pointer #3
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+4]
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [.regptrc],eax

    ; get pointer #4
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+6]
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [.regptrd],eax

    mov dx,[esi+5]      ; Number of bytes to transfer
    xor ebx,ebx
    mov bl,[esi+4]      ; Bank #
    mov ecx,[esi+2]      ; address offset #
    and ecx,0FFFFh
    mov [.curbank],bl
    mov word[esi+5],0

    mov ebx,[.curbank]
    mov eax,snesmap2
    test ecx,8000h
    jz .nomap1
    mov eax,snesmmap
.nomap1
    and edx,0FFFFh
    mov ebx,[eax+ebx*4]

;snesmmap times 256 dd 0         ; addresses 8000-FFFF
;snesmap2 times 256 dd 0         ; addresses 0000-7FFF

    push esi
    mov esi,ebx
    xor ebx,ebx
    mov bl,[.curbank]
    ; do loop
    cmp edx,0
    jne .no0
    mov edx,65536
.no0
    mov ebx,[memtabler8+ebx*4]
    mov [.readaddr],ebx
    xor ebx,ebx
    mov bl,[.curbank]
    mov [.cebx],ebx
.againloop
    cmp edx,4
    jbe .deccheckloop
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptra]
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrb]
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrc]
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrd]
    sub edx,4
    jmp .againloop
.deccheckloop
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptra]
    dec edx
    jz .findma
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrb]
    dec edx
    jz .findma
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrc]
    dec edx
    jz .findma
    mov ebx,[.cebx]
    call dword near [.readaddr]
    add cx,[.addrincr]
    call dword near [.regptrd]
.findma
    pop esi
    mov [esi+2],cx
    pop eax
    mov byte[AddrNoIncr],0
    ret

ALIGN32

.curbank   dd 0
.addrincr  dd 0
.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
; pointer address of registers
.regptra   dd 0
.regptrb   dd 0
.regptrc   dd 0
.regptrd   dd 0
.readaddr  dd 0
.cebx      dd 0

NEWSYM transdmappu2cpu
    ; set address increment value
    mov dword[.addrincr],0
    test al,00001000b
    jnz .skipaddrincr
    test al,00010000b
    jnz .autodec
    mov dword[.addrincr],1
    jmp .skipaddrincr
.autodec
    mov dword[.addrincr],0FFFFFFFFh
.skipaddrincr

    ; get address order to be written
    xor ebx,ebx
    and al,00000111b
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi]
    shl ebx,2
    add ebx,[regptr]
    mov eax,[ebx]
    mov [.regptra],eax

    ; get pointer #2
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+2]
    shl ebx,2
    add ebx,[regptr]
    mov eax,[ebx]
    mov [.regptrb],eax

    ; get pointer #3
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+4]
    shl ebx,2
    add ebx,[regptr]
    mov eax,[ebx]
    mov [.regptrc],eax

    ; get pointer #4
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
    mov bh,21h
    add bx,[edi+6]
    shl ebx,2
    add ebx,[regptr]
    mov eax,[ebx]
    mov [.regptrd],eax

    mov dx,[esi+5]      ; Number of bytes to transfer
    xor ebx,ebx
    mov bl,[esi+4]      ; Bank #
    mov ecx,[esi+2]      ; address offset #
    and ecx,0FFFFh
    mov [.curbank],bl
    mov word[esi+5],0

    mov ebx,[.curbank]
    mov eax,snesmap2
    test ecx,8000h
    jz .nomap1
    mov eax,snesmmap
.nomap1
    and edx,0FFFFh
    mov ebx,[eax+ebx*4]

;snesmmap times 256 dd 0         ; addresses 8000-FFFF
;snesmap2 times 256 dd 0         ; addresses 0000-7FFF

    push esi
    mov esi,ebx
    xor ebx,ebx
    mov bl,[.curbank]
    ; do loop
    cmp edx,0
    jne .no0
    mov edx,65536
.no0
    mov ebx,[memtablew8+ebx*4]
    mov [.writeaddr],ebx
    xor ebx,ebx
    mov bl,[.curbank]
    mov [.cebx],ebx
.againloop
    cmp edx,4
    jbe .deccheckloop
    call dword near [.regptra]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    call dword near [.regptrb]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    call dword near [.regptrc]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    call dword near [.regptrd]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    sub edx,4
    jmp .againloop
.deccheckloop
    call dword near [.regptra]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    dec edx
    jz .findma
    call dword near [.regptrb]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    dec edx
    jz .findma
    call dword near [.regptrc]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
    dec edx
    jz .findma
    call dword near [.regptrd]
    mov ebx,[.cebx]
    call dword near [.writeaddr]
    add cx,[.addrincr]
.findma
    pop esi
    mov [esi+2],cx
    pop eax
    ret

ALIGN32

.curbank   dd 0
.addrincr  dd 0
.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
; pointer address of registers
.regptra   dd 0
.regptrb   dd 0
.regptrc   dd 0
.regptrd   dd 0
.writeaddr dd 0
.cebx      dd 0

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
    call transdma
.notransa
    add esi,16
    test al,02h
    jz .notransb
    TestDMA
    call transdma
.notransb
    add esi,16
    test al,04h
    jz .notransc
    TestDMA
    call transdma
.notransc
    add esi,16
    test al,08h
    jz .notransd
    TestDMA
    call transdma
.notransd
    add esi,16
    test al,10h
    jz .notranse
    TestDMA
    call transdma
.notranse
    add esi,16
    test al,20h
    jz .notransf
    TestDMA
    call transdma
.notransf
    add esi,16
    test al,40h
    jz .notransg
    TestDMA
    call transdma
.notransg
    add esi,16
    test al,80h
    jz .notransh
    TestDMA
    call transdma
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
    xor eax,eax
    xor ecx,ecx
    mov al,[esi]
    and al,00000111b
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx],eax

    ; get pointer #2
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+4],eax

    ; get pointer #3
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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

    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+8],eax

    ; get pointer #4
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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

    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+12],eax

    xor ebx,ebx
    mov byte[esi+10],0
    pop eax
    or [hdmatype],ah
    ret

.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4

NEWSYM setuphdmars
    push eax

    ; get address order to be written
    xor ebx,ebx
    xor eax,eax
    xor ecx,ecx
    mov al,[esi]
    and al,00000111b
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx],eax

    ; get pointer #2
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+4],eax

    ; get pointer #3
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+8],eax

    ; get pointer #4
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+12],eax

    xor ebx,ebx
    pop eax
    ret

.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4

NEWSYM setuphdma2
    push eax

    cmp byte[esi+10],0
    je near .nohdma

    ; transfer old address to new address
    mov ax,[esi+8]
    mov [edx+17],ax
    ; get address order to be written
    xor ebx,ebx
    xor eax,eax
    xor ecx,ecx
    mov al,[esi]
    and al,00000111b
    mov ah,[.addrnumt+eax]
    mov [edx+16],ah
    mov bl,al
    shl bl,3
    add ebx,.addrwrite
    mov edi,ebx

    ; get pointer #1
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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

    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx],eax

    ; get pointer #2
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+4],eax
    ; get pointer #3
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+8],eax

    ; get pointer #4
    xor ebx,ebx
    mov bl,[esi+1]      ; PPU memory - 21xx
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
    shl ebx,2
    add ebx,[regptw]
    mov eax,[ebx]
    mov [edx+12],eax

    xor ebx,ebx
    pop eax
    and [hdmatype],ah
    ret
.nohdma
    pop eax
    and [nexthdma],ah
    ret

.addrwrite dw 0,0,0,0, 0,1,0,1, 0,0,0,0, 0,0,1,1, 0,1,2,3, 0,1,2,3, 0,1,2,3
           dw 0,1,2,3
.addrnumt  db 1,2,2,4,4,4,4,4

NEWSYM hdmastartsc, db 0
NEWSYM hdmarestart, db 0

NEWSYM reg420Cw

    mov [curhdma],al
    mov bx,[resolutn]
    cmp word[curypos],bx
    jae near .nohdma
    cmp byte[disablehdma],0
    jne near .nohdma
;    jmp starthdma
    mov al,[curhdma]
    mov [nexthdma],al
    cmp al,0
    je near .nohdma
    test al,80h
    ja near .nohdma
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
    mov bl,[hdmaearlstart]
    mov [hdmadelay],bl
    pop edx
    pop ecx
    pop edi
    pop esi
    pop ebx
;    call exechdma
;    call exechdma
.nohdma
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
    mov byte[esi+10],al
    test al,0FFh
    jnz .yeszero
    xor [nexthdma],ah
    jmp .finhdma2
.yeszero
    cmp byte[esi+10],80h
    ja near hdmatype2
    mov al,[edx+16]
    mov [.tempdecr],al
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc cx
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    add cx,2
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
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

.tempdecr db 0

NEWSYM hdmatype2
    mov al,[edx+16]
    mov [.tempdecr],al
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
    mov cx,[edx+17]         ; increment/decrement/keep pointer location
    inc word[edx+17]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+4]
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

.tempdecr db 0

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
    mov byte[esi+10],al
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
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc cx
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    add cx,2
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
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

.tempdecr db 0
.fname2 db 9,'vram2.dat',0

NEWSYM hdmatype2indirect
    mov al,[edx+16]
    mov [.tempdecr],al
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .finhdma
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]         ; increment/decrement/keep pointer location
    inc word[esi+5]
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
.finhdma
    pop eax
    dec byte[esi+10]
    ret

.tempdecr db 0


NEWSYM dohdma2
    ; new hdma routines
    xor ebx,ebx
    push eax
    test [hdmatype],ah
    jz .nostart
    xor ah,0FFh
    xor ecx,ecx
    and [hdmatype],ah
    test byte[esi],40h
    jnz near .indir
    mov bl,[esi+4]
    mov cx,[esi+8]
    call dword near [memtabler8+ebx*4]
    inc word[esi+8]
    xor ebx,ebx
    mov [esi+10],al
    jmp .nostart
.indir
    mov bl,[esi+4]
    mov [esi+7],bl
    mov cx,[esi+2]
    mov [esi+5],cx
    call dword near [memtabler8+ebx*4]
    xor ebx,ebx
    inc word[esi+5]
    mov bl,[esi+7]
    mov [esi+10],al
    mov cx,[esi+5]
    call dword near [memtabler8+ebx*4]
    xor ebx,ebx
    inc word[esi+5]
    mov bl,[esi+7]
    mov [esi+8],al
    mov cx,[esi+5]
    call dword near [memtabler8+ebx*4]
    inc word[esi+5]
    mov [esi+9],al
    xor ebx,ebx
.nostart

    cmp byte[esi+10],0
    je near .endhdma
    mov bl,[esi+4]
    mov al,[edx+16]
    mov [.tempdecr],al
    xor ecx,ecx
    mov bl,[esi+4]
    mov cx,[esi+8]         ; increment/decrement/keep pointer location
    call dword near [memtabler8+ebx*4]
    call dword near [edx]
    dec byte[.tempdecr]
    jz .nomorehdma
    xor ebx,ebx
    mov cx,[esi+8]         ; increment/decrement/keep pointer location
    mov bl,[esi+4]
    inc cx
    call dword near [memtabler8+ebx*4]
    call dword near [edx+4]
    dec byte[.tempdecr]
    jz .nomorehdma
    xor ebx,ebx
    mov cx,[esi+8]         ; increment/decrement/keep pointer location
    mov bl,[esi+4]
    add cx,2
    call dword near [memtabler8+ebx*4]
    call dword near [edx+8]
    dec byte[.tempdecr]
    jz .nomorehdma
    xor ebx,ebx
    mov cx,[esi+8]         ; increment/decrement/keep pointer location
    mov bl,[esi+4]
    add cx,3
    call dword near [memtabler8+ebx*4]
    call dword near [edx+12]
.nomorehdma
    dec byte[esi+10]
    test byte[esi+10],7Fh
    jz .zeroed
    test byte[esi+10],80h
    jz .noadd
    xor ebx,ebx
    mov bl,[edx+16]
    add [esi+8],bx
.noadd
    pop eax
    ret
.zeroed
    test byte[esi],40h
    jnz near .indirzero
    xor ebx,ebx
    mov bl,[edx+16]
    add [esi+8],bx
    mov bl,[esi+4]
    mov cx,[esi+8]
    call dword near [memtabler8+ebx*4]
    inc word[esi+8]
    mov [esi+10],al
    pop eax
    ret
.indirzero
    xor ebx,ebx
    mov bl,[esi+7]
    mov cx,[esi+5]
    call dword near [memtabler8+ebx*4]
    xor ebx,ebx
    inc word[esi+5]
    mov bl,[esi+7]
    mov [esi+10],al
    mov cx,[esi+5]
    call dword near [memtabler8+ebx*4]
    xor ebx,ebx
    inc word[esi+5]
    mov bl,[esi+7]
    mov [esi+8],al
    mov cx,[esi+5]
    call dword near [memtabler8+ebx*4]
    inc word[esi+5]
    mov [esi+9],al
    pop eax
    ret
.endhdma
    pop eax
    xor [nexthdma],ah
    ret

.tempdecr db 0

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



NEWSYM DmaAsmEnd
