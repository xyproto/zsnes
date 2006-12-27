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

EXTSYM cwinptr,dualstartprocess,dualwinbg,dualwinsp,dwinptrproc,pwinbgenab
EXTSYM pwinbgtype,pwinspenab,pwinsptype,winbgdata,winlogicb,winonbtype
EXTSYM winonstype,winspdata,interlval,MMXSupport,bg1scrolx,bg1scroly,curmosaicsz
EXTSYM curypos,drawmode716t,makewindow,mode7set,mosaicon,mosaicsz,scrnon
EXTSYM winbg1en,winenabm,drawmode716textbg,drawmode716textbg2,extbgdone
EXTSYM drawmode716tb,drawmode716b,drawmode716extbg,drawmode716extbg2,cursprloc
EXTSYM drawsprites16b,scrndis,sprprifix,winonsp,bgfixer,bgfixer2,scaddtype
EXTSYM alreadydrawn,bg1cachloc,bg1tdabloc,bg1tdatloc,bg1vbufloc,bg1xposloc
EXTSYM bg1yaddval,bgcoloradder,bgmode,bgtilesz,colormodeofs,curbgnum
EXTSYM draw16x1616b,draw8x816b,drawn,winenabs,curbgpr,draw16x1616tms,ngptrdat2
EXTSYM draw8x816tms,bg3high2,currentobjptr,curvidoffset,cwinenabm,makewindowsp
EXTSYM preparesprpr,procbackgrnd,setpalette16b,spritetablea,sprleftpr,sprlefttot
EXTSYM numwin,scaddset,wincolen,windowdata,winl1,winl2,winon,winr1,winr2
EXTSYM vidbuffer,coladdb,coladdg,coladdr,vesa2_bpos,vesa2_gpos,vesa2_rpos
EXTSYM vidbright,winptrref,fulladdtab,pal16b,vesa2_clbit,csprbit,sprclprio
EXTSYM csprprlft,sprsingle,sprpriodata,pal16bcl,pal16bxcl,bgofwptr,bgsubby
EXTSYM bshifter,domosaic16b,temp,tempcach,temptile,tileleft16b,xtravbuf,yadder
EXTSYM yrevadder,vcache2b,vcache4b,vcache8b,draw8x816boffset,osm2dis
EXTSYM hirestiledat,res512switch,bg1objptr,bg1ptr,bg3ptr,bg3scrolx,bg3scroly
EXTSYM vidmemch4,vram,ofsmcptr,ofsmady,ofsmadx,yposngom,flipyposngom,ofsmtptr
EXTSYM ofsmmptr,ofsmcyps,bgtxadd,bg1ptrx,bg1ptry,a16x16xinc,a16x16yinc
EXTSYM bg1scrolx_m7,bg1scroly_m7,OMBGTestVal,Testval,cachesingle4bng,m7starty
EXTSYM ofsmtptrs,ofsmcptr2,ofshvaladd

%include "video/vidmacro.mac"

; clearback16bts clearback16bdual

;*******************************************************
; DrawLine 16bit Transparent      Draws the current line
;*******************************************************
; use curypos+bg1scroly for y location and bg1scrolx for x location
; use bg1ptr(b,c,d) for the pointer to the tile number contents
; use bg1objptr for the pointer to the object tile contents

SECTION .text

NEWSYM makedualwincol
    mov dl,[winlogicb]
    shr dl,2
    and dl,03h
    mov cl,dl
    mov byte[winon],1
    mov ebx,[winl1]
    ; check if data matches previous sprite data
    cmp cl,[dualwinsp]
    jne .skipsprcheck
    cmp al,[pwinspenab]
    jne .skipsprcheck
    cmp ebx,[pwinsptype]
    jne .skipsprcheck
    mov dword[cwinptr],winspdata+16
    mov al,[winonstype]
    mov [winon],al
    ret
.skipsprcheck
    ; check if data matches previous data
    cmp cl,[dualwinbg]
    jne .skipenab3
    cmp al,[pwinbgenab]
    jne .skipenab
    cmp ebx,[pwinbgtype]
    jne .skipenab2
    mov dword[cwinptr],winbgdata+16
    mov al,[winonbtype]
    mov [winon],al
    ret
.skipenab3
    mov [dualwinbg],cl
.skipenab
    mov [pwinbgenab],al
    mov ebx,[winl1]
.skipenab2
    mov [pwinbgtype],ebx
    mov dword[dwinptrproc],winbgdata+16
    mov dword[cwinptr],winbgdata+16
    mov byte[winon],1
    mov byte[winonbtype],1
    jmp dualstartprocess

NEWSYM procmode716tsub
    test word[scrnon+1],01h
    jz near .noback1
    test word[scrnon],01h
    jnz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    call drawmode716t
.noback1
    ret



NEWSYM procmode716tsubextbg
    test word[scrnon+1],02h
    jz near .noback1
    test word[scrnon],02h
    jnz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    mov byte[extbgdone],1
    call drawmode716extbg
.noback1
    ret

NEWSYM procmode716tsubextbgb
    cmp byte[extbgdone],0
    jne near .noback1
    test word[scrnon+1],01h
    jz near .noback1
    test word[scrnon],01h
    jnz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    mov byte[extbgdone],1
    call drawmode716textbg
.noback1
    ret

NEWSYM procmode716tsubextbg2
    cmp byte[extbgdone],0
    je near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    call drawmode716extbg2
.noback1
    ret

NEWSYM procmode716tmain
    test word[scrnon],01h
    jz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    test byte[scaddset],02h
    jz .noscrnadd
    test word[scrnon+1],01h
    jnz near .mode7b
.noscrnadd
    test byte[scaddtype],01h
    jz .notransp
    call drawmode716t
.noback1
    ret
.notransp
    call drawmode716b
    ret
.mode7b
    call drawmode716tb
    ret

NEWSYM procmode716tmainextbg
    test word[scrnon],02h
    jz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    test byte[scaddtype],01h
    jz .notransp
    mov byte[extbgdone],1
    call drawmode716textbg
.noback1
    ret
.notransp
    mov byte[extbgdone],1
    call drawmode716extbg
    ret

NEWSYM procmode716tmainextbgb
    cmp byte[extbgdone],0
    jne near .noback1
    test word[scrnon],01h
    jz near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    test byte[scaddtype],01h
    jz .notransp
    mov byte[extbgdone],1
    call drawmode716textbg
.noback1
    ret
.notransp
    mov byte[extbgdone],1
    call drawmode716extbg
    ret

