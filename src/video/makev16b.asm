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

EXTSYM cursprloc,curypos,scrndis,scrnon,winon,winonsp,drawmode716extbg
EXTSYM drawmode716extbg2,alreadydrawn,bg1cachloc,bg1tdabloc,bg1tdatloc
EXTSYM bg1vbufloc,bg1xposloc,bg1yaddval,bgcoloradder,bgmode,bgtilesz,curbgnum
EXTSYM drawn,makewindow,winbg1en,winenabs,mosaicon,winenabm,vidbuffer,bg3high2
EXTSYM colormodedef,colormodeofs,curbgpr,curblank,currentobjptr,curvidoffset
EXTSYM cwinenabm,drawline16t,forceblnk,makewindowsp,maxbr,newengen,newengine16b
EXTSYM preparesprpr,procbackgrnd,scaddset,scaddtype,spritetablea,sprleftpr
EXTSYM ForceNewGfxOff,bg1scrolx,bg1scroly,drawmode716b,mode7set,mosaicsz
EXTSYM sprleftpr1,sprleftpr2,sprleftpr3,sprlefttot,sprprifix,interlval,extbgdone
EXTSYM coladdb,coladdg,coladdr,pal16b,vesa2_bpos,V8Mode,doveg,pal16bcl,pal16bxcl
EXTSYM prevbright,prevpal,vesa2_clbit,vesa2_gpos,vesa2_rpos,vidbright,cgmod
EXTSYM cgram,gammalevel16b,dovegrest,winspdata,csprbit,csprprlft,sprclprio
EXTSYM sprsingle,sprpriodata,bgofwptr,bgsubby,bshifter,curmosaicsz,cwinptr
EXTSYM osm2dis,temp,tempcach,temptile,winptrref,xtravbuf,yadder,yrevadder
EXTSYM vcache2b,vcache4b,vcache8b,hirestiledat,res512switch,numwin,windowdata
EXTSYM bg1objptr,bg1ptr,bg3ptr,bg3scrolx,bg3scroly,vidmemch4,vram,ofsmcptr
EXTSYM ofsmady,ofsmadx,yposngom,flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd
EXTSYM bg1ptrx,bg1ptry,a16x16xinc,a16x16yinc,bg1scrolx_m7,bg1scroly_m7,ngptrdat2
EXTSYM OMBGTestVal,Testval,cachesingle4bng,m7starty,ofsmtptrs,ofsmcptr2
EXTSYM ofshvaladd

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
    inc ax
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
    call drawsprites16b
.nosprites
    ret

NEWSYM procspritesmain16b
    test byte[scrndis],10h
    jnz .nosprites
    test byte[scrnon],10h
    jz .nosprites
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
    call drawsprites16b
.nosprites
    ret

NEWSYM drawbackgrndsub16b
    mov esi,[colormodeofs]
    mov bl,[esi+ebp]
    cmp bl,0
    je near .noback
    mov al,[curbgnum]
    test byte[scrnon+1],al
    jz near .noback
    test byte[scrnon],al
    jnz near .noback
    test byte[alreadydrawn],al
    jnz near .noback
    test byte[scrndis],al
    jnz near .noback
    mov byte[winon],0
    test byte[winenabs],al
    jz near .nobackwin
;    procwindow [winbg1en+ebp]
    mov al,[winbg1en+ebp]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback
.nobackwin
    mov bl,[curbgnum]
    mov byte[curmosaicsz],1
    test byte[mosaicon],bl
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
.nomos
    mov byte[bgcoloradder],0
    cmp byte[bgmode],0
    jne .nomode0
    mov eax,ebp
    mov bl,20h
    mul bl
    mov [bgcoloradder],al
.nomode0
    mov esi,[bg1vbufloc+ebp*4]
    mov edi,[bg1tdatloc+ebp*4]
    mov edx,[bg1tdabloc+ebp*4]
    mov ebx,[bg1cachloc+ebp*4]
    mov eax,[bg1xposloc+ebp*4]
    mov cl,[curbgnum]
    test byte[bgtilesz],cl
    jnz .16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816b
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
    jmp .noback
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
.noback
    ret

NEWSYM drawbackgrndmain16b
    mov esi,[colormodeofs]
    mov bl,[esi+ebp]
    cmp bl,0
    je near .noback
    mov al,[curbgnum]
    test byte[scrnon],al
    jz near .noback
    test byte[alreadydrawn],al
    jnz near .noback
    test byte[scrndis],al
    jnz near .noback
    mov byte[winon],0
    test byte[winenabm],al
    jz near .nobackwin
