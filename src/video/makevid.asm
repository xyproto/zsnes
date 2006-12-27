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

EXTSYM disableeffects,winl1,winl2,winbgdata,winr1,winr2,winspdata,winlogica
EXTSYM winenabm,winobjen,winlogicb,scrndis,scrnon,bgmode,bgtilesz,winbg1en
EXTSYM winenabs,bg1objptr,bg1ptr,bg1ptrb,bg1ptrc,bg1ptrd,bg1scrolx,bg1scroly
EXTSYM cachebg1,curbgofs1,curcolbg1,vcache2b,vcache4b,vcache8b
EXTSYM vidbuffer,bg3highst,cbitmode,colormodedef,ngptrdat2
EXTSYM colormodeofs,drawline16b,forceblnk,newengine8b,preparesprpr,scaddset
EXTSYM spritetablea,sprleftpr,vidbright,ForceNewGfxOff,curypos,drawmode7
EXTSYM mode7set,mosaicon,mosaicsz,sprleftpr1,sprleftpr2,sprleftpr3,sprlefttot
EXTSYM sprprifix,drawmode7extbg,interlval,drawmode7extbg2,sprclprio,sprpriodata
EXTSYM sprsingle,cachetile2b,cachetile4b,cachetile8b,vram,newengen,ofshvaladd
EXTSYM cachetile2b16x16,cachetile4b16x16,cachetile8b16x16,osm2dis,xtravbuf
EXTSYM bg3ptr,bg3scrolx,bg3scroly,vidmemch4,ofsmcptr,ofsmady,ofsmadx,yposngom
EXTSYM flipyposngom,ofsmtptr,ofsmmptr,ofsmcyps,bgtxadd,bg1ptrx,bg1ptry
EXTSYM bg1scrolx_m7,bg1scroly_m7,OMBGTestVal,Testval,cachesingle4bng,m7starty
EXTSYM ofsmtptrs,ofsmcptr2

%include "video/vidmacro.mac"

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

%macro decideonmode 0
    cmp bl,2
    je .yes4bit
    cmp bl,1
    je .yes2bit
    mov byte[bshifter],6
    mov edx,[vcache8b]
    jmp .skipbits
.yes4bit
    mov byte[bshifter],2
    mov edx,[vcache4b]
    shl eax,1
    jmp .skipbits
.yes2bit
    mov byte[bshifter],0
    shl eax,2
    mov edx,[vcache2b]
.skipbits
%endmacro

%macro procmode7 3
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

; is this macro even used?
%macro ClearWindowData 0
    mov edi,winbgdata+16
    xor eax,eax
    mov ecx,64
    rep stosd
%endmacro

NEWSYM makewindow
    ; upon entry, al = win enable bits
    cmp byte[disableeffects],1
    je near .finishwin
    mov bl,al
    and bl,00001010b
    cmp bl,00001010b
    je near makedualwin
    cmp bl,0
    je near .finishwin
    mov byte[winon],1
    mov ebx,[winl1]
    ; check if data matches previous sprite data
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
    cmp al,[pwinbgenab]
    jne .skipenab
    cmp ebx,[pwinbgtype]
    jne .skipenab2
    mov dword[cwinptr],winbgdata+16
    mov al,[winonbtype]
    mov [winon],al
    ret
.skipenab
    mov [pwinbgenab],al
    mov ebx,[winl1]
.skipenab2
    mov [pwinbgtype],ebx
    mov dl,[winl1]
    mov dh,[winr1]
    test al,00000010b
    jnz .win1
    mov dl,[winl2]
    mov dh,[winr2]
    shr al,2
.win1
    test al,01h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    mov byte[winon],0
    mov byte[winonbtype],0
    ret
.clip
    mov edi,winbgdata+16
    xor eax,eax
    ; start drawing 1's from 0 to left
    cmp dl,0
    je .nextdot2
.nextdot
    mov byte[edi+eax],0
    inc al
    cmp al,dl
    jb .nextdot         ; blah
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
    mov byte[winon],1
    mov byte[winonbtype],1
    mov dword[cwinptr],winbgdata+16
    ret
.outside
    cmp dl,dh
    jb .clip2
    mov byte[winon],0FFh
    mov byte[winonbtype],0FFh
    mov dword[cwinptr],winbgdata+16
    ret
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,winbgdata+16
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
    mov byte[winon],1
    mov byte[winonbtype],1
    mov dword[cwinptr],winbgdata+16
    ret
.finishwin
    ret

NEWSYM makedualwin
    mov ecx,ebp
    shl cl,1
    mov dl,[winlogica]
    shr dl,cl
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

NEWSYM makewindowsp
    mov al,[winobjen]
    mov byte[winonsp],0
    test dword[winenabm],1010h
    jz near .finishwin
    ; upon entry, al = win enable bits
    cmp byte[disableeffects],1
    je near .finishwin
    mov bl,al
    and bl,00001010b
    cmp bl,00001010b
    je near makedualwinsp
    cmp bl,0
    je near .finishwin
    mov byte[winonsp],1
    ; check if data matches previous data
    cmp al,[pwinspenab]
    jne .skipenab
    mov ebx,[winl1]
    cmp ebx,[pwinsptype]
    jne .skipenab2
    mov dword[cwinptr],winspdata+16
    mov al,[winonstype]
    mov [winonsp],al
    ret
.skipenab
    mov [pwinspenab],al
    mov ebx,[winl1]
.skipenab2
    mov [pwinsptype],ebx
    mov dl,[winl1]
    mov dh,[winr1]
    test al,00000010b
    jnz .win1
    mov dl,[winl2]
    mov dh,[winr2]
    shr al,2
.win1
    test al,01h
    jnz near .outside
    cmp dl,254
    je .clipped
    cmp dl,dh
    jb .clip
.clipped
    mov byte[winonsp],0
    mov byte[winonstype],0
    ret
.clip
    mov edi,winspdata+16
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
    mov byte[winonsp],1
    mov byte[winonstype],1
    mov dword[cwinptr],winspdata+16
    ret
.outside
    cmp dl,dh
    jb .clip2
    mov byte[winonsp],0FFh
    mov byte[winonstype],0FFh
    mov dword[cwinptr],winspdata+16
    ret
.clip2
    cmp dl,1
    ja .nooutclip
    cmp dh,254
    jae near .clipped
.nooutclip
    mov edi,winspdata+16
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
    mov byte[winonsp],1
    mov byte[winonstype],1
    mov dword[cwinptr],winspdata+16
    ret
.finishwin
    ret

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

NEWSYM procspritessub
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
    or cl,cl
    jz .nosprites
    call drawsprites
