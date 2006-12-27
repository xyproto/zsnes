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

EXTSYM bgcoloradder,bgofwptr,bgsubby,bshifter,curbgpr,curmosaicsz,curvidoffset
EXTSYM cwinptr,domosaic16b,drawn,pal16b,scaddtype,scrnon,temp,tempcach,temptile
EXTSYM tileleft16b,transpbuf,winon,winptrref,xtravbuf,yadd,yadder,yrevadder
EXTSYM draw16x816t,bgmode,vcache2b,vcache4b,vcache8b,fulladdtab,pal16bcl
EXTSYM pal16bxcl,coadder16,a16x16xinc,a16x16yinc,curypos,yflipadd

%include "video/vidmacro.mac"

;*******************************************************
; Processes & Draws 8x8 tiles in 2, 4, & 8 bit mode
;*******************************************************

%macro draw8x816tams 2
    mov al,[ebx+%1]
    or al,al
    jz %%loop
    add al,dh
    mov ecx,[ebp+%2]
    mov eax,[pal16b+eax*4]
    mov [ebp+%2],ax
    test ecx,0FFFFh
    je %%noadd
    and eax,1111011111011110b  ; [vesa2_clbit]
    and ecx,1111011111011110b  ; [vesa2_clbit]
    add eax,ecx
    shr eax,1
%%noadd
    mov [esi+%2],ax
    xor eax,eax
%%loop
%endmacro

