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
EXTSYM bgcoloradder
EXTSYM drawn
EXTSYM curbgpr,curvidoffset
EXTSYM pal16b
EXTSYM bgofwptr,bgsubby,bshifter
EXTSYM temp,tempcach,winptrref,xtravbuf,yadder,yrevadder
EXTSYM vidmemch4,vram,ofsmcptr
EXTSYM ofsmady,ofsmadx,yposngom,flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd
EXTSYM ngptrdat2
EXTSYM OMBGTestVal,cachesingle4bng,ofsmtptrs,ofsmcptr2,ofshvaladd

%include "video/vidmacro.mac"

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

NEWSYM draw8x816bwinonoffset
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
    mov ebp,[winptrref]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    offsetmcachechk
    mov ebx,[tempcach]
    shl eax,6
    add ebx,eax
    cmp ebx,[bgofwptr]
    jb .noclip
    sub ebx,[bgsubby]
.noclip
    test dh,80h
    jz .normadd
    add ebx,[yrevadder]
    jmp .skipadd
.normadd
    add ebx,[yadder]
.skipadd
    test dh,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    ; Start loop
    cmp dword[ebx],0
    je .loopd4
    Draw8x816bwinmacro 0
    Draw8x816bwinmacro 1
    Draw8x816bwinmacro 2
    Draw8x816bwinmacro 3
.loopd4
    cmp dword[ebx+4],0
    je .loopd8
    Draw8x816bwinmacro 4
    Draw8x816bwinmacro 5
    Draw8x816bwinmacro 6
    Draw8x816bwinmacro 7
.loopd8
.hprior
    procoffsetmode
    add esi,16
    add ebp,8
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    cmp dword[ebx+4],0
    je .loopd4b
    Draw8x816bwinflipmacro 0
    Draw8x816bwinflipmacro 1
    Draw8x816bwinflipmacro 2
    Draw8x816bwinflipmacro 3
.loopd4b
    cmp dword[ebx],0
    je .loopd8b
    Draw8x816bwinflipmacro 4
    Draw8x816bwinflipmacro 5
    Draw8x816bwinflipmacro 6
    Draw8x816bwinflipmacro 7
.loopd8b
    procoffsetmode
    add esi,16
    add ebp,8
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret
