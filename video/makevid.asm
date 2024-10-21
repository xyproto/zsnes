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

EXTSYM disableeffects,winl1,winl2,winbgdata,winr1,winr2,winspdata,winlogica
EXTSYM winenabm,winlogicb,scrndis,scrnon,bgmode,bgtilesz,winbg1en
EXTSYM winenabs
EXTSYM vcache2b,vcache4b,vcache8b
EXTSYM vidbuffer,bg3highst,cbitmode,colormodedef,ngptrdat2
EXTSYM colormodeofs,drawline16b,forceblnk,preparesprpr,scaddset
EXTSYM spritetablea,sprleftpr,vidbright,ForceNewGfxOff,curypos
EXTSYM mode7set,mosaicon,mosaicsz,sprleftpr1,sprleftpr2,sprleftpr3,sprlefttot
EXTSYM sprprifix,interlval,sprclprio,sprpriodata
EXTSYM sprsingle,vram,newengen,ofshvaladd
EXTSYM osm2dis,xtravbuf
EXTSYM bg3ptr,bg3scrolx,bg3scroly,vidmemch4,ofsmcptr,ofsmady,ofsmadx,yposngom
EXTSYM flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd,bg1ptrx,bg1ptry
EXTSYM bg1scrolx_m7,bg1scroly_m7,OMBGTestVal,cachesingle4bng,m7starty
EXTSYM ofsmtptrs,ofsmcptr2
EXTSYM newengine16b
EXTSYM makewindow

SECTION .bss
NEWSYM bgcoloradder, resb 1
NEWSYM res512switch, resb 1

SECTION .text

;*******************************************************
; DrawLine                        Draws the current line
;*******************************************************
; use curypos+bg1scroly for y location and bg1scrolx for x location
; use bg1ptr(b,c,d) for the pointer to the tile number contents
; use bg1objptr for the pointer to the object tile contents

%macro procmode7 3
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
    xor edx,edx
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
    xor edx,edx
    mov dl,[mosaicsz]
    add ax,[MosaicYAdder+edx*2]
%%nomos
    mov [m7starty],ax
    mov ax,%1
    mov dx,%2
    call drawmode7
%endmacro

%macro procmode7extbg 3
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
    xor edx,edx
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
    xor edx,edx
    mov dl,[mosaicsz]
    add ax,[MosaicYAdder+edx*2]
%%nomos
    mov [m7starty],ax
    mov ax,%1
    mov dx,%2
    call drawmode7extbg
%endmacro

%macro procmode7extbg2 3
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
    xor edx,edx
    xor bh,bh
    div bx
    xor edx,edx
    mul bx
    xor edx,edx
    mov dl,[mosaicsz]
    add ax,[MosaicYAdder+edx*2]
%%nomos
    mov [m7starty],ax
    mov ax,%1
    mov dx,%2
    call drawmode7extbg2
%endmacro

SECTION .data
NEWSYM MosaicYAdder, dw 0,0,0,1,0,2,1,0,0,4,2,2,3,1,0,7

NEWSYM cwinptr,    dd winbgdata

SECTION .bss
NEWSYM pwinbgenab, resb 1
NEWSYM pwinbgtype, resd 1
NEWSYM winonbtype, resb 1
NEWSYM dualwinbg,  resb 1
NEWSYM pwinspenab, resb 1
NEWSYM pwinsptype, resd 1
NEWSYM winonstype, resb 1
NEWSYM dualwinsp,  resb 1
NEWSYM dwinptrproc, resd 1
SECTION .text

NEWSYM dualstartprocess

    mov dl,[winl1]
    mov dh,[winr1]

    push eax
    push ecx
    test al,01h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    mov edi,[dwinptrproc]
    xor eax,eax
    mov ecx,64
    rep stosd
    jmp .donextwin
.clip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
.nextdot
    mov byte[edi+eax],0
    inc al
    cmp al,dl
    jbe .nextdot
.nextdot2
    mov byte[edi+eax],1
    inc al
    cmp al,dh
    jb .nextdot2
    mov byte[edi+eax],1
    cmp dh,255
    je .nextdot4
    ; start drawing 1's from right to 255
.nextdot3
    mov byte[edi+eax],0
    inc al
    jnz .nextdot3
.nextdot4
    jmp .donextwin
.outside
    cmp dl,dh
    jb .clip2
    mov edi,[dwinptrproc]
    mov eax,01010101h
    mov ecx,64
    rep stosd
    jmp .donextwin
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
.nextdoti
    mov byte[edi+eax],1
    inc al
    cmp al,dl
    jb .nextdoti
.nextdot2i
    mov byte[edi+eax],0
    inc al
    cmp al,dh
    jb .nextdot2i
    mov byte[edi+eax],0
    cmp al,255
    je .nextdot4i
    inc al
    ; start drawing 1's from right to 255
.nextdot3i
    mov byte[edi+eax],1
    inc al
    jnz .nextdot3i
.nextdot4i
.donextwin
    pop ecx
    pop eax
    cmp cl,0
    je near dualwinor
    cmp cl,2
    je near dualwinxor
    cmp cl,3
    je near dualwinxnor

NEWSYM dualwinand
    mov dl,[winl2]
    mov dh,[winr2]
    test al,04h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    mov edi,[dwinptrproc]
    xor eax,eax
    mov ecx,64
    rep stosd
    jmp .donextwin
.clip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
.nextdot
    mov byte[edi+eax],0
    inc al
    cmp al,dl
    jbe .nextdot
.nextdot2
    and byte[edi+eax],1
    inc al
    cmp al,dh
    jb .nextdot2
    and byte[edi+eax],1
    cmp dh,255
    je .nextdot4
    ; start drawing 1's from right to 255
.nextdot3
    mov byte[edi+eax],0
    inc al
    jnz .nextdot3
.nextdot4
    jmp .donextwin
.outside
    cmp dl,dh
    jb .clip2
    jmp .donextwin
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
.nextdoti
    and byte[edi+eax],1
    inc al
    cmp al,dl
    jb .nextdoti
.nextdot2i
    mov byte[edi+eax],0
    inc al
    cmp al,dh
    jb .nextdot2i
    mov byte[edi+eax],0
    cmp al,255
    je .nextdot4i
    inc al
    ; start drawing 1's from right to 255
.nextdot3i
    and byte[edi+eax],1
    inc al
    jnz .nextdot3i
.nextdot4i
.donextwin
    ret

NEWSYM dualwinor
    mov dl,[winl2]
    mov dh,[winr2]
    test al,04h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    jmp .donextwin
.clip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
    mov al,dl
    inc al
.nextdot2
    mov byte[edi+eax],1
    inc al
    cmp al,dh
    jb .nextdot2
    mov byte[edi+eax],1
    jmp .donextwin
.outside
    cmp dl,dh
    jb .clip2
    mov edi,[dwinptrproc]
    mov eax,01010101h
    mov ecx,64
    rep stosd
    jmp .donextwin
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
.nextdoti
    mov byte[edi+eax],1
    inc al
    cmp al,dl
    jb .nextdoti
    mov al,dh
    cmp al,255
    je .nextdot4i
    inc al
    ; start drawing 1's from right to 255
.nextdot3i
    mov byte[edi+eax],1
    inc al
    jnz .nextdot3i
.nextdot4i
.donextwin
    ret

NEWSYM dualwinxor
    mov dl,[winl2]
    mov dh,[winr2]
    test al,04h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    jmp .donextwin
.clip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
    mov al,dl
    inc al
.nextdot2
    xor byte[edi+eax],1
    inc al
    cmp al,dh
    jb .nextdot2
    xor byte[edi+eax],1
    jmp .donextwin
.outside
    cmp dl,dh
    jb .clip2
    mov edi,[dwinptrproc]
    mov ecx,64
.loopxor
    xor dword[edi],01010101h
    add edi,4
    dec ecx
    jnz .loopxor
    jmp .donextwin
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
.nextdoti
    xor byte[edi+eax],1
    inc al
    cmp al,dl
    jb .nextdoti
    mov al,dh
    cmp al,255
    je .nextdot4i
    inc al
    ; start drawing 1's from right to 255
.nextdot3i
    xor byte[edi+eax],1
    inc al
    jnz .nextdot3i
.nextdot4i
.donextwin
    ret

NEWSYM dualwinxnor
    mov dl,[winl2]
    mov dh,[winr2]
    test al,04h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    jmp .donextwin
.clip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
    mov al,dl
    inc al
.nextdot2
    xor byte[edi+eax],1
    inc al
    cmp al,dh
    jb .nextdot2
    xor byte[edi+eax],1
    jmp .donextwin
.outside
    cmp dl,dh
    jb .clip2
    mov edi,[dwinptrproc]
    mov ecx,64
.loopxor
    xor dword[edi],01010101h
    add edi,4
    dec ecx
    jnz .loopxor
    jmp .donextwin
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,[dwinptrproc]
    xor eax,eax
    ; start drawing 1's from 0 to left