.nosprites
    ret

NEWSYM procspritesmain
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
    or cl,cl
    jz .nosprites
    call drawsprites
.nosprites
    ret

SECTION .bss
NEWSYM curbgnum, resb 1
SECTION .text

NEWSYM drawbackgrndsub
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
    shl eax,5
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
    call draw8x8
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
    jmp .noback
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x16
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
.noback
    ret

NEWSYM drawbackgrndmain
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
    test byte[bgtilesz],cl
    jnz .16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw8x8
    cmp byte[drawn],33
    jne .notalldrawn
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawn
    jmp .noback
.16x16
    mov ecx,[bg1yaddval+ebp*4]
    call draw16x16
    cmp byte[drawn],33
    jne .notalldrawnb
    mov al,[curbgnum]
    or [alreadydrawn],al
.notalldrawnb
.noback
    ret

NEWSYM procbackgrnd
    mov esi,[colormodeofs]
    mov bl,[esi+ebp]
    cmp bl,0
    je near .noback
    mov al,[curbgnum]
    mov ah,al
    test byte[scrndis],al
    jnz near .noback
    test [scrnon],ax
    jz near .noback
    push ebp
    shl ebp,6
    mov edi,cachebg1
    add edi,ebp
    pop ebp
    cmp bl,[curcolbg1+ebp]
    je .skipclearcache
    mov [curcolbg1+ebp],bl
    mov ax,[bg1ptr+ebp*2]
    mov [curbgofs1+ebp*2],ax
    call fillwithnothing
.skipclearcache
    xor eax,eax
    mov [curcolor],bl
    mov ax,[bg1objptr+ebp*2]
    decideonmode
    add edx,eax
    xor eax,eax
    mov [tempcach],edx
    xor edx,edx
    mov ax,[bg1objptr+ebp*2]
    mov [curtileptr],ax
    mov ax,[bg1ptr+ebp*2]
    mov [bgptr],ax
    cmp ax,[curbgofs1+ebp*2]
    je .skipclearcacheb
    mov [curbgofs1+ebp*2],ax
    call fillwithnothing
.skipclearcacheb
    mov ax,[bg1ptrb+ebp*2]
    mov [bgptrb],ax
    mov ax,[bg1ptrc+ebp*2]
    mov [bgptrc],ax
    mov ax,[bg1ptrd+ebp*2]
    mov [bgptrd],ax
    mov bl,[curbgnum]
    mov ax,[curypos]

    mov byte[curmosaicsz],1
    test byte[mosaicon],bl
    jz .nomos
    mov bl,[mosaicsz]
    cmp bl,0
    je .nomos
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
.nomos

    add ax,[bg1scroly+ebp*2]
    mov dx,[bg1scrolx+ebp*2]
    mov cl,[curbgnum]
    test byte[bgtilesz],cl
    jnz .16x16
    call proc8x8
    mov [bg1vbufloc+ebp*4],esi
    mov [bg1tdatloc+ebp*4],edi
    mov [bg1tdabloc+ebp*4],edx
    mov [bg1cachloc+ebp*4],ebx
    mov [bg1yaddval+ebp*4],ecx
    mov [bg1xposloc+ebp*4],eax
    ret
.16x16
    call proc16x16
    mov [bg1vbufloc+ebp*4],esi
    mov [bg1tdatloc+ebp*4],edi
    mov [bg1tdabloc+ebp*4],edx
    mov [bg1cachloc+ebp*4],ebx
    mov [bg1yaddval+ebp*4],ecx
    mov [bg1xposloc+ebp*4],eax
.noback
    ret

SECTION .bss
NEWSYM nextprimode, resb 1
NEWSYM cursprloc,   resd 1
NEWSYM curcolor,    resb 1
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
SECTION .text

NEWSYM fillwithnothing
    push edi
    xor eax,eax
    mov ecx,16
.loop
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .loop
    pop edi
    ret

SECTION .bss
NEWSYM bg3draw, resb 1
NEWSYM maxbr,   resb 1
SECTION .text

NEWSYM blanker
    ; calculate current video offset
    push ebx
    push esi
    xor ebx,ebx
    mov bx,[curypos]
    mov esi,ebx
    shl esi,8
    shl ebx,5
    add esi,ebx
    add esi,16
    add esi,[vidbuffer]
    mov bl,64
.next
    mov dword[esi],0
    add esi,4
    dec bl
    jnz .next
    pop esi
    pop ebx
    ret

ALIGN32
SECTION .bss
NEWSYM bg3high2, resd 1
NEWSYM cwinenabm, resd 1
SECTION .text

NEWSYM drawline
    mov al,[winenabs]
    mov [cwinenabm],al

    mov byte[bg3high2],0
    cmp byte[bgmode],1
    jne .nohigh
    mov al,[bg3highst]
    mov [bg3high2],al
.nohigh
    cmp byte[cbitmode],1
    je near drawline16b
    mov al,[vidbright]
    cmp al,[maxbr]
    jbe .nochange
    mov [maxbr],al
.nochange
    cmp byte[ForceNewGfxOff],0
    jne .nonewgfx
    cmp byte[newengen],1
    je near newengine8b
.nonewgfx
    cmp byte[forceblnk],0
    jne near blanker
    mov byte[alreadydrawn],0
;    cmp byte[curypos],70
;    jne .no
;    push ebx
;    mov bl,[winlogica]
;    mov [bg1sx],bl
;    pop ebx
;.no
    push ebx
    xor ebx,ebx
    mov bl,[bgmode]
    shl bl,2
    add ebx,colormodedef
    mov [colormodeofs],ebx
    pop ebx

    cmp byte[bgmode],7
    je near processmode7

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
    ; calculate current video offset
    xor ebx,ebx
    mov bx,[curypos]
    mov esi,ebx
    shl esi,8
    shl ebx,5
    add esi,ebx
    add esi,16
    add esi,[vidbuffer]
    mov [curvidoffset],esi
    ; clear video buffer
    mov edi,esi
    mov ecx,64
    xor eax,eax
    rep stosd
    ; do sprite windowing
    call makewindowsp
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
    ; clear registers
    xor eax,eax
    xor ecx,ecx
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
    ja near priority2
    test byte[scaddset],02h
    jz near .nosubsc
; draw backgrounds
    mov byte[curbgpr],0h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndsub
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub
    mov ebp,0
    call procspritessub
    mov byte[curbgpr],20h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndsub
; do background 3
    cmp byte[bg3high2],1
    je .bg3nothigh
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub
.bg3nothigh
    mov ebp,1
    call procspritessub
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub
    mov ebp,2
    call procspritessub
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub
    mov ebp,3
    call procspritessub
    cmp byte[bg3high2],1
    jne .bg3high
