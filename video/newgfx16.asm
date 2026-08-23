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
EXTSYM prevbrightdc,mosstart,moscountdown,BackAreaAdd
EXTSYM BackAreaUnFillCol,BackAreaFillCol,clinemainsub,cpalptrng
EXTSYM c_process_transparencies
EXTSYM ngmsdraw,CMainWinScr,CSubWinScr,Prevcoladdr
EXTSYM ColResult,CPalPtrng,WindowRedraw,mostranspval
EXTSYM mosclineval,startlinet,endlinet,palchanged
EXTSYM c_procbg16b,c_procspr16b,c_procmode7ng16b
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
%include "video/newgfx16.mac"

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




section .text

NEWSYM newengine16b
    ; Everything up to the windowing section - the per-line state tables - is
    ; video/c_ngline.c. It takes and returns nothing in registers, so a plain
    ; ccallv is the whole seam; eax is re-established below because the rest of
    ; this routine indexes the same line with it.
    ccallv newengine16b_lines
    mov eax,[curypos]
    and eax,0FFh
    xor ebx,ebx

; Windowing Stuff
;NEWSYM winl1,      0             ; window 1 left position
;NEWSYM winr1,      0             ; window 1 right position
;NEWSYM winl2,      0             ; window 2 left position
;NEWSYM winr2,      0             ; window 2 right position
;NEWSYM winbg1en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG1
;NEWSYM winbg2en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG2
;NEWSYM winbg3en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG3
;NEWSYM winbg4en,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on BG4
;NEWSYM winobjen,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on sprites
;NEWSYM wincolen,   0             ; Win1 on (IN/OUT) or Win2 on (IN/OUT) on backarea
;NEWSYM winlogica,  0             ; Window logic type for BG1 to 4
;NEWSYM winlogicb,  0             ; Window logic type for Sprites and Backarea
;NEWSYM winenabm,   0             ; Window logic enable for main screen
;NEWSYM winenabs,   0             ; Window logic enable for sub screen

    mov byte[bgwinchange+eax],0
    cmp byte[disableeffects],1
    je near .finishwin
    push ecx
    push edx
    ccallv newengine16b_windows
    mov byte[bgwinchange+eax],1
.winnchanged4

    ; The whole sprite-window build is video/c_ngline.c.
    ccallv newengine16b_sprwin
.skipobjw
    pop edx
    pop ecx
    jmp .okaywin
.finishwin
    mov byte[winbg1enval+eax],0
    mov byte[winbg2enval+eax],0
    mov byte[winbg3enval+eax],0
    mov byte[winbg4enval+eax],0
    mov byte[winbgobjenval+eax],0
    mov byte[winbg1envalm+eax],0
    mov byte[winbg1envalm+eax+256],0
    mov byte[winbg1envalm+eax+256*2],0
    mov byte[winbg1envalm+eax+256*3],0
    mov byte[winbg1envalm+eax+256*4],0
    mov byte[winbg1envals+eax],0
    mov byte[winbg1envals+eax+256],0
    mov byte[winbg1envals+eax+256*2],0
    mov byte[winbg1envals+eax+256*3],0
    mov byte[winbg1envals+eax+256*4],0
.okaywin

    mov ebx,[coladdr-1]
    mov bl,[vidbright]
    cmp [Prevcoladdr],ebx
    je .samecolor
    mov [Prevcoladdr],ebx
    push ecx
    push eax
    mov al,[coladdr]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngrposng]
    xor ebx,ebx
    shl ax,cl
    add bx,ax
    mov al,[coladdg]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[nggposng]
    shl ax,cl
    add bx,ax
    mov al,[coladdb]
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngbposng]
    shl ax,cl
    add bx,ax
    mov [ColResult],bx
    mov [ColResult+2],bx
    pop eax
    pop ecx
.samecolor
    cmp word[ColResult],0
    je .black
    cmp byte[FillSubScr+eax],0
    je .notblack
    or byte[FillSubScr+eax],2
    jmp .notblack
.black
;    cmp byte[scrnon+1],0
;    jne .notblack
;    mov byte[clinemainsub],1
;    test byte[scadtng+eax],40h
;    jnz .notblack
;    xor byte[scadtng+eax],1
;    mov byte[FillSubScr+eax],0
.notblack


    ; fill back area
    ; get back window settings
    mov dword[ngwinen],0
    push ecx
    push edx
    cmp byte[winbg1enval+eax+5*256],0
    je .nowindowb
    mov bl,[winlogicb]
    shr bl,2
    and bl,03h
    mov [nglogicval],bl
    mov ebx,5*256
    add ebx,eax
    ccallv BuildWindow2, eax, ebx