;    procwindow [winbg1en+ebp]
    mov al,[winbg1en+ebp]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback
.nobackwin
    mov bl,[curbgnum]
    mov byte[curmosaicsz],1
    test byte[mosaicon],bl
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
.nomos
    mov byte[bgcoloradder],0
    cmp byte[bgmode],0
    jne .nomode0
    mov eax,ebp
    mov bl,20h
    mul bl
    mov [bgcoloradder],al
.nomode0
    mov esi,[bg1vbufloc+ebp*4]
    mov edi,[bg1tdatloc+ebp*4]
    mov edx,[bg1tdabloc+ebp*4]
    mov ebx,[bg1cachloc+ebp*4]
    mov eax,[bg1xposloc+ebp*4]
    mov cl,[curbgnum]
    test byte[bgtilesz],cl
    jnz .16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816b
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
    jmp .noback
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
.noback
    ret
NEWSYM blanker16b
    ; calculate current video offset
    push ebx
    push esi
    xor ebx,ebx
    mov bx,[curypos]
    mov esi,ebx
    shl esi,9
    shl ebx,6
    add esi,ebx
    add esi,32
    add esi,[vidbuffer]
    mov bl,128
.next
    mov dword[esi],0
    add esi,4
    dec bl
    jnz .next
    pop esi
    pop ebx
    ret

NEWSYM drawline16b
    cmp byte[ForceNewGfxOff],0
    jne .nonewgfx
    cmp byte[newengen],1
    je near newengine16b
.nonewgfx
    cmp byte[curblank],0
    jne near nodrawline16b
    mov al,[vidbright]
    cmp al,[maxbr]
    jbe .nochange
    mov [maxbr],al
.nochange
    cmp byte[forceblnk],0
    jne blanker16b
    mov byte[alreadydrawn],0
    push ebx
    xor ebx,ebx
    mov bl,[bgmode]
    shl bl,2
    add ebx,colormodedef
    mov [colormodeofs],ebx
    pop ebx

    cmp word[scrnon],1317h
    jne .noscrnona
    cmp byte[scaddtype],0
    jne .noscrnona
    mov word[scrnon],1317h
    mov byte[scaddtype],44h
    mov byte[scaddset],02h
.noscrnona
    cmp word[scrnon],0117h
    jne .notransph
    cmp word[scaddset],8202h
    jne .notransph
    mov word[scrnon],0116h
.notransph

    test byte[scaddset],02h
    jnz near drawline16t
    cmp dword[coladdr],0
    je .nocoladd
    test byte[scaddtype],3Fh
    jnz near drawline16t
.nocoladd
    cmp byte[bgmode],7
    je near processmode716b
    push esi
    push edi
    push ebx
    push edx
    push ebp
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
    call setpalette16b
    ; clear back area w/ back color
    call clearback16b
    ; clear registers
    xor eax,eax
    xor ecx,ecx
    ; get current sprite table
    xor ebx,ebx
    mov bl,[curypos]
    shl ebx,9
    add ebx,[spritetablea]
    mov [currentobjptr],ebx
    mov dword[cursprloc],sprleftpr
    ; setup priorities
    cmp byte[sprprifix],0
    je .nosprprio
    mov dword[cursprloc],sprlefttot
    call preparesprpr
.nosprprio
; process backgrounds
; do background 2
    mov byte[curbgnum],02h
    mov ebp,01h
    call procbackgrnd
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call procbackgrnd
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call procbackgrnd
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call procbackgrnd

    cmp byte[bgmode],1
    ja near priority216b
    mov al,[winenabm]
    mov [cwinenabm],al
    mov byte[curbgpr],0h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain16b
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16b
    mov ebp,0
    call procspritesmain16b
    mov byte[curbgpr],20h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain16b
; do background 3
    cmp byte[bg3high2],1
    je .bg3nothighb
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16b
.bg3nothighb
    mov ebp,1
    call procspritesmain16b
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16b
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16b
    mov ebp,2
    call procspritesmain16b
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16b
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16b
    mov ebp,3
    call procspritesmain16b
    cmp byte[bg3high2],1
    jne .bg3highb
; do background 3
    mov byte[curbgpr],20h
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16b
.bg3highb
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
NEWSYM nodrawline16b
    ret