NEWSYM procmode716tmainextbg2
    cmp byte[extbgdone],0
    je near .noback1
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin1
    test word[winenabm],0100h
    jnz near .nobackwin1
;    procwindow [winbg1en]
    mov al,[winbg1en]
    mov ebp,0
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    xor eax,eax
    xor edx,edx
    mov ax,[curypos]
    inc ax
    test byte[mode7set],02h
    jz .noflip
    neg ax
    add ax,255
.noflip
    mov byte[curmosaicsz],1
    test byte[mosaicon],1
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
    inc bl
    mov [curmosaicsz],bl
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
.nomos
    mov [m7starty],ax
    mov ax,[bg1scroly_m7]
    mov dx,[bg1scrolx_m7]
    test byte[scaddtype],01h
    jz .notransp
    call drawmode716textbg2
.noback1
    ret
.notransp
    call drawmode716extbg2
    ret

NEWSYM procspritessub16t
    cmp byte[bgfixer],1
    je near procspritessub16tfix
    cmp byte[bgfixer2],1
    je near procspritessub16tfix
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

NEWSYM procspritesmain16t
    cmp byte[bgfixer],1
    je near procspritesmain16tfix
    cmp byte[bgfixer2],1
    je near procspritesmain16tfix
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
    test byte[scrnon+1],10h
    jnz .spritesubmain
    test byte[scaddtype],10h
    jz .nospriteadd
    call drawsprites16t
.nosprites
    ret
.nospriteadd
    call drawsprites16b
    xor eax,eax
    ret
.spritesubmain
    call drawsprites16bt
    ret

NEWSYM drawbackgrndsub16t
    cmp byte[bgfixer],1
    je near drawbackgrndsub16tfix
    cmp byte[bgfixer2],1
    je near drawbackgrndsub16tfix
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
.noback
    ret
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
    ret

NEWSYM drawbackgrndmain16t
    cmp byte[bgfixer],1
    je near drawbackgrndmain16tfix
    cmp byte[bgfixer2],1
    je near drawbackgrndmain16tfix
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
    shl eax,5
    mov [bgcoloradder],al
.nomode0
    mov esi,[bg1vbufloc+ebp*4]
    mov edi,[bg1tdatloc+ebp*4]
    mov edx,[bg1tdabloc+ebp*4]
    mov ebx,[bg1cachloc+ebp*4]
    mov eax,[bg1xposloc+ebp*4]
    mov cl,[curbgnum]
    cmp byte[curbgpr],0h
    jne .test2
;    test byte[scaddtype],cl
;    jnz .transp
.test2
    test byte[scaddset],02h
    jz .noscrnadd
    test byte[scrnon+1],cl
    jnz near .mainandsub
.noscrnadd
    test byte[scaddtype],cl
    jnz .transp
    test byte[bgtilesz],cl
    jnz .16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816b
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
.noback
    ret
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
    ret
.transp
    test byte[bgtilesz],cl
    jnz .16x16b
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816t
    cmp byte[drawn],33
    jne .notalldrawnc
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnc
    ret
.16x16b
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616t
    cmp byte[drawn],33
    jne .notalldrawnd
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnd
    ret
.mainandsub
    test byte[curbgpr],20h
    jnz .firstpr
    test byte[scaddtype],cl
    jnz .transpb
.firstpr
    test byte[bgtilesz],cl
    jnz .16x16c
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816bt
    cmp byte[drawn],33
    jne .notalldrawne
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawne
    ret
.16x16c
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616bt
    cmp byte[drawn],33
    jne .notalldrawnf
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnf
    ret
.transpb
    test byte[bgtilesz],cl
    jnz .16x16d
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816tms
    cmp byte[drawn],33
    jne .notalldrawng
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawng
    ret
.16x16d
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616tms
    cmp byte[drawn],33
    jne .notalldrawnh
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnh
    ret

NEWSYM procspritessub16tfix
    test byte[scrndis],10h
    jnz .nosprites
    test byte[scrnon+1],10h
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

NEWSYM procspritesmain16tfix
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
    test byte[scaddtype],10h
    jz .nospriteadd
    call drawsprites16t
.nosprites
    ret
.nospriteadd
    call drawsprites16b
    xor eax,eax
    ret

NEWSYM drawbackgrndsub16tfix
    mov esi,[colormodeofs]
    mov bl,[esi+ebp]
    cmp bl,0
    je near .noback
    mov al,[curbgnum]
    test byte[scrnon+1],al
    jz near .noback
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
.noback
    ret
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
    ret

NEWSYM drawbackgrndmain16tfix
    mov esi,[colormodeofs]
    mov bl,[esi+ebp]
    cmp bl,0
    je near .noback
    mov al,[curbgnum]
    test byte[scrnon],al
    jz near .noback
    test byte[alreadydrawn],al
;    jnz near .noback
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
    shl eax,5
    mov [bgcoloradder],al
.nomode0
    mov esi,[bg1vbufloc+ebp*4]
    mov edi,[bg1tdatloc+ebp*4]
    mov edx,[bg1tdabloc+ebp*4]
    mov ebx,[bg1cachloc+ebp*4]
    mov eax,[bg1xposloc+ebp*4]
    mov cl,[curbgnum]
    cmp byte[curbgpr],0h
    jne .test2
;    test byte[scaddtype],cl
;    jnz .transp
.test2
    test byte[scaddtype],cl
    jnz .transp
    test byte[bgtilesz],cl
    jnz .16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816b
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
.noback
    ret
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616b
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
    ret
.transp
    test byte[bgtilesz],cl
    jnz .16x16b
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x816t
    cmp byte[drawn],33
    jne .notalldrawnc
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnc
    ret
.16x16b
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x1616t
    cmp byte[drawn],33
    jne .notalldrawnd
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnd
    ret

ALIGN32

SECTION .bss
NEWSYM transpbuf, resb 576+16+288*2        ; Transparent buffer
SECTION .text

NEWSYM drawline16t
    cmp byte[bgmode],7
    je near processmode716t
    mov al,[scrnon]
    test [scrnon+1],al
    jz .nomainsub
    test byte[scrnon],10h
    jnz .nomainsub
    test byte[scrnon+1],10h
    jz .nomainsub
    mov al,[scrnon+1]
    xor al,0FFh
    and [scrnon],al
.nomainsub
    push esi
    push edi
    push ebx
    push edx
    push ebp
    ; current video offset
    mov dword[curvidoffset],transpbuf+32
    ; set palette
    call setpalette16b
    ; clear back area w/ back color
    procwindowback
    call clearback16bts
    ; do sprite windowing
    call makewindowsp
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
    ja near priority216t
    test byte[scaddset],02h
    jz near .noscrnadd
