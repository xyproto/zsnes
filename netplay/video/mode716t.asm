;Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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

EXTSYM cwinptr
EXTSYM coladdr,curmosaicsz,curvidoffset,domosaic16b,mode7A,drawmode7dcolor
EXTSYM mode7B,mode7C,mode7D,mode7X0,mode7Y0,mode7set,mode7tab,DoTransp
EXTSYM pal16b,pal16bcl,pal16bxcl,scaddtype,scrnon,transpbuf,drawmode716b
EXTSYM vesa2_clbit,vram,vrama,winon,xtravbuf,winptrref,scaddset
EXTSYM fulladdtab

%include "video/mode7.mac"





;*******************************************************
; Processes & Draws Mode 7
;*******************************************************


%macro mode7halfadd 0
    or dl,dl
    jz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bcl+edx*4]
    or cx,cx
    jz %%noadd
    and edx,[vesa2_clbit]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
%%noadd
    mov [esi],dx
    xor ecx,ecx
    xor edx,edx
%%nodraw
    add esi,2
    add ebp,2
%endmacro

%macro mode7fulladd 0
    or dl,dl
    jz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bcl+edx*4]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
    mov edx,[fulladdtab+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodraw
    add esi,2
    add ebp,2
%endmacro

%macro mode7fullsub 0
    or dl,dl
    jz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bxcl+edx*4]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
    mov edx,[fulladdtab+edx*2]
    xor edx,0FFFFh
    mov [esi],dx
    xor edx,edx
%%nodraw
    add esi,2
    add ebp,2
%endmacro

%macro mode7mainsub 0
    or dl,dl
    jz %%nodraw
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
    mov [ebp],cx
%%nodraw
    add esi,2
    add ebp,2
%endmacro

%macro mode7halfaddwinon 0
    mov ecx,[cwinptr2]
    or dl,dl
    jz %%nodraw
    test byte[ecx],0FFh
    jnz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bcl+edx*4]
    or cx,cx
    je %%noadd
    and edx,[vesa2_clbit]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
%%noadd
    mov [esi],dx
    xor edx,edx
%%nodraw
    xor ecx,ecx
    inc dword[cwinptr2]
    add esi,2
    add ebp,2
%endmacro

%macro mode7fulladdwinon 0
    mov ecx,[cwinptr2]
    or dl,dl
    jz %%nodraw
    test byte[ecx],0FFh
    jnz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bcl+edx*4]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
    mov edx,[fulladdtab+edx*2]
    mov [esi],dx
    xor edx,edx
%%nodraw
    inc dword[cwinptr2]
    xor ecx,ecx
    add esi,2
    add ebp,2
%endmacro

%macro mode7fullsubwinon 0
    mov ecx,[cwinptr2]
    or dl,dl
    jz %%nodraw
    test byte[ecx],0FFh
    jnz %%nodraw
    mov ecx,[ebp]
    mov edx,[pal16bxcl+edx*4]
    and ecx,[vesa2_clbit]
    add edx,ecx
    shr edx,1
    mov edx,[fulladdtab+edx*2]
    xor edx,0FFFFh
    mov [esi],dx
    xor edx,edx
%%nodraw
    xor ecx,ecx
    inc dword[cwinptr2]
    add esi,2
    add ebp,2
%endmacro

%macro mode7mainsubwinon 0
    mov ecx,[cwinptr2]
    or dl,dl
    jz %%nodraw
    test byte[ecx],0FFh
    jnz %%nodraw
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
    mov [ebp],cx
%%nodraw
    inc dword[cwinptr2]
    xor ecx,ecx
    add esi,2
    add ebp,2
%endmacro

%macro mode716tmacro 2
    Mode7Calculate
    mov ebp,transpbuf+32

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov edi,[vram]

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
.nomosaic

    ; esi = pointer to video buffer
    ; edi = pointer to vram
    ; [.mode7xadder] = dword value to add to x value (decimal between 7 & 8bit)
    ; [.mode7yadder] = dword value to add to y value (decimal between 7 & 8bit)
    ; [.mode7xpos]   = dword value of x position, decimal between 7 & 8bit
    ; [.mode7xpos+1] = word value of x position
    ; [.mode7ypos]   = dword value of y position, decimal between 7 & 8bit
    ; [.mode7ypos+1] = word value of y position
    xor ebx,ebx
    xor edx,edx
    xor ecx,ecx
    mov dword[.mode7xadd2],800h
    mov byte[.mode7xinc],2
    mov byte[.mode7xincc],0
    test dword[.mode7xadder],80000000h
    jz .noneg
    mov dword[.mode7xadd2],-800h
    mov byte[.mode7xinc],-2
    mov byte[.mode7xincc],0FEh