NEWSYM priority216b
    mov al,[winenabm]
    mov [cwinenabm],al
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16b
    mov ebp,0
    call procspritesmain16b
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16b
    mov ebp,1
    call procspritesmain16b
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16b
    mov ebp,2
    call procspritesmain16b
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16b
    mov ebp,3
    call procspritesmain16b
    cmp byte[bg3high2],1
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
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
    call setpalette16b
    ; clear back area w/ back color
    call clearback16b
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
    call drawsprites16b
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
    call drawsprites16b
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
    call drawsprites16b
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
    call drawsprites16b
.nosprites4
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret

;*******************************************************
; Clear Backarea, 16-bit mode
;*******************************************************
NEWSYM clearback16b
    test byte[scaddtype],00100000b
    jz near .noaddition
    test byte[scaddtype],10000000b
    jnz near .noaddition
    mov dx,[cgram]
    mov ax,dx
    and ax,001Fh
    add al,[coladdr]
    cmp al,01Fh
    jb .noadd
    mov al,01Fh
.noadd
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov bx,ax
    mov ax,dx
    shr ax,5
    and ax,001Fh
    add al,[coladdg]
    cmp al,01Fh
    jb .noaddb
    mov al,01Fh
.noaddb
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and ax,001Fh
    add al,[coladdb]
    cmp al,01Fh
    jb .noaddc
    mov al,01Fh
.noaddc
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    mov ax,bx
    shl eax,16
    mov ax,bx
    mov edi,[curvidoffset]
    mov ecx,128
    rep stosd
    xor eax,eax
    ret
.noaddition
    mov edi,[curvidoffset]
    mov ax,[pal16b]
    shl eax,16
    mov ax,[pal16b]
    mov ecx,128
    rep stosd
    xor eax,eax
    ret

;*******************************************************
; Set palette 16bit
;*******************************************************
NEWSYM setpalall
    cmp byte[V8Mode],1
    jne .noveg
    call doveg
.noveg
    xor esi,esi
    mov byte[colleft16b],0
.loopa
    mov dx,[cgram+esi]
    mov [prevpal+esi],dx
    mov ax,dx
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    xor bx,bx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    cmp bx,0
    jne .col0
    cmp byte[vidbright],0
    je .col0
    or bx,0000000000100000b
.col0
    mov ax,bx
    mov [pal16b+esi*2],bx
    and bx,[vesa2_clbit]
    mov [pal16bcl+esi*2],bx
    xor ax,0FFFFh
    and ax,[vesa2_clbit]
    mov [pal16bxcl+esi*2],ax
    add esi,2
    inc byte[colleft16b]
    jnz near .loopa
    mov al,[vidbright]
    mov [prevbright],al
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret

SECTION .bss
NEWSYM colleft16b, resb 1
SECTION .text

NEWSYM setpalette16b
    cmp byte[gammalevel16b],0
    jne near setpalette16bgamma
    cmp byte[V8Mode],1
    jne .noveg
    call doveg
.noveg
    mov al,[vidbright]
    cmp al,[prevbright]
    jne near setpalall
    cmp byte[cgmod],0
    je near .skipall
    mov byte[cgmod],0
    xor esi,esi
    mov byte[colleft16b],0
.loopa
    mov dx,[cgram+esi]
    cmp [prevpal+esi],dx
    je near .skipa
    mov [prevpal+esi],dx
    mov ax,dx
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    xor bx,bx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    cmp bx,0
    jne .col0
    cmp byte[vidbright],0
    je .col0
    or bx,0000000000100000b
.col0
    mov [pal16b+esi*2],bx
    mov ax,bx
    and bx,[vesa2_clbit]
    mov [pal16bcl+esi*2],bx
    xor ax,0FFFFh
    and ax,[vesa2_clbit]
    mov [pal16bxcl+esi*2],ax
.skipa
    add esi,2
    inc byte[colleft16b]
    jnz near .loopa
.skipall
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret

NEWSYM setpalallgamma
    xor esi,esi
    mov byte[colleft16b],0
.loopa
    mov dx,[cgram+esi]
    mov [prevpal+esi],dx
    mov ax,dx
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norr
    mov al,31
.norr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    xor bx,bx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .nogr
    mov al,31
.nogr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .nobr
    mov al,31
.nobr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    cmp bx,0
    jne .col0
    cmp byte[vidbright],0
    je .col0
    or bx,0000000000100000b
.col0
    mov ax,bx
    mov [pal16b+esi*2],bx
    and bx,[vesa2_clbit]
    mov [pal16bcl+esi*2],bx
    xor ax,0FFFFh
    and ax,[vesa2_clbit]
    mov [pal16bxcl+esi*2],ax
    add esi,2
    inc byte[colleft16b]
    jnz near .loopa
    mov al,[vidbright]
    mov [prevbright],al
    ret