.nowindowb
    mov dword[BackAreaAdd],0

    cmp byte[clinemainsub],1
    jne near .domainscreen
    mov ebx,[ColResult]
    or ebx,[UnusedBit]
    mov edx,[UnusedBit]
    mov cl,[scaddset]
.filledscreen
    ; get sub-screen colors
    test cl,10h
    jnz .inside
    test cl,20h
    jnz .outside
.insideb
    mov [BackAreaUnFillCol],ebx
    mov [BackAreaFillCol],ebx
    jmp .donesubscreen
.inside
    test cl,20h
    jnz .filled
    mov [BackAreaUnFillCol],edx
    mov [BackAreaFillCol],ebx
    jmp .donesubscreen
.outside
    mov [BackAreaUnFillCol],ebx
    mov [BackAreaFillCol],edx
    jmp .donesubscreen
.filled
    xor ebx,ebx
    mov [BackAreaUnFillCol],edx
    mov [BackAreaFillCol],edx
    jmp .donesubscreen
.domainscreen
    mov ebx,[vbufdptr]
    add ebx,dword[cpalptrng]
    xor edx,edx
    mov cx,[ebx]
    shl ecx,16
    mov cx,[ebx]
    mov ebx,ecx
    mov cl,[scaddset]
    shr cl,2
    test byte[scaddtype],20h
    jz .notaddbackub
    or ebx,[UnusedBit]
    mov edx,[UnusedBit]
.notaddbackub
    jmp .filledscreen
.donesubscreen
    cmp dword[ngwinen],0
    jne .nowinsc2
    mov edx,[BackAreaUnFillCol]
    mov [BackAreaFillCol],edx
.nowinsc2
    cmp byte[forceblnk],0
    je .notforceblanked
    mov dword[BackAreaUnFillCol],0
    mov dword[BackAreaFillCol],0
.notforceblanked
    ccallv BackAreaFill, eax
    test byte[FillSubScr+eax],1
    jz near .nosubscreen2
    mov dword[BackAreaAdd],75036*2
    mov ebx,[ColResult]
    mov edx,[UnusedBit]
    test byte[scaddset],02h
    jz .notbackfixed
    or ebx,[UnusedBit]
.notbackfixed
    mov cl,[scaddset]
    ; get sub-screen colors
    test cl,10h
    jnz .inside2
    test cl,20h
    jnz .outside2
.inside2b
    mov [BackAreaUnFillCol],ebx
    mov [BackAreaFillCol],ebx
    jmp .donesubscreen2
.inside2
    test cl,20h
    jnz .filled2
    mov [BackAreaUnFillCol],edx
    mov [BackAreaFillCol],ebx
    jmp .donesubscreen2
.outside2
    mov [BackAreaUnFillCol],ebx
    mov [BackAreaFillCol],edx
    jmp .donesubscreen2
.filled2
    xor ebx,ebx
    mov [BackAreaUnFillCol],edx
    mov [BackAreaFillCol],edx
.donesubscreen2
    cmp dword[ngwinen],0
    jne .nowinsc
    mov edx,[BackAreaUnFillCol]
    mov [BackAreaFillCol],edx
.nowinsc
    ccallv BackAreaFill, eax
.nosubscreen2
    pop edx
    pop ecx

    mov byte[SpecialLine+eax],0
    cmp byte[scanlines],0
    jne .nomode7hr
    cmp byte[bgmode],7
    jb .hrstuff
    test byte[interlval],40h
    jnz .nomode7hr
    cmp byte[Mode7HiRes16b],1
    jne .nomode7hr
    jmp .hrstuff
.nomode7hr
    jmp .no512
.hrstuff
    cmp byte[res640],0
    je near .no512
    cmp byte[bgmode],5
    jb .no512
    mov byte[SpecialLine+eax],3
    cmp byte[bgmode],7
    je .hires
    mov byte[SpecialLine+eax],2
.hires
    mov byte[hiresstuff],1
    push edi
    push esi
    push eax
    push ecx
    push eax
    mov ebx,eax
    mov edi,[vidbuffer]
    add edi,16*2
    shl eax,9
    add edi,eax
    mov eax,ebx
    shl eax,6
    add edi,eax
    mov esi,edi
    add edi,75036*4
    push esi
    mov ecx,128
    rep movsd
    pop esi
    pop eax
    test byte[FillSubScr+eax],1
    jz .nosubscreenhires
    add esi,75036*2
    mov edi,esi
    add edi,75036*4
    mov ecx,128
    rep movsd