; draw backgrounds
    mov byte[curbgpr],0h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndsub16t
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub16t
    mov ebp,0
    call procspritessub16t
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndsub16t
; do background 3
    mov byte[curbgpr],20h
    cmp byte[bg3high2],1
    je .bg3nothigh
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub16t
.bg3nothigh
    mov ebp,1
    call procspritessub16t
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub16t
    mov ebp,2
    call procspritessub16t
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub16t
    mov ebp,3
    call procspritessub16t
; do background 3
    cmp byte[bg3high2],1
    jne .bg3high
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub16t
.bg3high
.noscrnadd
    mov al,[winenabm]
    mov [cwinenabm],al

NEWSYM NextDrawLine16bt
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
    ; clear back area w/ back color
    call clearback16t
    mov byte[curbgpr],0h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain16t
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16t
    mov ebp,0
    call procspritesmain16t
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain16t
; do background 3
    mov byte[curbgpr],20h
    cmp byte[bg3high2],1
    je .bg3nothighb
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16t
.bg3nothighb
    mov ebp,1
    call procspritesmain16t
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16t
    mov ebp,2
    call procspritesmain16t
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16t
    mov ebp,3
    call procspritesmain16t
    cmp byte[bg3high2],1
    jne .bg3highb
; do background 3
    mov byte[curbgpr],20h
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain16t
.bg3highb
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret

NEWSYM priority216t
    test byte[scaddset],02h
    jz near .noscrnadd
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub16t
    mov ebp,0
    call procspritessub16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub16t
    mov ebp,1
    call procspritessub16t
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub16t
    mov ebp,2
    call procspritessub16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub16t
    mov ebp,3
    call procspritessub16t
.noscrnadd
    mov al,[winenabm]
    mov [cwinenabm],al
NEWSYM Priority2NextDrawLine16bt
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
    ; clear back area w/ back color
    call clearback16t
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16t
    mov ebp,0
    call procspritesmain16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16t
    mov ebp,1
    call procspritesmain16t
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain16t
    mov ebp,2
    call procspritesmain16t
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain16t
    mov ebp,3
    call procspritesmain16t
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret

NEWSYM processmode716t
    push esi
    push edi
    push ebx
    push edx
    push ebp
    ; current video offset
    mov dword[curvidoffset],transpbuf+32
    ; set palette
    call setpalette16b
    ; clear back area w/ back color
    procwindowback
    call clearback16bts
    ; do sprite windowing
    call makewindowsp
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
    mov byte[extbgdone],0
    test byte[scaddset],02h
    jz .nosubscr
    test byte[interlval],40h
    jz .noback0s
    call procmode716tsubextbg
.noback0s
    mov ebp,0
    call procspritessub16t
    test byte[interlval],40h
    jnz .noback1s
    call procmode716tsub
.noback1s
    mov ebp,1
    call procspritessub16t
    test byte[interlval],40h
    jz .noback2s
    call procmode716tsubextbgb
    call procmode716tsubextbg2
.noback2s
    mov ebp,2
    call procspritessub16t
    mov ebp,3
    call procspritessub16t
.nosubscr
    mov al,[winenabm]
    mov [cwinenabm],al
NEWSYM processmode716t2
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
    ; clear back area w/ back color
    call clearback16t
    ; clear registers
    xor eax,eax
    xor ecx,ecx
    mov byte[extbgdone],0
    test byte[interlval],40h
    jz .noback0m
    call procmode716tmainextbg
.noback0m
    mov ebp,0
    call procspritesmain16t
    ; do background 1
    test byte[interlval],40h
    jnz .noback1m
    call procmode716tmain
.noback1m
    mov ebp,1
    call procspritesmain16t
    test byte[interlval],40h
    jz .noback2m
    call procmode716tmainextbgb
    call procmode716tmainextbg2
.noback2m
    mov ebp,2
    call procspritesmain16t
    mov ebp,3
    call procspritesmain16t
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret

;*******************************************************
; Clear Backarea, with 0s
;*******************************************************

SECTION .bss
NEWSYM prevrgbcol, resd 1
NEWSYM prevrgbpal, resd 1
SECTION .text

NEWSYM clearback16bts
    mov byte[DoTransp],0
    cmp dword[vesa2_rpos],0
    je near clearback16bts0.clear
    cmp byte[winon],0
    je near clearback16bts0b
    cmp byte[winon],2
    je near clearback16bts0
    cmp byte[winon],4
    je near clearback16bts0.clear
.noclear
    mov bl,[scaddset]
    and bl,30h
    cmp bl,20h
    jne .dontclear
    cmp byte[winon],5
    je near clearback16bts0.clear
.dontclear
    cmp byte[winon],5
    je near clearback16bts0b
    cmp byte[winon],3
    je near clearback16bts0b
    mov eax,[coladdr]
    shl eax,8
    mov bx,[prevrgbpal]
    mov al,[vidbright]
    cmp eax,[prevrgbcol]
    je .useprevpal
    mov [prevrgbcol],eax
    xor eax,eax
    mov al,[coladdr]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov bx,ax
    xor eax,eax
    mov al,[coladdg]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    xor eax,eax
    mov al,[coladdb]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    mov [prevrgbpal],bx
.useprevpal
    mov ax,bx
    cmp eax,0
    je .nowin
    jmp dowindowback16b
.nowin
    mov edi,[curvidoffset]
    or eax,eax
    jz near clearback16bts0.clearing
    cmp byte[MMXSupport],1
    je .dommxclear
    mov ecx,128
    rep stosd
    xor eax,eax
    ret
.dommxclear
    mov [mmxtempdat],eax
    mov [mmxtempdat+4],eax
    mov ecx,32
    movq mm0,[mmxtempdat]
.mmxloop
    movq [edi],mm0
    movq [edi+8],mm0
    add edi,16
    dec ecx
    jnz .mmxloop
    emms
    xor eax,eax
    ret

SECTION .bss
mmxtempdat resd 2
SECTION .text

NEWSYM clearback16bts0b
    mov eax,[coladdr]
    shl eax,8
    mov bx,[prevrgbpal]
    mov al,[vidbright]
    cmp eax,[prevrgbcol]
    je .useprevpal2
    mov [prevrgbcol],eax
    xor eax,eax
    mov al,[coladdr]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov bx,ax
    xor eax,eax
    mov al,[coladdg]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_gpos]
    shl ax,cl
    add bx,ax
    xor eax,eax
    mov al,[coladdb]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[vesa2_bpos]
    shl ax,cl
    add bx,ax
    mov [prevrgbpal],bx
.useprevpal2
    mov ax,bx
    shl eax,16
    mov ax,bx
    cmp byte[winon],3
    je near clearback16bdual
    mov edi,[curvidoffset]
    or eax,eax
    jz near clearback16bts0.clearing
    cmp byte[MMXSupport],1
    je .dommxclear
    mov ecx,128
    rep stosd
    xor eax,eax
    ret
.dommxclear
    mov [mmxtempdat],eax
    mov [mmxtempdat+4],eax
    mov ecx,32
    movq mm0,[mmxtempdat]
.mmxloop
    movq [edi],mm0
    movq [edi+8],mm0
    add edi,16
    dec ecx
    jnz .mmxloop
    emms
    xor eax,eax
    ret

NEWSYM clearback16bts0
    mov bl,[scaddset]
    and bl,30h
    cmp bl,20h
    je near clearback16bts0b
.clear
    mov edi,[curvidoffset]
    xor eax,eax
.clearing
    test byte[scrnon+1],10h
    jnz .notnotransp
    mov byte[DoTransp],1
.notnotransp
    cmp byte[MMXSupport],1
    je .dommxclear
    mov ecx,128
    rep stosd
    ret
.dommxclear
    mov [mmxtempdat],eax
    mov [mmxtempdat+4],eax
    mov ecx,32
    movq mm0,[mmxtempdat]
.mmxloop
    movq [edi],mm0
    movq [edi+8],mm0
    add edi,16
    dec ecx
    jnz .mmxloop
    emms
    ret

NEWSYM dowindowback16b
    test byte[scrnon+1],10h
    jnz .notnotransp
    mov byte[DoTransp],1
.notnotransp
    mov bl,[scaddset]
    and bl,30h
    cmp bl,20h
    je near dowindowback16brev
    mov ebx,windowdata
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
    mov byte[DoTransp],0
.loopa
    mov [edi+edx*2],ax
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
.loopb
    mov word[edi+edx*2],0
    inc dl
    dec cl
    jnz .loopb
    jmp .procnext
.finishwin
    xor eax,eax
    ret

NEWSYM dowindowback16brev
    mov ebx,windowdata
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
    mov word[edi+edx*2],0
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
    mov byte[DoTransp],0
.loopb
    mov [edi+edx*2],ax
    inc dl
    dec cl
    jnz .loopb
    jmp .procnext
.finishwin
    xor eax,eax
    ret

NEWSYM clearback16bdual
    cmp byte[bgmode],7
    jne .notmode7
    test byte[scrnon+1],10h
    jnz .notmode7
    jmp clearback16bdualb2
.notmode7

    mov bl,[scaddset]
    and bl,30h
    cmp bl,10h
    je near clearback16bdualrev

    mov edi,[curvidoffset]
    mov esi,[cwinptr]
    mov ecx,64
    cmp dword[esi],01010101h
    je near .drawnone
.nextpart
    cmp dword[esi],0
    jne .drawpart
.drawall
    mov [edi],eax
    mov [edi+4],eax
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpart
    jmp .findraw
.nextpartp
    cmp dword[esi],01010101h
    je near .drawnone
    cmp dword[esi],0h
    je .drawall
.drawpart
    xor ebx,ebx
    cmp byte[esi],1
    je .nodraw
    mov ebx,eax
.nodraw
    mov [edi],bx
    xor ebx,ebx
    cmp byte[esi+1],1
    je .nodraw1
    mov ebx,eax
.nodraw1
    mov [edi+2],bx
    xor ebx,ebx
    cmp byte[esi+2],1
    je .nodraw2
    mov ebx,eax
.nodraw2
    mov [edi+4],bx
    xor ebx,ebx
    cmp byte[esi+3],1
    je .nodraw3
    mov ebx,eax
.nodraw3
    mov [edi+6],bx
    add edi,8
    add esi,4
    dec ecx
    jnz near .nextpartp
    jmp .findraw
.nextpartn
    cmp dword[esi],01010101h
    jne .drawpart
.drawnone
    mov dword[edi],0
    mov dword[edi+4],0
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpartn
.findraw
    ret

NEWSYM clearback16bdualrev

;    xor eax,eax                 ;aaaaaaaaaaa
;    ret

    mov edi,[curvidoffset]
    mov esi,[winptrref]
    mov esi,[cwinptr]
    mov ecx,64
    cmp dword[esi],0
    je near .drawnone
.nextpart
    cmp dword[esi],01010101h
    jne .drawpart
.drawall
    mov [edi],eax
    mov [edi+4],eax
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpart
    jmp .findraw
.nextpartp
    cmp dword[esi],0
    je near .drawnone
    cmp dword[esi],01010101h
    je .drawall
.drawpart
    xor ebx,ebx
    cmp byte[esi],1
    jne .nodraw
    mov ebx,eax
.nodraw
    mov [edi],bx
    xor ebx,ebx
    cmp byte[esi+1],1
    jne .nodraw1
    mov ebx,eax
.nodraw1
    mov [edi+2],bx
    xor ebx,ebx
    cmp byte[esi+2],1
    jne .nodraw2
    mov ebx,eax
.nodraw2
    mov [edi+4],bx
    xor ebx,ebx
    cmp byte[esi+3],1
    jne .nodraw3
    mov ebx,eax
.nodraw3
    mov [edi+6],bx
    add edi,8
    add esi,4
    dec ecx
    jnz near .nextpartp
    jmp .findraw
.nextpartn
    cmp dword[esi],0
    jne .drawpart
.drawnone
    mov dword[edi],0
    mov dword[edi+4],0
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpartn
.findraw
    ret

NEWSYM clearback16bdualb2
    mov byte[DoTransp],1
    mov bl,[scaddset]
    and bl,30h
    cmp bl,10h
    je near clearback16bdualrev2

    mov edi,[curvidoffset]
    mov esi,[cwinptr]
    mov ecx,64
    cmp dword[esi],01010101h
    je near .drawnone
    mov byte[DoTransp],0
.nextpart
    cmp dword[esi],0
    jne .drawpart
.drawall
    mov [edi],eax
    mov [edi+4],eax
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpart
    jmp .findraw
.nextpartp
    cmp dword[esi],01010101h
    je near .drawnone
    cmp dword[esi],0h
    je .drawall
.drawpart
    mov byte[DoTransp],0
    xor ebx,ebx
    cmp byte[esi],1
    je .nodraw
    mov ebx,eax
