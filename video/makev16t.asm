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
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif

%macro ALIGN32 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1+
	push eax
	ccall %1
	pop eax
%endmacro
EXTSYM cwinptr,dualwinbg,dualwinsp,dwinptrproc,pwinbgenab
EXTSYM pwinbgtype,pwinspenab,pwinsptype,winbgdata,winlogicb,winonbtype
EXTSYM winonstype,winspdata,interlval,bg1scrolx,bg1scroly,curmosaicsz
EXTSYM curypos,drawmode716t,makewindow,mode7set,mosaicon,mosaicsz,scrnon
EXTSYM makedualwincol
EXTSYM M7TAX,M7TBX,M7TDX,M7TBP,M7TTail
EXTSYM c_procmode716tsub,c_procmode716tsubextbg,c_procmode716tsubextbgb
EXTSYM c_procmode716tsubextbg2,c_procmode716tmain,c_procmode716tmainextbg
EXTSYM c_procmode716tmainextbgb,c_procmode716tmainextbg2
EXTSYM SPRAX,SPRBX,SPRCX,SPRBP,SPRDX,SPRTail
EXTSYM c_procspritessub16t,c_procspritesmain16t
EXTSYM c_procspritessub16tfix,c_procspritesmain16tfix
EXTSYM BGAX,BGBX,BGCX,BGDX,BGSI,BGDI,BGBP,BGTail,c_bg_mark_drawn
EXTSYM c_drawbackgrndsub16t,c_drawbackgrndmain16t
EXTSYM c_drawbackgrndsub16tfix,c_drawbackgrndmain16tfix
EXTSYM CBAX,CBBX,CBCX,CBDX,CBSI,CBDI,CBBP
EXTSYM c_clearback16t,c_clearback16ts
EXTSYM CLBAX,CLBBX,CLBCX,CLBDX,CLBSI,CLBDI
EXTSYM c_clearback16bts
EXTSYM SPBAX,SPBBX,SPBCX,SPBDX,SPBSI,SPBDI,SPBBP
EXTSYM c_drawsprites16bt
EXTSYM SPTAX,SPTBX,SPTCX,SPTDX,SPTSI,SPTDI,SPTBP
EXTSYM c_drawsprites16t
EXTSYM SPPAX,SPPBX,SPPCX,SPPDX,SPPSI,SPPDI,SPPBP
EXTSYM c_drawsprites16tprio
EXTSYM T8AX,T8BX,T8CX,T8DX,T8SI,T8DI,T8BP,T8Tail
EXTSYM c_draw8x816bt
EXTSYM T16AX,T16BX,T16CX,T16DX,T16SI,T16DI,T16BP,T16Tail
EXTSYM c_draw16x1616bt
EXTSYM TTAX,TTBX,TTCX,TTDX,TTSI,TTDI,TTBP,TTTail
EXTSYM c_draw8x816t
EXTSYM TXAX,TXBX,TXCX,TXDX,TXSI,TXDI,TXBP,TXTail
EXTSYM c_draw16x1616t
EXTSYM winbg1en,winenabm,drawmode716textbg,drawmode716textbg2,extbgdone
EXTSYM drawmode716tb,drawmode716b,drawmode716extbg,drawmode716extbg2,cursprloc
EXTSYM drawsprites16b,scrndis,sprprifix,winonsp,bgfixer,scaddtype
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
EXTSYM bg1scrolx_m7,bg1scroly_m7,OMBGTestVal,cachesingle4bng,m7starty
EXTSYM ofsmtptrs,ofsmcptr2,ofshvaladd
EXTSYM transpbuf,prevrgbcol,prevrgbpal,DoTransp,coadder16,yadd,yflipadd

%include "video/vidmacro.mac"

;*******************************************************
; DrawLine 16bit Transparent      Draws the current line
;*******************************************************
; use curypos+bg1scroly for y location and bg1scrolx for x location
; use bg1ptr(b,c,d) for the pointer to the tile number contents
; use bg1objptr for the pointer to the object tile contents

SECTION .text

; makedualwincol has been ported to C (video/c_makevid.c); it is called from
; the procwindowback macro (video/vidmacro.mac).