.nosubscreenhires
    pop ecx
    pop eax
    pop esi
    pop edi
.no512

    cmp byte[scanlines],0
    jne .notinterl
    test byte[interlval],01h
    jz .notinterl
    or byte[SpecialLine+eax],4
.notinterl

    mov ebx,[sprleftpr+eax*4]
    cmp ebx,00000001h
    je .single
    cmp ebx,00000100h
    je .single
    cmp ebx,00010000h
    je .single
    cmp ebx,01000000h
    je .single
    xor ebx,ebx
    ret
.single
    or dword[sprleftpr+eax*4],80000000h
    xor ebx,ebx
    ret

section .text

NEWSYM StartDrawNewGfx16b
    push edx
    push esi
    push edi
    push ebp
    mov byte[WindowRedraw],1
    sub dword[endlinet],8

    cmp dword[scfbl],0
    jne near .dontdraw

    ; Sprite Layering :
    ; Mode 0/1 - BG4/BG3pr0 * BG4/BG3pr1 * BG2/BG1pr0 * BG2/BG1pr0 * BG3pr1
    ; Mode 2-6 - BG2pr0 * BG1pr0 * BG2pr1 * BG1pr1 *
    ; Mode 7 - * BG1pr0 * BG1pr1 * *

    ; Copy data to sprite table
    mov ecx,256
    mov eax,[spritetablea]
    mov ebx,sprlefttot
    mov edi,sprtbng
.loop
    mov [edi],eax
    add eax,512
    inc ebx
    add edi,4
    dec ecx
    jnz .loop

    xor eax,eax
    mov edi,sprtlng
    mov ecx,64
    rep stosd

    mov dword[bg1totng],0
    mov dword[bg2totng],0
    mov dword[bg3totng],0
    mov dword[bg4totng],0
    mov dword[bg1drwng],0
    mov dword[bg2drwng],0
    mov dword[bg3drwng],0
    mov dword[bg4drwng],0

    cmp byte[ngmsdraw],0
    je near .skipallsub

    mov dword[CMainWinScr],winbg1envals
    mov dword[CSubWinScr],winbg1envals

    ; Draw SubScreens
    test byte[scrndis],8h
    jnz near .nodobg4s
    test dword[bgcmsung],800h
    jz near .nodobg4s
    mov dword[mode0ads],60606060h
    ccallv c_procbg16b, 3, drawbg4line16b, drawbg4tile16b, 0, 0, 8h, 0
.nodobg4s
    test byte[scrndis],4h
    jnz near .nodobg3s
    test dword[bgcmsung],400h
    jz near .nodobg3s
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3line16b, drawbg3tile16b, 0, 0, 4h, 1
.nodobg3s

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprpra
    test dword[modeused],00000101h
    jz near .nosprpra
    test dword[bgcmsung],1000h
    jz near .nosprpra
    ccallv c_procspr16b, 0, 10h, 0
.nosprpra

    test byte[scrndis],8h
    jnz near .nodobg4sb
    test dword[bgcmsung],800h
    jz near .nodobg4sb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4sb
    mov dword[mode0ads],60606060h
    ccallv c_procbg16b, 3, drawbg4linepr116b, drawbg4tilepr116b, prdata, 0, 8h, 2
.nodobg4sb
    test byte[scrndis],4h
    jnz near .nodobg3sb
    test dword[bgcmsung],400h
    jz near .nodobg3sb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3sb
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, 0, 4h, 3
.nodobg3sb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprb
    test dword[modeused],00000101h
    jz near .nosprprb
    test dword[bgcmsung],1000h
    jz near .nosprprb
    ccallv c_procspr16b, 0, 10h, 0
.nosprprb

    test byte[scrndis],2h
    jnz near .nodobg2s
    test dword[bgcmsung],200h
    jz near .nodobg2s
    mov dword[mode0ads],20202020h
    ccallv c_procbg16b, 1, drawbg2line16b, drawbg2tile16b, 0, 0, 2h, 0
.nodobg2s

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgsc
    test dword[bgcmsung],300h
    jz near .noextbgsc
    ccallv c_procmode7ng16b, 0, 3h, 1