.nodraw
    mov [edi],bx
    xor ebx,ebx
    cmp byte[esi+1],1
    je .nodraw1
    mov ebx,eax
.nodraw1
    mov [edi+2],bx
    xor ebx,ebx
    cmp byte[esi+2],1
    je .nodraw2
    mov ebx,eax
.nodraw2
    mov [edi+4],bx
    xor ebx,ebx
    cmp byte[esi+3],1
    je .nodraw3
    mov ebx,eax
.nodraw3
    mov [edi+6],bx
    add edi,8
    add esi,4
    dec ecx
    jnz near .nextpartp
    jmp .findraw
.nextpartn
    cmp dword[esi],01010101h
    jne .drawpart
.drawnone
    mov dword[edi],0
    mov dword[edi+4],0
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpartn
.findraw
    ret

NEWSYM clearback16bdualrev2
    mov edi,[curvidoffset]
    mov esi,[winptrref]
    mov esi,[cwinptr]
    mov ecx,64
    cmp dword[esi],0
    je near .drawnone
    mov byte[DoTransp],0
.nextpart
    cmp dword[esi],01010101h
    jne .drawpart
.drawall
    mov [edi],eax
    mov [edi+4],eax
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpart
    jmp .findraw
.nextpartp
    cmp dword[esi],0
    je near .drawnone
    cmp dword[esi],01010101h
    je .drawall
.drawpart
    mov byte[DoTransp],0
    xor ebx,ebx
    cmp byte[esi],1
    jne .nodraw
    mov ebx,eax
.nodraw
    mov [edi],bx
    xor ebx,ebx
    cmp byte[esi+1],1
    jne .nodraw1
    mov ebx,eax
.nodraw1
    mov [edi+2],bx
    xor ebx,ebx
    cmp byte[esi+2],1
    jne .nodraw2
    mov ebx,eax
.nodraw2
    mov [edi+4],bx
    xor ebx,ebx
    cmp byte[esi+3],1
    jne .nodraw3
    mov ebx,eax
.nodraw3
    mov [edi+6],bx
    add edi,8
    add esi,4
    dec ecx
    jnz near .nextpartp
    jmp .findraw
.nextpartn
    cmp dword[esi],0
    jne .drawpart
.drawnone
    mov dword[edi],0
    mov dword[edi+4],0
    add edi,8
    add esi,4
    dec ecx
    jnz .nextpartn
.findraw
    ret

SECTION .bss
NEWSYM DoTransp, resb 1
SECTION .text

;*******************************************************
; Clear Backarea, 16-bit mode w/ transparency
;*******************************************************
NEWSYM clearback16t
    test byte[scaddtype],20h
    jz near .backcopy
    test byte[scaddtype],80h
    jnz near clearback16ts
    mov eax,[pal16b]
    mov esi,[curvidoffset]
    mov ebp,transpbuf+32
    mov dx,ax
    and eax,[vesa2_clbit]
    shr eax,1
    test byte[scaddtype],40h
    jz .fulladd
    cmp byte[scrnon+1],0
    je .fulladd
    mov ecx,128
.loopa
    mov ebx,[ebp]
    or bx,bx
    jz .noadd
    and bx,[vesa2_clbit]
    shr bx,1
    add bx,ax
    mov [esi],bx
    jmp .skip
.noadd
    mov [esi],dx
.skip
    shr ebx,16
    or bx,bx
    je .noadd2
    and bx,word[vesa2_clbit]
    shr bx,1
    add bx,ax
    mov [esi+2],bx
    jmp .skip2
.noadd2
    mov [esi+2],dx
.skip2
    add ebp,4
    add esi,4
    dec ecx
    jnz .loopa
    xor eax,eax
    ret
.fulladd
    cmp eax,0
    je .subcopy
    mov ecx,256
    xor ebx,ebx
.loopc
    mov ebx,[ebp]
    and ebx,[vesa2_clbit]
    shr ebx,1
    add ebx,eax
    add ebp,2
    mov ebx,[fulladdtab+ebx*2]
    mov [esi],bx
    add esi,2
    dec ecx
    jnz .loopc
    xor eax,eax
    ret
.subcopy
    cmp byte[MMXSupport],1
    je .dommxcopy
    mov ecx,128
    xor ebx,ebx
    mov edi,esi
    mov esi,ebp
    rep movsd
    xor eax,eax
    ret
.dommxcopy
    mov ecx,32
    xor ebx,ebx
    mov edi,esi
    mov esi,ebp
.mmxloop2
    movq mm0,[esi]
    movq mm1,[esi+8]
    movq [edi],mm0
    movq [edi+8],mm1
    add esi,16
    add edi,16
    dec ecx
    jnz .mmxloop2
    emms
    xor eax,eax
    ret
.backcopy
    mov edi,[curvidoffset]
    mov ecx,128
    mov ax,[pal16b]
    shl eax,16
    mov ax,[pal16b]
    cmp byte[MMXSupport],1
    je .dommxclear
    rep stosd
    xor eax,eax
    ret
.dommxclear
    mov [mmxtempdat],eax
    mov [mmxtempdat+4],eax
    mov ecx,32
    movq mm0,[mmxtempdat]
.mmxloop
    movq [edi],mm0
    movq [edi+8],mm0
    add edi,16
    dec ecx
    jnz .mmxloop
    emms
    xor eax,eax
    ret

NEWSYM clearback16ts
    mov eax,[pal16b]
    mov esi,[curvidoffset]
    mov ebp,transpbuf+32
    xor eax,0FFFFh
    and eax,[vesa2_clbit]
    shr eax,1
    mov ecx,256
    xor ebx,ebx
.loopc
    mov ebx,[ebp]
    and ebx,[vesa2_clbit]
    shr ebx,1
    add ebx,eax
    add ebp,2
    mov ebx,[fulladdtab+ebx*2]
    xor ebx,0FFFFh
    mov [esi],bx
    add esi,2
    dec ecx
    jnz .loopc
    xor eax,eax
    ret

NEWSYM drawsprites16bt
    cmp byte[sprprifix],1
    je near drawsprites16btprio
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawsprites16btwinon
.drawnowin
    mov esi,[currentobjptr]
    xor ebx,ebx
    xor eax,eax