%macro draw8x816tbms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    add al,dh
    mov ebx,[ebp+%2]
    mov ecx,[pal16bcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    mov [esi+%2],cx
%%loop
%endmacro

%macro draw8x816tcms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    add al,dh
    mov ebx,[ebp+%2]
    mov ecx,[pal16bxcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    xor ecx,0FFFFh
    mov [esi+%2],cx
%%loop
%endmacro

%macro draw8x816tawinonms 2
    mov al,[ebx+%1]
    or al,al
    jz %%loop
    test byte[edx+%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ecx,[ebp+%2]
    mov eax,[pal16b+eax*4]
    mov [ebp+%2],ax
    test ecx,0FFFFh
    je %%noadd
    and eax,1111011111011110b  ; [vesa2_clbit]
    and ecx,1111011111011110b  ; [vesa2_clbit]
    add eax,ecx
    shr eax,1
%%noadd
    mov [esi+%2],ax
    xor eax,eax
%%loop
%endmacro

%macro draw8x816tbwinonms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    test byte[edx+%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ebx,[ebp+%2]
    mov ecx,[pal16bcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    mov [esi+%2],cx
%%loop
%endmacro

%macro draw8x816tcwinonms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    test byte[edx+%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ebx,[ebp+%2]
    mov ecx,[pal16bxcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    xor ecx,0FFFFh
    mov [esi+%2],cx
%%loop
%endmacro

%macro draw8x816tawinonbms 2
    mov al,[ebx+%1]
    or al,al
    jz %%loop
    test byte[edx+7-%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ecx,[ebp+%2]
    mov eax,[pal16b+eax*4]
    mov [ebp+%2],ax
    test ecx,0FFFFh
    je %%noadd
    and eax,1111011111011110b  ; [vesa2_clbit]
    and ecx,1111011111011110b  ; [vesa2_clbit]
    add eax,ecx
    shr eax,1
%%noadd
    mov [esi+%2],ax
    xor eax,eax
%%loop
%endmacro

%macro draw8x816tbwinonbms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    test byte[edx+7-%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ebx,[ebp+%2]
    mov ecx,[pal16bcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    mov [esi+%2],cx
%%loop
%endmacro

%macro draw8x816tcwinonbms 2
    mov al,[edi+%1]
    or al,al
    jz %%loop
    test byte[edx+7-%1],0FFh
    jnz %%loop
    add al,[coadder16]
    mov ebx,[ebp+%2]
    mov ecx,[pal16bxcl+eax*4]
    mov [ebp+%2],cx
    and ebx,1111011111011110b  ; [vesa2_clbit]
    add ecx,ebx
    shr ecx,1
    mov ecx,[fulladdtab+ecx*2]
    xor ecx,0FFFFh
    mov [esi+%2],cx
%%loop
%endmacro

SECTION .text

NEWSYM draw8x816tms
    cmp byte[bgmode],5
    je near draw16x816t

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
    jne near draw8x816twinonms
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw8x816tsms
    test byte[scaddtype],40h
    jz near draw8x8fulladdms
    cmp byte[scrnon+1],0
    jz near draw8x8fulladdms
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
    xor ecx,ecx
    ; Start loop
    drawtilegrp draw8x816tams
.hprior
    add esi,16
    add ebp,16
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
    xor ecx,ecx
    drawtilegrpf draw8x816tams
    add esi,16
    add ebp,16
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

NEWSYM draw8x8fulladdms
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    push edi
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test dh,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test dh,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tbms
.hprior
    pop edi
    add esi,16
    add ebp,16
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
    xor ecx,ecx
    drawtilegrpfullf draw8x816tbms
    pop edi
    add esi,16
    add ebp,16
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

NEWSYM draw8x816tsms
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    push edi
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test dh,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test dh,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tcms
.hprior
    pop edi
    add esi,16
    add ebp,16
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tcms
    pop edi
    add esi,16
    add ebp,16
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw8x816twinonms
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw8x816tswinonms
    test byte[scaddtype],40h
    jz near draw8x8fulladdwinonms
    cmp byte[scrnon+1],0
    jz near draw8x8fulladdwinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
    mov byte[drawn],0
.loopa
    mov ax,[edi]
    mov cl,ah
    add edi,2
    xor cl,[curbgpr]
    test cl,20h
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
    test cl,80h
    jz .normadd
    add ebx,[yrevadder]
    jmp .skipadd
.normadd
    add ebx,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrp draw8x816tawinonms
.hprior
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    drawtilegrpf draw8x816tawinonbms
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw8x8fulladdwinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
    mov byte[drawn],0
.loopa
    mov ax,[edi]
    mov cl,ah
    add edi,2
    push edi
    xor cl,[curbgpr]
    test cl,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test cl,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tbwinonms
.hprior
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tbwinonbms
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw8x816tswinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
    mov byte[drawn],0
.loopa
    mov ax,[edi]
    mov cl,ah
    add edi,2
    push edi
    xor cl,[curbgpr]
    test cl,20h
    jnz near .hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test cl,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop

    ; Begin Normal Loop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tcwinonms
.hprior
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tcwinonbms
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

;*******************************************************
; Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
;*******************************************************
NEWSYM draw16x1616tms
    mov byte[drawn],0
    mov [temp],eax
    mov eax,esi
    mov [yadder],ecx
    mov [tempcach],ebx
    mov ebx,56
    sub ebx,ecx
    mov [yrevadder],ebx
    xor ebx,ebx
    mov bl,[curypos]
    mov [temptile],edx
    ; set up y adders
    test byte[a16x16yinc],01h
    jz .noincrc
    mov word[yadd],16
    mov word[yflipadd],0
    jmp .yesincrc
.noincrc
    mov word[yadd],0
    mov word[yflipadd],16
.yesincrc
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
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x1616twinonms
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616tsms
    test byte[scaddtype],40h
    jz near draw16x16fulladdms
    cmp byte[scrnon+1],0
    je near draw16x16fulladdms
    mov byte[tileleft16b],33
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    xor byte[a16x16xinc],1
    test dh,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor dh,[curbgpr]
    test dh,20h
    jnz near .hprior
    inc byte[drawn]
    test dh,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip
    and ax,03FFh                ; filter out tile #
    mov ebx,[tempcach]
    shl ax,6
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
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrp draw8x816tams
.hprior
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc dl
.noincrb2
    cmp dl,20h
    jne .loopc2
    xor dl,dl
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpf draw8x816tams
.skiploop2b
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc dl
.noincrb
    cmp dl,20h
    jne .loopc
    xor dl,dl
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

draw16x16fulladdms:
    mov byte[tileleft16b],33
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    xor byte[a16x16xinc],1

    test dh,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor dh,[curbgpr]
    push edi
    test dh,20h
    jnz near .hprior
    inc byte[drawn]

    test dh,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip

    and ax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl ax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test dh,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test dh,40h
    jnz near .rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tbms
.hprior
    pop edi
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc dl
.noincrb2
    cmp dl,20h
    jne .loopc2
    xor dl,dl
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tbms
    pop edi
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc dl
.noincrb
    cmp dl,20h
    jne .loopc
    xor dl,dl
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

NEWSYM draw16x1616tsms
    mov byte[tileleft16b],33
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    xor byte[a16x16xinc],1

    test dh,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor dh,[curbgpr]
    push edi
    test dh,20h
    jnz near .hprior
    inc byte[drawn]

    test dh,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip

    and ax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl ax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test dh,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test dh,40h
    jnz near .rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tcms
.hprior
    pop edi
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc dl
.noincrb2
    cmp dl,20h
    jne .loopc2
    xor dl,dl
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tcms
    pop edi
    add esi,16
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc dl
.noincrb
    cmp dl,20h
    jne .loopc
    xor dl,dl
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

NEWSYM draw16x1616twinonms
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616tswinonms
    test byte[scaddtype],40h
    jz near draw16x16fulladdwinonms
    cmp byte[scrnon+1],0
    je near draw16x16fulladdwinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
.loopa
    mov ax,[edi]
    mov cl,ah
    xor byte[a16x16xinc],1
    test cl,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor cl,[curbgpr]
    test cl,20h
    jnz near .hprior
    inc byte[drawn]
    test cl,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip
    and ax,03FFh                ; filter out tile #
    mov ebx,[tempcach]
    shl ax,6
    add ebx,eax
    cmp ebx,[bgofwptr]
    jb .noclip
    sub ebx,[bgsubby]
.noclip
    test cl,80h
    jz .normadd
    add ebx,[yrevadder]
    jmp .skipadd
.normadd
    add ebx,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrp draw8x816tawinonms
.hprior
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc byte[temp]
.noincrb2
    cmp byte[temp],20h
    jne .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpf draw8x816tawinonbms
.skiploop2b
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc byte[temp]
.noincrb
    cmp byte[temp],20h
    jne near .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw16x16fulladdwinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
.loopa
    mov ax,[edi]
    mov cl,ah
    xor byte[a16x16xinc],1

    test cl,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor cl,[curbgpr]
    push edi
    test cl,20h
    jnz near .hprior
    inc byte[drawn]

    test cl,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip

    and ax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl ax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test cl,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tbwinonms
.hprior
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc byte[temp]
.noincrb2
    cmp byte[temp],20h
    jne .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tbwinonbms
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc byte[temp]
.noincrb
    cmp byte[temp],20h
    jne near .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw16x1616tswinonms
    mov byte[tileleft16b],33
    mov edx,[winptrref]
.loopa
    mov ax,[edi]
    mov cl,ah
    xor byte[a16x16xinc],1

    test cl,40h
    jnz .noxflip
    test byte[a16x16xinc],01h
    jnz .noincr2
    inc ax
    add edi,2
.noincr2
    jmp .yesxflip
.noxflip
    test byte[a16x16xinc],01h
    jnz .noincr
    add edi,2
    jmp .yesincr
.noincr
    inc ax
.yesincr
.yesxflip
    xor cl,[curbgpr]
    push edi
    test cl,20h
    jnz near .hprior
    inc byte[drawn]

    test cl,80h
    jnz .noyflip
    add ax,word[yadd]
    jmp .yesyflip
.noyflip
    add ax,word[yflipadd]
.yesyflip

    and ax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl ax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb .noclip
    sub edi,[bgsubby]
.noclip
    test cl,80h
    jz .normadd
    add edi,[yrevadder]
    jmp .skipadd
.normadd
    add edi,[yadder]
.skipadd
    test cl,40h
    jnz near .rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    drawtilegrpfull draw8x816tcwinonms
.hprior
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc byte[temp]
.noincrb2
    cmp byte[temp],20h
    jne .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc2
    dec byte[tileleft16b]
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov al,cl
    mov cl,[bshifter]
    and al,1Ch
    shl al,cl                    ; process palette # (bits 10-12)
    add al,[bgcoloradder]
    mov [coadder16],al
    xor eax,eax
    xor ecx,ecx
    drawtilegrpfullf draw8x816tcwinonbms
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc byte[temp]
.noincrb
    cmp byte[temp],20h
    jne near .loopc2
    mov byte[temp],0
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret
