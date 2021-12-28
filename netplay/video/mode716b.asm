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

EXTSYM curmosaicsz,curvidoffset,domosaic16b,winptrref,scaddset
EXTSYM mode7A,mode7B,mode7C,mode7D,mode7X0,mode7Y0,mode7set
EXTSYM pal16b,vram,vrama,winon,mode7tab,xtravbuf,drawmode7dcolor
EXTSYM cwinptr

%include "video/mode7.mac"

;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

ALIGN16

%macro Mode7Normal 0
    or dl,dl
    jz %%nodrawb
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Window 0
    or dl,dl
    jz %%nodrawbw
    test byte[ebp],0FFh
    jnz %%nodrawbw
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
%%nodrawbw
    add esi,2
    inc ebp
%endmacro


NEWSYM drawmode716b
    test byte[scaddset],1
    jnz near drawmode7dcolor
    mov esi,[cwinptr]
    mov [winptrref],esi

    Mode7Calculate

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
    Mode7Process Mode7Normal, domosaic16b, 2
.nextval3
    Mode7ProcessB Mode7Normal, domosaic16b, 2

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
    mov ebp,[cwinptr]
    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3w
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3w

    Mode7Process Mode7Window, domosaic16b, 2
.nextval3w
    Mode7ProcessB Mode7Window, domosaic16b, 2