.noextbgsc

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprc
    test dword[modeused],01010000h
    jnz near .yessprprc
    test dword[modeused+4],01010101h
    jz near .nosprprc
.yessprprc
    test dword[bgcmsung],1000h
    jz near .nosprprc
    ccallv c_procspr16b, 0, 10h, 1
.nosprprc

    test byte[scrndis],1h
    jnz near .nodobg1s
    test dword[bgcmsung],100h
    jz near .nodobg1s
    mov dword[mode0ads],00000000h
    ccallv c_procbg16b, 0, drawbg1line16b, drawbg1tile16b, 0, 0, 1h, 0
.nodobg1s

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7
    test dword[bgcmsung],300h
    jz near .nomode7
    ccallv c_procmode7ng16b, 0, 1h, 0
.nomode7

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprd
    test dword[bgcmsung],1000h
    jz near .nosprprd
    ccallv c_procspr16b, 0, 10h, 2
.nosprprd

    test byte[scrndis],2h
    jnz near .nodobg2sb
    test dword[bgcmsung],200h
    jz near .nodobg2sb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2sb
    mov dword[mode0ads],20202020h
    ccallv c_procbg16b, 1, drawbg2linepr116b, drawbg2tilepr116b, prdata, 0, 2h, 2
.nodobg2sb

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgscb
    test dword[bgcmsung],300h
    jz near .noextbgscb
    ccallv c_procmode7ng16b, 0, 2h, 2
.noextbgscb

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprpre
    test dword[modeused],01010000h
    jnz near .yessprpre
    test dword[modeused+4],01010101h
    jz near .nosprpre
.yessprpre
    test dword[bgcmsung],1000h
    jz near .nosprpre
    ccallv c_procspr16b, 0, 10h, 1
.nosprpre

    test byte[scrndis],1h
    jnz near .nodobg1sb
    test dword[bgcmsung],100h
    jz near .nodobg1sb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1sb
    mov dword[mode0ads],00000000h
    ccallv c_procbg16b, 0, drawbg1linepr116b, drawbg1tilepr116b, prdatb, 0, 1h, 2
.nodobg1sb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprf
    test dword[bgcmsung],1000h
    jz near .nosprprf
    ccallv c_procspr16b, 0, 10h, 2
.nosprprf

    test byte[scrndis],4h
    jnz near .nodobg3sb2
    cmp byte[modeused+1],0
    je near .nodobg3sb2
    test dword[bgcmsung],400h
    jz near .nodobg3sb2
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3sb2
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, 0, 4h, 4
.nodobg3sb2

    mov dword[bg1totng],0
    mov dword[bg2totng],0
    mov dword[bg3totng],0
    mov dword[bg4totng],0
    mov dword[bg1drwng],0
    mov dword[bg2drwng],0
    mov dword[bg3drwng],0
    mov dword[bg4drwng],0

.skipallsub
    mov dword[CMainWinScr],winbg1envalm
    mov dword[CSubWinScr],winbg1envals

    ; Draw MainScreens
    test byte[scrndis],8h
    jnz near .nodobg4m
    test dword[bgcmsung],808h
    jz near .nodobg4m
    mov dword[mode0ads],60606060h
    ccallv c_procbg16b, 3, drawbg4line16b, drawbg4tile16b, 0, 1, 8h, 0
.nodobg4m
    test byte[scrndis],4h
    jnz near .nodobg3m
    test dword[bgcmsung],404h
    jz near .nodobg3m
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3line16b, drawbg3tile16b, 0, 1, 4h, 1
.nodobg3m

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprma
    test dword[modeused],00000101h
    jz near .nosprprma
    test dword[bgcmsung],1010h
    jz near .nosprprma
    ccallv c_procspr16b, 1, 10h, 0
.nosprprma

    test byte[scrndis],8h
    jnz near .nodobg4mb
    test dword[bgcmsung],808h
    jz near .nodobg4mb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4mb
    mov dword[mode0ads],60606060h
    ccallv c_procbg16b, 3, drawbg4linepr116b, drawbg4tilepr116b, prdata, 1, 8h, 2
.nodobg4mb
    test byte[scrndis],4h
    jnz near .nodobg3mb
    test dword[bgcmsung],404h
    jz near .nodobg3mb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3mb
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, 1, 4h, 3
.nodobg3mb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprmb
    test dword[modeused],00000101h
    jz near .nosprprmb
    test dword[bgcmsung],1010h
    jz near .nosprprmb
    ccallv c_procspr16b, 1, 10h, 0