.loopobj
    mov edi,[curvidoffset]
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    shl bx,1
    mov esi,[esi+2]
    mov ebp,ebx
    add edi,ebx
    add ebp,transpbuf+32
    drawspr16bt 0, 16
    drawspr16bt 1, 14
    drawspr16bt 2, 12
    drawspr16bt 3, 10
    drawspr16bt 4, 8
    drawspr16bt 5, 6
    drawspr16bt 6, 4
    drawspr16bt 7, 2
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
    shl bx,1
    mov esi,[esi+2]
    mov ebp,ebx
    add edi,ebx
    add ebp,transpbuf+32
    drawspr16bt 7, 16
    drawspr16bt 6, 14
    drawspr16bt 5, 12
    drawspr16bt 4, 10
    drawspr16bt 3, 8
    drawspr16bt 2, 6
    drawspr16bt 1, 4
    drawspr16bt 0, 2
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16btwinon
    mov esi,[currentobjptr]
    xor ebx,ebx
    xor eax,eax
.loopobj
    mov edi,[curvidoffset]
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    shl bx,1
    mov esi,[esi+2]
    mov ebp,ebx
    add edi,ebx
    shr ebx,1
    add ebp,transpbuf+32
    drawspr16btwo 0, 16, 0
    drawspr16btwo 1, 14, 1
    drawspr16btwo 2, 12, 2
    drawspr16btwo 3, 10, 3
    drawspr16btwo 4, 8, 4
    drawspr16btwo 5, 6, 5
    drawspr16btwo 6, 4, 6
    drawspr16btwo 7, 2, 7
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
    shl bx,1
    mov esi,[esi+2]
    mov ebp,ebx
    add edi,ebx
    shr ebx,1
    add ebp,transpbuf+32
    drawspr16btwo 7, 16, 0
    drawspr16btwo 6, 14, 1
    drawspr16btwo 5, 12, 2
    drawspr16btwo 4, 10, 3
    drawspr16btwo 3, 8, 4
    drawspr16btwo 2, 6, 5
    drawspr16btwo 1, 4, 6
    drawspr16btwo 0, 2, 7
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16btprio
    sprpriorityinit drawsprites16btpriow
    sprprioritydrawbt sprdrawpra16bt, sprdrawprb16bt

NEWSYM drawsprites16btpriow
    sprprioritydrawbt sprdrawpra16bt, sprdrawprb16bt

NEWSYM drawsprites16t
    cmp byte[sprprifix],1
    je near drawsprites16tprio
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawsprites16twinon
.drawnowin
    test byte[scaddtype],40h
    jz near drawspritesfulladd
    cmp byte[scrnon+1],0
    je near drawspritesfulladd
    test byte[scaddtype],80h
    jnz near drawspritesfulladd
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
    shl bx,1
    mov esi,[esi+2]
    cmp ch,12*16
    jae near .transparentobjnf
    drawsprgrp drawspr16ta
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
    shl bx,1
    mov esi,[esi+2]
    cmp ch,12*16
    jae near .transparentobj
    drawsprgrpf drawspr16ta
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnf
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrp drawspr16tb
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobj
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpf drawspr16tb
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

drawspritesfulladd:
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
    shl bx,1
    mov esi,[esi+2]
    cmp ch,12*16
    jae near .transparentobjnf
    drawsprgrp drawspr16ta
    pop esi
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.drawspriteflipx
    push esi
    mov bx,[esi]
    mov ch,[esi+6]
    shl bx,1
    mov esi,[esi+2]
    cmp ch,12*16
    jae near .transparentobj
    drawsprgrpf drawspr16ta
    pop esi
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnf
    test byte[scaddtype],80h
    jnz near .transparentobjnfs
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrp drawspr16tc
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnfs
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrp drawspr16td
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobj
    test byte[scaddtype],80h
    jnz near .transparentobjs
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpf drawspr16tc
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjs
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpf drawspr16td
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16twinon
    test byte[scaddtype],40h
    jz near drawspritesfulladdwinon
    cmp byte[scrnon+1],0
    je near drawspritesfulladdwinon
    test byte[scaddtype],80h
    jnz near drawspritesfulladdwinon
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
    cmp ch,12*16
    jae near .transparentobjnf
    drawsprgrpwin drawspr16tawinon
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
    cmp ch,12*16
    jae near .transparentobj
    drawsprgrpfwin drawspr16tawinon
    pop esi
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnf
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpwin drawspr16tbwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobj
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpfwin drawspr16tbwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawspritesfulladdwinon
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
    cmp ch,12*16
    jae near .transparentobjnf
    drawsprgrpwin drawspr16tawinon
    pop esi
    xor edx,edx
    xor ebx,ebx
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
    cmp ch,12*16
    jae near .transparentobj
    drawsprgrpfwin drawspr16tawinon
    pop esi
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnf
    test byte[scaddtype],80h
    jnz near .transparentobjnfs
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpwin drawspr16tcwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjnfs
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpwin drawspr16tdwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobj
    test byte[scaddtype],80h
    jnz near .transparentobjs
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpfwin drawspr16tcwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.transparentobjs
    mov ebp,ebx
    add edi,ebx
    mov ebp,ebx
    add edi,ebx
    xor edx,edx
    add ebp,transpbuf+32
    drawsprgrpfwin drawspr16tdwinon
    pop esi
    mov edi,[curvidoffset]
    xor edx,edx
    xor ebx,ebx
    add esi,8
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawsprites16tprio
    test byte[scaddtype],40h
    jz near drawspritesfulladdprio
    cmp byte[scrnon+1],0
    je near drawspritesfulladdprio
    test byte[scaddtype],80h
    jnz near drawspritesfulladdprio
    ; half add
    sprpriorityinit drawsprites16tpriow
    sprprioritydrawt16b sprdrawpra16bha, sprdrawprb16bha, sprdrawpra16b, sprdrawprb16b

NEWSYM drawsprites16tpriow
    sprprioritydrawt16b sprdrawpraw16bha, sprdrawprbw16bha, sprdrawpraw16b, sprdrawprbw16b

NEWSYM drawspritesfulladdprio
    test byte[scaddtype],80h
    jnz near drawspritesfullsubprio
    ; full add
    sprpriorityinit drawspritesfulladdpriow
    sprprioritydrawt16b sprdrawpra16bfa, sprdrawprb16bfa, sprdrawpra16b, sprdrawprb16b

NEWSYM drawspritesfulladdpriow
    sprprioritydrawt16b sprdrawpraw16bfa, sprdrawprbw16bfa, sprdrawpraw16b, sprdrawprbw16b

NEWSYM drawspritesfullsubprio
    ; full sub
    sprpriorityinit drawspritesfullsubpriow
    sprprioritydrawt16b sprdrawpra16bfs, sprdrawprb16bfs, sprdrawpra16b, sprdrawprb16b

