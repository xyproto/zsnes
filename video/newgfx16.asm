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
EXTSYM BG116x16t,BG1SXl,BG1SYl,BG216x16t,BG2SXl,BG2SYl,BG316x16t,BG3PRI,BG3SXl
EXTSYM BG3SYl,BG416x16t,BG4SXl,BG4SYl,BGFB,BGMA,BGMS1,BGOPT1,BGOPT2,BGOPT3
EXTSYM BGOPT4,BGPT1,BGPT1X,BGPT1Y,BGPT2,BGPT2X,BGPT2Y,BGPT3,BGPT3X,BGPT3Y,BGPT4
EXTSYM BGPT4X,BGPT4Y,bg1drwng,bg1objptr,bg1ptr,bg1ptrx,bg1ptry,bg1scrolx
EXTSYM bg1scroly,bg1totng,bg2drwng,bg2objptr,bg2ptr,bg2ptrx,bg2ptry,bg2scrolx
EXTSYM bg2scroly,bg2totng,bg3drwng,bg3highst,bg3objptr,bg3ptr,bg3ptrx,bg3ptry
EXTSYM bg3scrolx,bg3scroly,bg3totng,bg4drwng,bg4objptr,bg4ptr,bg4ptrx,bg4ptry
EXTSYM bg4scrolx,bg4scroly,bg4totng,bgcmsung,bgmode,bgtxad,bgtxadd,ngextbg
EXTSYM cfieldad,cgram,coladdb,coladdg
EXTSYM coladdr,colormodedef,cpalval,csprbit,csprival,curmosaicsz
EXTSYM curvidoffset,curypos,flipyposng,forceblnk,interlval,intrlng
EXTSYM mode0add,mode0ads,mode7A,mode7C,mode7X0,mode7ab,mode7cd,mode7set,mode7st
EXTSYM mode7xy,modeused,mosaicon,mosaicsz,mosenng,mosszng,ngceax,ngcedi
EXTSYM ngptrdat,pesimpng,prdata,prdatb,prdatc
EXTSYM reslbyl,resolutn,scaddset,scaddtype,scadsng,scadtng,scfbl,scrndis,scrnon
EXTSYM spritetablea,sprleftpr,sprlefttot,sprpriodata,sprtbng,sprtlng
EXTSYM t16x161,t16x162,t16x163,t16x164,taddfy16x16,taddnfy16x16,ngptrdat2
EXTSYM vidbright,ofshvaladd
EXTSYM vidbuffer,winon,xtravbuf,yposng
EXTSYM vbufdptr,drawtileng2b16b,drawtileng4b16b,drawtileng8b16b,bgwinchange
EXTSYM drawtileng16x162b16b,drawtileng16x164b16b,drawtileng16x168b16b,winbg1en
EXTSYM drawlineng2b16b,drawlineng4b16b,drawlineng8b16b,BuildWindow,winenabs
EXTSYM drawlineng16x162b16b,drawlineng16x164b16b,drawlineng16x168b16b,winenabm
EXTSYM disableeffects,winl1,winbg1enval,winbg1envalm,winlogica,winlogicaval
EXTSYM winboundary,winobjen,winlogicb,nglogicval,ngwintable,winbg2enval
EXTSYM winbg3enval,winbg4enval,winbgobjenval,Mode7HiRes16b,res640,hiresstuff
EXTSYM Mode7BackA,Mode7BackC,Mode7BackX0,Mode7BackSet,ngwinen
EXTSYM drawlineng16x84b16b,drawlineng16x82b16b,ofsmcyps,vram,ofsmcptr,ofsmady
EXTSYM ofsmadx,ofsmtptr,yposngom,flipyposngom,ofsmmptr,ofsmval,ofsmvalh
EXTSYM winbg1envals,m7starty,bgallchange
EXTSYM FillSubScr,scanlines,SpecialLine,vidmemch2s
EXTSYM drawlinengom2b16b,drawlinengom4b16b,drawlinengom8b16b
EXTSYM drawlinengom16x162b16b,drawlinengom16x164b16b,drawlinengom16x168b16b
EXTSYM bg1change,bg2change,bg3change,bg4change,ngwinptr,objwlrpos,objwen
EXTSYM objclineptr,CSprWinPtr,BuildWindow2,NGNumSpr,fulladdtab
EXTSYM bgtxadd2,osm2dis,ofsmtptrs,ofsmcptr2
EXTSYM c_drawbg1tile16b,c_drawbg2tile16b,c_drawbg3tile16b,c_drawbg4tile16b
EXTSYM c_drawbg1tilepr116b,c_drawbg2tilepr116b,c_drawbg3tilepr116b,c_drawbg4tilepr116b
EXTSYM c_drawbg1line16b,c_drawbg2line16b,c_drawbg3line16b,c_drawbg4line16b
EXTSYM c_drawbg1linepr116b,c_drawbg2linepr116b,c_drawbg3linepr116b,c_drawbg4linepr116b
EXTSYM prevbrightdc,mosstart,moscountdown,BackAreaAdd
EXTSYM BackAreaUnFillCol,BackAreaFillCol,clinemainsub,cpalptrng
EXTSYM ngmsdraw,CMainWinScr,CSubWinScr,Prevcoladdr
EXTSYM ColResult,CPalPtrng,WindowRedraw,mostranspval
EXTSYM mosclineval,startlinet,endlinet,palchanged
EXTSYM c_startdrawnewgfx16b
EXTSYM NGSAX,NGSBX,NGSCX,NGSDX,NGSSI,NGSDI,NGSBP
EXTSYM c_drawsprng16b,c_drawsprng16bhr
EXTSYM newengine16b_lines,newengine16b_windows,newengine16b_sprwin
EXTSYM MOSAX,MOSBX,MOSCX,MOSDX,MOSSI,MOSDI,MOSBP,c_domosaicng16b
EXTSYM ng16bbgval,ng16bprval,mosjmptab16b,mosjmptab16bt
EXTSYM mosjmptab16btms,mosjmptab16bntms,UnusedBit,HalfTrans
EXTSYM UnusedBitXor,ngrposng,nggposng,ngbposng
EXTSYM HiResDone,FullBitAnd,HalfTransB,HalfTransC
EXTSYM NGNoTransp
EXTSYM dcolortab,setpalallng,setpalette16bng,BackAreaFill