; do background 3
    mov byte[curbgpr],20h
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndsub
.bg3high

.nosubsc
    mov al,[winenabm]
    mov [cwinenabm],al
    mov byte[curbgpr],0h

; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain
; do background 3
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain
    mov ebp,0
    call procspritesmain
    mov byte[curbgpr],20h
; do background 4
    mov byte[curbgnum],08h
    mov ebp,03h
    call drawbackgrndmain
; do background 3
    cmp byte[bg3high2],1
    je .bg3nothighb
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain
.bg3nothighb
    mov ebp,1
    call procspritesmain
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain
    mov ebp,2
    call procspritesmain
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain
    mov ebp,3
    call procspritesmain
    cmp byte[bg3high2],1
    jne .bg3highb
; do background 3
    mov byte[curbgpr],20h
    mov byte[curbgnum],04h
    mov ebp,02h
    call drawbackgrndmain
.bg3highb
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
NEWSYM nodrawline
    ret

NEWSYM priority2
    test byte[scaddset],02h
    jz near .nosubsc
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub
    mov ebp,0
    call procspritessub
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub
    mov ebp,1
    call procspritessub
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndsub
    mov ebp,2
    call procspritessub
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndsub
    mov ebp,3
    call procspritessub
.nosubsc
    mov al,[winenabm]
    mov [cwinenabm],al
; do background 2
    mov byte[curbgpr],0h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain
    mov ebp,0
    call procspritesmain
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain
    mov ebp,1
    call procspritesmain
; do background 2
    mov byte[curbgpr],20h
    mov byte[curbgnum],02h
    mov ebp,01h
    call drawbackgrndmain
    mov ebp,2
    call procspritesmain
; do background 1
    mov byte[curbgnum],01h
    mov ebp,00h
    call drawbackgrndmain
    mov ebp,3
    call procspritesmain
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    xor eax,eax
    xor ecx,ecx
    ret

ALIGN32
SECTION .bss
NEWSYM tempbuffer, resd 33
NEWSYM currentobjptr, resd 1
NEWSYM curmosaicsz,   resd 1
NEWSYM extbgdone, resb 1
SECTION .text


NEWSYM processmode7
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
    shl esi,8
    shl ebx,5
    add esi,ebx
    add esi,16
    add esi,[vidbuffer]
    mov [curvidoffset],esi
    ; clear video buffer
    mov edi,esi
    mov ecx,64
    xor eax,eax
    rep stosd
    ; do sprite windowing
    call makewindowsp
    ; clear registers
    xor eax,eax
    xor ecx,ecx

    mov byte[extbgdone],0

    test byte[interlval],40h
    jz near .noback0
    test byte[scrndis],02h
    jnz near .noback0
    ; do background 1, extbg pr 0
    test word[scrnon],0202h
    jz near .noback0
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin0
    test word[winenabm],0100h
    jnz near .nobackwin0
    mov ebp,0
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback0
.nobackwin0
    mov byte[extbgdone],1
    procmode7extbg [bg1scroly_m7],[bg1scrolx_m7],1
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
    call drawsprites
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
    mov ebp,0
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback1
.nobackwin1
    procmode7 [bg1scroly_m7],[bg1scrolx_m7],1
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
    call drawsprites
.nosprites2

    test byte[interlval],40h
    jz near .noback0b
    cmp byte[extbgdone],0
    jne near .noback0b
    test byte[scrndis],02h
    jnz near .noback0b
    ; do background 1, extbg pr 0
    test word[scrnon],0101h
    jz near .noback0b
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin0b
    test word[winenabm],0100h
    jnz near .nobackwin0b
    mov ebp,0
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback0b
.nobackwin0b
    mov byte[extbgdone],1
    procmode7extbg [bg1scroly_m7],[bg1scrolx_m7],1
.noback0b

    test byte[interlval],40h
    jz near .noback2
    cmp byte[extbgdone],1
    jne near .noback2
    mov byte[winon],0
    test word[winenabm],0001h
    jz near .nobackwin2
    test word[winenabm],0100h
    jnz near .nobackwin2
    mov ebp,0
    mov al,[winbg1en]
    call makewindow
    cmp byte[winon],0FFh
    je near .noback2
.nobackwin2
    procmode7extbg2 [bg1scroly_m7],[bg1scrolx_m7],1
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
    call drawsprites
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
    call drawsprites
.nosprites4
    pop ebp
    pop edx
    pop ebx
    pop edi
    pop esi
    ret

;*******************************************************
; Processes & Draws 4-bit sprites
;*******************************************************

NEWSYM drawsprites
    cmp byte[sprprifix],1
    je near drawspritesprio
.returnfrompr
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawspriteswinon
.drawnowin
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov eax,[esi]
    test al,0Fh
    jz .skipa
    add al,ch
    mov [edi+ebx-8],al
.skipa
    test ah,0Fh
    jz .skipb
    add ah,ch
    mov [edi+ebx-7],ah
.skipb
    shr eax,16
    test al,0Fh
    jz .skipc
    add al,ch
    mov [edi+ebx-6],al
.skipc
    test ah,0Fh
    jz .skipd
    add ah,ch
    mov [edi+ebx-5],ah
.skipd
    mov eax,[esi+4]
    test al,0Fh
    jz .skipe
    add al,ch
    mov [edi+ebx-4],al
.skipe
    test ah,0Fh
    jz .skipf
    add ah,ch
    mov [edi+ebx-3],ah
.skipf
    shr eax,16
    test al,0Fh
    jz .skipg
    add al,ch
    mov [edi+ebx-2],al
.skipg
    test ah,0Fh
    jz .skiph
    add ah,ch
    mov [edi+ebx-1],ah
.skiph
    add edx,8
    mov esi,edx
    dec cl
    jnz .loopobj
    mov [currentobjptr],esi
    ret

.drawspriteflipx
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov eax,[esi]
    test al,0Fh
    jz .skipa2
    add al,ch
    mov [edi+ebx-1],al
.skipa2
    test ah,0Fh
    jz .skipb2
    add ah,ch
    mov [edi+ebx-2],ah
.skipb2
    shr eax,16
    test al,0Fh
    jz .skipc2
    add al,ch
    mov [edi+ebx-3],al
.skipc2
    test ah,0Fh
    jz .skipd2
    add ah,ch
    mov [edi+ebx-4],ah
.skipd2
    mov eax,[esi+4]
    test al,0Fh
    jz .skipe2
    add al,ch
    mov [edi+ebx-5],al