NEWSYM drawspritesfullsubpriow
    sprprioritydrawt16b sprdrawpraw16bfs, sprdrawprbw16bfs, sprdrawpraw16b, sprdrawprbw16b


NEWSYM draw8x816bt
    cmp byte[bgmode],2
    je near draw8x816boffset
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
    jne near draw8x816btwinon
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
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
    drawtilegrp draw8x816bta
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
    drawtilegrpf draw8x816bta
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

NEWSYM draw8x816btwinon
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    mov byte[tileleft16b],33
    mov ecx,[winptrref]
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
    mov eax,ecx
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    mov ecx,eax
    xor eax,eax
    ; Start loop
    drawtilegrp draw8x816btawinon
.hprior
    add esi,16
    add ecx,8
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
    mov eax,ecx
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    mov ecx,eax
    xor eax,eax
    drawtilegrpf draw8x816btawinonb
    add esi,16
    add ecx,8
    add ebp,16
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec byte[tileleft16b]
    jnz near .loopa
    ret

NEWSYM draw8x816t
    cmp byte[osm2dis],1
    je .osm2dis
    cmp byte[bgmode],2
    je near draw8x816toffset
.osm2dis
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
    jne near draw8x816twinon
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax

    test byte[scaddtype],80h
    jnz near draw8x816ts
    test byte[scaddtype],40h
    jz near draw8x8fulladd
    cmp byte[scrnon+1],0
    jz near draw8x8fulladd

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
    drawtilegrp draw8x816ta
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
    drawtilegrpf draw8x816ta
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

NEWSYM draw8x8fulladd
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
    drawtilegrpfull draw8x816tb
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
    drawtilegrpfullf draw8x816tb
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

NEWSYM draw8x816ts
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
    drawtilegrpfull draw8x816tc
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
    drawtilegrpfullf draw8x816tc
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

ALIGN32
SECTION .bss
NEWSYM coadder16, resd 1
SECTION .text

NEWSYM draw8x816twinon
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw8x816tswinon
    test byte[scaddtype],40h
    jz near draw8x8fulladdwinon
    cmp byte[scrnon+1],0
    jz near draw8x8fulladdwinon
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
    drawtilegrp draw8x816tawinon
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
    drawtilegrpf draw8x816tawinonb
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

NEWSYM draw8x8fulladdwinon
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
    drawtilegrpfull draw8x816tbwinon
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
    drawtilegrpfullf draw8x816tbwinonb
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

NEWSYM draw8x816tswinon
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
    drawtilegrpfull draw8x816tcwinon
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
    drawtilegrpfullf draw8x816tcwinonb
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
; Processes & Draws 16x8 tiles
;*******************************************************

%macro Process16x816t 2
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
%%loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
    push edi
    xor dh,[curbgpr]
    test dh,20h
    jnz near %%hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb %%noclip
    sub edi,[bgsubby]
%%noclip
    test dh,80h
    jz %%normadd
    add edi,[yrevadder]
    jmp %%skipadd
%%normadd
    add edi,[yadder]
%%skipadd
    test dh,40h
    jnz near %%rloop

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    %2 0+%1, 0
    %2 2+%1, 2
    %2 4+%1, 4
    %2 6+%1, 6
    add edi,64
    ; Start loop
    %2 0+%1, 8
    %2 2+%1, 10
    %2 4+%1, 12
    %2 6+%1, 14
%%hprior
    pop edi
    add esi,16
    add ebp,16
    inc dl
    cmp dl,20h
    jne %%loopc2
    mov edi,[temptile]
%%loopc2
    dec byte[tileleft16b]
    jnz near %%loopa
    cmp byte[drawn],0
    je %%nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
%%nodraw
    ret

%%rloop
    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    xor eax,eax
    xor ecx,ecx
    ; Start loop
    %2 1-%1, 14
    %2 3-%1, 12
    %2 5-%1, 10
    %2 7-%1, 8
    add edi,64
    ; Start loop
    %2 1-%1, 6
    %2 3-%1, 4
    %2 5-%1, 2
    %2 7-%1, 0
    pop edi
    add esi,16
    add ebp,16
    inc dl
    cmp dl,20h
    jne %%loopc3
    mov edi,[temptile]
%%loopc3
    dec byte[tileleft16b]
    jnz near %%loopa
    cmp byte[drawn],0
    je %%nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
%%nodraw2
    ret
%endmacro

%macro Process16x816twin 2
    mov byte[tileleft16b],33
    mov byte[drawn],0
    mov dl,[temp]
%%loopa
    mov ax,[edi]
    mov cl,ah
    add edi,2
    push edi
    xor cl,[curbgpr]
    test cl,20h
    jnz near %%hprior
    inc byte[drawn]
    and eax,03FFh                ; filter out tile #
    mov edi,[tempcach]
    shl eax,6
    add edi,eax
    cmp edi,[bgofwptr]
    jb %%noclip
    sub edi,[bgsubby]
%%noclip
    test cl,80h
    jz %%normadd
    add edi,[yrevadder]
    jmp %%skipadd
%%normadd
    add edi,[yadder]
%%skipadd
    test cl,40h
    jnz near %%rloop
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
    %2 0+%1, 0, 0
    %2 2+%1, 2, 1
    %2 4+%1, 4, 2
    %2 6+%1, 6, 3
    add edi,64
    ; Start loop
    %2 0+%1, 8, 4
    %2 2+%1, 10, 5
    %2 4+%1, 12, 6
    %2 6+%1, 14, 7
%%hprior
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne %%loopc2
    mov edi,[temptile]
%%loopc2
    dec byte[tileleft16b]
    jnz near %%loopa
    ret

%%rloop
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
    %2 1-%1, 14, 0
    %2 3-%1, 12, 1
    %2 5-%1, 10, 2
    %2 7-%1, 8, 3
    add edi,64
    ; Start loop
    %2 1-%1, 6, 4
    %2 3-%1, 4, 5
    %2 5-%1, 2, 6
    %2 7-%1, 0, 7
    pop edi
    add esi,16
    add edx,8
    add ebp,16
    inc byte[temp]
    cmp byte[temp],20h
    jne %%loopc3
    mov edi,[temptile]
%%loopc3
    dec byte[tileleft16b]
    jnz near %%loopa
    ret
%endmacro

NEWSYM draw16x816t
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
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax

    test byte[scaddtype],80h
    jnz near draw16x816ts
    test byte[scaddtype],40h
    jz near draw16x816tfa
    cmp byte[scrnon+1],0
    jz near draw16x816tfa

    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x816twinon
