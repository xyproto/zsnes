;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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
EXTSYM mode7A,mode7B,mode7C,mode7D,mode7X0,mode7Y0,mode7set,cwinptr
EXTSYM pal16b,vram,vrama,winon,mode7tab,xtravbuf,dcolortab,vidbright

NEWSYM Mode716DAsmStart
%include "video/mode7.mac"






NEWSYM Gendcolortable
   ; generate Direct Color Table
   push eax
   push edx
   xor ecx,ecx
.loopdct
   mov al,cl
   and eax,00000111b
    mov bl,[vidbright]
    mul bl
    mov bl,15
    div bl
    xor ah,ah
   shl eax,13
   mov edx,eax
   mov al,cl
   and eax,00111000b
   shr eax,3
    mov bl,[vidbright]
    mul bl
    mov bl,15
    div bl
    xor ah,ah
   shl eax,8
   or edx,eax
   mov al,cl
   and eax,11000000b
   shr eax,6
    mov bl,[vidbright]
    mul bl
    mov bl,15
    div bl
    xor ah,ah
   shl eax,3
   or edx,eax
   and edx,0FFFFh
   mov [dcolortab+ecx*4],edx
   inc cl
   jnz .loopdct
   pop edx
   pop eax
   ret

%macro Mode7Normal 0
    or dl,dl
    jz %%nodrawb
    mov ecx,[dcolortab+edx*4]
    mov [esi],cx
%%nodrawb
    add esi,2
%endmacro

%macro Mode7Window 0
    or dl,dl
    jz %%nodrawbw
    test byte[ebp],0FFh
    jnz %%nodrawbw
    mov ecx,[dcolortab+edx*4]
    mov [esi],cx
%%nodrawbw
    add esi,2
    inc ebp
%endmacro

;*******************************************************
; Processes & Draws Mode 7
;*******************************************************
NEWSYM prevbrightdc, db 0
NEWSYM drawmode7dcolor
    mov bl,[vidbright]
    cmp bl,[prevbrightdc]
    je .nodcchange
    mov [prevbrightdc],bl
    call Gendcolortable
.nodcchange
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
NEWSYM Mode716DAsmEnd
