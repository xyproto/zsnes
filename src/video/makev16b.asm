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

EXTSYM cursprloc,curypos,scrndis,scrnon,winon,winonsp,drawmode716extbg
EXTSYM drawmode716extbg2
EXTSYM bgcoloradder
EXTSYM drawn,makewindow,winbg1en,mosaicon,winenabm,vidbuffer
EXTSYM curbgpr,currentobjptr,curvidoffset
EXTSYM cwinenabm,makewindowsp
EXTSYM preparesprpr,spritetablea,sprleftpr
EXTSYM bg1scrolx,bg1scroly,drawmode716b,mode7set,mosaicsz
EXTSYM sprleftpr1,sprleftpr2,sprleftpr3,sprlefttot,sprprifix,interlval,extbgdone
EXTSYM pal16b
EXTSYM bgofwptr,bgsubby,bshifter,curmosaicsz,cwinptr
EXTSYM temp,tempcach,temptile,winptrref,xtravbuf,yadder,yrevadder
EXTSYM vcache2b,vcache4b,vcache8b,hirestiledat,res512switch,numwin,windowdata
EXTSYM vidmemch4,vram,ofsmcptr
EXTSYM ofsmady,ofsmadx,yposngom,flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd
EXTSYM bg1scrolx_m7,bg1scroly_m7,ngptrdat2
EXTSYM OMBGTestVal,cachesingle4bng,m7starty,ofsmtptrs,ofsmcptr2,ofshvaladd
EXTSYM clearback16b,setpalette16b,drawsprites16b

%include "video/vidmacro.mac"

;drawspritesprio

SECTION .bss
NEWSYM tempstuff, resd 1

;ALIGN16
.stuff resd 1
.stuff2 resb 2

SECTION .text

%macro procmode716bextbg 3
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    test byte[mode7set],02h
    jz %%noflip
    neg ax
    add ax,255
%%noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],%3
    jz %%nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je %%nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
%%nomos
    mov [m7starty],ax
    mov ax,%1
    mov dx,%2
    call drawmode716extbg
%endmacro

%macro procmode716bextbg2 3
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    mov byte[curmosaicsz],1
    test byte[mosaicon],%3
    jz %%nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je %%nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
%%nomos
    call drawmode716extbg2
%endmacro

NEWSYM procspritessub16b
    test byte[scrndis],10h
    jnz .nosprites
    test byte[scrnon+1],10h
    jz .nosprites
    test byte[scrnon],10h
    jnz .nosprites
    cmp byte[winonsp],0FFh
    je .nosprites
    xor ebx,ebx
    mov bl,[curypos]
    add ebx,[cursprloc]
    mov cl,[ebx]
    cmp byte[sprprifix],0
    jne .sprprio
    add dword[cursprloc],256
.sprprio
    cmp cl,0
    je .nosprites
    ccallv drawsprites16b, ecx, ebp
.nosprites
    ret


NEWSYM processmode716b
    mov al,[winenabm]
    mov [cwinenabm],al
    push esi
    push edi
    push ebx
    push edx
    push ebp
    ; get current sprite table
    xor ebx,ebx
    mov bl,[curypos]
    shl ebx,9
    add ebx,[spritetablea]
    mov [currentobjptr],ebx
    ; setup priorities
    cmp byte[sprprifix],0
    je .nosprprio
    mov dword[cursprloc],sprlefttot
    call preparesprpr
.nosprprio
    ; calculate current video offset
    xor ebx,ebx
    mov bx,[curypos]
    mov esi,ebx
    shl esi,9
    shl ebx,6
    add esi,ebx
    add esi,32
    add esi,[vidbuffer]
    mov [curvidoffset],esi
    ; do sprite windowing
    call makewindowsp
    ; set palette
    ccallv setpalette16b
    ; clear back area w/ back color
    ccallv clearback16b
    ; clear registers
    xor eax,eax
    xor ecx,ecx

    mov byte[extbgdone],0
    ; mode 7 extbg
    test byte[interlval],40h
    jz near .noback0
    test byte[scrndis],02h
    jnz near .noback0
    ; do background 1
    test word[scrnon],0202h
    jz near .noback0
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin0
    test word[winenabm],0100h
    jnz near .nobackwin0
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback0
.nobackwin0
    mov byte[extbgdone],1
    procmode716bextbg [bg1scroly_m7],[bg1scrolx_m7],1