.domosaic
    cmp byte[res512switch],0
    jne near draw16x816tb
    Process16x816t 0, draw8x816ta2

NEWSYM draw16x816tb
    Process16x816t 1, draw8x816ta2

NEWSYM draw16x816twinon
    mov edx,[winptrref]
    cmp byte[res512switch],0
    jne near draw16x816twinonb
    Process16x816twin 0, draw8x816tawinon2

NEWSYM draw16x816twinonb
    Process16x816twin 1, draw8x816tawinon2

draw16x816tfa:
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x816twinonfa
.domosaic
    cmp byte[res512switch],0
    jne near draw16x816tbfa
    Process16x816t 0, draw8x816tb
NEWSYM draw16x816tbfa
    Process16x816t 1, draw8x816tb
NEWSYM draw16x816twinonfa
    mov edx,[winptrref]
    cmp byte[res512switch],0
    jne near draw16x816twinonbfa
    Process16x816twin 0, draw8x816tbwinon2
NEWSYM draw16x816twinonbfa
    Process16x816twin 1, draw8x816tbwinon2

draw16x816ts:
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x816twinons
.domosaic
    cmp byte[res512switch],0
    jne near draw16x816tbs
    Process16x816t 0, draw8x816tc
NEWSYM draw16x816tbs
    Process16x816t 1, draw8x816tc
NEWSYM draw16x816twinons
    mov edx,[winptrref]
    cmp byte[res512switch],0
    jne near draw16x816twinonbs
    Process16x816twin 0, draw8x816tcwinon2
NEWSYM draw16x816twinonbs
    Process16x816twin 1, draw8x816tcwinon2

;*******************************************************
; Processes & Draws 8x8 tiles, offset mode
;*******************************************************


NEWSYM draw8x816toffset
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
    jne near draw8x816twinonoffset
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw8x816tsoffset
    test byte[scaddtype],40h
    jz near draw8x8fulladdoffset
    cmp byte[scrnon+1],0
    jz near draw8x8fulladdoffset
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
    xor ecx,ecx
    ; Start loop
    drawtilegrp draw8x816ta
.hprior
    procoffsetmode
    add esi,16
    add ebp,16
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
    xor ecx,ecx
    drawtilegrpf draw8x816ta
    procoffsetmode
    add esi,16
    add ebp,16
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

NEWSYM draw8x8fulladdoffset
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
    offsetmcachechk
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
    drawtilegrpfull draw8x816tb
.hprior
    pop edi
    procoffsetmode
    add esi,16
    add ebp,16
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
    xor ecx,ecx
    drawtilegrpfullf draw8x816tb
    pop edi
    procoffsetmode
    add esi,16
    add ebp,16
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

NEWSYM draw8x816tsoffset
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
    offsetmcachechk
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
    drawtilegrpfull draw8x816tc
.hprior
    pop edi
    procoffsetmode
    add esi,16
    add ebp,16
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
    xor ecx,ecx
    drawtilegrpfullf draw8x816tc
    pop edi
    procoffsetmode
    add esi,16
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret

NEWSYM draw8x816twinonoffset
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw8x816tswinonoffset
    test byte[scaddtype],40h
    jz near draw8x8fulladdwinonoffset
    cmp byte[scrnon+1],0
    jz near draw8x8fulladdwinonoffset
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
    offsetmcachechk
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
    drawtilegrp draw8x816tawinon
.hprior
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
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
    drawtilegrpf draw8x816tawinonb
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret

NEWSYM draw8x8fulladdwinonoffset
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
    offsetmcachechk
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
    drawtilegrpfull draw8x816tbwinon
.hprior
    pop edi
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
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
    drawtilegrpfullf draw8x816tbwinonb
    pop edi
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret

NEWSYM draw8x816tswinonoffset
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
    offsetmcachechk
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
    drawtilegrpfull draw8x816tcwinon
.hprior
    pop edi
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
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
    drawtilegrpfullf draw8x816tcwinonb
    pop edi
    procoffsetmode
    add esi,16
    add edx,8
    add ebp,16
    dec byte[tileleft16b]
    jnz near .loopa
    xor eax,eax
    ret

;*******************************************************
; Processes & Draws 16x16 tiles in main and sub screen
;*******************************************************
NEWSYM draw16x1616bt
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
    jne near draw16x1616btwinon
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
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

    ; Start loop
    drawtilegrp draw8x816bta
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
    drawtilegrpf draw8x816bta
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

NEWSYM draw16x1616btwinon
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    mov byte[tileleft16b],33
    mov ecx,[winptrref]
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
    mov eax,ecx
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    mov ecx,eax
    xor eax,eax

    ; Start loop
    drawtilegrp draw8x816btawinon
.hprior
    add esi,16
    add ecx,8
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
    ret

    ; reversed loop
.rloop
    mov eax,ecx
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    mov ecx,eax
    xor eax,eax
    drawtilegrpf draw8x816btawinon
.skiploop2b
    add esi,16
    add ecx,8
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
    ret

;*******************************************************
; Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
;*******************************************************

NEWSYM draw16x1616t
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
    jne near draw16x1616twinon
.domosaic
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616ts
    test byte[scaddtype],40h
    jz near draw16x16fulladd
    cmp byte[scrnon+1],0
    je near draw16x16fulladd
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
    drawtilegrp draw8x816ta
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
    drawtilegrpf draw8x816ta
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

SECTION .bss
NEWSYM yadd,   resw 1
NEWSYM yflipadd,  resw 1
SECTION .text

NEWSYM draw16x16fulladd
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
    drawtilegrpfull draw8x816tb
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
    drawtilegrpfullf draw8x816tb
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

NEWSYM draw16x1616ts
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
    drawtilegrpfull draw8x816tc
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
    drawtilegrpfullf draw8x816tc
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

NEWSYM draw16x1616twinon
    mov ebp,transpbuf+32
    sub ebp,eax
    sub ebp,eax
    test byte[scaddtype],80h
    jnz near draw16x1616tswinon
    test byte[scaddtype],40h
    jz near draw16x16fulladdwinon
    cmp byte[scrnon+1],0
    je near draw16x16fulladdwinon
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
    drawtilegrp draw8x816tawinon
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
    drawtilegrpf draw8x816tawinonb
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

NEWSYM draw16x16fulladdwinon
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
    drawtilegrpfull draw8x816tbwinon
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
    drawtilegrpfullf draw8x816tbwinonb
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

NEWSYM draw16x1616tswinon
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
    drawtilegrpfull draw8x816tcwinon
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
    drawtilegrpfullf draw8x816tcwinonb
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