NEWSYM setpalette16bgamma
    mov al,[vidbright]
    cmp al,[prevbright]
    jne near setpalallgamma
    cmp byte[cgmod],0
    je near .skipall
    mov byte[cgmod],0
    xor esi,esi
    mov byte[colleft16b],0
.loopa
    mov dx,[cgram+esi]
    cmp [prevpal+esi],dx
    je near .skipa
    mov [prevpal+esi],dx
    mov ax,dx
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norb
    mov al,31
.norb
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    xor bx,bx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norr
    mov al,31
.norr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norg
    mov al,31
.norg
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    cmp bx,0
    jne .col0
    cmp byte[vidbright],0
    je .col0
    or bx,0000000000100000b
.col0
    mov [pal16b+esi*2],bx
    mov ax,bx
    and bx,[vesa2_clbit]
    mov [pal16bcl+esi*2],bx
    xor ax,0FFFFh
    and ax,[vesa2_clbit]
    mov [pal16bxcl+esi*2],ax
.skipa
    add esi,2
    inc byte[colleft16b]
    jnz near .loopa
.skipall
    ret

;*******************************************************
; Processes & Draws 4-bit sprites
;*******************************************************

NEWSYM drawsprites16b
    cmp byte[sprprifix],1
    je near drawsprites16bprio
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawsprites16bwinon
.drawnowin
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    xor ebx,ebx
    xor eax,eax
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    shl bx,1
    mov al,[esi]
    test al,0Fh
    jz .skipa
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-16],dx
.skipa
    mov al,[esi+1]
    test al,0Fh
    jz .skipb
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-14],dx
.skipb
    mov al,[esi+2]
    test al,0Fh
    jz .skipc
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-12],dx
.skipc
    mov al,[esi+3]
    test al,0Fh
    jz .skipd
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-10],dx
.skipd
    mov al,[esi+4]
    test al,0Fh
    jz .skipe
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-8],dx
.skipe
    mov al,[esi+5]
    test al,0Fh
    jz .skipf
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-6],dx
.skipf
    mov al,[esi+6]
    test al,0Fh
    jz .skipg
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-4],dx
.skipg
    mov al,[esi+7]
    test al,0Fh
    jz .skiph
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-2],dx
.skiph
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    shl bx,1
    mov al,[esi+7]
    test al,0Fh
    jz .skipa2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-16],dx
.skipa2
    mov al,[esi+6]
    test al,0Fh
    jz .skipb2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-14],dx
.skipb2
    mov al,[esi+5]
    test al,0Fh
    jz .skipc2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-12],dx
.skipc2
    mov al,[esi+4]
    test al,0Fh
    jz .skipd2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-10],dx
.skipd2
    mov al,[esi+3]
    test al,0Fh
    jz .skipe2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-8],dx
.skipe2
    mov al,[esi+2]
    test al,0Fh
    jz .skipf2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-6],dx
.skipf2
    mov al,[esi+1]
    test al,0Fh
    jz .skipg2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-4],dx
.skipg2
    mov al,[esi]
    test al,0Fh
    jz .skiph2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx-2],dx
.skiph2
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16bwinon
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    xor ebx,ebx
    xor eax,eax
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov al,[esi]
    test al,0Fh
    jz .skipa
    cmp byte[winspdata+ebx-8+16],0
    jne .skipa
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-16],dx
.skipa
    mov al,[esi+1]
    test al,0Fh
    jz .skipb
    cmp byte[winspdata+ebx-7+16],0
    jne .skipb
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-14],dx
.skipb
    mov al,[esi+2]
    test al,0Fh
    jz .skipc
    cmp byte[winspdata+ebx-6+16],0
    jne .skipc
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-12],dx
.skipc
    mov al,[esi+3]
    test al,0Fh
    jz .skipd
    cmp byte[winspdata+ebx-5+16],0
    jne .skipd
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-10],dx
.skipd
    mov al,[esi+4]
    test al,0Fh
    jz .skipe
    cmp byte[winspdata+ebx-4+16],0
    jne .skipe
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-8],dx
.skipe
    mov al,[esi+5]
    test al,0Fh
    jz .skipf
    cmp byte[winspdata+ebx-3+16],0
    jne .skipf
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-6],dx
.skipf
    mov al,[esi+6]
    test al,0Fh
    jz .skipg
    cmp byte[winspdata+ebx-2+16],0
    jne .skipg
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-4],dx
.skipg
    mov al,[esi+7]
    test al,0Fh
    jz .skiph
    cmp byte[winspdata+ebx-1+16],0
    jne .skiph
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-2],dx
.skiph
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov al,[esi+7]
    test al,0Fh
    jz .skipa2
    cmp byte[winspdata+ebx-8+16],0
    jne .skipa2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-16],dx