.nextdoti
    xor byte[edi+eax],1
    inc al
    cmp al,dl
    jb .nextdoti
    mov al,dh
    cmp al,255
    je .nextdot4i
    inc al
    ; start drawing 1's from right to 255
.nextdot3i
    xor byte[edi+eax],1
    inc al
    jnz .nextdot3i
.nextdot4i
.donextwin
    mov edi,[dwinptrproc]
    mov ecx,64
.loopxor2
    xor dword[edi],01010101h
    add edi,4
    dec ecx
    jnz .loopxor2
    ret

SECTION .bss
NEWSYM winonsp, resb 1
SECTION .text

NEWSYM makedualwinsp
    mov ecx,ebp
    shl cl,1
    mov dl,[winlogicb]
    and dl,03h
    mov cl,dl
    mov byte[winonsp],1
    ; check if data matches previous data
    cmp cl,[dualwinsp]
    jne .skipenab3
    cmp al,[pwinspenab]
    jne .skipenab
    mov ebx,[winl1]
    cmp ebx,[pwinsptype]
    jne .skipenab2
    mov dword[cwinptr],winspdata+16
    mov al,[winonstype]
    mov [winonsp],al
    ret
.skipenab3
    mov [dualwinsp],cl
.skipenab
    mov [pwinspenab],al
    mov ebx,[winl1]
.skipenab2
    mov [pwinsptype],ebx
    mov dword[dwinptrproc],winspdata+16
    mov dword[cwinptr],winspdata+16
    mov byte[winonsp],1
    mov byte[winonstype],1
    jmp dualstartprocess

; window logic data
SECTION .bss
NEWSYM windowdata, resb 16
NEWSYM numwin, resb 1
NEWSYM multiwin, resb 1
NEWSYM multiclip, resb 1
NEWSYM multitype, resb 1
SECTION .text

;    jmp .finishwin
%macro procwindow 1
    cmp byte[disableeffects],1
    je near .finishwin
    mov al,%1
    test al,00001010b
    jz near .finishwin
    mov esi,windowdata
    mov bl,al
    mov byte[winon],1
    and bl,00001010b
    and al,00000101b
    mov byte[numwin],0
    cmp bl,00001010b
    je near .multiwin
    mov byte[multiwin],0
    test bl,00000010b
    jnz .win1
    mov cl,[winl2]
    mov ch,[winr2]
    shr al,2
    jmp .okaywin
.win1
    mov cl,[winl1]
    mov ch,[winr1]
    and al,01h
.okaywin
    cmp ch,255
    je .noinc
    inc ch
.noinc
    test al,01h
    jnz .wininside
    cmp cl,ch
    jae .noinsidemask
    mov [esi],cl
    mov byte[esi+1],01h
    mov [esi+2],ch
    mov byte[esi+3],0FFh
    mov byte[numwin],2
    jmp .finishwin
.noinsidemask
    mov byte[winon],0
    jmp .finishwin
.wininside
    cmp cl,ch
    ja .nooutsidemask
.nonotoutside
    cmp ch,254
    jb .skipnodraw
    cmp cl,1
    jbe .noinsidemask
.skipnodraw
    mov byte[esi],0
    mov byte[esi+1],01h
    mov [esi+2],cl
    mov byte[esi+3],0FFh
    mov [esi+4],ch
    mov byte[esi+5],01h
    mov byte[numwin],3
    jmp .finishwin
.nooutsidemask
    mov byte[esi],0
    mov byte[esi+1],01h
    mov byte[numwin],1
    jmp .finishwin
    ; **************
    ; *Multiwindows*
    ; **************
.multiwin
    mov byte[winon],0
    mov byte[multiwin],0
    mov [multiclip],al
    mov al,[winlogica]
    mov ecx,ebp
    shl ecx,1
    shr al,cl
    and al,3h
    mov [multitype],al
    mov cl,[winl1]
    mov ch,[winr1]
    mov esi,windowdata
    cmp ch,255
    je .noinc2
    inc ch
.noinc2
    test byte[multiclip],01h
    jnz .wininside2
    cmp cl,ch
    jae .nowina
    mov [esi],cl
    mov byte[esi+1],01h
    mov [esi+2],ch
    mov byte[esi+3],0FFh
    add esi,4
    mov byte[numwin],2
    jmp .secondwin
.nowina
    mov cl,[winl2]
    mov ch,[winr2]
    mov al,[multiclip]
    shr al,2
    jmp .okaywin