.nosprprmb

    test byte[scrndis],2h
    jnz near .nodobg2m
    test dword[bgcmsung],202h
    jz near .nodobg2m
    mov dword[mode0ads],20202020h
    ccallv c_procbg16b, 1, drawbg2line16b, drawbg2tile16b, 0, 1, 2h, 0
.nodobg2m

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgmn
    test dword[bgcmsung],303h
    jz near .noextbgmn
    ccallv c_procmode7ng16b, 1, 3h, 1
.noextbgmn

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprmc
    test dword[modeused],01010000h
    jnz near .yessprprmc
    test dword[modeused+4],01010101h
    jz near .nosprprmc
.yessprprmc
    test dword[bgcmsung],1010h
    jz near .nosprprmc
    ccallv c_procspr16b, 1, 10h, 1
.nosprprmc

    test byte[scrndis],1h
    jnz near .nodobg1m
    test dword[bgcmsung],101h
    jz near .nodobg1m
    mov dword[mode0ads],00000000h
    ccallv c_procbg16b, 0, drawbg1line16b, drawbg1tile16b, 0, 1, 1h, 0
.nodobg1m

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7m
    test dword[bgcmsung],101h
    jz near .nomode7m
    ccallv c_procmode7ng16b, 1, 1h, 0
.nomode7m

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmd
    test dword[bgcmsung],1010h
    jz near .nosprprmd
    ccallv c_procspr16b, 1, 10h, 2
.nosprprmd

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgmn2
    test dword[bgcmsung],303h
    jz near .noextbgmn2
    ccallv c_procmode7ng16b, 1, 2h, 2
.noextbgmn2

    test byte[scrndis],2h
    jnz near .nodobg2mb
    test dword[bgcmsung],202h
    jz near .nodobg2mb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2mb
    mov dword[mode0ads],20202020h
    ccallv c_procbg16b, 1, drawbg2linepr116b, drawbg2tilepr116b, prdata, 1, 2h, 2
.nodobg2mb

    ; draw sprites mode 2-7
    test byte[scrndis],10h
    jnz near .nosprprme
    test dword[modeused],01010000h
    jnz near .yessprprme
    test dword[modeused+4],01010101h
    jz near .nosprprme
.yessprprme
    test dword[bgcmsung],1010h
    jz near .nosprprme
    ccallv c_procspr16b, 1, 10h, 1
.nosprprme

    test byte[scrndis],1h
    jnz near .nodobg1mb
    test dword[bgcmsung],101h
    jz near .nodobg1mb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1mb
    mov dword[mode0ads],00000000h
    ccallv c_procbg16b, 0, drawbg1linepr116b, drawbg1tilepr116b, prdatb, 1, 1h, 2
.nodobg1mb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmf
    test dword[bgcmsung],1010h
    jz near .nosprprmf
    ccallv c_procspr16b, 1, 10h, 2
.nosprprmf

    test byte[scrndis],4h
    jnz near .nodobg3mb2
    cmp byte[modeused+1],0
    je near .nodobg3mb2
    test dword[bgcmsung],404h
    jz near .nodobg3mb2
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3mb2
    mov dword[mode0ads],40404040h
    ccallv c_procbg16b, 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, 1, 4h, 4
.nodobg3mb2
    call MainScreenClip
    call ProcessTransparencies
.dontdraw
    xor ebx,ebx
    xor ecx,ecx
    xor eax,eax
    pop ebp
    pop edi
    pop esi
    pop edx
    ret


; The mosaic pass is video/c_ngmosaic.c. It is reached by tail-jump from a
; dozen places, so the thunk's ret has to land in *their* caller - which is
; what a plain call/ret seam does. The sixty mosdraw* entry points and the four
; jump tables that reached them are gone with it.
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
    drawbgtileng16b 0,0
    ret

NEWSYM drawbg2tile16b
    mov byte[prdata+ebx],1
    drawbgtileng16b 1,1
    ret

NEWSYM drawbg3tile16b
    mov byte[prdatc+ebx],1
    drawbgtileng16b 2,2
    ret

NEWSYM drawbg4tile16b
    mov byte[prdata+ebx],1
    drawbgtileng16b 3,3
    ret

NEWSYM drawbg1tilepr116b
    drawbgtilengpr116b 0,0
    ret

NEWSYM drawbg2tilepr116b
    drawbgtilengpr116b 1,1
    ret