.skipe2
    test ah,0Fh
    jz .skipf2
    add ah,ch
    mov [edi+ebx-6],ah
.skipf2
    shr eax,16
    test al,0Fh
    jz .skipg2
    add al,ch
    mov [edi+ebx-7],al
.skipg2
    test ah,0Fh
    jz .skiph2
    add ah,ch
    mov [edi+ebx-8],ah
.skiph2
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawspriteswinon
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov eax,[esi]
    test al,0Fh
    jz .skipa
    cmp byte[winspdata+ebx-8+16],0
    jne .skipa
    add al,ch
    mov [edi+ebx-8],al
.skipa
    test ah,0Fh
    jz .skipb
    cmp byte[winspdata+ebx-7+16],0
    jne .skipb
    add ah,ch
    mov [edi+ebx-7],ah
.skipb
    shr eax,16
    test al,0Fh
    jz .skipc
    cmp byte[winspdata+ebx-6+16],0
    jne .skipc
    add al,ch
    mov [edi+ebx-6],al
.skipc
    test ah,0Fh
    jz .skipd
    cmp byte[winspdata+ebx-5+16],0
    jne .skipd
    add ah,ch
    mov [edi+ebx-5],ah
.skipd
    mov eax,[esi+4]
    test al,0Fh
    jz .skipe
    cmp byte[winspdata+ebx-4+16],0
    jne .skipe
    add al,ch
    mov [edi+ebx-4],al
.skipe
    test ah,0Fh
    jz .skipf
    cmp byte[winspdata+ebx-3+16],0
    jne .skipf
    add ah,ch
    mov [edi+ebx-3],ah
.skipf
    shr eax,16
    test al,0Fh
    jz .skipg
    cmp byte[winspdata+ebx-2+16],0
    jne .skipg
    add al,ch
    mov [edi+ebx-2],al
.skipg
    test ah,0Fh
    jz .skiph
    cmp byte[winspdata+ebx-1+16],0
    jne .skiph
    add ah,ch
    mov [edi+ebx-1],ah
.skiph
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

.drawspriteflipx
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    mov eax,[esi]
    test al,0Fh
    jz .skipa2
    cmp byte[winspdata+ebx-1+16],0
    jne .skipa2
    add al,ch
    mov [edi+ebx-1],al
.skipa2
    test ah,0Fh
    jz .skipb2
    cmp byte[winspdata+ebx-2+16],0
    jne .skipb2
    add ah,ch
    mov [edi+ebx-2],ah
.skipb2
    shr eax,16
    test al,0Fh
    jz .skipc2
    cmp byte[winspdata+ebx-3+16],0
    jne .skipc2
    add al,ch
    mov [edi+ebx-3],al
.skipc2
    test ah,0Fh
    jz .skipd2
    cmp byte[winspdata+ebx-4+16],0
    jne .skipd2
    add ah,ch
    mov [edi+ebx-4],ah
.skipd2
    mov eax,[esi+4]
    test al,0Fh
    jz .skipe2
    cmp byte[winspdata+ebx-5+16],0
    jne .skipe2
    add al,ch
    mov [edi+ebx-5],al
.skipe2
    test ah,0Fh
    jz .skipf2
    cmp byte[winspdata+ebx-6+16],0
    jne .skipf2
    add ah,ch
    mov [edi+ebx-6],ah
.skipf2
    shr eax,16
    test al,0Fh
    jz .skipg2
    cmp byte[winspdata+ebx-7+16],0
    jne .skipg2
    add al,ch
    mov [edi+ebx-7],al
.skipg2
    test ah,0Fh
    jz .skiph2
    cmp byte[winspdata+ebx-8+16],0
    jne .skiph2
    add ah,ch
    mov [edi+ebx-8],ah
.skiph2
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    mov [currentobjptr],esi
    ret

NEWSYM drawspritesprio
    cmp byte[sprclprio+ebp],0
    je near .endobj
    test byte[cwinenabm],10h
    jz .drawnowin
    cmp byte[winonsp],0
    jne near drawspritespriowinon
.drawnowin
    cmp dword[sprsingle],1
    je near .drawsingle
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,ebp
    jne near .notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,ebp
    jne near .notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra
    pop edx
    add edx,8
    mov esi,edx
    dec cl
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
    pop edx
    add edx,8
    mov esi,edx
    dec cl
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
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    ret
.drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprb
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    ret

NEWSYM drawspritespriowinon
    cmp dword[sprsingle],1
    je near .drawsingle
    mov esi,[currentobjptr]
    mov edi,[curvidoffset]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,ebp
    jne near .notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpraw
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,ebp
    jne near .notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpraw
    pop edx
    add edx,8
    mov esi,edx
    dec cl
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
    pop edx
    add edx,8
    mov esi,edx
    dec cl
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
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa sprdrawprbw
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    ret
.drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf sprdrawprbw
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    ret

SECTION .data
NEWSYM prfixobjl, db 0
NEWSYM csprbit, db 1
NEWSYM csprprlft, db 0
SECTION .text
;*******************************************************
; Processes & Draws 8x8 tiles in 2, 4, & 8 bit mode
;*******************************************************
NEWSYM proc8x8
    cmp byte[bgmode],5
    je near proc16x8
    ; ax = # of rows down
    mov ebx,eax
    shr eax,3
    and eax,63
    and ebx,07h
    cmp byte[edi+eax],0
    jne .nocachereq
;.docache
;    cmp byte[ccud],0
;    jne .nocachereq
    mov byte[edi+eax],1
    cmp byte[curcolor],2
    jne .no4b
    ; cache 4-bit
    call cachetile4b
    jmp .nocachereq
.no4b
    cmp byte[curcolor],1
    je .2b
    ; cache 8-bit
    call cachetile8b
    jmp .nocachereq
.2b
    ; cache 2-bit
    call cachetile2b
.nocachereq
    test edx,0100h
    jz .tilexa
    test al,20h
    jz .tileya
    ; bgptrd/bgptrc
    mov ecx,[bgptrd]
    mov [bgptrx1],ecx
    mov ecx,[bgptrc]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya
    ; bgptrb/bgptra
    mov ecx,[bgptrb]
    mov [bgptrx1],ecx
    mov ecx,[bgptr]
    mov [bgptrx2],ecx
    jmp .skiptile
.tilexa
    test al,20h
    jz .tileya2
    ; bgptrc/bgptrd
    mov ecx,[bgptrc]
    mov [bgptrx1],ecx
    mov ecx,[bgptrd]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya2
    ; bgptra/bgptrb
    mov ecx,[bgptr]
    mov [bgptrx1],ecx
    mov ecx,[bgptrb]
    mov [bgptrx2],ecx
