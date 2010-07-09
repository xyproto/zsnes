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

; This file compiles to zero if not OpenBSD, thus it can be
; left in the Makefile.



%include "macros.mac"

EXTSYM ScreenPtr,SurfBufD,pitch,MMXSupport,resolutn

ALIGN32

SECTION .text

NEWSYM DrawWin320x240x16
    pushad
    cmp byte[MMXSupport],0
    je  near .noMMX
    xor  eax,eax
    xor  ebx,ebx
    mov  esi, [ScreenPtr]
    mov  edi, [SurfBufD]
.Blank1MMX:
    mov  ecx,160
    rep stosd
    sub  edi,160
    add  edi, [pitch]
    add  ebx,1
    cmp  ebx,8
    jne .Blank1MMX
    xor  ebx,ebx
    pxor mm0,mm0
.Copying2MMX:
    mov  ecx,4
.MMXLoopA:
    movq [edi+0],mm0
    movq [edi+8],mm0
    add  edi,16
    dec  ecx
    jnz .MMXLoopA
    mov  ecx,32
.MMXLoopB:
    movq mm1,[esi+0]
    movq mm2,[esi+8]
    movq [edi+0],mm1
    movq [edi+8],mm2
    add  esi,16
    add  edi,16
    dec  ecx
    jnz .MMXLoopB
    mov  ecx,4
.MMXLoopC:
    movq [edi+0],mm0
    movq [edi+8],mm0
    add  edi,16
    dec  ecx
    jnz .MMXLoopC
    inc  ebx
    add  edi, [pitch]
    sub  edi,640
    add  esi,64
%ifdef __WIN32__
    cmp  ebx,239
%else
    cmp  ebx,223
%endif
    jne .Copying2MMX
    mov  ecx,128
    rep stosd
    emms
    popad
    ret
.noMMX:
    mov  ax,ds
    mov  es,ax
    xor  eax,eax
    xor  ebx,ebx
    mov  esi, [ScreenPtr]
    mov  edi, [SurfBufD]
    movsx edx, word[resolutn]
.Blank1:
    xor  eax,eax
    mov  ecx,160
    rep  stosd
    sub  edi,640
    add  edi, [pitch]
    add  ebx,1
    cmp  ebx,8
    jne .Blank1
    xor  ebx,ebx
.Copying2:
    xor  eax,eax
    mov  ecx,16
    rep  stosd
    mov  ecx,128
    rep  movsd
    xor  eax,eax
    mov  ecx,16
    rep  stosd
    inc  ebx
    add  edi, [pitch]
    sub  edi,640
    sub  esi,512
    add  esi,576
    cmp  ebx,edx
    jne .Copying2
    xor  eax,eax
    mov  ecx,128
    rep  stosd
    popad
    ret