NEWSYM drawbg3tilepr116b
    drawbgtilengpr116b 2,2
    ret

NEWSYM drawbg4tilepr116b
    drawbgtilengpr116b 3,3
    ret

NEWSYM drawbg1line16b
    mov byte[prdatb+ebx],0
    drawbglineng16b 0,0

NEWSYM drawbg2line16b
    mov byte[prdata+ebx],0
    drawbglineng16b 1,1

NEWSYM drawbg3line16b
    mov byte[prdatc+ebx],0
    drawbglineng16b 2,2

NEWSYM drawbg4line16b
    mov byte[prdata+ebx],0
    drawbglineng16b 3,3

NEWSYM drawbg1linepr116b
    drawbglinengpr116b 0,0

NEWSYM drawbg2linepr116b
    drawbglinengpr116b 1,1

NEWSYM drawbg3linepr116b
    drawbglinengpr116b 2,2

NEWSYM drawbg4linepr116b
    drawbglinengpr116b 3,3




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

ProcessTransparencies:
    ; The whole colour-maths pass is video/c_ngtransp.c. It runs with the
    ; caller's registers live - the plain half-add path never clears the top
    ; of edx - so the seam hands over the whole file.
    pushad
    mov eax, esp
    ccall c_process_transparencies, eax
    popad
    ret

section .text

%macro SCMainA 0
%endmacro

%macro SCSubA 0
    shl al,2
%endmacro

%macro SCMainB 0
    and word[esi],bx
    or word[esi+75036*2],bx
%endmacro

%macro SCSubB 0
    and word[esi],bx
%endmacro

%macro SCMainC 0
    mov ebx,[UnusedBit]
%endmacro

%macro SCSubC 0
    mov ebx,[UnusedBitXor]
%endmacro

%macro SCMainD 0
    and dword[esi],ebx
    or dword[esi+75036*2],ebx
%endmacro

%macro SCSubD 0
    and dword[esi],ebx
%endmacro

%macro ScreenClip 4
    mov esi,[vidbuffer]
    add esi,16*2+288*2
    mov ebx,1
.nextline
    mov al,[scadsng+ebx]
    %1
    test al,0C0h
    jz near .notthisone
    push esi
    push ebx
    and al,0C0h
    cmp al,0C0h
    jne .notentire
    %3
    jmp .startclippingfull
.notentire

    mov dword[ngwinen],0
    test byte[winbg1enval+ebx+5*256],0Ah
    jz .nowindowing
    push eax
    push ebx
    mov al,[winlogicaval+ebx*2+1]
    shr al,2
    and al,03h
    mov [nglogicval],al
    mov eax,ebx
    add ebx,5*256
    ccallv BuildWindow, eax, ebx
;ngwintable
    pop ebx
    pop eax
.nowindowing

    %3
    mov edx,256
    cmp dword[ngwinen],0
    jne .windowenabled
    cmp al,80h
    je near .finclipping
    jmp .startclippingfull
.windowenabled
    mov edi,ngwintable
    mov ecx,[edi]
    add edi,4
    cmp ecx,0
    je .nodec
    dec ecx
    jmp .notzero
.nodec
    dec dword[edi]
.notzero
    cmp al,80h
    je near .outsideclipping
    or ecx,ecx
    jnz near .startclippingb
    jmp .skipclipping
.outsideclipping
    or ecx,ecx
    jnz .noclipping
    mov ecx,[edi]
    add edi,4
.startclippingb
    cmp ecx,256
    jae near .startclippingfull
.startclippingc
    or ecx,ecx
    jz .skipclipping
.startclipping
    %2
    add esi,2
    dec edx
    jz .finclipping
    dec ecx
    jnz .startclipping
.skipclipping
    mov ecx,[edi]
    add edi,4
.noclipping
    sub edx,ecx
    jz .finclipping
    jc .finclipping
    add ecx,ecx
    add esi,ecx
    mov ecx,[edi]
    add edi,4
    jmp .startclippingc
.startclippingfull
    mov ecx,128
.loopclipfull
    %4
    add esi,4
    dec ecx
    jnz .loopclipfull
.finclipping
    pop ebx
    pop esi
.notthisone
    inc ebx
    add esi,288*2
    cmp [resolutn],bx
    jae near .nextline
%endmacro

MainScreenClip:
    ScreenClip SCMainA,SCMainB,SCMainC,SCMainD
SubScreenClip:
    ScreenClip SCSubA,SCSubB,SCSubC,SCSubD
    ret