.skiptile
    ; set up edi & yadder to point to tile data
    shl ebx,3
    mov [yadder],ebx
    and al,1Fh
    mov edi,[vram]
    mov ebx,eax
    shl ebx,6
    mov eax,[bgptrx1]
    add edi,ebx
    mov [temptile],edi
    add edi,eax
    ; dx = # of columns right
    ; cx = bgxlim
    mov eax,edx
    shr edx,3
    mov bl,[curypos]
    and edx,1Fh
    mov [temp],dl
    and eax,07h
    add dl,dl
    add edi,edx

    mov esi,eax
    mov ebx,[tempcach]
    mov edx,[temptile]
    mov eax,[bgptrx2]
    and eax,0FFFFh
    add edx,eax
    mov al,[temp]
    mov ecx,[yadder]
    mov ah,[bshifter]
    ; fill up tempbuffer with pointer #s that point to cached video mem
    ; to calculate pointer, get first byte
    ; esi = pointer to video buffer
    ; edi = pointer to tile data
    ; ebx = cached memory
    ; ecx = y adder
    ; edx = secondary tile pointer
    ; al = current x position
    ret

NEWSYM proc16x8
    ; ax = # of rows down
    mov ebx,eax
    shr eax,3
    and ebx,07h
    and eax,63
    cmp byte[edi+eax],0
    jne .nocachereq
;    cmp byte[ccud],0
;    jne .nocachereq
    mov byte[edi+eax],1
    cmp byte[curcolor],2
    jne .no4b
    ; cache 4-bit
    call cachetile4b16x16
    jmp .nocachereq
.no4b
    cmp byte[curcolor],1
    je .2b
    ; cache 8-bit
    call cachetile8b16x16
    jmp .nocachereq
.2b
    ; cache 2-bit
    call cachetile2b16x16
.nocachereq
    test edx,0100h
    jz .tilexa
    test al,20h
    jz .tileya
    ; bgptrd/bgptrc
    mov ecx,[bgptrd]
    mov [bgptrx1],ecx
    mov ecx,[bgptrc]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya
    ; bgptrb/bgptra
    mov ecx,[bgptrb]
    mov [bgptrx1],ecx
    mov ecx,[bgptr]
    mov [bgptrx2],ecx
    jmp .skiptile
.tilexa
    test al,20h
    jz .tileya2
    ; bgptrc/bgptrd
    mov ecx,[bgptrc]
    mov [bgptrx1],ecx
    mov ecx,[bgptrd]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya2
    ; bgptra/bgptrb
    mov ecx,[bgptr]
    mov [bgptrx1],ecx
    mov ecx,[bgptrb]
    mov [bgptrx2],ecx
.skiptile
    ; set up edi & yadder to point to tile data
    shl ebx,3
    mov [yadder],ebx
    and al,1Fh
    mov edi,[vram]
    mov ebx,eax
    shl ebx,6
    mov eax,[bgptrx1]
    add edi,ebx
    mov [temptile],edi
    add edi,eax
    ; dx = # of columns right
    ; cx = bgxlim
    mov eax,edx
    shr edx,3
    mov bl,[curypos]
    and edx,1Fh
    mov [temp],dl
    and eax,07h
    add dl,dl
    add edi,edx

    mov esi,eax
    mov ebx,[tempcach]
    mov edx,[temptile]
    mov eax,[bgptrx2]
    and eax,0FFFFh
    add edx,eax
    mov al,[temp]
    mov ecx,[yadder]
    mov ah,[bshifter]
    ; fill up tempbuffer with pointer #s that point to cached video mem
    ; to calculate pointer, get first byte
    ; esi = pointer to video buffer
    ; edi = pointer to tile data
    ; ebx = cached memory
    ; ecx = y adder
    ; edx = secondary tile pointer
    ; al = current x position
    ret

SECTION .bss
NEWSYM drawn, resb 1
NEWSYM curbgpr, resb 1    ; 00h = low priority, 20h = high priority
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

NEWSYM draw8x8
    cmp byte[osm2dis],1
    je .osm2dis
    cmp byte[bgmode],2
    je near draw8x8offset
.osm2dis
    cmp byte[bgmode],5
    jae near draw16x8
    mov [temp],al
    mov [bshifter],ah
    mov eax,esi
    mov [yadder],ecx
    mov [tempcach],ebx
    mov ebx,56
    sub ebx,ecx
    mov [yrevadder],ebx
.retfromoffset
    ; esi = pointer to video buffer
    mov esi,[cwinptr]
    sub esi,eax
    mov [winptrref],esi
    mov esi,[curvidoffset]
    sub esi,eax           ; esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
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
    jne near draw8x8winon
.domosaic
    mov ch,33
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
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1
    drawpixel8b8x8 al, .loopd1, 0
    drawpixel8b8x8 ah, .loopd2, 1
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3, 2
    drawpixel8b8x8 ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8 al, .loopd5, 4
    drawpixel8b8x8 ah, .loopd6, 5
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7, 6
    drawpixel8b8x8 ah, .loopd8, 7
.skiploop2
.hprior
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8 al, .loopd1b, 7
    drawpixel8b8x8 ah, .loopd2b, 6
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3b, 5
    drawpixel8b8x8 ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8 al, .loopd5b, 3
    drawpixel8b8x8 ah, .loopd6b, 2
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7b, 1
    drawpixel8b8x8 ah, .loopd8b, 0
.skiploop2b
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw2
    ret

NEWSYM draw8x8winon
    mov ch,33
    mov byte[drawn],0
    mov ebp,[winptrref]
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
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1
    drawpixel8b8x8win al, .loopd1, 0
    drawpixel8b8x8win ah, .loopd2, 1
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3, 2
    drawpixel8b8x8win ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8win al, .loopd5, 4
    drawpixel8b8x8win ah, .loopd6, 5
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7, 6
    drawpixel8b8x8win ah, .loopd8, 7
.skiploop2
.hprior
    add esi,8
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8win al, .loopd1b, 7
    drawpixel8b8x8win ah, .loopd2b, 6
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3b, 5
    drawpixel8b8x8win ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8win al, .loopd5b, 3
    drawpixel8b8x8win ah, .loopd6b, 2
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7b, 1
    drawpixel8b8x8win ah, .loopd8b, 0
.skiploop2b
    add esi,8
    add ebp,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    ret

SECTION .bss
NEWSYM alttile, resb 1
NEWSYM hirestiledat, resb 256
SECTION .text