.skipa2
    mov al,[esi+6]
    test al,0Fh
    jz .skipb2
    cmp byte[winspdata+ebx-7+16],0
    jne .skipb2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-14],dx
.skipb2
    mov al,[esi+5]
    test al,0Fh
    jz .skipc2
    cmp byte[winspdata+ebx-6+16],0
    jne .skipc2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-12],dx
.skipc2
    mov al,[esi+4]
    test al,0Fh
    jz .skipd2
    cmp byte[winspdata+ebx-5+16],0
    jne .skipd2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-10],dx
.skipd2
    mov al,[esi+3]
    test al,0Fh
    jz .skipe2
    cmp byte[winspdata+ebx-4+16],0
    jne .skipe2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-8],dx
.skipe2
    mov al,[esi+2]
    test al,0Fh
    jz .skipf2
    cmp byte[winspdata+ebx-3+16],0
    jne .skipf2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-6],dx
.skipf2
    mov al,[esi+1]
    test al,0Fh
    jz .skipg2
    cmp byte[winspdata+ebx-2+16],0
    jne .skipg2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-4],dx
.skipg2
    mov al,[esi]
    test al,0Fh
    jz .skiph2
    cmp byte[winspdata+ebx-1+16],0
    jne .skiph2
    add al,ch
    mov edx,[pal16b+eax*4]
    mov [edi+ebx*2-2],dx
.skiph2
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16bprio
    cmp byte[sprclprio+ebp],0
    je near .endobj
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawspritesprio16bwinon
.drawnowin
    cmp dword[sprsingle],1
    je near .drawsingle
    mov [csprprlft],cl
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov dl,[esi+7]
    xor eax,eax
    and edx,03h
    cmp edx,ebp
    jne near .notprio
    mov esi,[esi+2]
    mov cl,[csprbit]
    sprdrawa16b sprdrawpra16b
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.drawspriteflipx
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov dl,[esi+7]
    xor eax,eax
    and edx,03h
    cmp edx,ebp
    jne near .notpriof
    mov esi,[esi+2]
    mov cl,[csprbit]
    sprdrawaf16b sprdrawpra16b
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    ret
.drawsingle
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,ecx
    and edx,0FFh
    xor eax,eax
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa16b sprdrawprb16b
    pop esi
    sub esi,8
    dec cl
    jnz near .loopobj2
    ret
.drawspriteflipx2
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf16b sprdrawprb16b
    pop esi
    sub esi,8
    dec cl
    jnz near .loopobj2
    ret

NEWSYM drawspritesprio16bwinon
    cmp dword[sprsingle],1
    je near .drawsingle
    mov [csprprlft],cl
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov dl,[esi+7]
    xor eax,eax
    and edx,03h
    cmp edx,ebp
    jne near .notprio
    mov esi,[esi+2]
    mov cl,[csprbit]
    sprdrawa16b sprdrawpraw16b
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.drawspriteflipx
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov dl,[esi+7]
    xor eax,eax
    and edx,03h
    cmp edx,ebp
    jne near .notpriof
    mov esi,[esi+2]
    mov cl,[csprbit]
    sprdrawaf16b sprdrawpraw16b
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop esi
    add esi,8
    dec byte[csprprlft]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    ret
.drawsingle
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,ecx
    and edx,0FFh
    xor eax,eax
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa16b sprdrawprbw16b
    pop esi
    sub esi,8
    dec cl
    jnz near .loopobj2
    ret
.drawspriteflipx2
    mov bx,[esi]
    push esi
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf16b sprdrawprbw16b
    pop esi
    sub esi,8
    dec cl
    jnz near .loopobj2
    ret


NEWSYM draw8x816b
    cmp byte[osm2dis],1
    je .osm2dis
    cmp byte[bgmode],2
    je near draw8x816boffset
.osm2dis
    cmp byte[bgmode],5
    je near draw16x816
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
    jne near draw8x816bwinon
.domosaic
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
    cmp dword[ebx],0
    je .loopd4
    Draw8x816bmacro 0
    Draw8x816bmacro 1
    Draw8x816bmacro 2
    Draw8x816bmacro 3