.noneg
    mov dword[.mode7yadd2],800h
    mov byte[.mode7yinc],1
    test dword[.mode7yadder],80000000h
    jz .noneg2
    mov dword[.mode7yadd2],-800h
    mov byte[.mode7yinc],-1
.noneg2

    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3

    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near .drawmode7win
.domosaic

    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3
    Mode7Process %1, domosaic16b, 2
.nextval3
    Mode7ProcessB %1, domosaic16b, 2

ALIGN32
SECTION .bss
.temp        resd 1       ; for byte move left
.mode7xpos   resd 1       ; x position
.tempa2      resd 1       ; keep this blank!
.mode7xrpos  resd 1       ; x position
.tempa       resd 1       ; keep this blank!
.mode7ypos   resd 1       ; y position
.tempb2      resd 1       ; keep this blank!
.mode7yrpos  resd 1       ; y position
.tempb       resd 1       ; keep this blank!
.mode7xadder resd 1       ; number to add for x
.tempc2      resd 1       ; keep this blank!
.mode7xadd2  resd 1       ; number to add for x
.tempc       resd 1       ; keep this blank!
.mode7yadder resd 1       ; number to add for y
.tempd2      resd 1       ; keep this blank!
.mode7yadd2  resd 1       ; number to add for y
.tempd       resd 1       ; keep this blank!
.mode7ptr    resd 1       ; pointer value
.mode7xinc   resd 1       ; number to add for x
.mode7xincc  resd 1       ; range check for x
.mode7yinc   resd 1       ; number to add for y
.mode7xsloc  resd 1       ; which screen x
.mode7ysloc  resd 1       ; which screen y
.mode7xsrl   resd 1       ; which relative screen x
.mode7ysrl   resd 1       ; which relative screen y
.cxloc       resw 1       ; cx location
.cyloc       resw 1       ; cy location
.m7xaddofa   resd 1
.m7xaddof2a  resd 1
.m7yaddofa   resd 1
.m7yaddof2a  resd 1
SECTION .text

.drawmode7win
.domosaicw
    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3w
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3w

    Mode7Process %2, domosaic16b, 2
.nextval3w
    Mode7ProcessB %2, domosaic16b, 2
%endmacro

;*******************************************************
; Processes & Draws Mode 7 half Addition
;*******************************************************
NEWSYM drawmode716t
    test byte[scaddset],1
    jnz near drawmode7dcolor
    cmp byte[DoTransp],1
    jne .transpfull
    jmp drawmode716b
.transpfull
    mov esi,[cwinptr]
    mov [winptrref],esi
    mov [cwinptr2],esi
    test byte[scaddtype],80h
    jnz near drawmode716tsub
    test byte[scaddtype],40h
    jz near drawmode716tfulladd
    cmp byte[scrnon+1],0
    je near drawmode716tfulladd
    cmp dword[coladdr],0
    jnz near drawmode716tfulladd
;    cmp byte[scrnon+1],10h
;    je near drawmode716tfulladd
;.n
    mode716tmacro mode7halfadd,mode7halfaddwinon


;*******************************************************
; Processes & Draws Mode 7 Full Addition
;*******************************************************
NEWSYM drawmode716tfulladd
    mode716tmacro mode7fulladd,mode7fulladdwinon

;**********************************************************
; Processes and draws Mode 7 subtract
;**********************************************************

drawmode716tsub:
    mode716tmacro mode7fullsub,mode7fullsubwinon

;**********************************************************
; Mode 7, main & sub mode
;**********************************************************

NEWSYM drawmode716tb
    mov esi,[cwinptr]
    mov [winptrref],esi
    mov [cwinptr2],esi
    mode716tmacro mode7mainsub,mode7mainsubwinon

SECTION .bss
cwinptr2 resd 1