.wininside2
    cmp cl,ch
    ja .nooutsidemask2
    cmp ch,254
    jb .skipnodraw2
    cmp cl,1
    jbe .nooutsidemask2
.skipnodraw2
    mov byte[esi],0
    mov byte[esi+1],01h
    mov [esi+2],cl
    mov byte[esi+3],0FFh
    mov [esi+4],ch
    mov byte[esi+5],01h
    mov byte[numwin],3
    jmp .secondwin
.nooutsidemask2
    mov byte[esi],0
    mov byte[esi+1],01h
    mov byte[numwin],1
.secondwin
    mov byte[multiwin],1
    mov byte[winon],1
.finishwin
%endmacro

SECTION .bss
NEWSYM curbgnum, resb 1
SECTION .text

SECTION .bss
NEWSYM nextprimode, resb 1
NEWSYM cursprloc,   resd 1
NEWSYM curtileptr,  resw 1
; esi = pointer to video buffer
; edi = pointer to tile data
; ebx = cached memory
; al = current x position
NEWSYM bg1vbufloc,  resd 1
NEWSYM bg2vbufloc,  resd 1
NEWSYM bg3vbufloc,  resd 1
NEWSYM bg4vbufloc,  resd 1
NEWSYM bg1tdatloc,  resd 1
NEWSYM bg2tdatloc,  resd 1
NEWSYM bg3tdatloc,  resd 1
NEWSYM bg4tdatloc,  resd 1
NEWSYM bg1tdabloc,  resd 1
NEWSYM bg2tdabloc,  resd 1
NEWSYM bg3tdabloc,  resd 1
NEWSYM bg4tdabloc,  resd 1
NEWSYM bg1cachloc,  resd 1
NEWSYM bg2cachloc,  resd 1
NEWSYM bg3cachloc,  resd 1
NEWSYM bg4cachloc,  resd 1
NEWSYM bg1yaddval,  resd 1
NEWSYM bg2yaddval,  resd 1
NEWSYM bg3yaddval,  resd 1
NEWSYM bg4yaddval,  resd 1
NEWSYM bg1xposloc,  resd 1
NEWSYM bg2xposloc,  resd 1
NEWSYM bg3xposloc,  resd 1
NEWSYM bg4xposloc,  resd 1
NEWSYM alreadydrawn, resb 1

NEWSYM bg3draw, resb 1
NEWSYM maxbr,   resb 1
SECTION .text

ALIGN32
SECTION .bss
NEWSYM bg3high2, resd 1
NEWSYM cwinenabm, resd 1
SECTION .text

NEWSYM drawline

    cmp byte[ForceNewGfxOff],0
    jne .drawline16b
    cmp byte[newengen],0
    je .drawline16b
    jmp newengine16b
.drawline16b
    ccallv drawline16b
    ret

ALIGN32
SECTION .bss
NEWSYM tempbuffer, resd 33
NEWSYM currentobjptr, resd 1
NEWSYM curmosaicsz,   resd 1
NEWSYM extbgdone, resb 1
SECTION .text

SECTION .data
NEWSYM csprbit, db 1
NEWSYM csprprlft, db 0

SECTION .bss
NEWSYM drawn, resb 1
NEWSYM curbgpr, resb 1
SECTION .text

%macro drawpixel8b8x8 3
    or %1,%1
    jz %2
    add %1,dh
    mov [esi+%3],%1
%2
%endmacro

%macro drawpixel8b8x8win 3
    or %1,%1
    jz %2
    test byte[ebp+%3],0FFh
    jnz %2
    add %1,dh
    mov [esi+%3],%1
%2
%endmacro

SECTION .bss
NEWSYM winptrref, resd 1
SECTION .text

SECTION .bss
NEWSYM hirestiledat, resb 256
SECTION .text

ALIGN32

SECTION .bss
NEWSYM yadder,     resd 1
NEWSYM yrevadder,  resd 1
NEWSYM tempcach,   resd 1
NEWSYM temptile,   resd 1
NEWSYM bgptr,      resd 1
NEWSYM bgptrc,     resd 1
NEWSYM bgptrd,     resd 1
NEWSYM bgptrx1,    resd 1
NEWSYM bgptrx2,    resd 1
NEWSYM curvidoffset, resd 1
NEWSYM winon,      resd 1
NEWSYM bgofwptr,   resd 1
NEWSYM bgsubby,    resd 1
SECTION .text

SECTION .bss
NEWSYM temp,       resb 1
NEWSYM bshifter,   resb 1
NEWSYM a16x16xinc, resb 1
NEWSYM a16x16yinc, resb 1
SECTION .text
