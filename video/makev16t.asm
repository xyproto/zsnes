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
EXTSYM T16AX,T16BX,T16CX,T16DX,T16SI,T16DI,T16BP,T16Tail
EXTSYM c_draw16x1616bt
EXTSYM TXAX,TXBX,TXCX,TXDX,TXSI,TXDI,TXBP,TXTail
EXTSYM c_draw16x1616t
EXTSYM DLR,DLFN
EXTSYM winbg1en,winenabm,drawmode716textbg,drawmode716textbg2,extbgdone
EXTSYM drawmode716tb,drawmode716b,drawmode716extbg,drawmode716extbg2,cursprloc
EXTSYM drawsprites16b,scrndis,sprprifix,winonsp,bgfixer,scaddtype
EXTSYM alreadydrawn,bg1cachloc,bg1tdabloc,bg1tdatloc,bg1vbufloc,bg1xposloc
EXTSYM bg1yaddval,bgcoloradder,bgmode,bgtilesz,colormodeofs,curbgnum
EXTSYM draw16x1616b,draw8x816b,drawn,winenabs,curbgpr,ngptrdat2
EXTSYM bg3high2,currentobjptr,curvidoffset,cwinenabm,makewindowsp
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

; procwindowback16t and the macro it was built from are video/c_procwin.c;
; makedualwincol, which it calls, is video/c_makevid.c.

; The eight procmode716t* bodies are video/c_mv16tm7.c; each returns which
; renderer to run in M7TTail (0 = the layer is off). Tail-jumping is what the
; originals did - a call followed by ret.



















NEWSYM calldl16t
    push ebx
    push esi
    push edi
    push ebp
    mov ebx, [DLR+4]
    mov ecx, [DLR+8]
    mov edx, [DLR+12]
    mov esi, [DLR+16]
    mov edi, [DLR+20]
    mov ebp, [DLR+24]
    mov eax, [DLR]
    call dword[DLFN]
    mov [DLR], eax
    mov [DLR+4], ebx
    mov [DLR+8], ecx
    mov [DLR+12], edx
    mov [DLR+16], esi
    mov [DLR+20], edi
    mov [DLR+24], ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

;*******************************************************
; Clear Backarea, with 0s
;*******************************************************

SECTION .text


SECTION .text

;*******************************************************
; Clear Backarea, 16-bit mode w/ transparency
;*******************************************************