; The eight procmode716t* bodies are video/c_mv16tm7.c; each returns which
; renderer to run in M7TTail (0 = the layer is off). Tail-jumping is what the
; originals did - a call followed by ret.
m716t_tail:
    cmp dword[M7TTail],0
    je .off
    cmp dword[M7TTail],1
    je near drawmode716t
    cmp dword[M7TTail],2
    je near drawmode716b
    cmp dword[M7TTail],3
    je near drawmode716tb
    cmp dword[M7TTail],4
    je near drawmode716extbg
    cmp dword[M7TTail],5
    je near drawmode716textbg
    cmp dword[M7TTail],6
    je near drawmode716extbg2
    jmp near drawmode716textbg2
.off
    ret

NEWSYM procmode716tsub
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tsub
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail




NEWSYM procmode716tsubextbg
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tsubextbg
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tsubextbgb
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tsubextbgb
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tsubextbg2
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tsubextbg2
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tmain
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tmain
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tmainextbg
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tmainextbg
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tmainextbgb
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tmainextbgb
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procmode716tmainextbg2
    mov [M7TAX], eax
    mov [M7TBX], ebx
    mov [M7TDX], edx
    mov [M7TBP], ebp
    ; The originals never touch ecx unless makewindow does, so callers may
    ; rely on it; cdecl would let the C half clobber it.
    push ecx
    call c_procmode716tmainextbg2
    mov eax, [M7TAX]
    mov ebx, [M7TBX]
    mov edx, [M7TDX]
    pop ecx
    mov ebp, [M7TBP]
    jmp near m716t_tail


NEWSYM procspritessub16t
    cmp byte[bgfixer],1
    je near procspritessub16tfix
    mov [SPRAX], eax
    mov [SPRBX], ebx
    mov [SPRCX], ecx
    mov [SPRBP], ebp
    mov [SPRDX], edx
    call c_procspritessub16t
    mov eax, [SPRAX]
    mov ebx, [SPRBX]
    mov ecx, [SPRCX]
    mov ebp, [SPRBP]
    mov edx, [SPRDX]
    cmp dword[SPRTail],0
    je .done
    cmp dword[SPRTail],1
    je near drawsprites16t
    jmp near drawsprites16bt
.done
    ret


NEWSYM procspritesmain16t
    cmp byte[bgfixer],1
    je near procspritesmain16tfix
    mov [SPRAX], eax
    mov [SPRBX], ebx
    mov [SPRCX], ecx
    mov [SPRBP], ebp
    mov [SPRDX], edx
    call c_procspritesmain16t
    mov eax, [SPRAX]
    mov ebx, [SPRBX]
    mov ecx, [SPRCX]
    mov ebp, [SPRBP]
    mov edx, [SPRDX]
    cmp dword[SPRTail],0
    je .done
    cmp dword[SPRTail],1
    je near drawsprites16t
    jmp near drawsprites16bt
.done
    ret


NEWSYM drawbackgrndsub16t
    cmp byte[bgfixer],1
    je near drawbackgrndsub16tfix
    mov [BGAX], eax
    mov [BGBX], ebx
    mov [BGCX], ecx
    mov [BGDX], edx
    mov [BGSI], esi
    mov [BGDI], edi
    mov [BGBP], ebp
    call c_drawbackgrndsub16t
    mov eax, [BGAX]
    mov ebx, [BGBX]
    mov ecx, [BGCX]
    mov edx, [BGDX]
    mov esi, [BGSI]
    mov edi, [BGDI]
    mov ebp, [BGBP]
    cmp dword[BGTail],0
    je .done
    cmp dword[BGTail],1
    jne .n1
    call draw8x816t
    jmp .after
.n1
    cmp dword[BGTail],2
    jne .n2
    call draw16x1616t
    jmp .after
.n2
    cmp dword[BGTail],3
    jne .n3
    call draw8x816bt
    jmp .after
.n3
    cmp dword[BGTail],4
    jne .n4
    call draw16x1616bt
    jmp .after
.n4
    cmp dword[BGTail],5
    jne .n5
    call draw8x816tms
    jmp .after
.n5
    cmp dword[BGTail],6
    jne .n6
    call draw16x1616tms
    jmp .after
.n6
.after
    ; the renderers are called, not jumped to: the drawn==33 mark happens after
    ; the renderers leave ecx/edx live for the caller; cdecl would let the
    ; drawn==33 bookkeeping clobber them
    push ecx
    push edx
    mov [BGAX], eax
    call c_bg_mark_drawn
    mov eax, [BGAX]
    pop edx
    pop ecx
.done
    ret


