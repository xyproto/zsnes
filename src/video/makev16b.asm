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

EXTSYM curypos,winon
EXTSYM bgcoloradder
EXTSYM drawn
EXTSYM curbgpr,curvidoffset
EXTSYM pal16b
EXTSYM bgofwptr,bgsubby,bshifter,curmosaicsz,cwinptr
EXTSYM temp,tempcach,temptile,winptrref,xtravbuf,yadder,yrevadder
EXTSYM vcache2b,vcache4b,vcache8b,hirestiledat,res512switch,numwin,windowdata
EXTSYM vidmemch4,vram,ofsmcptr
EXTSYM ofsmady,ofsmadx,yposngom,flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd
EXTSYM ngptrdat2
EXTSYM OMBGTestVal,cachesingle4bng,ofsmtptrs,ofsmcptr2,ofshvaladd

%include "video/vidmacro.mac"

;drawspritesprio

SECTION .bss
NEWSYM tileleft16b, resb 1
SECTION .text

NEWSYM draw8x816bwinon
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
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw
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
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
    ret

NEWSYM draw16x816
    push eax
    xor eax,eax
    mov al,[curypos]
    mov byte[hirestiledat+eax],1
    pop eax
    mov [temp],al
    mov [bshifter],ah
    mov eax,esi
    mov [yadder],ecx
    mov [tempcach],ebx
    mov ebx,56
    sub ebx,ecx
    mov [yrevadder],ebx
    ; esi = pointer to video buffer
    mov esi,[cwinptr]
    sub esi,eax
    mov [winptrref],esi
    mov esi,[curvidoffset]
    sub esi,eax           ; esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    sub esi,eax
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+32
    mov ecx,128
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+32
    sub esi,eax
    sub esi,eax
.nomosaic
    mov [temptile],edx
    push ecx
    mov dword[bgsubby],262144
    mov ecx,[vcache2b]
    add ecx,262144
    mov [bgofwptr],ecx
    cmp dword[tempcach],ecx
    jb .nobit
    mov dword[bgsubby],131072
    mov ecx,[vcache4b]
    add ecx,131072
    mov [bgofwptr],ecx
    cmp dword[tempcach],ecx
    jb .nobit
    mov ecx,[vcache8b]
    add ecx,65536
    mov [bgofwptr],ecx
    mov dword[bgsubby],65536
.nobit
    pop ecx
; tile value : bit 15 = flipy, bit 14 = flipx, bit 13 = priority value
;              bit 10-12 = palette, 0-9=tile#
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x816bwinon
.domosaic
    cmp byte[res512switch],0
    jne near draw16x816b
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
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
    drawpixel16b8x8 0, .loopd1, 0
    drawpixel16b8x8 2, .loopd3, 2
    drawpixel16b8x8 4, .loopd5, 4
    drawpixel16b8x8 6, .loopd7, 6
    add ebx,64
    ; Start loop
    drawpixel16b8x8 0, .loopd1c, 8
    drawpixel16b8x8 2, .loopd3c, 10
    drawpixel16b8x8 4, .loopd5c, 12
    drawpixel16b8x8 6, .loopd7c, 14
.hprior
    add esi,16
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw
    ret
    ; reversed loop
.rloop
    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    ; Start loop
    drawpixel16b8x8 1, .loopd1b, 14
    drawpixel16b8x8 3, .loopd3b, 12
    drawpixel16b8x8 5, .loopd5b, 10
    drawpixel16b8x8 7, .loopd7b, 8
    add ebx,64
    ; Start loop
    drawpixel16b8x8 1, .loopd1d, 6
    drawpixel16b8x8 3, .loopd3d, 4
    drawpixel16b8x8 5, .loopd5d, 2
    drawpixel16b8x8 7, .loopd7d, 0
    add esi,16
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
    ret

NEWSYM draw16x816b
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
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
    drawpixel16b8x8 1, .loopd1, 0
    drawpixel16b8x8 3, .loopd3, 2
    drawpixel16b8x8 5, .loopd5, 4
    drawpixel16b8x8 7, .loopd7, 6
    add ebx,64
    ; Start loop
    drawpixel16b8x8 1, .loopd1c, 8
    drawpixel16b8x8 3, .loopd3c, 10
    drawpixel16b8x8 5, .loopd5c, 12
    drawpixel16b8x8 7, .loopd7c, 14
.hprior
    add esi,16
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw
    ret
    ; reversed loop