%include "video/vidmacro.mac"

; Different routines for:
;   Normal (just one screen)
;   Transparency
;   No Transparency

; Sub+Main:
; Different Window Modes for each:
;   SubWin+MainWin
;   Sub+MainWin
;   SubWin+Main

; cgfxmod :
;   0 = No addition whatsoever
;   1 = Addition in the back area only
;   2 = All of subscreen added to all of mainscreen
;   3 = All of subscreen added to specific mainscreens
;   4 = Add+Sub enabled

; This file's .data is in video/c_newgfx16data.c.
section .text

NEWSYM StartDrawNewGfx16b
    ; The whole frame driver is video/c_ngframe.c. It ran on the live register
    ; block - the colour-maths pass it ends with reads eax and edx - so the
    ; seam hands the block over, and popad puts back what the routine's own
    ; pops used to.
    pushad
    mov eax, esp
    ccall c_startdrawnewgfx16b, eax
    popad
    ret


NEWSYM domosaicng16b
    mov [MOSAX], eax
    mov [MOSBX], ebx
    mov [MOSCX], ecx
    mov [MOSDX], edx
    mov [MOSSI], esi
    mov [MOSDI], edi
    mov [MOSBP], ebp
    call c_domosaicng16b
    mov eax, [MOSAX]
    mov ebx, [MOSBX]
    mov ecx, [MOSCX]
    mov edx, [MOSDX]
    mov esi, [MOSSI]
    mov edi, [MOSDI]
    mov ebp, [MOSBP]
    ret

NEWSYM drawbg1tile16b
    mov byte[prdatb+ebx],1
    ; The dispatcher and the renderer it used to tail-jump into are
    ; both video/c_ngbg.c and video/c_ng2tile.c now, so the push/pop
    ; ebx pairing that spanned them is gone with them.
    pushad
    mov eax, esp
    ccall c_drawbg1tile16b, eax
    popad
    ret

NEWSYM drawbg2tile16b
    mov byte[prdata+ebx],1
    ; The dispatcher and the renderer it used to tail-jump into are
    ; both video/c_ngbg.c and video/c_ng2tile.c now, so the push/pop
    ; ebx pairing that spanned them is gone with them.
    pushad
    mov eax, esp
    ccall c_drawbg2tile16b, eax
    popad
    ret