NEWSYM drawbackgrndmain16t
    cmp byte[bgfixer],1
    je near drawbackgrndmain16tfix
    mov [BGAX], eax
    mov [BGBX], ebx
    mov [BGCX], ecx
    mov [BGDX], edx
    mov [BGSI], esi
    mov [BGDI], edi
    mov [BGBP], ebp
    call c_drawbackgrndmain16t
    mov eax, [BGAX]
    mov ebx, [BGBX]
    mov ecx, [BGCX]
    mov edx, [BGDX]
    mov esi, [BGSI]
    mov edi, [BGDI]
    mov ebp, [BGBP]
    cmp dword[BGTail],0
    je .done
    cmp dword[BGTail],1
    jne .n1
    call draw8x816t
    jmp .after
.n1
    cmp dword[BGTail],2
    jne .n2
    call draw16x1616t
    jmp .after
.n2
    cmp dword[BGTail],3
    jne .n3
    call draw8x816bt
    jmp .after
.n3
    cmp dword[BGTail],4
    jne .n4
    call draw16x1616bt
    jmp .after
.n4
    cmp dword[BGTail],5
    jne .n5
    call draw8x816tms
    jmp .after
.n5
    cmp dword[BGTail],6
    jne .n6
    call draw16x1616tms
    jmp .after
.n6
.after
    ; the renderers are called, not jumped to: the drawn==33 mark happens after
    ; the renderers leave ecx/edx live for the caller; cdecl would let the
    ; drawn==33 bookkeeping clobber them
    push ecx
    push edx
    mov [BGAX], eax
    call c_bg_mark_drawn
    mov eax, [BGAX]
    pop edx
    pop ecx
.done
    ret


NEWSYM procspritessub16tfix
    mov [SPRAX], eax
    mov [SPRBX], ebx
    mov [SPRCX], ecx
    mov [SPRBP], ebp
    mov [SPRDX], edx
    call c_procspritessub16tfix
    mov eax, [SPRAX]
    mov ebx, [SPRBX]
    mov ecx, [SPRCX]
    mov ebp, [SPRBP]
    mov edx, [SPRDX]
    cmp dword[SPRTail],0
    je .done
    cmp dword[SPRTail],1
    je near drawsprites16t
    jmp near drawsprites16bt
.done
    ret


NEWSYM procspritesmain16tfix
    mov [SPRAX], eax
    mov [SPRBX], ebx
    mov [SPRCX], ecx
    mov [SPRBP], ebp
    mov [SPRDX], edx
    call c_procspritesmain16tfix
    mov eax, [SPRAX]
    mov ebx, [SPRBX]
    mov ecx, [SPRCX]
    mov ebp, [SPRBP]
    mov edx, [SPRDX]
    cmp dword[SPRTail],0
    je .done
    cmp dword[SPRTail],1
    je near drawsprites16t
    jmp near drawsprites16bt
.done
    ret


NEWSYM drawbackgrndsub16tfix
    mov [BGAX], eax
    mov [BGBX], ebx
    mov [BGCX], ecx
    mov [BGDX], edx
    mov [BGSI], esi
    mov [BGDI], edi
    mov [BGBP], ebp
    call c_drawbackgrndsub16tfix
    mov eax, [BGAX]
    mov ebx, [BGBX]
    mov ecx, [BGCX]
    mov edx, [BGDX]
    mov esi, [BGSI]
    mov edi, [BGDI]
    mov ebp, [BGBP]
    cmp dword[BGTail],0
    je .done
    cmp dword[BGTail],1
    jne .n1
    call draw8x816t
    jmp .after
.n1
    cmp dword[BGTail],2
    jne .n2
    call draw16x1616t
    jmp .after
.n2
    cmp dword[BGTail],3
    jne .n3
    call draw8x816bt
    jmp .after
.n3
    cmp dword[BGTail],4
    jne .n4
    call draw16x1616bt
    jmp .after
.n4
    cmp dword[BGTail],5
    jne .n5
    call draw8x816tms
    jmp .after
.n5
    cmp dword[BGTail],6
    jne .n6
    call draw16x1616tms
    jmp .after
.n6
.after
    ; the renderers are called, not jumped to: the drawn==33 mark happens after
    ; the renderers leave ecx/edx live for the caller; cdecl would let the
    ; drawn==33 bookkeeping clobber them
    push ecx
    push edx
    mov [BGAX], eax
    call c_bg_mark_drawn
    mov eax, [BGAX]
    pop edx
    pop ecx
.done
    ret