NEWSYM draw16x8
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
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
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
    ;mov dword[bgofwptr],vcache8b+65536
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
    jne near draw16x8winon
.domosaic
    cmp byte[res512switch],0
    jne near draw16x8b
    mov ch,33
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
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1
    drawpixel8b8x8 al, .loopd1, 0
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3, 1
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8 al, .loopd5, 2
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7, 3
.skiploop2
    add ebx,64
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1c
    drawpixel8b8x8 al, .loopd1c, 4
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3c, 5
.skiploop1c
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2c
    drawpixel8b8x8 al, .loopd5c, 6
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7c, 7
.skiploop2c
.hprior
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8 ah, .loopd1b, 7
    mov eax,[ebx+2]
    drawpixel8b8x8 ah, .loopd3b, 6
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8 ah, .loopd5b, 5
    mov eax,[ebx+6]
    drawpixel8b8x8 ah, .loopd7b, 4
.skiploop2b
    add ebx,64
    mov eax,[ebx]
    or eax,eax
    je .skiploop1d
    drawpixel8b8x8 ah, .loopd1d, 3
    mov eax,[ebx+2]
    drawpixel8b8x8 ah, .loopd3d, 2
.skiploop1d
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2d
    drawpixel8b8x8 ah, .loopd5d, 1
    mov eax,[ebx+6]
    drawpixel8b8x8 ah, .loopd7d, 0
.skiploop2d
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    cmp byte[winon],0
    jne near dowindow
.nodraw2
    ret

NEWSYM draw16x8b
    mov ch,33
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
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1
    drawpixel8b8x8 ah, .loopd1, 0
    mov eax,[ebx+2]
    drawpixel8b8x8 ah, .loopd3, 1
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8 ah, .loopd5, 2
    mov eax,[ebx+6]
    drawpixel8b8x8 ah, .loopd7, 3
.skiploop2
    add ebx,64
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1c
    drawpixel8b8x8 ah, .loopd1c, 4
    mov eax,[ebx+2]
    drawpixel8b8x8 ah, .loopd3c, 5
.skiploop1c
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2c
    drawpixel8b8x8 ah, .loopd5c, 6
    mov eax,[ebx+6]
    drawpixel8b8x8 ah, .loopd7c, 7
.skiploop2c
.hprior
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8 al, .loopd2b, 7
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd4b, 6
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8 al, .loopd5b, 5
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7b, 4
.skiploop2b
    add ebx,64
    mov eax,[ebx]
    or eax,eax
    je .skiploop1d
    drawpixel8b8x8 al, .loopd1d, 3
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3d, 2
.skiploop1d
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2d
    drawpixel8b8x8 al, .loopd5d, 1
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7d, 0
.skiploop2d
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw2
    ret

NEWSYM draw16x8winon
    cmp byte[res512switch],0
    jne near draw16x8bwinon
    mov ch,33
    mov ebp,[winptrref]
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
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1
    drawpixel8b8x8win al, .loopd1, 0
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3, 1
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8win al, .loopd5, 2
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7, 3
.skiploop2
    add ebx,64
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1c
    drawpixel8b8x8win al, .loopd1c, 4
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3c, 5
.skiploop1c
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2c
    drawpixel8b8x8win al, .loopd5c, 6
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7c, 7
.skiploop2c
.hprior
    add ebp,8
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8win ah, .loopd1b, 7
    mov eax,[ebx+2]
    drawpixel8b8x8win ah, .loopd3b, 6
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8win ah, .loopd5b, 5
    mov eax,[ebx+6]
    drawpixel8b8x8win ah, .loopd7b, 4
.skiploop2b
    add ebx,64
    mov eax,[ebx]
    or eax,eax
    je .skiploop1d
    drawpixel8b8x8win ah, .loopd1d, 3
    mov eax,[ebx+2]
    drawpixel8b8x8win ah, .loopd3d, 2
.skiploop1d
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2d
    drawpixel8b8x8win ah, .loopd5d, 1
    mov eax,[ebx+6]
    drawpixel8b8x8win ah, .loopd7d, 0
.skiploop2d
    add ebp,8
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    ret

NEWSYM draw16x8bwinon
    mov ch,33
    mov ebp,[winptrref]
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
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1
    drawpixel8b8x8win ah, .loopd1, 0
    mov eax,[ebx+2]
    drawpixel8b8x8win ah, .loopd3, 1
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8win ah, .loopd5, 2
    mov eax,[ebx+6]
    drawpixel8b8x8win ah, .loopd7, 3
.skiploop2
    add ebx,64
    ; Start loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1c
    drawpixel8b8x8win ah, .loopd1c, 4
    mov eax,[ebx+2]
    drawpixel8b8x8win ah, .loopd3c, 5
.skiploop1c
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2c
    drawpixel8b8x8win ah, .loopd5c, 6
    mov eax,[ebx+6]
    drawpixel8b8x8win ah, .loopd7c, 7
.skiploop2c
.hprior
    add ebp,8
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc2
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    mov eax,[ebx]
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8win al, .loopd2b, 7
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd4b, 6
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8win al, .loopd5b, 5
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7b, 4
.skiploop2b
    add ebx,64
    mov eax,[ebx]
    or eax,eax
    je .skiploop1d
    drawpixel8b8x8win al, .loopd1d, 3
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3d, 2
.skiploop1d
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2d
    drawpixel8b8x8win al, .loopd5d, 1
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7d, 0
.skiploop2d
    add ebp,8
    add esi,8
    inc dl
    cmp dl,20h
    jne .loopc
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    ret

SECTION .data
NEWSYM extraleft, db 0,0,0,1,0,1,2,2,0,2,3,1,2,4,2,1
SECTION .text

NEWSYM domosaic
    mov esi,xtravbuf+16
    mov edi,[curvidoffset]
    xor ecx,ecx
    mov cl,dh
    mov dl,dh
    sub dl,[extraleft+ecx]
    mov ecx,256
    mov al,[esi]
    cmp byte[winon],0
    jne near domosaicwin
    test al,0FFh
    jz .zeroloop
.loopm
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jz .doneloop
    dec dl
    jnz .loopm
    mov al,[esi]
    mov dl,dh
    test al,0FFh
    jnz .loopm
.zeroloop
    inc esi
    inc edi
    dec ecx
    jz .doneloop
    dec dl
    jnz .zeroloop
    mov al,[esi]
    mov dl,dh
    test al,0FFh
    jnz .loopm
    jmp .zeroloop
.doneloop
    ret

NEWSYM domosaicwin
    mov ebp,[winptrref]
    test al,0FFh
    jz .zeroloop