.rloop
    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    ; Start loop
    drawpixel16b8x8 0, .loopd1b, 14
    drawpixel16b8x8 2, .loopd3b, 12
    drawpixel16b8x8 4, .loopd5b, 10
    drawpixel16b8x8 6, .loopd7b, 8
    add ebx,64
    ; Start loop
    drawpixel16b8x8 0, .loopd1d, 6
    drawpixel16b8x8 2, .loopd3d, 4
    drawpixel16b8x8 4, .loopd5d, 2
    drawpixel16b8x8 6, .loopd7d, 0
    add esi,16
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
    ret

NEWSYM draw16x816bwinon
    cmp byte[res512switch],0
    jne near draw16x816winonb
    mov ebp,[winptrref]
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
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
    drawpixel16b8x8winon 0, .loopd1, 0, 0
    drawpixel16b8x8winon 2, .loopd3, 2, 1
    drawpixel16b8x8winon 4, .loopd5, 4, 2
    drawpixel16b8x8winon 6, .loopd7, 6, 3
    add ebx,64
    ; Start loop
    drawpixel16b8x8winon 0, .loopd1c, 8, 4
    drawpixel16b8x8winon 2, .loopd3c, 10, 5
    drawpixel16b8x8winon 4, .loopd5c, 12, 6
    drawpixel16b8x8winon 6, .loopd7c, 14, 7
.hprior
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw
    ret
    ; reversed loop
.rloop
    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    ; Start loop
    drawpixel16b8x8winon 1, .loopd1b, 14, 0
    drawpixel16b8x8winon 3, .loopd3b, 12, 1
    drawpixel16b8x8winon 5, .loopd5b, 10, 2
    drawpixel16b8x8winon 7, .loopd7b, 8, 3
    add ebx,64
    ; Start loop
    drawpixel16b8x8winon 1, .loopd1d, 6, 4
    drawpixel16b8x8winon 3, .loopd3d, 4, 5
    drawpixel16b8x8winon 5, .loopd5d, 2, 6
    drawpixel16b8x8winon 7, .loopd7d, 0, 7
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
    ret

NEWSYM draw16x816winonb
    mov ebp,[winptrref]
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
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
    drawpixel16b8x8winon 1, .loopd1, 0, 0
    drawpixel16b8x8winon 3, .loopd3, 2, 1
    drawpixel16b8x8winon 5, .loopd5, 4, 2
    drawpixel16b8x8winon 7, .loopd7, 6, 3
    add ebx,64
    ; Start loop
    drawpixel16b8x8winon 1, .loopd1c, 8, 4
    drawpixel16b8x8winon 3, .loopd3c, 10, 5
    drawpixel16b8x8winon 5, .loopd5c, 12, 6
    drawpixel16b8x8winon 7, .loopd7c, 14, 7
.hprior
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw
    ret
    ; reversed loop
.rloop
    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    ; Start loop
    drawpixel16b8x8winon 0, .loopd1b, 14, 0
    drawpixel16b8x8winon 2, .loopd3b, 12, 1
    drawpixel16b8x8winon 4, .loopd5b, 10, 2
    drawpixel16b8x8winon 6, .loopd7b, 8, 3
    add ebx,64
    ; Start loop
    drawpixel16b8x8winon 0, .loopd1d, 6, 4
    drawpixel16b8x8winon 2, .loopd3d, 4, 5
    drawpixel16b8x8winon 4, .loopd5d, 2, 6
    drawpixel16b8x8winon 6, .loopd7d, 0, 7
    add esi,16
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
    ret

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

NEWSYM dowindow16b
    mov ebx,windowdata
    mov esi,xtravbuf+32
    mov edi,[curvidoffset]
    xor edx,edx
    xor ch,ch
.getnext
    mov cl,[ebx]
    cmp dl,cl
    je .procnext
.dorest
    sub cl,dl
    cmp ch,0
    ja .nodraw
.loopa
    mov ax,[esi+edx*2]
    test ax,0FFFFh
    jz .nocopy
    mov [edi+edx*2],ax
.nocopy
    inc dl
    dec cl
    jnz .loopa
.procnext
    add ch,[ebx+1]
    add ebx,2
    test byte[numwin],0FFh
    jz .finishwin
    dec byte[numwin]
    jnz .getnext
    xor cl,cl
    jmp .dorest
.nodraw
    add dl,cl
    jmp .procnext
.finishwin
    xor eax,eax
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