NEWSYM drawbackgrndmain16tfix
    mov [BGAX], eax
    mov [BGBX], ebx
    mov [BGCX], ecx
    mov [BGDX], edx
    mov [BGSI], esi
    mov [BGDI], edi
    mov [BGBP], ebp
    call c_drawbackgrndmain16tfix
    mov eax, [BGAX]
    mov ebx, [BGBX]
    mov ecx, [BGCX]
    mov edx, [BGDX]
    mov esi, [BGSI]
    mov edi, [BGDI]
    mov ebp, [BGBP]
    cmp dword[BGTail],0
    je .done
    cmp dword[BGTail],1
    jne .n1
    call draw8x816t
    jmp .after
.n1
    cmp dword[BGTail],2
    jne .n2
    call draw16x1616t
    jmp .after
.n2
    cmp dword[BGTail],3
    jne .n3
    call draw8x816bt
    jmp .after
.n3
    cmp dword[BGTail],4
    jne .n4
    call draw16x1616bt
    jmp .after
.n4
    cmp dword[BGTail],5
    jne .n5
    call draw8x816tms
    jmp .after
.n5
    cmp dword[BGTail],6
    jne .n6
    call draw16x1616tms
    jmp .after
.n6
.after
    ; the renderers are called, not jumped to: the drawn==33 mark happens after
    ; the renderers leave ecx/edx live for the caller; cdecl would let the
    ; drawn==33 bookkeeping clobber them
    push ecx
    push edx
    mov [BGAX], eax
    call c_bg_mark_drawn
    mov eax, [BGAX]
    pop edx
    pop ecx
.done
    ret


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
    ccallv setpalette16b
    ; clear back area w/ back color
    procwindowback
    call clearback16bts
    ; do sprite windowing
    ccallv makewindowsp
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
    ccallv preparesprpr
.nosprprio
; process backgrounds
; do background 2
    mov byte[curbgnum],02h
    ccallv procbackgrnd, 0x01
; do background 1
    mov byte[curbgnum],01h
    ccallv procbackgrnd, 0x00
; do background 4
    mov byte[curbgnum],08h
    ccallv procbackgrnd, 0x03
; do background 3
    mov byte[curbgnum],04h
    ccallv procbackgrnd, 0x02

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
    ccallv setpalette16b
    ; clear back area w/ back color
    procwindowback
    call clearback16bts
    ; do sprite windowing
    ccallv makewindowsp
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
    ccallv preparesprpr
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
    ccallv preparesprpr
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

SECTION .text

NEWSYM clearback16bts
    mov [CLBAX], eax
    mov [CLBBX], ebx
    mov [CLBCX], ecx
    mov [CLBDX], edx
    mov [CLBSI], esi
    mov [CLBDI], edi
    call c_clearback16bts
    mov eax, [CLBAX]
    mov ebx, [CLBBX]
    mov ecx, [CLBCX]
    mov edx, [CLBDX]
    mov esi, [CLBSI]
    mov edi, [CLBDI]
    ret

SECTION .text

;*******************************************************
; Clear Backarea, 16-bit mode w/ transparency
;*******************************************************
NEWSYM clearback16t
    mov [CBAX], eax
    mov [CBBX], ebx
    mov [CBCX], ecx
    mov [CBDX], edx
    mov [CBSI], esi
    mov [CBDI], edi
    mov [CBBP], ebp
    call c_clearback16t
    mov eax, [CBAX]
    mov ebx, [CBBX]
    mov ecx, [CBCX]
    mov edx, [CBDX]
    mov esi, [CBSI]
    mov edi, [CBDI]
    mov ebp, [CBBP]
    ret


NEWSYM clearback16ts
    mov [CBAX], eax
    mov [CBBX], ebx
    mov [CBCX], ecx
    mov [CBDX], edx
    mov [CBSI], esi
    mov [CBDI], edi
    mov [CBBP], ebp
    call c_clearback16ts
    mov eax, [CBAX]
    mov ebx, [CBBX]
    mov ecx, [CBCX]
    mov edx, [CBDX]
    mov esi, [CBSI]
    mov edi, [CBDI]
    mov ebp, [CBBP]
    ret


NEWSYM drawsprites16bt
    mov [SPBAX], eax
    mov [SPBBX], ebx
    mov [SPBCX], ecx
    mov [SPBDX], edx
    mov [SPBSI], esi
    mov [SPBDI], edi
    mov [SPBBP], ebp
    call c_drawsprites16bt
    mov eax, [SPBAX]
    mov ebx, [SPBBX]
    mov ecx, [SPBCX]
    mov edx, [SPBDX]
    mov esi, [SPBSI]
    mov edi, [SPBDI]
    mov ebp, [SPBBP]
    ret