.loopm
    cmp byte[ebp],0
    jne .nozero2
    mov [edi],al
.nozero2
    inc esi
    inc edi
    inc ebp
    dec ecx
    jz .doneloop
    dec dl
    jnz .loopm
    mov al,[esi]
    mov dl,dh
    test al,0FFh
    jnz .loopm
.zeroloop
    inc esi
    inc edi
    inc ebp
    dec ecx
    jz .doneloop
    dec dl
    jnz .zeroloop
    mov al,[esi]
    mov dl,dh
    test al,0FFh
    jnz .loopm
    jmp .zeroloop
.doneloop
    ret

NEWSYM dowindow
    mov ebx,windowdata
    mov esi,xtravbuf+16
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
    mov eax,[esi+edx]
    test al,0FFh
    jz .nocopy
    mov [edi+edx],al
.nocopy
    inc dl
    dec cl
    jz .procnext
    test ah,0FFh
    jz .nocopyb
    mov [edi+edx],ah
.nocopyb
    inc dl
    dec cl
    jz .procnext
    shr eax,16
    test al,0FFh
    jz .nocopyc
    mov [edi+edx],al
.nocopyc
    inc dl
    dec cl
    jz .procnext
    test ah,0FFh
    jz .nocopyd
    mov [edi+edx],ah
.nocopyd
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

ALIGN32

SECTION .bss
NEWSYM yadder,     resd 1
NEWSYM yrevadder,  resd 1
NEWSYM tempcach,   resd 1        ; points to cached memory
NEWSYM temptile,   resd 1        ; points to the secondary video pointer
NEWSYM bgptr,      resd 1
NEWSYM bgptrb,     resd 1
NEWSYM bgptrc,     resd 1
NEWSYM bgptrd,     resd 1
NEWSYM bgptrx1,    resd 1
NEWSYM bgptrx2,    resd 1
NEWSYM curvidoffset, resd 1
NEWSYM winon,      resd 1
NEWSYM bgofwptr,   resd 1
NEWSYM bgsubby,    resd 1
SECTION .text


NEWSYM draw8x8offset
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
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
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
    jne near draw8x8winon
.domosaic
    mov byte[offsettilel],33
    mov byte[drawn],0
    mov dl,[temp]
.loopa
    mov ax,[edi]
    mov dh,ah
    add edi,2
;    add dword[offsetptrb],2
;    add dword[offsetcedi],2
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
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1
    drawpixel8b8x8 al, .loopd1, 0
    drawpixel8b8x8 ah, .loopd2, 1
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3, 2
    drawpixel8b8x8 ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8 al, .loopd5, 4
    drawpixel8b8x8 ah, .loopd6, 5
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7, 6
    drawpixel8b8x8 ah, .loopd8, 7
.skiploop2
.hprior
    procoffsetmode
    add esi,8
    dec byte[offsettilel]
    jnz near .loopa
    cmp byte[drawn],0
    je .nodraw
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8 al, .loopd1b, 7
    drawpixel8b8x8 ah, .loopd2b, 6
    mov eax,[ebx+2]
    drawpixel8b8x8 al, .loopd3b, 5
    drawpixel8b8x8 ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8 al, .loopd5b, 3
    drawpixel8b8x8 ah, .loopd6b, 2
    mov eax,[ebx+6]
    drawpixel8b8x8 al, .loopd7b, 1
    drawpixel8b8x8 ah, .loopd8b, 0
.skiploop2b
    procoffsetmode
    add esi,8
    dec byte[offsettilel]
    jnz near .loopa
    xor eax,eax
    cmp byte[drawn],0
    je .nodraw2
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
.nodraw2
    ret

NEWSYM draw8x8winonoffset
    mov ch,33
    mov byte[drawn],0
    mov ebp,[winptrref]
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
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1
    drawpixel8b8x8win al, .loopd1, 0
    drawpixel8b8x8win ah, .loopd2, 1
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3, 2
    drawpixel8b8x8win ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2
    drawpixel8b8x8win al, .loopd5, 4
    drawpixel8b8x8win ah, .loopd6, 5
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7, 6
    drawpixel8b8x8win ah, .loopd8, 7
.skiploop2
.hprior
    procoffsetmode
    add esi,8
    add ebp,8
    dec ch
    jnz near .loopa
    xor eax,eax
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    or eax,eax
    je .skiploop1b
    drawpixel8b8x8win al, .loopd1b, 7
    drawpixel8b8x8win ah, .loopd2b, 6
    mov eax,[ebx+2]
    drawpixel8b8x8win al, .loopd3b, 5
    drawpixel8b8x8win ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    or eax,eax
    je .skiploop2b
    drawpixel8b8x8win al, .loopd5b, 3
    drawpixel8b8x8win ah, .loopd6b, 2
    mov eax,[ebx+6]
    drawpixel8b8x8win al, .loopd7b, 1
    drawpixel8b8x8win ah, .loopd8b, 0
.skiploop2b
    procoffsetmode
    add esi,8
    add ebp,8
    dec ch
    jnz near .loopa
    xor eax,eax
    ret

ALIGN32

SECTION .bss
NEWSYM offsetmodeptr, resd 1
NEWSYM offsetptra,    resd 1
NEWSYM offsetptrb,    resd 1
NEWSYM prevtempcache, resd 1
NEWSYM prevoffsetdat, resd 1
NEWSYM offsetenab,    resd 1
NEWSYM offsettilel,   resd 1
NEWSYM offsetrevval,  resd 1
NEWSYM posyscroll,    resd 1
NEWSYM offsetmcol,    resd 1
NEWSYM offsetmshl,    resd 1
NEWSYM offsetmptr,    resd 1
NEWSYM offsetmtst,    resd 1
NEWSYM offsetmclr,    resd 1
NEWSYM offsetcedi,    resd 1
SECTION .text

;*******************************************************
; Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
;*******************************************************

NEWSYM proc16x16
    ; ax = # of rows down
    xor ebx,ebx
    mov ebx,eax
    and ebx,07h
    mov byte[a16x16yinc],0
    test eax,08h
    jz .noincb
    mov byte[a16x16yinc],1
.noincb
    shr eax,4
    and eax,63
    cmp byte[edi+eax],0
    jne .nocachereq
    mov byte[edi+eax],1
    cmp byte[curcolor],2
    jne .no4b
    ; cache 4-bit
    call cachetile4b16x16
    jmp .nocachereq
.no4b
    cmp byte[curcolor],1
    je .2b
    ; cache 8-bit
    call cachetile8b16x16
    jmp .nocachereq
.2b
    ; cache 2-bit
    call cachetile2b16x16
