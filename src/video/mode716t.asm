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
SECTION .data

.temp        dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

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
SECTION .text

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