NEWSYM drawsprites16t
    cmp byte[sprprifix],1
    je near drawsprites16tprio
    mov [SPTAX], eax
    mov [SPTBX], ebx
    mov [SPTCX], ecx
    mov [SPTDX], edx
    mov [SPTSI], esi
    mov [SPTDI], edi
    mov [SPTBP], ebp
    call c_drawsprites16t
    mov eax, [SPTAX]
    mov ebx, [SPTBX]
    mov ecx, [SPTCX]
    mov edx, [SPTDX]
    mov esi, [SPTSI]
    mov edi, [SPTDI]
    mov ebp, [SPTBP]
    ret

NEWSYM drawsprites16tprio
    mov [SPPAX], eax
    mov [SPPBX], ebx
    mov [SPPCX], ecx
    mov [SPPDX], edx
    mov [SPPSI], esi
    mov [SPPDI], edi
    mov [SPPBP], ebp
    call c_drawsprites16tprio
    mov eax, [SPPAX]
    mov ebx, [SPPBX]
    mov ecx, [SPPCX]
    mov edx, [SPPDX]
    mov esi, [SPPSI]
    mov edi, [SPPDI]
    mov ebp, [SPPBP]
    ret


NEWSYM draw8x816bt
    cmp byte[bgmode],2
    jne .nodraw8x816boffset
    ccallv draw8x816boffset, eax, ecx, edx, ebx, ebp, esi, edi
.nodraw8x816boffset
    cmp byte[bgmode],5
    je near draw16x816t
    mov [T8AX], eax
    mov [T8BX], ebx
    mov [T8CX], ecx
    mov [T8DX], edx
    mov [T8SI], esi
    mov [T8DI], edi
    mov [T8BP], ebp
    call c_draw8x816bt
    mov eax, [T8AX]
    mov ebx, [T8BX]
    mov ecx, [T8CX]
    mov edx, [T8DX]
    mov esi, [T8SI]
    mov edi, [T8DI]
    mov ebp, [T8BP]
    ; the mosaic tail is a jump, not a call: domosaic16b returns to our caller
    cmp dword[T8Tail],0
    jne near domosaic16b
    ret


NEWSYM draw8x816t
    cmp byte[osm2dis],1
    je .osm2dis
    cmp byte[bgmode],2
    je near draw8x816toffset
.osm2dis
    cmp byte[bgmode],5
    je near draw16x816t
    mov [TTAX], eax
    mov [TTBX], ebx
    mov [TTCX], ecx
    mov [TTDX], edx
    mov [TTSI], esi
    mov [TTDI], edi
    mov [TTBP], ebp
    call c_draw8x816t
    mov eax, [TTAX]
    mov ebx, [TTBX]
    mov ecx, [TTCX]
    mov edx, [TTDX]
    mov esi, [TTSI]
    mov edi, [TTDI]
    mov ebp, [TTBP]
    ; the mosaic tail is a jump, not a call: domosaic16b returns to our caller
    cmp dword[TTTail],0
    jne near domosaic16b
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
    mov [T16AX], eax
    mov [T16BX], ebx
    mov [T16CX], ecx
    mov [T16DX], edx
    mov [T16SI], esi
    mov [T16DI], edi
    mov [T16BP], ebp
    call c_draw16x1616bt
    mov eax, [T16AX]
    mov ebx, [T16BX]
    mov ecx, [T16CX]
    mov edx, [T16DX]
    mov esi, [T16SI]
    mov edi, [T16DI]
    mov ebp, [T16BP]
    ; the mosaic tail is a jump, not a call: domosaic16b returns to our caller
    cmp dword[T16Tail],0
    jne near domosaic16b
    ret

;*******************************************************
; Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
;*******************************************************

NEWSYM draw16x1616t
    mov [TXAX], eax
    mov [TXBX], ebx
    mov [TXCX], ecx
    mov [TXDX], edx
    mov [TXSI], esi
    mov [TXDI], edi
    mov [TXBP], ebp
    call c_draw16x1616t
    mov eax, [TXAX]
    mov ebx, [TXBX]
    mov ecx, [TXCX]
    mov edx, [TXDX]
    mov esi, [TXSI]
    mov edi, [TXDI]
    mov ebp, [TXBP]
    ; the mosaic tail is a jump, not a call: domosaic16b returns to our caller
    cmp dword[TXTail],0
    jne near domosaic16b
    ret

