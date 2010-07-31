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



%include "macros.mac"

EXTSYM winon
EXTSYM curvidoffset
EXTSYM winptrref,xtravbuf

;drawspritesprio

SECTION .bss
NEWSYM tileleft16b, resb 1
SECTION .text

NEWSYM domosaic16b
    mov esi,xtravbuf+32
    mov edi,[curvidoffset]
    mov dl,dh
    mov cl,0
    mov ax,[esi]
    cmp byte[winon],0
    jne near domosaicwin16b
    test ax,0FFFFh
    jz .zeroloop
.loopm
    mov [edi],ax
    add esi,2
    add edi,2
    dec cl
    jz .doneloop
    dec dl
    jnz .loopm
    mov ax,[esi]
    mov dl,dh
    test ax,0FFFFh
    jnz .loopm
.zeroloop
    add esi,2
    add edi,2
    dec cl
    jz .doneloop
    dec dl
    jnz .zeroloop
    mov ax,[esi]
    mov dl,dh
    test ax,0FFFFh
    jnz .loopm
    jmp .zeroloop
.doneloop
    ret

NEWSYM domosaicwin16b
    mov ebp,[winptrref]
    test ax,0FFFFh
    jz .zeroloop
.loopm
    cmp byte[ebp],0
    jne .nozero
    mov [edi],ax
.nozero
    add esi,2
    add edi,2
    inc ebp
    dec cl
    jz .doneloop
    dec dl
    jnz .loopm
    mov ax,[esi]
    mov dl,dh
    test ax,0FFFFh
    jnz .loopm
.zeroloop
    add esi,2
    add edi,2
    inc ebp
    dec cl
    jz .doneloop
    dec dl
    jnz .zeroloop
    mov ax,[esi]
    mov dl,dh
    test ax,0FFFFh
    jnz .loopm
    jmp .zeroloop
.doneloop
    ret
