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

; This file compiles to zero if not OpenBSD, thus it can be
; left in the Makefile.



%include "macros.mac"

EXTSYM SurfaceX,SurfaceY,ScreenPtr,SurfBufD,pitch,MMXSupport,resolutn,copymaskRB
EXTSYM copymaskG,copymagic

ALIGN32

SECTION .text

NEWSYM ClearWin16
    pushad
    mov  edi, [SurfBufD]
    xor  eax,eax
    xor  ebx,ebx
.Blank2:
    mov  ecx, [SurfaceX]
    rep stosw
    mov  edx, [SurfaceX]
    add  edi, [pitch]
    shl  edx,1
    add  ebx,1
    sub  edi,edx
    cmp  ebx, [SurfaceY]
    jne .Blank2
    popad
    ret

NEWSYM ClearWin32
    pushad
    mov  ax,ds
    mov  es,ax
    mov  edi, [SurfBufD]
    xor  ebx,ebx
.Blank3:
    xor  eax,eax
    mov  ecx, [SurfaceX]
    rep  stosd
    add  edi, [pitch]
    sub  edi, [SurfaceX]
    sub  edi, [SurfaceX]
    sub  edi, [SurfaceX]
    sub  edi, [SurfaceX]
    add  ebx,1
    cmp  ebx, [SurfaceY]
    jne .Blank3
    popad
    ret

NEWSYM DrawWin256x224x16
    pushad
    cmp byte[MMXSupport],0
    je .noMMX
    mov  esi, [ScreenPtr]
    mov  edi, [SurfBufD]
    xor  eax,eax
    movsx edx, word[resolutn]
    sub  edx,2
.Copying3:
    mov  ecx,32
.CopyLoop:
    movq mm0,[esi]
    movq mm1,[esi+8]
    movq [edi],mm0
    movq [edi+8],mm1
    add  esi,16
    add  edi,16
    dec  ecx
    jnz .CopyLoop
    inc  eax
    add  edi, [pitch]
    sub  edi,512
    add  esi,64
%ifdef __WIN32__
    cmp  eax,edx
%else
    cmp  eax,223
%endif
    jne .Copying3
    xor  eax,eax
    mov  ecx,128
    rep  stosd
    emms
    popad
    ret
.noMMX:
    mov  ax,ds
    mov  es,ax
    xor  eax,eax
    mov  esi, [ScreenPtr]
    mov  edi, [SurfBufD]
    movsx edx, word[resolutn]
    sub  edx,2
.Copying:
    mov  ecx,128
    rep  movsd
    inc  eax
    add  edi, [pitch]
    sub  edi,512
    sub  esi,512
    add  esi,576
%ifdef __WIN32__
    cmp  eax,edx
%else
    cmp  eax,223
%endif
    jne .Copying
    xor  eax,edx
    mov  ecx,128
    rep  stosd
    popad
    ret

NEWSYM DrawWin256x224x32
    pushad
    mov  ax,ds
    mov  es,ax
    xor  eax,eax
    movsx edx, word[resolutn]
    sub  edx,2
    mov  esi, [ScreenPtr]
    mov  edi, [SurfBufD]
    movq mm4, [copymaskRB]
    movq mm5, [copymaskG]
    movq mm6, [copymagic]
.Copying32b:
    mov  ecx,64
.CopyLoop32b:
    movq mm0, [esi]
    movq mm1,mm0
    punpcklwd mm0,mm0
    movq mm2,mm0
    pand mm0,mm4
    pmaddwd mm0,mm6
    punpckhwd mm1,mm1
    movq mm3,mm1
    pand mm1,mm4
    pmaddwd mm1,mm6
    pslld mm2,5
    pslld mm3,5
    pand mm2,mm5
    pand mm3,mm5
    por  mm0,mm2
    add  esi,8
    por  mm1,mm3
    movq [edi],mm0
    movq [edi+8],mm1
    add  edi,16
    dec  ecx
    jnz .CopyLoop32b
    inc  eax
    add  edi, [pitch]
    sub  edi,1024
    sub  esi,512
    add  esi,576
    cmp  eax,edx
    jne .Copying32b
    popad
    emms
    ret

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
    sub  edx,2
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