.noback0

    ; do objects
    test byte[scrndis],10h
    jnz near .nosprites1
    test word[scrnon],1010h
    jz near .nosprites1
    cmp byte[winonsp],0FFh
    je .nosprites1
    xor ebx,ebx
    mov bl,[curypos]
    mov cl,[sprleftpr+ebx]
    cmp byte[sprprifix],0
    je .nosprprio2
    mov cl,[sprlefttot+ebx]
.nosprprio2
    cmp cl,0
    je .nosprites1
    mov ebp,0
    ccallv drawsprites16b, ecx, ebp
.nosprites1

    ; display mode7
    test byte[interlval],40h
    jnz near .noback1
    test byte[scrndis],01h
    jnz near .noback1
    ; do background 1
    test word[scrnon],0101h
    jz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    procmode716b [bg1scroly_m7],[bg1scrolx_m7],1
.noback1

    ; do objects
    test byte[scrndis],10h
    jnz near .nosprites2
    test word[scrnon],1010h
    jz near .nosprites2
    cmp byte[winonsp],0FFh
    je .nosprites2
    xor ebx,ebx
    mov bl,[curypos]
    mov cl,[sprleftpr1+ebx]
    cmp byte[sprprifix],0
    je .nosprprio3
    mov cl,[sprlefttot+ebx]
.nosprprio3
    cmp cl,0
    je .nosprites2
    mov ebp,1
    ccallv drawsprites16b, ecx, ebp
.nosprites2

    test byte[interlval],40h
    jz near .noback0b
    test byte[scrndis],01h
    jnz near .noback0b
    cmp byte[extbgdone],0
    jne near .noback0b
    ; do background 1
    test word[scrnon],0101h
    jz near .noback0b
    mov byte[winon],0
    test word[winenabm],0002h
    jz near .nobackwin0b
    test word[winenabm],0200h
    jnz near .nobackwin0b
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback0b
.nobackwin0b
    mov byte[extbgdone],1
    procmode716bextbg [bg1scroly_m7],[bg1scrolx_m7],1
.noback0b

    ; mode 7 extbg
    test byte[interlval],40h
    jz near .noback2
    cmp byte[extbgdone],0
    je near .noback2
    test byte[scrndis],01h
    jnz near .noback2
    ; do background 1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin2
    test word[winenabm],0100h
    jnz near .nobackwin2
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback2
.nobackwin2
    procmode716bextbg2 [bg1scroly_m7],[bg1scrolx_m7],1
.noback2

    ; do objects
    test byte[scrndis],10h
    jnz near .nosprites3
    test word[scrnon],1010h
    jz near .nosprites3
    cmp byte[winonsp],0FFh
    je .nosprites3
    xor ebx,ebx
    mov bl,[curypos]
    mov cl,[sprleftpr2+ebx]
    cmp byte[sprprifix],0
    je .nosprprio4
    mov cl,[sprlefttot+ebx]
.nosprprio4
    cmp cl,0
    je .nosprites3
    mov ebp,2
    ccallv drawsprites16b, ecx, ebp
.nosprites3
    ; do objects
    test byte[scrndis],10h
    jnz near .nosprites4
    test word[scrnon],1010h
    jz near .nosprites4
    cmp byte[winonsp],0FFh
    je .nosprites4
    xor ebx,ebx
    mov bl,[curypos]
    mov cl,[sprleftpr3+ebx]
    cmp byte[sprprifix],0
    je .nosprprio5
    mov cl,[sprlefttot+ebx]
.nosprprio5
    cmp cl,0
    je .nosprites4
    mov ebp,3
    ccallv drawsprites16b, ecx, ebp
.nosprites4
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret


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
