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
EXTSYM BG116x16t,BG1SXl,BG1SYl,BG216x16t,BG2SXl,BG2SYl,BG316x16t,BG3PRI,BG3SXl
EXTSYM BG3SYl,BG416x16t,BG4SXl,BG4SYl,BGFB,BGMA,BGMS1,BGOPT1,BGOPT2,BGOPT3
EXTSYM BGOPT4,BGPT1,BGPT1X,BGPT1Y,BGPT2,BGPT2X,BGPT2Y,BGPT3,BGPT3X,BGPT3Y,BGPT4
EXTSYM BGPT4X,BGPT4Y,StartDrawNewGfx16b,bg1objptr,bg1ptr,bg1ptrx,bg1ptry
EXTSYM bg1scrolx,bg1scroly,bg2objptr,bg2ptr,bg2ptrx,bg2ptry,bg2scrolx,bg2scroly
EXTSYM bg3highst,bg3objptr,bg3ptr,bg3ptrx,bg3ptry,bg3scrolx,bg3scroly,bg4objptr
EXTSYM bg4ptr,bg4ptrx,bg4ptry,bg4scrolx,bg4scroly,bgmode,bgtxad,cachesingle2bng
EXTSYM cachesingle8bng,cbitmode,cfield,colormodedef,csprbit,curmosaicsz
EXTSYM curvidoffset,curypos,forceblnk,interlval,intrlng,mode7A,m7starty
EXTSYM mode7C,mode7X0,mode7ab,mode7cd,mode7set,mode7st,mode7xy,mosaicon,mosaicsz
EXTSYM mosenng,mosszng,ngceax,ngcedi,ngpalcon2b,ngpalcon8b,ngptrdat,prdata
EXTSYM prdatb,prdatc,res640,resolutn,scrndis,scrnon,spritetablea,sprleftpr
EXTSYM sprlefttot,sprpriodata,sprtbng,sprtlng,t16x161,t16x162,t16x163,t16x164
EXTSYM tltype2b,tltype8b,vcache2b,vcache8b,vidbuffer,vidmemch2,ngptrdat2
EXTSYM vidmemch8,vram,vrama,winon,xtravbuf,ng16bbgval,ng16bprval,ofshvaladd
EXTSYM bgwinchange,res480
EXTSYM osm2dis
EXTSYM winboundary
EXTSYM winbg1enval,winbg2enval,winbg3enval,winbg4enval,winbgobjenval
EXTSYM winlogicaval,disableeffects,winenabs,scanlines,winl1,winbg1en,winobjen
EXTSYM winlogica,winenabm,bgallchange,bg1change,bg2change,bg3change,bg4change
EXTSYM hiresstuff,WindowRedraw
EXTSYM winlogicb,ngwinptr,objwlrpos,objwen,objclineptr,CSprWinPtr
EXTSYM ofsmtptrs,ofsmcptr2


; BuildWindow, BuildWindow2 and Process1DualWindow have been ported to C
; (video/c_makev16b.c).

SECTION .bss
NEWSYM bgcmsung, resd 1
NEWSYM modeused, resd 2
NEWSYM reslbyl,  resd 1
NEWSYM csprival, resd 1
NEWSYM cfieldad, resd 1
NEWSYM ofsmcptr, resd 1
NEWSYM ofsmtptr, resd 1
NEWSYM ofsmmptr, resd 1
NEWSYM ofsmcyps, resd 1
NEWSYM ofsmady,  resd 1
NEWSYM ofsmadx,  resd 1

SECTION .data
ALIGN32

NEWSYM ngwintable, times 32 dd 0EE00h
NEWSYM ngwintablec, times 32 dd 0EE00h
NEWSYM ngcwinptr, dd ngwintable

SECTION .bss
NEWSYM ngwinen, resd 1
NEWSYM ngcwinmode, resd 1
NEWSYM ngcpixleft, resd 1
NEWSYM Mode7BackA, resd 1
NEWSYM Mode7BackC, resd 1
NEWSYM Mode7BackX0, resd 1
NEWSYM Mode7BackSet, resd 1
NEWSYM ngextbg, resd 1
NEWSYM ofsmval, resd 1
NEWSYM ofsmvalh, resd 1

SECTION .data
NEWSYM pwinen, dd 0FFFFh
NEWSYM pngwinen, dd 0FFFFh

SECTION .bss
NEWSYM pwinbound, resd 1
NEWSYM WinPtrAPos, resd 1
NEWSYM WinPtrBPos, resd 1

SECTION .data
NEWSYM OrLogicTable, db 0,1,1,0
NEWSYM AndLogicTable, db 0,0,1,0
NEWSYM XorLogicTable, db 0,1,0,0
NEWSYM XNorLogicTable, db 1,0,1,0

SECTION .bss
NEWSYM nglogicval, resd 1
NEWSYM mosjmptab, resd 15
NEWSYM Mode7HiRes, resb 1
NEWSYM pesimpng, resd 1
NEWSYM bgtxadd2, resd 1
SECTION .text

NEWSYM StartDrawNewGfx
    mov byte[WindowRedraw],1
    mov dword[cfieldad],0
    cmp byte[res480],1
    jne .scan2
    cmp byte[scanlines],0
    jne .scan2
    mov al,[cfield]
    mov [cfieldad],al
.scan2
    mov ax,[resolutn]
    sub ax,8
    mov [reslbyl],ax
    jmp StartDrawNewGfx16b

SECTION .bss
NEWSYM bgtxadd,  resd 1
NEWSYM tleftn,   resd 1
NEWSYM tleftnb,  resd 1
NEWSYM bg1totng, resd 1
NEWSYM bg2totng, resd 1
NEWSYM bg3totng, resd 1
NEWSYM bg4totng, resd 1
NEWSYM bg1drwng, resd 1
NEWSYM bg2drwng, resd 1
NEWSYM bg3drwng, resd 1
NEWSYM bg4drwng, resd 1
NEWSYM scfbl,    resd 1
NEWSYM mode0ads, resd 1
NEWSYM mode0add, resd 1
NEWSYM taddnfy16x16, resd 1
NEWSYM taddfy16x16, resd 1
NEWSYM switch16x16, resd 1
NEWSYM yposng,     resd 1
NEWSYM flipyposng, resd 1
NEWSYM yposngom,     resd 1
NEWSYM flipyposngom, resd 1
SECTION .text

SECTION .bss
NEWSYM NGNumSpr, resb 1
SECTION .text

;*******************************************************
; Prepare Sprite Priorities
;*******************************************************
; preparesprpr has been ported to C (video/c_makev16b.c).

SECTION .bss
NEWSYM sprclprio,  resd 1
NEWSYM sprsingle,  resd 1
SECTION .text