.nocachereq
    test edx,0200h
    jz .tilexa
    test eax,20h
    jz .tileya
    ; bgptrd/bgptrc
    mov ecx,[bgptrd]
    mov [bgptrx1],ecx
    mov ecx,[bgptrc]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya
    ; bgptrb/bgptra
    mov ecx,[bgptrb]
    mov [bgptrx1],ecx
    mov ecx,[bgptr]
    mov [bgptrx2],ecx
    jmp .skiptile
.tilexa
    test ax,20h
    jz .tileya2
    ; bgptrc/bgptrd
    mov ecx,[bgptrc]
    mov [bgptrx1],ecx
    mov ecx,[bgptrd]
    mov [bgptrx2],ecx
    jmp .skiptile
.tileya2
    ; bgptra/bgptrb
    mov ecx,[bgptr]
    mov [bgptrx1],ecx
    mov ecx,[bgptrb]
    mov [bgptrx2],ecx
.skiptile
    and eax,1Fh
    shl ebx,3
    mov [yadder],ebx
    ; set up edi to point to tile data
    mov edi,[vram]
    mov ebx,eax
    shl ebx,6
    mov ax,[bgptrx1]
    add edi,ebx
    mov [temptile],edi
    add edi,eax
    ; dx = # of columns right
    ; cx = bgxlim
    mov eax,edx
    mov byte[a16x16xinc],0
    test edx,08h
    jz .noincd
    mov byte[a16x16xinc],1
.noincd
    shr edx,4
    and edx,1Fh
    mov [temp],dl
    and eax,07h
    shl dl,1
    xor ebx,ebx
    add edi,edx

    mov esi,eax
    mov ebx,[tempcach]
    xor eax,eax
    mov edx,[temptile]
    mov ax,[bgptrx2]
    add edx,eax
    mov ecx,[yadder]
    mov eax,[temp]
    ; fill up tempbuffer with pointer #s that point to cached video mem
    ; to calculate pointer, get first byte
    ; esi = pointer to video buffer
    ; edi = pointer to tile data
    ; ebx = cached memory
    ; ecx = y adder
    ; edx = secondary tile pointer
    ; al = current x position
    ret

NEWSYM draw16x16
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
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
    sub esi,eax
.nomosaic
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near draw16x16winon
.domosaic
    mov ch,33
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

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    cmp eax,0
    je .skiploop1
    drawpixel8b8x8 al, .loopd1, 0
    drawpixel8b8x8 ah, .loopd2, 1
    shr eax,16
    drawpixel8b8x8 al, .loopd3, 2
    drawpixel8b8x8 ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    cmp eax,0
    je .skiploop2
    drawpixel8b8x8 al, .loopd5, 4
    drawpixel8b8x8 ah, .loopd6, 5
    shr eax,16
    drawpixel8b8x8 al, .loopd7, 6
    drawpixel8b8x8 ah, .loopd8, 7
.skiploop2
.hprior
    add esi,8
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc dl
.noincrb2
    cmp dl,20h
    jne .loopc2
    xor dl,dl
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    cmp eax,0
    je .skiploop1b
    drawpixel8b8x8 al, .loopd1b, 7
    drawpixel8b8x8 ah, .loopd2b, 6
    shr eax,16
    drawpixel8b8x8 al, .loopd3b, 5
    drawpixel8b8x8 ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    cmp eax,0
    je .skiploop2b
    drawpixel8b8x8 al, .loopd5b, 3
    drawpixel8b8x8 ah, .loopd6b, 2
    shr eax,16
    drawpixel8b8x8 al, .loopd7b, 1
    drawpixel8b8x8 ah, .loopd8b, 0
.skiploop2b
    add esi,8
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc dl
.noincrb
    cmp dl,20h
    jne .loopc
    xor dl,dl
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret

SECTION .bss
.yadd      resw 1
.yflipadd  resw 1
SECTION .text

NEWSYM draw16x16winon
.domosaic
    mov ch,33
    mov ebp,[winptrref]
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
    add ax,word[draw16x16.yadd]
    jmp .yesyflip
.noyflip
    add ax,word[draw16x16.yflipadd]
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

    ; Begin Normal Loop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    cmp eax,0
    je .skiploop1
    drawpixel8b8x8win al, .loopd1, 0
    drawpixel8b8x8win ah, .loopd2, 1
    shr eax,16
    drawpixel8b8x8win al, .loopd3, 2
    drawpixel8b8x8win ah, .loopd4, 3
.skiploop1
    mov eax,[ebx+4]
    cmp eax,0
    je .skiploop2
    drawpixel8b8x8win al, .loopd5, 4
    drawpixel8b8x8win ah, .loopd6, 5
    shr eax,16
    drawpixel8b8x8win al, .loopd7, 6
    drawpixel8b8x8win ah, .loopd8, 7
.skiploop2
.hprior
    add ebp,8
    add esi,8
    test byte[a16x16xinc],01h
    jnz .noincrb2
    inc dl
.noincrb2
    cmp dl,20h
    jne .loopc2
    xor dl,dl
    mov edi,[temptile]
.loopc2
    dec ch
    jnz near .loopa
    ret

    ; reversed loop
.rloop
    mov cl,[bshifter]
    and dh,1Ch
    mov eax,[ebx]
    shl dh,cl                    ; process palette # (bits 10-12)
    add dh,[bgcoloradder]
    ; Start fast loop
    cmp eax,0
    je .skiploop1b
    drawpixel8b8x8win al, .loopd1b, 7
    drawpixel8b8x8win ah, .loopd2b, 6
    shr eax,16
    drawpixel8b8x8win al, .loopd3b, 5
    drawpixel8b8x8win ah, .loopd4b, 4
.skiploop1b
    mov eax,[ebx+4]
    cmp eax,0
    je .skiploop2b
    drawpixel8b8x8win al, .loopd5b, 3
    drawpixel8b8x8win ah, .loopd6b, 2
    shr eax,16
    drawpixel8b8x8win al, .loopd7b, 1
    drawpixel8b8x8win ah, .loopd8b, 0
.skiploop2b
    add ebp,8
    add esi,8
    test byte[a16x16xinc],01h
    jnz .noincrb
    inc dl
.noincrb
    cmp dl,20h
    jne .loopc
    xor dl,dl
    mov edi,[temptile]
.loopc
    dec ch
    jnz near .loopa
    ret

SECTION .bss
NEWSYM temp,       resb 1
NEWSYM bshifter,   resb 1
NEWSYM a16x16xinc, resb 1
NEWSYM a16x16yinc, resb 1
SECTION .text