NEWSYM drawbg3tile16b
    mov byte[prdatc+ebx],1
    ; The dispatcher and the renderer it used to tail-jump into are
    ; both video/c_ngbg.c and video/c_ng2tile.c now, so the push/pop
    ; ebx pairing that spanned them is gone with them.
    pushad
    mov eax, esp
    ccall c_drawbg3tile16b, eax
    popad
    ret

NEWSYM drawbg4tile16b
    mov byte[prdata+ebx],1
    ; The dispatcher and the renderer it used to tail-jump into are
    ; both video/c_ngbg.c and video/c_ng2tile.c now, so the push/pop
    ; ebx pairing that spanned them is gone with them.
    pushad
    mov eax, esp
    ccall c_drawbg4tile16b, eax
    popad
    ret

NEWSYM drawbg1tilepr116b
    pushad
    mov eax, esp
    ccall c_drawbg1tilepr116b, eax
    popad
    ret

NEWSYM drawbg2tilepr116b
    pushad
    mov eax, esp
    ccall c_drawbg2tilepr116b, eax
    popad
    ret

NEWSYM drawbg3tilepr116b
    pushad
    mov eax, esp
    ccall c_drawbg3tilepr116b, eax
    popad
    ret

NEWSYM drawbg4tilepr116b
    pushad
    mov eax, esp
    ccall c_drawbg4tilepr116b, eax
    popad
    ret

NEWSYM drawbg1line16b
    mov byte[prdatb+ebx],0
    ; As the tile dispatchers above: the renderer is C too, so the
    ; push/pop ebx that used to span the tail-jump is gone.
    pushad
    mov eax, esp
    ccall c_drawbg1line16b, eax
    popad
    ret

NEWSYM drawbg2line16b
    mov byte[prdata+ebx],0
    pushad
    mov eax, esp
    ccall c_drawbg2line16b, eax
    popad
    ret

NEWSYM drawbg3line16b
    mov byte[prdatc+ebx],0
    pushad
    mov eax, esp
    ccall c_drawbg3line16b, eax
    popad
    ret

NEWSYM drawbg4line16b
    mov byte[prdata+ebx],0
    pushad
    mov eax, esp
    ccall c_drawbg4line16b, eax
    popad
    ret

NEWSYM drawbg1linepr116b
    pushad
    mov eax, esp
    ccall c_drawbg1linepr116b, eax
    popad
    ret

NEWSYM drawbg2linepr116b
    pushad
    mov eax, esp
    ccall c_drawbg2linepr116b, eax
    popad
    ret

NEWSYM drawbg3linepr116b
    pushad
    mov eax, esp
    ccall c_drawbg3linepr116b, eax
    popad
    ret

NEWSYM drawbg4linepr116b
    pushad
    mov eax, esp
    ccall c_drawbg4linepr116b, eax
    popad
    ret




; FillSubScr bit 0 sets to 1 if there is no subscreen present
; ms,wms,wm,ws
; FillSubScr scadtng
; The whole sprite cluster - both entry points, the fourteen arms they
; dispatch into and the 58 writer macros - is video/c_ngspr.c.
NEWSYM drawsprng16b
    mov [NGSAX], eax
    mov [NGSBX], ebx
    mov [NGSCX], ecx
    mov [NGSDX], edx
    mov [NGSSI], esi
    mov [NGSDI], edi
    mov [NGSBP], ebp
    call c_drawsprng16b
    mov eax, [NGSAX]
    mov ebx, [NGSBX]
    mov ecx, [NGSCX]
    mov edx, [NGSDX]
    mov esi, [NGSSI]
    mov edi, [NGSDI]
    mov ebp, [NGSBP]
    ret
NEWSYM drawsprng16bhr
    mov [NGSAX], eax
    mov [NGSBX], ebx
    mov [NGSCX], ecx
    mov [NGSDX], edx
    mov [NGSSI], esi
    mov [NGSDI], edi
    mov [NGSBP], ebp
    call c_drawsprng16bhr
    mov eax, [NGSAX]
    mov ebx, [NGSBX]
    mov ecx, [NGSCX]
    mov edx, [NGSDX]
    mov esi, [NGSSI]
    mov edi, [NGSDI]
    mov ebp, [NGSBP]
    ret