.loopd4
    cmp dword[ebx+4],0
    je .loopd8
    Draw8x816bmacro 4
    Draw8x816bmacro 5
    Draw8x816bmacro 6
    Draw8x816bmacro 7
.loopd8
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
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    cmp dword[ebx+4],0
    je .loopd4b
    Draw8x816bflipmacro 0
    Draw8x816bflipmacro 1
    Draw8x816bflipmacro 2
    Draw8x816bflipmacro 3
.loopd4b
    cmp dword[ebx],0
    je .loopd8b
    Draw8x816bflipmacro 4
    Draw8x816bflipmacro 5
    Draw8x816bflipmacro 6
    Draw8x816bflipmacro 7
.loopd8b
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

;*******************************************************
; Processes & Draws 8x8 tiles, offset mode
;*******************************************************

NEWSYM draw8x816boffset
    mov [temp],al
    mov [bshifter],ah
    mov eax,esi
    mov [yadder],ecx
    mov [tempcach],ebx
    mov ebx,56
    sub ebx,ecx
    mov [yrevadder],ebx
    initoffsetmode
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
    jne near draw8x816bwinonoffset
.domosaic
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
    Draw8x816bmacro 0
    Draw8x816bmacro 1
    Draw8x816bmacro 2
    Draw8x816bmacro 3
.loopd4
    cmp dword[ebx+4],0
    je .loopd8
    Draw8x816bmacro 4
    Draw8x816bmacro 5
    Draw8x816bmacro 6
    Draw8x816bmacro 7
.loopd8
.hprior
    procoffsetmode
    add esi,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
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
    Draw8x816bflipmacro 0
    Draw8x816bflipmacro 1
    Draw8x816bflipmacro 2
    Draw8x816bflipmacro 3
.loopd4b
    cmp dword[ebx],0
    je .loopd8b
    Draw8x816bflipmacro 4
    Draw8x816bflipmacro 5
    Draw8x816bflipmacro 6
    Draw8x816bflipmacro 7
.loopd8b
    procoffsetmode
    add esi,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
.nodraw2
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

NEWSYM draw16x1616b
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
    mov word[.yadd],16
    mov word[.yflipadd],0
    jmp .yesincrc
.noincrc
    mov word[.yadd],0
    mov word[.yflipadd],16
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
    jne near draw16x1616bwinon
.domosaic
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
    add ax,word[.yadd]
    jmp .yesyflip
.noyflip
    add ax,word[.yflipadd]
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

    ; Start loop
    Draw16x1616b 0, 0
    Draw16x1616b 1, 2
    Draw16x1616b 2, 4
    Draw16x1616b 3, 6
    Draw16x1616b 4, 8
    Draw16x1616b 5, 10
    Draw16x1616b 6, 12
    Draw16x1616b 7, 14
.hprior
    add esi,16
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
    Draw16x1616b 7, 0
    Draw16x1616b 6, 2
    Draw16x1616b 5, 4
    Draw16x1616b 4, 6
    Draw16x1616b 3, 8
    Draw16x1616b 2, 10
    Draw16x1616b 1, 12
    Draw16x1616b 0, 14
.skiploop2b
    add esi,16
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

SECTION .bss
.yadd      resw 1
.yflipadd  resw 1
SECTION .text

draw16x1616bwinon:
    mov byte[tileleft16b],33
    mov dl,[temp]
    mov ebp,[winptrref]
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
    add ax,word[draw16x1616b.yadd]
    jmp .yesyflip
.noyflip
    add ax,word[draw16x1616b.yflipadd]
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

    ; Start loop
    Draw16x1616bwin 0, 0, 0
    Draw16x1616bwin 1, 2, 1
    Draw16x1616bwin 2, 4, 2
    Draw16x1616bwin 3, 6, 3
    Draw16x1616bwin 4, 8, 4
    Draw16x1616bwin 5, 10, 5
    Draw16x1616bwin 6, 12, 6
    Draw16x1616bwin 7, 14, 7
.hprior
    add esi,16
    add ebp,8
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
    Draw16x1616bwin 7, 0, 0
    Draw16x1616bwin 6, 2, 1
    Draw16x1616bwin 5, 4, 2
    Draw16x1616bwin 4, 6, 3
    Draw16x1616bwin 3, 8, 4
    Draw16x1616bwin 2, 10, 5
    Draw16x1616bwin 1, 12, 6
    Draw16x1616bwin 0, 14, 7
.skiploop2b
    add esi,16
    add ebp,8
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
