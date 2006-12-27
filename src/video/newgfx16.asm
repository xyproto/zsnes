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

EXTSYM BG116x16t,BG1SXl,BG1SYl,BG216x16t,BG2SXl,BG2SYl,BG316x16t,BG3PRI,BG3SXl
EXTSYM BG3SYl,BG416x16t,BG4SXl,BG4SYl,BGFB,BGMA,BGMS1,BGOPT1,BGOPT2,BGOPT3
EXTSYM BGOPT4,BGPT1,BGPT1X,BGPT1Y,BGPT2,BGPT2X,BGPT2Y,BGPT3,BGPT3X,BGPT3Y,BGPT4
EXTSYM BGPT4X,BGPT4Y,bg1drwng,bg1objptr,bg1ptr,bg1ptrx,bg1ptry,bg1scrolx
EXTSYM bg1scroly,bg1totng,bg2drwng,bg2objptr,bg2ptr,bg2ptrx,bg2ptry,bg2scrolx
EXTSYM bg2scroly,bg2totng,bg3drwng,bg3highst,bg3objptr,bg3ptr,bg3ptrx,bg3ptry
EXTSYM bg3scrolx,bg3scroly,bg3totng,bg4drwng,bg4objptr,bg4ptr,bg4ptrx,bg4ptry
EXTSYM bg4scrolx,bg4scroly,bg4totng,bgcmsung,bgmode,bgtxad,bgtxadd,ngextbg
EXTSYM cachesingle2bng,cachesingle8bng,cfieldad,cgmod,cgram,coladdb,coladdg
EXTSYM coladdr,colleft16b,colormodedef,cpalval,csprbit,csprival,curmosaicsz
EXTSYM curvidoffset,curypos,firstdrawn,flipyposng,forceblnk,interlval,intrlng
EXTSYM mode0add,mode0ads,mode7A,mode7C,mode7X0,mode7ab,mode7cd,mode7set,mode7st
EXTSYM mode7xy,modeused,mosaicon,mosaicsz,mosenng,mosszng,ngceax,ngcedi
EXTSYM ngpalcon2b,ngpalcon8b,ngptrdat,pesimpng,prdata,prdatb,prdatc,prevbright
EXTSYM reslbyl,resolutn,scaddset,scaddtype,scadsng,scadtng,scfbl,scrndis,scrnon
EXTSYM spritetablea,sprleftpr,sprlefttot,sprprdrn,sprpriodata,sprtbng,sprtlng
EXTSYM t16x161,t16x162,t16x163,t16x164,taddfy16x16,taddnfy16x16,ngptrdat2
EXTSYM tleftn,tltype2b,tltype8b,vcache2b,vcache8b,vidbright,ofshvaladd
EXTSYM vidbuffer,vidmemch2,vidmemch8,vrama,winon,xtravbuf,yposng
EXTSYM vbufdptr,drawtileng2b16b,drawtileng4b16b,drawtileng8b16b,bgwinchange
EXTSYM drawtileng16x162b16b,drawtileng16x164b16b,drawtileng16x168b16b,winbg1en
EXTSYM drawlineng2b16b,drawlineng4b16b,drawlineng8b16b,BuildWindow,winenabs
EXTSYM drawlineng16x162b16b,drawlineng16x164b16b,drawlineng16x168b16b,winenabm
EXTSYM disableeffects,winl1,winbg1enval,winbg1envalm,winlogica,winlogicaval
EXTSYM winboundary,winobjen,winlogicb,nglogicval,ngwintable,winbg2enval,doveg
EXTSYM winbg3enval,winbg4enval,winbgobjenval,Mode7HiRes16b,res640,hiresstuff
EXTSYM Mode7BackA,Mode7BackC,Mode7BackX0,Mode7BackSet,drawmode7win16b,ngwinen
EXTSYM drawlineng16x84b16b,drawlineng16x82b16b,ofsmcyps,vram,ofsmcptr,ofsmady
EXTSYM ofsmadx,ofsmtptr,yposngom,flipyposngom,ofsmmptr,ofsmval,ofsmvalh,V8Mode
EXTSYM cbgval,drawlinengom4b16b,ignor512,winbg1envals,m7starty
EXTSYM FillSubScr,scanlines,drawmode7win16bd,SpecialLine,vidmemch2s,dovegrest
EXTSYM drawlinengom16x164b16b,bgallchange
EXTSYM bg1change,bg2change,bg3change,bg4change,ngwinptr,objwlrpos,objwen
EXTSYM objclineptr,CSprWinPtr,BuildWindow2,NGNumSpr,fulladdtab,MMXSupport
EXTSYM bgtxadd2,gammalevel16b,drawmode7ngextbg16b,processmode7hires16b
EXTSYM processmode7hires16bd,drawmode7ngextbg216b,osm2dis,ofsmtptrs,ofsmcptr2

%ifdef __MSDOS__
EXTSYM smallscreenon,ScreenScale
%endif

%include "video/vidmacro.mac"
%include "video/newgfx16.mac"
%include "video/newg162.mac"

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

SECTION .text

NEWSYM setpalallng

    mov dword[palchanged],1
    mov byte[cgmod],0
    push esi
    push eax
    push edx
    push ebp
    mov esi,[cpalptrng]
    mov byte[colleft16b],0
    add esi,1024
    xor ebp,ebp
    and esi,255*1024
    mov [cpalptrng],esi

    add esi,[vbufdptr]

.loopa
    mov dx,[cgram+ebp]
    mov [prevpal2+ebp],dx
    mov ax,dx
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .nogr
    mov al,31
.nogr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngrposng]
    xor ebx,ebx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .nogg
    mov al,31
.nogg
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[nggposng]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .nogb
    mov al,31
.nogb
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngbposng]
    shl ax,cl
    add bx,ax
    mov ax,bx
    mov [esi],bx     ; standard
    or bx,[UnusedBit]
    mov [esi+512],bx     ; standard
    add esi,2
    add ebp,2
    inc byte[colleft16b]
    jnz near .loopa
    mov al,[vidbright]
    mov [prevbright],al
    pop ebp
    pop edx
    pop eax
    pop esi
    xor ecx,ecx
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret

NEWSYM setpalette16bng
    cmp byte[V8Mode],1
    jne .noveg
    call doveg
.noveg
    mov bl,[vidbright]
    cmp bl,[prevbright]
    jne near setpalallng
    cmp byte[cgmod],0
    je near .skipall
    push esi
    push edi
    push eax
    push edx
    push ebp
    mov byte[cgmod],0
    xor ebp,ebp
    mov esi,[cpalptrng]
    mov edi,esi
    add esi,1024
    and esi,255*1024
    mov [cpalptrng],esi

    add esi,[vbufdptr]
    add edi,[vbufdptr]

    mov byte[colleft16b],0
    jmp .loopa
.skipa
    mov bx,[edi]
    mov [esi],bx
    mov bx,[edi+512]
    mov [esi+512],bx
    add edi,2
    add esi,2
    add ebp,2
    inc byte[colleft16b]
    jz near .endpal
.loopa
    mov dx,[cgram+ebp]
    cmp [prevpal2+ebp],dx
    je .skipa
    mov [prevpal2+ebp],dx
    cmp byte[colleft16b],0
    je .notchanged
    mov dword[palchanged],1
.notchanged
    mov ax,dx
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norr
    mov al,31
.norr
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngrposng]
    xor ebx,ebx
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,5
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norg
    mov al,31
.norg
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[nggposng]
    shl ax,cl
    add bx,ax
    mov ax,dx
    shr ax,10
    and al,01Fh
    add al,[gammalevel16b]
    cmp al,31
    jbe .norb
    mov al,31
.norb
    mov cl,[vidbright]
    mul cl
    mov cl,15
    div cl
    xor ah,ah
    mov cl,[ngbposng]
    shl ax,cl
    add bx,ax
    mov [esi],bx     ; standard
    or bx,[UnusedBit]
    mov [esi+512],bx     ; standard
    add edi,2
    add esi,2
    add ebp,2
    inc byte[colleft16b]
    jnz near .loopa
.endpal
    pop ebp
    pop edx
    pop eax
    pop edi
    pop esi
    xor ecx,ecx
.skipall
    cmp byte[V8Mode],1
    jne .noveg2
    call dovegrest
.noveg2
    ret

section .data
prevpal2 times 256 dw 0F00Fh
section .text

%macro WinBGCheck 1
    mov bl,[winbg1en+%1]
    mov bh,bl
    test bl,0Ah
    jz %%disable

    test byte[scrnon],1 << %1
    jz %%nomainbg
    test byte[scrnon+1],1 << %1
    jnz %%bg
    xor bh,bh
    jmp %%bg
%%nomainbg
    xor bl,bl
    test byte[scrnon+1],1 << %1
    jnz %%bg
    xor bh,bh
    jmp %%skip
%%disable
    xor bl,bl
    xor bh,bh
    jmp %%skip
%%bg
    test byte[winenabs],1 << %1
    jnz %%nbgs
    xor bh,bh
%%nbgs
    test byte[winenabm],1 << %1
    jnz %%nbgm
    xor bl,bl
%%nbgm
    jmp %%skip
    mov cl,bl
    or cl,bh
    and cl,0Ah
    cmp cl,0Ah
    je %%skip
    mov ch,bl
    or ch,bh
    mov edx,[winl1]
    cmp cl,02h
    je %%bg1
    shr ch,2
    shr edx,16
%%bg1
    test ch,01h
    jnz %%outside
    cmp dl,dh
    jbe %%skip
    xor bl,bl
    xor bh,bh
    jmp %%skip
%%outside
    cmp dl,0
    ja %%skip
    cmp dh,255
    jb %%skip
    xor bl,bl
    xor bh,bh
%%skip
    mov [winbg1envalm+eax+%1*256],bl
    mov [winbg1envals+eax+%1*256],bh
    or bl,bh
    mov [winbg1enval+eax+%1*256],bl
%endmacro

%macro WinBGCheckb 1
    mov bl,[winbg1en+%1]
    test bl,0Ah
    jz %%disable
    test byte[scrnon],1 << %1
    jnz %%nbgen
    test byte[winenabs],1 << %1
    jnz %%bg
    jmp %%disable
%%nbgen
    test byte[winenabm],1 << %1
    jnz %%bg
%%disable
    xor bl,bl
    jmp %%skip
%%bg
    mov cl,bl
    and cl,0Ah
    cmp cl,0Ah
    je %%skip
;    jne %%notskip
;    and bl,03h
;    and cl,03h
;%%notskip
    mov ch,bl
    mov edx,[winl1]
    cmp cl,02h
    je %%bg1
    shr ch,2
    shr edx,16
%%bg1
    test ch,01h
    jnz %%outside
    cmp dl,dh
    jbe %%skip
    xor bl,bl
    jmp %%skip
%%outside
    cmp dl,0
    ja %%skip
    cmp dh,255
    jb %%skip
    xor bl,bl
%%skip
    mov [winbg1enval+eax+%1*256],bl
    mov [winbg1envalm+eax+%1*256],bl
    mov [winbg1envals+eax+%1*256],bl
%endmacro

%macro WinBGCheck2 1
    mov bl,[winbg1en+%1]
    test bl,0Ah
    jnz %%nodisable
    xor bl,bl
    jmp %%skip
%%nodisable
    mov cl,bl
    or cl,bh
    and cl,0Ah
    cmp cl,0Ah
    je %%skip
    mov ch,bl
    or ch,bh
    mov edx,[winl1]
    cmp cl,02h
    je %%bg1
    shr ch,2
    shr edx,16
%%bg1
    test ch,01h
    jnz %%outside
    cmp dl,dh
    jbe %%skip
    xor bl,bl
    jmp %%skip
%%outside
    cmp dl,0
    ja %%skip
    cmp dh,255
    jb %%skip
    xor bl,bl
%%skip
    mov [winbg1enval+eax+%1*256],bl
%endmacro

section .data
BackAreaAdd dd 0
BackAreaUnFillCol dd 0
BackAreaFillCol dd 0
clinemainsub    dd 0
section .text

BackAreaFill:
    cmp byte[winbg1enval+eax+5*256],0
    je near .nowindowb
    mov ebx,[BackAreaFillCol]
    cmp ebx,[BackAreaUnFillCol]
    je near .nowindowb
    push ecx
    push edx
    push eax
    push edi

    mov ebx,eax
    mov ecx,[vidbuffer]
    add ecx,16*2
    shl eax,9
    add ecx,eax
    mov eax,ebx
    shl eax,6
    add ecx,eax
    add ecx,[BackAreaAdd]

    ; Construct Window in ecx
    mov edi,ngwintable
    sub ecx,2
    mov eax,256
.procnotemptyb
    mov edx,[edi]
    add edi,4
    or edx,edx
    jz .procemptyb
    dec edx
    mov ebx,[BackAreaUnFillCol]
.swloopb
    mov [ecx],ebx
    mov [ecx+4],ebx
    add ecx,8
    sub eax,4
    jc .doneb
    sub edx,4
    jnc .swloopb
    sub eax,edx
    add ecx,edx
    add ecx,edx
    dec eax
    add ecx,2
.procemptyb
    mov edx,[edi]
    dec edx
    add edi,4
    mov ebx,[BackAreaFillCol]
.swloop2b
    mov [ecx],ebx
    mov [ecx+4],ebx
    add ecx,8
    sub eax,4
    jc .doneb
    sub edx,4
    jnc .swloop2b
    sub eax,edx
    add ecx,edx
    add ecx,edx
    dec eax
    add ecx,2
    jmp .procnotemptyb
.doneb
    pop edi
    pop eax
    pop edx
    pop ecx
    jmp .yeswindowb
.nowindowb
    push eax
    push ecx
    mov ebx,eax
    mov ecx,[vidbuffer]
    mov eax,ebx
    add ecx,16*2
    shl eax,9
    add ecx,eax
    mov eax,ebx
    shl eax,6
    add ecx,eax
    add ecx,[BackAreaAdd]
    mov ebx,[BackAreaUnFillCol]
    mov eax,64
.nowinloop
    mov [ecx],ebx
    mov [ecx+4],ebx
    add ecx,8
    dec eax
    jnz .nowinloop
    pop ecx
    pop eax
.yeswindowb
    ret

NEWSYM newengine16b
    ; store line by line data
    ; BGMode, BGxScrollX, BGxScrollY, both BGPtrs
    mov eax,[curypos]
    and eax,0FFh

    cmp eax,1
    jne .noclearcache
    push eax
    push ecx
    mov ebx,vidmemch2s
    mov ecx,1024+512+256
    mov eax,0FFFFFFFFh
.loopcc
    mov [ebx],eax
    add ebx,4
    dec ecx
    jnz .loopcc
    pop ecx
    pop eax
    mov dword[startlinet],0
    mov dword[endlinet],255
    xor ebx,ebx
.noclearcache

    mov byte[bgallchange+eax],0
    mov byte[bgallchange+eax+1],0FFh
    mov byte[bg1change+eax],0
    mov byte[bg2change+eax],0
    mov byte[bg3change+eax],0
    mov byte[bg4change+eax],0
    mov dword[palchanged],0

    ; BG3 Priority
    test byte[scaddset],2
    jz .noscaddset
    or [bgcmsung],ebx
.noscaddset

    mov bl,[bg3highst]
    ;cmp byte[bgmode],7
    ;mov bl,1
    ;je .notmode7
;.notmode7
    mov [BG3PRI+eax],bl
    cmp [BG3PRI+eax-1],bl
    je .nosbg3pr
    mov byte[bgallchange+eax],1
.nosbg3pr

    mov ebx,[scrnon]
    ; clear
    push ecx
    mov cl,[scrnon]
    and cl,1Fh
    or cl,20h
    and cl,[scaddtype]
    and cl,3Fh
    mov byte[FillSubScr+eax],1
    or cl,cl
    jnz .yessub
    xor bh,bh
    mov byte[FillSubScr+eax],0
.yessub
    pop ecx
    test byte[scaddset],2
    jnz .subscrnon
    xor bh,bh
.subscrnon
    or [bgcmsung],ebx
    mov [BGMS1+eax*2],ebx
    cmp [BGMS1+eax*2-2],bx
    je .nosbgms1
    mov byte[bgallchange+eax],1
.nosbgms1

    ; if palette[0] = 0 and transparency is just add to back area,
    ;   set ngmsdraw to 1
    mov byte[clinemainsub],0
    cmp word[cgram],0
    jne .ngmsdraw0
    mov bl,[scrnon]
    and bl,1Fh
    or bl,0E0h
    and bl,[scaddtype]
    cmp bl,20h
    jne .ngmsdraw0
    mov byte[ngmsdraw],1
    mov byte[FillSubScr+eax],0
    mov byte[clinemainsub],1
.ngmsdraw0

    ; Scroll Values
    mov bx,[bg1scrolx]
    mov [BG1SXl+eax*2],bx
    cmp [BG1SXl+eax*2-2],bx
    je .nosbgx1
    mov byte[bg1change+eax],1
.nosbgx1
    mov bx,[bg2scrolx]
    mov [BG2SXl+eax*2],bx
    cmp [BG2SXl+eax*2-2],bx
    je .nosbgx2
    mov byte[bg2change+eax],1
.nosbgx2
    mov bx,[bg3scrolx]
    mov [BG3SXl+eax*2],bx
    cmp [BG3SXl+eax*2-2],bx
    je .nosbgx3
    mov byte[bg3change+eax],1
.nosbgx3
    mov bx,[bg4scrolx]
    mov [BG4SXl+eax*2],bx
    cmp [BG4SXl+eax*2-2],bx
    je .nosbgx4
    mov byte[bg4change+eax],1
.nosbgx4

    mov bx,[bg1scroly]
    mov [BG1SYl+eax*2],bx
    cmp [BG1SYl+eax*2-2],bx
    je .nosbgy1
    mov byte[bg1change+eax],1
.nosbgy1
    mov bx,[bg2scroly]
    mov [BG2SYl+eax*2],bx
    cmp [BG2SYl+eax*2-2],bx
    je .nosbgy2
    mov byte[bg2change+eax],1
.nosbgy2
    mov bx,[bg3scroly]
    mov [BG3SYl+eax*2],bx
    cmp [BG3SYl+eax*2-2],bx
    je .nosbgy3
    mov byte[bg3change+eax],1
.nosbgy3
    mov bx,[bg4scroly]
    mov [BG4SYl+eax*2],bx
    cmp [BG4SYl+eax*2-2],bx
    je .nosbgy4
    mov byte[bg1change+eax],1
.nosbgy4

    ; Background Mode
    mov bl,[bgmode]
    and bl,07h
    mov [BGMA+eax],bl
    cmp bl,4
    je .changedmode4
    cmp [BGMA+eax-1],bl
    je .nobgma
.changedmode4
    mov byte[bgallchange+eax],1
.nobgma

    ; new graphics fix, thanks to TRAC
    and ebx,07h
    mov byte[modeused+ebx],1

    ; Pointer to OBJ tile data
    mov ebx,[bg1objptr]
    mov [BGOPT1+eax*2],ebx
    cmp [BGOPT1+eax*2-2],bx
    je .nosbgo1
    mov byte[bg1change+eax],1
.nosbgo1
    mov ebx,[bg2objptr]
    mov [BGOPT2+eax*2],ebx
    cmp [BGOPT2+eax*2-2],bx
    je .nosbgo2
    mov byte[bg2change+eax],1
.nosbgo2
    mov ebx,[bg3objptr]
    mov [BGOPT3+eax*2],ebx
    cmp [BGOPT3+eax*2-2],bx
    je .nosbgo3
    mov byte[bg3change+eax],1
.nosbgo3
    mov ebx,[bg4objptr]
    mov [BGOPT4+eax*2],ebx
    cmp [BGOPT4+eax*2-2],bx
    je .nosbgo4
    mov byte[bg4change+eax],1
.nosbgo4

    ; Pointer to tile status data
    mov ebx,[bg1ptr]
    mov [BGPT1+eax*2],ebx
    cmp [BGPT1+eax*2-2],bx
    je .nosbgp1
    mov byte[bg1change+eax],1
.nosbgp1
    mov ebx,[bg2ptr]
    mov [BGPT2+eax*2],ebx
    cmp [BGPT2+eax*2-2],bx
    je .nosbgp2
    mov byte[bg2change+eax],1
.nosbgp2
    mov ebx,[bg3ptr]
    mov [BGPT3+eax*2],ebx
    cmp [BGPT3+eax*2-2],bx
    je .nosbgp3
    mov byte[bg3change+eax],1
.nosbgp3
    mov ebx,[bg4ptr]
    mov [BGPT4+eax*2],ebx
    cmp [BGPT4+eax*2-2],bx
    je .nosbgp4
    mov byte[bg4change+eax],1
.nosbgp4

    mov ebx,[bg1ptrx]
    mov [BGPT1X+eax*2],ebx
    cmp [BGPT1X+eax*2-2],bx
    je .nosbgpx1
    mov byte[bg1change+eax],1
.nosbgpx1
    mov ebx,[bg2ptrx]
    mov [BGPT2X+eax*2],ebx
    cmp [BGPT2X+eax*2-2],bx
    je .nosbgpx2
    mov byte[bg2change+eax],1
.nosbgpx2
    mov ebx,[bg3ptrx]
    mov [BGPT3X+eax*2],ebx
    cmp [BGPT3X+eax*2-2],bx
    je .nosbgpx3
    mov byte[bg3change+eax],1
.nosbgpx3
    mov ebx,[bg4ptrx]
    mov [BGPT4X+eax*2],ebx
    cmp [BGPT4X+eax*2-2],bx
    je .nosbgpx4
    mov byte[bg4change+eax],1
.nosbgpx4

    mov ebx,[bg1ptry]
    mov [BGPT1Y+eax*2],ebx
    cmp [BGPT1Y+eax*2-2],bx
    je .nosbgpy1
    mov byte[bg1change+eax],1
.nosbgpy1
    mov ebx,[bg2ptry]
    mov [BGPT2Y+eax*2],ebx
    cmp [BGPT2Y+eax*2-2],bx
    je .nosbgpy2
    mov byte[bg2change+eax],1
.nosbgpy2
    mov ebx,[bg3ptry]
    mov [BGPT3Y+eax*2],ebx
    cmp [BGPT3Y+eax*2-2],bx
    je .nosbgpy3
    mov byte[bg3change+eax],1
.nosbgpy3
    mov ebx,[bg4ptry]
    mov [BGPT4Y+eax*2],ebx
    cmp [BGPT4Y+eax*2-2],bx
    je .nosbgpy4
    mov byte[bg4change+eax],1
.nosbgpy4
    mov ebx,[forceblnk]
    or bl,bl
    jne .dontdraw
    mov dword[scfbl],0
.dontdraw
    ; Variable size write error fix [TRAC]
    mov [BGFB+eax],bl
    cmp [BGFB+eax-1],bl
    je .nosbgfb
    mov byte[bgallchange+eax],1
.nosbgfb

    test byte[interlval],40h
    jz .nointrl
    mov byte[bgallchange+eax],1
.nointrl

    mov ebx,[mode7A]
    mov [mode7ab+eax*4],ebx
    mov ebx,[mode7C]
    mov [mode7cd+eax*4],ebx
    mov ebx,[mode7X0]
    mov [mode7xy+eax*4],ebx
    mov ebx,[mode7set]
    mov [mode7st+eax],ebx

    ; 16x16 tiles
    mov ebx,[BG116x16t]
    mov [t16x161+eax],ebx
    cmp [t16x161+eax-1],bl
    je .not16x161
    mov byte[bg1change+eax],1
.not16x161
    mov ebx,[BG216x16t]
    mov [t16x162+eax],ebx
    cmp [t16x162+eax-1],bl
    je .not16x162
    mov byte[bg2change+eax],1
.not16x162
    mov ebx,[BG316x16t]
    mov [t16x163+eax],ebx
    cmp [t16x163+eax-1],bl
    je .not16x163
    mov byte[bg3change+eax],1
.not16x163
    mov ebx,[BG416x16t]
    mov [t16x164+eax],ebx
    cmp [t16x164+eax-1],bl
    je .not16x164
    mov byte[bg4change+eax],1
.not16x164

;    mov byte[mode7hr+eax],0
    cmp byte[bgmode],7
    jne .noextbg
;    cmp byte[res640],0
;    je .nomode7512
;    mov byte[mode7hr+eax],1
;.nomode7512
    test byte[interlval],40h
    jz .noextbg
    mov byte[ngextbg],1
.noextbg

    ; mosaic
    mov ebx,[mosaicon]
    mov [mosenng+eax],ebx
    mov ebx,[mosaicsz]
    mov [mosszng+eax],ebx

    ; Interlaced
    mov ebx,[interlval]
    mov [intrlng+eax],ebx

    ; Set palette
    call setpalette16bng

    cmp dword[palchanged],1
    jne .notpchanged
    cmp eax,112
    jae .endl
    mov [startlinet],eax
    jmp .notpchanged
.endl
    cmp dword[endlinet],255
    jb .notpchanged
    mov [endlinet],eax
.notpchanged

    mov ebx,[cpalptrng]
    add ebx,[vbufdptr]
    mov [cpalval+eax*4],ebx

    ; Set Transparency
    mov bl,[scaddtype]
    mov [scadtng+eax],bl
    cmp [scadtng+eax-1],bl
    je .noscadt
    mov byte[bgallchange+eax],1
.noscadt

    mov bl,[scaddset]
    mov [scadsng+eax],bl
    cmp [scadsng+eax-1],bl
    je .noscads
    mov byte[bgallchange+eax],1
.noscads

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
    WinBGCheck 0
    WinBGCheck 1
    WinBGCheck 2
    WinBGCheck 3
    WinBGCheck 4
    WinBGCheck2 5

    mov ebx,[winlogica]
    mov [winlogicaval+eax*2],ebx
    cmp [winlogicaval+eax*2-2],bx
    je .winnchangedb
    mov byte[bgwinchange+eax],1
.winnchangedb
    mov ebx,[winl1]
    mov [winboundary+eax*4],ebx
    cmp [winboundary+eax*4-4],ebx
    je .winnchanged
    mov byte[bgwinchange+eax],1
.winnchanged

    ; generate sprite window
    cmp byte[winbg1enval+eax+4*256],0
    je near .skipobjw

    mov ebx,[winl1]
    mov dl,[winobjen]
    mov dh,[winlogicb]
    and dh,03h
    ; Same as previous line?
    cmp dword[objwlrpos+eax*4-4],0FFFFFFFFh
    je .changed
    cmp [objwlrpos+eax*4-4],ebx
    jne .changedb
    cmp [objwen+eax*2-2],dx
    je near .notchanged
.changedb
    cmp [objwlrpos+eax*4],ebx
    jne .changed
    cmp [objwen+eax*2],dx
    jne .changed
    mov ecx,[CSprWinPtr]
    cmp [objclineptr+eax*4],ecx
    ja near .usecurrent
.changed
    mov [objwlrpos+eax*4],ebx
    mov [objwen+eax*2],dx

    mov bl,[winlogicb]
    and bl,03h
    mov [nglogicval],bl
    mov ebx,4*256
    add ebx,eax
    mov dword[ngwinen],0
    call BuildWindow2
    cmp dword[ngwinen],0
    je near .disablesprwin
    mov ecx,[CSprWinPtr]
    add ecx,260
    mov [CSprWinPtr],ecx
    mov [objclineptr+eax*4],ecx
    add ecx,[ngwinptr]
    ; Construct Window in ecx
    push eax
    mov ebx,ngwintable
    dec ecx
    mov eax,256
.procnotempty
    mov edx,[ebx]
    add ebx,4
    or edx,edx
    jz .procempty
    dec edx
.swloop
    mov dword[ecx],0
    add ecx,4
    sub eax,4
    jc .done
    sub edx,4
    jnc .swloop
    sub eax,edx
    add ecx,edx
    dec eax
    inc ecx
.procempty
    mov edx,[ebx]
    dec edx
    add ebx,4
.swloop2
    mov dword[ecx],01010101h
    add ecx,4
    sub eax,4
    jc .done
    sub edx,4
    jnc .swloop2
    sub eax,edx
    add ecx,edx
    dec eax
    inc ecx
    jmp .procnotempty
.done
    pop eax
    jmp .skipobjw
.usecurrent
    mov ecx,[objclineptr+eax*4]
    mov [CSprWinPtr],ecx
    cmp ecx,0FFFFFFFFh
    jnz .skipobjw
    jmp .disablesprwin
    ; copy over if it's the same
.notchanged
    mov [objwlrpos+eax*4],ebx
    mov [objwen+eax*2],dx
    mov ebx,[objclineptr+eax*4-4]
    mov [objclineptr+eax*4],ebx
    cmp ebx,0FFFFFFFFh
    je .disablesprwin
.skipobjw
    pop edx
    pop ecx
    jmp .okaywin
.disablesprwin
    mov dword[objclineptr+eax*4],0FFFFFFFFh
    mov byte[winbg1enval+eax+4*256],0
    mov byte[winbg1envals+eax+4*256],0
    mov byte[winbg1envalm+eax+4*256],0
    pop edx
    pop ecx
    jmp .okaywin
.finishwin
    mov byte[winbg1enval+eax],0
    mov byte[winbg2enval+eax],0
    mov byte[winbg3enval+eax],0
    mov byte[winbg4enval+eax],0
    mov byte[winbgobjenval+eax],0
.okaywin
    xor ebx,ebx

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
    cmp byte[scrnon+1],0
    jne .notblack
;    mov byte[clinemainsub],1
    test byte[scadtng+eax],40h
    jnz .notblack
    xor byte[scadtng+eax],1
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
    call BuildWindow2
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
    call BackAreaFill
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
    call BackAreaFill
.nosubscreen2
    pop edx
    pop ecx

    mov byte[SpecialLine+eax],0
%ifdef __MSDOS__
    cmp byte[smallscreenon],1
    je .nomode7hr
    cmp byte[ScreenScale],1
    je .nomode7hr
%endif
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
    or [sprprdrn],ebx
    xor ebx,ebx
    ret
.single
    or [sprprdrn],ebx
    or dword[sprleftpr+eax*4],80000000h
    xor ebx,ebx
    ret

section .data
align 32
NEWSYM ngwinenval,  dd 0
NEWSYM cdrawbuffer, dd 0
NEWSYM draw16bnng,  dd 0
NEWSYM scaddsngb,   dd 0
NEWSYM scaddtngb,   dd 0
NEWSYM scaddtngbx,  dd 0
NEWSYM prevbcolng,  dd 0
NEWSYM bcolvalng,   dd 0
NEWSYM cebppos,     dd 0
NEWSYM subscreenonng, dd 0
NEWSYM cdrawmeth,   dd 0
NEWSYM cpalptrng,   dd 0
NEWSYM prevcoladdrng, dd 0
NEWSYM prevcolvalng,  dd 0
NEWSYM subscrng,      dd 0
NEWSYM ngmsdraw,      dd 0
NEWSYM CMainWinScr,   dd 0
NEWSYM CSubWinScr,    dd 0
NEWSYM Prevcoladdr,   dd 0
NEWSYM ColResult,     dd 0
NEWSYM CPalPtrng,     dd 0
NEWSYM WindowRedraw,  dd 0
NEWSYM mostranspval,  dd 0
NEWSYM mosclineval,   dd 0
NEWSYM startlinet,    dd 0
NEWSYM endlinet,      dd 0
NEWSYM palchanged,    dd 0

NEWSYM ng16bbgval, dd 0         ; bg # (mov dword[ng16bbgval],%1)
NEWSYM ng16bprval, dd 0         ; 0 = pr0, 2000h = pr1

NEWSYM mosjmptab16b, times 15 dd 0
NEWSYM mosjmptab16bt, times 15 dd 0
NEWSYM mosjmptab16btms, times 15 dd 0
NEWSYM mosjmptab16bntms, times 15 dd 0
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

    mov byte[firstdrawn],1
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
    mov dword[subscrng],1

    ; Draw SubScreens
    test byte[scrndis],8h
    jnz near .nodobg4s
    test dword[bgcmsung],800h
    jz near .nodobg4s
    mov dword[mode0ads],60606060h
    Procbgpr016b 3, drawbg4line16b, drawbg4tile16b, ngsub, 8h
.nodobg4s
    test byte[scrndis],4h
    jnz near .nodobg3s
    test dword[bgcmsung],400h
    jz near .nodobg3s
    mov dword[mode0ads],40404040h
    Procbg3pr016b 2, drawbg3line16b, drawbg3tile16b, ngsub, 4h
.nodobg3s

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprpra
    test dword[modeused],00000101h
    jz near .nosprpra
    test dword[bgcmsung],1000h
    jz near .nosprpra
    Procsprng0116b ngsub, 10h
.nosprpra

    test byte[scrndis],8h
    jnz near .nodobg4sb
    test dword[bgcmsung],800h
    jz near .nodobg4sb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4sb
    mov dword[mode0ads],60606060h
    Procbgpr116b 3, drawbg4linepr116b, drawbg4tilepr116b, prdata, ngsub, 8h
.nodobg4sb
    test byte[scrndis],4h
    jnz near .nodobg3sb
    test dword[bgcmsung],400h
    jz near .nodobg3sb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3sb
    mov dword[mode0ads],40404040h
    Procbg3pr116b 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, ngsub, 4h
.nodobg3sb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprb
    test dword[modeused],00000101h
    jz near .nosprprb
    test dword[bgcmsung],1000h
    jz near .nosprprb
    Procsprng0116b ngsub, 10h
.nosprprb

    test byte[scrndis],2h
    jnz near .nodobg2s
    test dword[bgcmsung],200h
    jz near .nodobg2s
    mov dword[mode0ads],20202020h
    Procbgpr016b 1, drawbg2line16b, drawbg2tile16b, ngsub, 2h
.nodobg2s

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgsc
    test dword[bgcmsung],300h
    jz near .noextbgsc
    ProcMode7ngextbg16b ngsub, 3h
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
    Procsprng23456716b ngsub, 10h
.nosprprc

    test byte[scrndis],1h
    jnz near .nodobg1s
    test dword[bgcmsung],100h
    jz near .nodobg1s
    mov dword[mode0ads],00000000h
    Procbgpr016b 0, drawbg1line16b, drawbg1tile16b, ngsub, 1h
.nodobg1s

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7
    test dword[bgcmsung],300h
    jz near .nomode7
    ProcMode7ng16b ngsub, 1h
.nomode7

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprd
    test dword[bgcmsung],1000h
    jz near .nosprprd
    Procsprng16b ngsub, 10h
.nosprprd

    test byte[scrndis],2h
    jnz near .nodobg2sb
    test dword[bgcmsung],200h
    jz near .nodobg2sb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2sb
    mov dword[mode0ads],20202020h
    Procbgpr116b 1, drawbg2linepr116b, drawbg2tilepr116b, prdata, ngsub, 2h
.nodobg2sb

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgscb
    test dword[bgcmsung],300h
    jz near .noextbgscb
    ProcMode7ngextbg216b ngsub, 2h
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
    Procsprng23456716b ngsub, 10h
.nosprpre

    test byte[scrndis],1h
    jnz near .nodobg1sb
    test dword[bgcmsung],100h
    jz near .nodobg1sb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1sb
    mov dword[mode0ads],00000000h
    Procbgpr116b 0, drawbg1linepr116b, drawbg1tilepr116b, prdatb, ngsub, 1h
.nodobg1sb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprf
    test dword[bgcmsung],1000h
    jz near .nosprprf
    Procsprng16b ngsub, 10h
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
    Procbg3pr1b16b 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, ngsub, 4h
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
    mov dword[subscrng],0

    ; Draw MainScreens
    test byte[scrndis],8h
    jnz near .nodobg4m
    test dword[bgcmsung],808h
    jz near .nodobg4m
    mov dword[mode0ads],60606060h
    Procbgpr016b 3, drawbg4line16b, drawbg4tile16b, ngmain, 8h
.nodobg4m
    test byte[scrndis],4h
    jnz near .nodobg3m
    test dword[bgcmsung],404h
    jz near .nodobg3m
    mov dword[mode0ads],40404040h
    Procbg3pr016b 2, drawbg3line16b, drawbg3tile16b, ngmain, 4h
.nodobg3m

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprma
    test dword[modeused],00000101h
    jz near .nosprprma
    test dword[bgcmsung],1010h
    jz near .nosprprma
    Procsprng0116b ngmain, 10h
.nosprprma

    test byte[scrndis],8h
    jnz near .nodobg4mb
    test dword[bgcmsung],808h
    jz near .nodobg4mb
    mov eax,[bg4totng]
    cmp eax,[bg4drwng]
    je near .nodobg4mb
    mov dword[mode0ads],60606060h
    Procbgpr116b 3, drawbg4linepr116b, drawbg4tilepr116b, prdata, ngmain, 8h
.nodobg4mb
    test byte[scrndis],4h
    jnz near .nodobg3mb
    test dword[bgcmsung],4h
    jz near .nodobg3mb
    mov eax,[bg3totng]
    cmp eax,[bg3drwng]
    je near .nodobg3mb
    mov dword[mode0ads],40404040h
    Procbg3pr116b 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, ngmain, 4h
.nodobg3mb

    ; draw sprites mode 0-1
    test byte[scrndis],10h
    jnz near .nosprprmb
    test dword[modeused],00000101h
    jz near .nosprprmb
    test dword[bgcmsung],1010h
    jz near .nosprprmb
    Procsprng0116b ngmain, 10h
.nosprprmb

    test byte[scrndis],2h
    jnz near .nodobg2m
    test dword[bgcmsung],202h
    jz near .nodobg2m
    mov dword[mode0ads],20202020h
    Procbgpr016b 1, drawbg2line16b, drawbg2tile16b, ngmain, 2h
.nodobg2m

    ; draw mode 7 extbg pr 0
    cmp byte[ngextbg],0
    je near .noextbgmn
    test dword[bgcmsung],303h
    jz near .noextbgmn
    ProcMode7ngextbg16b ngmain ,3h
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
    Procsprng23456716b ngmain ,10h
.nosprprmc

    test byte[scrndis],1h
    jnz near .nodobg1m
    test dword[bgcmsung],101h
    jz near .nodobg1m
    mov dword[mode0ads],00000000h
    Procbgpr016b 0, drawbg1line16b, drawbg1tile16b, ngmain,  1h
.nodobg1m

    ; draw mode 7, priority 0
    cmp byte[modeused+7],0
    je near .nomode7m
    test dword[bgcmsung],101h
    jz near .nomode7m
    ProcMode7ng16b ngmain ,1h
.nomode7m

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmd
    test dword[bgcmsung],1010h
    jz near .nosprprmd
    Procsprng16b ngmain ,10h
.nosprprmd

    ; draw mode 7 extbg pr 1
    cmp byte[ngextbg],0
    je near .noextbgmn2
    test dword[bgcmsung],303h
    jz near .noextbgmn2
    ProcMode7ngextbg216b ngmain ,2h
.noextbgmn2

    test byte[scrndis],2h
    jnz near .nodobg2mb
    test dword[bgcmsung],202h
    jz near .nodobg2mb
    mov eax,[bg2totng]
    cmp eax,[bg2drwng]
    je near .nodobg2mb
    mov dword[mode0ads],20202020h
    Procbgpr116b 1, drawbg2linepr116b, drawbg2tilepr116b, prdata, ngmain, 2h
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
    Procsprng23456716b ngmain ,10h
.nosprprme

    test byte[scrndis],1h
    jnz near .nodobg1mb
    test dword[bgcmsung],101h
    jz near .nodobg1mb
    mov eax,[bg1totng]
    cmp eax,[bg1drwng]
    je near .nodobg1mb
    mov dword[mode0ads],00000000h
    Procbgpr116b 0, drawbg1linepr116b, drawbg1tilepr116b, prdatb, ngmain, 1h
.nodobg1mb

    ; draw sprites mode 0-7
    test byte[scrndis],10h
    jnz near .nosprprmf
    test dword[bgcmsung],1010h
    jz near .nosprprmf
    Procsprng16b ngmain, 10h
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
    Procbg3pr1b16b 2, drawbg3linepr116b, drawbg3tilepr116b, prdatc, ngmain, 4h
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


NEWSYM domosaicng16b
    mov esi,[pesimpng]
    xor eax,eax
    mov edi,xtravbuf+32
    mov al,[curmosaicsz]
    cmp al,16
    ja .notokay
    cmp al,1
    jbe .notokay
    push esi
    push ebx
    mov ebx,[mosclineval]
    mov cl,[mostranspval]
    cmp byte[BGMA+ebx],7
    je .main
    test byte[BGMS1+ebx*2],cl
    jz .nosubmain
    test byte[FillSubScr+ebx],1
    jnz .transpstuff
    jmp .main
.nosubmain
    test byte[FillSubScr+ebx],1
    jz .main
    add esi,75036*2
.main
    pop ebx
    jmp [mosjmptab16b+eax*4-8]
    pop esi
.notokay
    ret
.transpstuff
    test byte[BGMS1+ebx*2+1],cl
    jnz near .mosaicms
    test byte[scadtng+ebx],cl
    jz near .main
    pop ebx
    jmp [mosjmptab16bt+eax*4-8]
    pop esi
    ret
.mosaicms
    test byte[scadtng+ebx],cl
    jz near .mosaicmsnt
    pop ebx
    jmp [mosjmptab16btms+eax*4-8]
    pop esi
    ret
.mosaicmsnt
    pop ebx
    jmp [mosjmptab16bntms+eax*4-8]
    pop esi
    ret

%macro mosaic2 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
%endmacro
%macro mosaic3 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
%endmacro
%macro mosaic4 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
%endmacro
%macro mosaic5 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
%endmacro
%macro mosaic6 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
%endmacro
%macro mosaic7 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
%endmacro
%macro mosaic8 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
%endmacro
%macro mosaic9 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
%endmacro
%macro mosaic10 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
%endmacro
%macro mosaic11 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
%endmacro
%macro mosaic12 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
    mov [esi+%1+22],ax
%endmacro
%macro mosaic13 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
    mov [esi+%1+22],ax
    mov [esi+%1+24],ax
%endmacro
%macro mosaic14 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
    mov [esi+%1+22],ax
    mov [esi+%1+24],ax
    mov [esi+%1+26],ax
%endmacro
%macro mosaic15 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
    mov [esi+%1+22],ax
    mov [esi+%1+24],ax
    mov [esi+%1+26],ax
    mov [esi+%1+28],ax
%endmacro
%macro mosaic16 1
    mov [esi+%1],ax
    mov [esi+%1+2],ax
    mov [esi+%1+4],ax
    mov [esi+%1+6],ax
    mov [esi+%1+8],ax
    mov [esi+%1+10],ax
    mov [esi+%1+12],ax
    mov [esi+%1+14],ax
    mov [esi+%1+16],ax
    mov [esi+%1+18],ax
    mov [esi+%1+20],ax
    mov [esi+%1+22],ax
    mov [esi+%1+24],ax
    mov [esi+%1+26],ax
    mov [esi+%1+28],ax
    mov [esi+%1+30],ax
%endmacro

%macro mosaicproc 3
    mov ecx,%1
.next
    mov ax,[edi]
    cmp ax,0FFFFh
    je .nodraw
    %2 0
    mosender %3
%endmacro

%macro mosaicproct 3
    mov ecx,%1
.next
    mov ax,[edi]
    cmp ax,0FFFFh
    je .nodraw
    or ax,[UnusedBit]
    %2 0
    mosender %3
%endmacro

%macro mosaicprocntms 3
    mov ecx,%1
.next
    mov ax,[edi]
    cmp ax,0FFFFh
    je near .nodraw
    %2 0
    %2 75036*2
    mosender %3
%endmacro

%macro mosaicproctms 3
    mov ecx,%1
.next
    mov ax,[edi]
    cmp ax,0FFFFh
    je near .nodraw
    or ax,[UnusedBit]
    %2 0
    and ax,[UnusedBit]
    %2 75036*2
    mosender %3
%endmacro

NEWSYM mosdraw216b
    mosaicproc 128,mosaic2,2
NEWSYM mosdraw316b
    mosaicproc 86,mosaic3,3
NEWSYM mosdraw416b
    mosaicproc 64,mosaic4,4
NEWSYM mosdraw516b
    mosaicproc 52,mosaic5,5
NEWSYM mosdraw616b
    mosaicproc 43,mosaic6,6
NEWSYM mosdraw716b
    mosaicproc 37,mosaic7,7
NEWSYM mosdraw816b
    mosaicproc 32,mosaic8,8
NEWSYM mosdraw916b
    mosaicproc 29,mosaic9,9
NEWSYM mosdraw1016b
    mosaicproc 26,mosaic10,10
NEWSYM mosdraw1116b
    mosaicproc 24,mosaic11,11
NEWSYM mosdraw1216b
    mosaicproc 22,mosaic12,12
NEWSYM mosdraw1316b
    mosaicproc 20,mosaic13,13
NEWSYM mosdraw1416b
    mosaicproc 19,mosaic14,14
NEWSYM mosdraw1516b
    mosaicproc 18,mosaic15,15
NEWSYM mosdraw1616b
    mosaicproc 16,mosaic16,16

NEWSYM mosdraw216bt
    mosaicproct 128,mosaic2,2
NEWSYM mosdraw316bt
    mosaicproct 86,mosaic3,3
NEWSYM mosdraw416bt
    mosaicproct 64,mosaic4,4
NEWSYM mosdraw516bt
    mosaicproct 52,mosaic5,5
NEWSYM mosdraw616bt
    mosaicproct 43,mosaic6,6
NEWSYM mosdraw716bt
    mosaicproct 37,mosaic7,7
NEWSYM mosdraw816bt
    mosaicproct 32,mosaic8,8
NEWSYM mosdraw916bt
    mosaicproct 29,mosaic9,9
NEWSYM mosdraw1016bt
    mosaicproct 26,mosaic10,10
NEWSYM mosdraw1116bt
    mosaicproct 24,mosaic11,11
NEWSYM mosdraw1216bt
    mosaicproct 22,mosaic12,12
NEWSYM mosdraw1316bt
    mosaicproct 20,mosaic13,13
NEWSYM mosdraw1416bt
    mosaicproct 19,mosaic14,14
NEWSYM mosdraw1516bt
    mosaicproct 18,mosaic15,15
NEWSYM mosdraw1616bt
    mosaicproct 16,mosaic16,16

NEWSYM mosdraw216btms
    mosaicproctms 128,mosaic2,2
NEWSYM mosdraw316btms
    mosaicproctms 86,mosaic3,3
NEWSYM mosdraw416btms
    mosaicproctms 64,mosaic4,4
NEWSYM mosdraw516btms
    mosaicproctms 52,mosaic5,5
NEWSYM mosdraw616btms
    mosaicproctms 43,mosaic6,6
NEWSYM mosdraw716btms
    mosaicproctms 37,mosaic7,7
NEWSYM mosdraw816btms
    mosaicproctms 32,mosaic8,8
NEWSYM mosdraw916btms
    mosaicproctms 29,mosaic9,9
NEWSYM mosdraw1016btms
    mosaicproctms 26,mosaic10,10
NEWSYM mosdraw1116btms
    mosaicproctms 24,mosaic11,11
NEWSYM mosdraw1216btms
    mosaicproctms 22,mosaic12,12
NEWSYM mosdraw1316btms
    mosaicproctms 20,mosaic13,13
NEWSYM mosdraw1416btms
    mosaicproctms 19,mosaic14,14
NEWSYM mosdraw1516btms
    mosaicproctms 18,mosaic15,15
NEWSYM mosdraw1616btms
    mosaicproctms 16,mosaic16,16

NEWSYM mosdraw216bntms
    mosaicprocntms 128,mosaic2,2
NEWSYM mosdraw316bntms
    mosaicprocntms 86,mosaic3,3
NEWSYM mosdraw416bntms
    mosaicprocntms 64,mosaic4,4
NEWSYM mosdraw516bntms
    mosaicprocntms 52,mosaic5,5
NEWSYM mosdraw616bntms
    mosaicprocntms 43,mosaic6,6
NEWSYM mosdraw716bntms
    mosaicprocntms 37,mosaic7,7
NEWSYM mosdraw816bntms
    mosaicprocntms 32,mosaic8,8
NEWSYM mosdraw916bntms
    mosaicprocntms 29,mosaic9,9
NEWSYM mosdraw1016bntms
    mosaicprocntms 26,mosaic10,10
NEWSYM mosdraw1116bntms
    mosaicprocntms 24,mosaic11,11
NEWSYM mosdraw1216bntms
    mosaicprocntms 22,mosaic12,12
NEWSYM mosdraw1316bntms
    mosaicprocntms 20,mosaic13,13
NEWSYM mosdraw1416bntms
    mosaicprocntms 19,mosaic14,14
NEWSYM mosdraw1516bntms
    mosaicprocntms 18,mosaic15,15
NEWSYM mosdraw1616bntms
    mosaicprocntms 16,mosaic16,16


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

%macro normalsprng16b 2
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    add edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa16b %1
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov ch,[esi+6]
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf16b %1
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,ecx
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    or byte[esi+4],0        ;this prevents some games from crashing
    jz near .exitnow
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawa16b %2
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
.exitnow
    pop ebx
    pop esi
    ret
.drawspriteflipx2
    mov bx,[esi]
    mov ch,[esi+6]
    mov esi,[esi+2]
    sprdrawaf16b %2
    sub edx,8
    mov esi,edx
    dec cl
    jnz near .loopobj2
    pop ebx
    pop esi
    ret
%endmacro


%macro normalwsprng16b 2
;    cmp byte[winbg1enval+eax+4*256],0
;    je near .skipobjw
    xor eax,eax
    mov [NGNumSpr],cl
    mov ecx,[objclineptr+ebx*4]
    add ecx,[ngwinptr]
    test dword[sprleftpr+ebx*4],80000000h
    jnz near .drawsingle
    push esi
    push ebx
    add edi,esi
    mov esi,[sprtbng+ebx*4]
    mov edx,esi
    xor ebx,ebx
.loopobj
    test byte[esi+7],20h
    jnz near .drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notprio
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa16b %1
    pop edx
.nodrawspr
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notprio
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawa sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx
    mov bx,[esi]
    push edx
    mov dl,[esi+7]
    and edx,03h
    cmp edx,[csprival]
    jne near .notpriof
    mov dh,[esi+6]
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf16b %1
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
.endobj
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.notpriof
    mov esi,[esi+2]
    mov dl,[csprbit]
    sprdrawaf sprdrawpra2
    pop edx
    add edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj
    rol byte[csprbit],1
    cmp byte[csprbit],1
    je near .clearcsprmem
    pop ebx
    pop esi
    xor ecx,ecx
    xor ecx,ecx
    ret
.clearcsprmem
    xor eax,eax
    mov ecx,64
    mov edi,sprpriodata+16
    rep stosd
    pop ebx
    pop esi
    ret

.drawsingle
    push esi
    push ebx
    mov edi,esi
    mov esi,[sprtbng+ebx*4]
    xor edx,edx
    mov dl,[NGNumSpr]
    and edx,0FFh
    shl edx,3
    sub edx,8
    add edx,esi
    mov esi,edx
    xor ebx,ebx
.loopobj2
    test byte[esi+7],20h
    jnz near .drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawa16b %2
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret
.drawspriteflipx2
    push edx
    mov bx,[esi]
    mov dh,[esi+6]
    mov esi,[esi+2]
    sprdrawaf16b %2
    pop edx
    sub edx,8
    mov esi,edx
    dec byte[NGNumSpr]
    jnz near .loopobj2
    pop ebx
    pop esi
    xor ecx,ecx
    ret
%endmacro

; FillSubScr bit 0 sets to 1 if there is no subscreen present
; ms,wms,wm,ws
; FillSubScr scadtng
NEWSYM drawsprng16b
    test byte[BGMS1+ebx*2],10h
    jz .nosubmain
    test byte[FillSubScr+ebx],1
    jnz near drawsprng16bt
.nosubmain
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    mov edi,[CMainWinScr]
    test byte[FillSubScr+ebx],1
    jz .main2
    test byte[BGMS1+ebx*2],10h
    jnz .main2
    mov edi,[CSubWinScr]
.main2
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16b
    test byte[FillSubScr+ebx],1
    jz .main
    test byte[BGMS1+ebx*2],10h
    jnz .main
    add esi,75036*2
.main
    xor edi,edi
    normalsprng16b sprdrawpra16bng,sprdrawprb16bng
NEWSYM drawsprngw16b
    xor edi,edi
    test byte[FillSubScr+ebx],1
    jz .main
    test byte[BGMS1+ebx*2],10h
    jnz .main
    add esi,75036*2
.main
    normalwsprng16b sprdrawprawb16bng,sprdrawprbwb16bng

drawsprng16bt:
    test byte[scadtng+ebx],10h
    jz near drawsprng16bnt
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    test byte[BGMS1+ebx*2+1],10h
    jnz near drawsprng16bmst
    mov al,[BGMS1+ebx*2]
    shr al,2
    test byte[BGMS1+ebx*2],al
    jnz .transpwin
    test byte[scaddset],0C0h
    jz .transpwin
    cmp byte[BGMS1+ebx*2+1],0
    jnz .main
.transpwin
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bt
.main
    xor edi,edi
    normalsprng16b sprdrawpra16bngt,sprdrawprb16bngt
NEWSYM drawsprngw16bt
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngt,sprdrawprbwb16bngt
drawsprng16bmst:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bst
    xor edi,edi
    normalsprng16b sprdrawpra16bngmst,sprdrawprb16bngmst
drawsprngw16bst:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngst,sprdrawprbwb16bngst
drawsprngw16bmt:
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmst
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmt,sprdrawprbwb16bngmt
drawsprngw16bmst:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmst,sprdrawprbwb16bngmst

drawsprng16bnt:
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    test byte[BGMS1+ebx*2+1],10h
    jnz near drawsprng16bmsnt
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16b
    xor edi,edi
    normalsprng16b sprdrawpra16bngnt,sprdrawprb16bngnt
drawsprngw16bnt
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngnt,sprdrawprbwb16bngnt
drawsprng16bmsnt:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmnt
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bsnt
    xor edi,edi
    normalsprng16b sprdrawpra16bngmsnt,sprdrawprb16bngmsnt
drawsprngw16bsnt:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngsnt,sprdrawprbwb16bngsnt
drawsprngw16bmnt:
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmsnt
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmnt,sprdrawprbwb16bngmnt
drawsprngw16bmsnt:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmsnt,sprdrawprbwb16bngmsnt

NEWSYM drawsprng16bhr
    test byte[BGMS1+ebx*2],10h
    jz .nosubmain
    test byte[FillSubScr+ebx],1
    jnz near drawsprng16bthr
.nosubmain
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    mov edi,[CMainWinScr]
    test byte[FillSubScr+ebx],1
    jz .main2
    test byte[BGMS1+ebx*2],10h
    jnz .main2
    mov edi,[CSubWinScr]
.main2
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bhr
    test byte[FillSubScr+ebx],1
    jz .main
    test byte[BGMS1+ebx*2],10h
    jnz .main
    add esi,75036*2
.main
    xor edi,edi
    normalsprng16b sprdrawpra16bnghr,sprdrawprb16bnghr
NEWSYM drawsprngw16bhr
    xor edi,edi
    test byte[FillSubScr+ebx],1
    jz .main
    test byte[BGMS1+ebx*2],10h
    jnz .main
    add esi,75036*2
.main
    normalwsprng16b sprdrawprawb16bnghr,sprdrawprbwb16bnghr
drawsprng16bthr:
    test byte[scadtng+ebx],10h
    jz near drawsprng16bnthr
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    test byte[BGMS1+ebx*2+1],10h
    jnz near drawsprng16bmsthr
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bthr
    xor edi,edi
    normalsprng16b sprdrawpra16bngthr,sprdrawprb16bngthr
NEWSYM drawsprngw16bthr
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngthr,sprdrawprbwb16bngthr
drawsprng16bmsthr:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmthr
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bsthr
    xor edi,edi
    normalsprng16b sprdrawpra16bngmsthr,sprdrawprb16bngmsthr
drawsprngw16bsthr:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngsthr,sprdrawprbwb16bngsthr
drawsprngw16bmthr:
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmsthr
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmthr,sprdrawprbwb16bngmthr
drawsprngw16bmsthr:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmsthr,sprdrawprbwb16bngmsthr
drawsprng16bnthr:
    mov ebp,[cpalval+ebx*4]
    xor eax,eax
    test byte[BGMS1+ebx*2+1],10h
    jnz near drawsprng16bmsnthr
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bhr
    xor edi,edi
    normalsprng16b sprdrawpra16bngnthr,sprdrawprb16bngnthr
drawsprngw16bnthr
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngnthr,sprdrawprbwb16bngnthr
drawsprng16bmsnthr:
    mov edi,[CMainWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmnthr
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bsnthr
    xor edi,edi
    normalsprng16b sprdrawpra16bngmsnthr,sprdrawprb16bngmsnthr
drawsprngw16bsnthr:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngsnthr,sprdrawprbwb16bngsnthr
drawsprngw16bmnthr:
    mov edi,[CSubWinScr]
    cmp byte[edi+ebx+4*256],0
    jne near drawsprngw16bmsnthr
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmnthr,sprdrawprbwb16bngmnthr
drawsprngw16bmsnthr:
    xor edi,edi
    normalwsprng16b sprdrawprawb16bngmsnthr,sprdrawprbwb16bngmsnthr


ProcessTransparencies:
    cmp byte[NGNoTransp],0
    je .yestransp
    ret
.yestransp
    cmp byte[MMXSupport],1
    je near ProcessTransparenciesMMX
    mov esi,[vidbuffer]
    add esi,16*2+288*2
    mov ebx,1
.nextline
    test byte[FillSubScr+ebx],1
    jz near .notransp
    mov dword[HiResDone],0
.againtransp
    test byte[scadtng+ebx],40h
    jz near .fulltransp
    test byte[scadtng+ebx],80h
    jnz near .subtract

    ; Half Add
    push esi
    push ebx
    ; filter out all fixed color sub-screen
    test byte[FillSubScr+ebx],2
    jnz .halfaddcomb
    mov ecx,256
    mov ebx,[UnusedBit]
    mov edi,[HalfTrans]
    xor eax,eax
    jmp .next2
.notranspha
    add esi,2
    dec ecx
    jz .done
.next2
    mov ax,[esi]
    test ax,bx
    jz .notranspha
    mov dx,[esi+75036*2]
    test dx,bx
    jnz .notranspha
    and eax,edi
    and edx,edi
    add eax,edx
    shr eax,1
    mov [esi],ax
    add esi,2
    dec ecx
    jnz .next2
.done
    pop ebx
    pop esi
    jmp .donetransp
.halfaddcomb
    mov ecx,256
    mov ebx,[UnusedBit]
    mov edi,[HalfTrans]
    xor eax,eax
    xor edx,edx
    jmp .next2c
.notransphac
    add esi,2
    dec ecx
    jz .donec
.next2c
    mov ax,[esi]
    test ax,bx
    jz .notransphac
    mov dx,[esi+75036*2]
    test dx,bx
    jnz .fulladdtranspc
    and eax,edi
    and edx,edi
    add eax,edx
    shr eax,1
    mov [esi],ax
    add esi,2
    dec ecx
    jnz .next2c
.donec
    pop ebx
    pop esi
    jmp .donetransp
.fulladdtranspc
    and eax,edi
    and edx,edi
    add eax,edx
    shr eax,1
    mov ax,[fulladdtab+eax*2]
    mov [esi],ax
    add esi,2
    dec ecx
    jnz .next2c
    pop ebx
    pop esi
    jmp .donetransp

.subtract
    push ebx
    push esi
    ; half adder
    mov ecx,256
    mov ebp,[HalfTrans]
    xor edx,edx
    mov bx,[UnusedBit]
.nextfshs
    mov ax,[esi]
    test ax,bx
    je .notranspfshs
    mov dx,[esi+75036*2]
    xor ax,0FFFFh
    and edx,ebp
    and eax,ebp
    add edx,eax
    shr edx,1
    mov dx,[fulladdtab+edx*2]
    xor dx,0FFFFh
    test word[esi+75036*2],bx
    jnz .nothalfhs
    and edx,ebp
    shr edx,1
.nothalfhs
    mov [esi],dx
.notranspfshs
    add esi,2
    dec ecx
    jnz .nextfshs
    pop esi
    pop ebx
    jmp .donetransp
.fulltransp
    test byte[scadtng+ebx],80h
    jnz near .fullsubtract
    push ebx
    push esi
    mov ecx,256
    mov ebp,[HalfTrans]
    xor edx,edx
    xor eax,eax
    mov bx,[UnusedBit]
.nextfa
    mov ax,[esi]
    test ax,bx
    jz .notranspfa
    mov dx,[esi+75036*2]
    and eax,ebp
    and edx,ebp
    add edx,eax
    shr edx,1
    mov dx,[fulladdtab+edx*2]
    mov [esi],dx
.notranspfa
    add esi,2
    dec ecx
    jnz .nextfa
    pop esi
    pop ebx
    jmp .donetransp
.fullsubtract
    push ebx
    push esi
    ; half adder
    mov ecx,256
    mov ebp,[HalfTrans]
    xor edx,edx
    xor eax,eax
    mov bx,[UnusedBit]
.nextfs
    mov ax,[esi]
    test ax,bx
    jz .notranspfs
    mov dx,[esi+75036*2]
    xor ax,0FFFFh
    and edx,ebp
    and eax,ebp
    add edx,eax
    shr edx,1
    mov dx,[fulladdtab+edx*2]
    xor dx,0FFFFh
    mov [esi],dx
.notranspfs
    add esi,2
    dec ecx
    jnz .nextfs
    pop esi
    pop ebx
.donetransp
    test byte[SpecialLine+ebx],3
    jz .notransp
    xor dword[HiResDone],1
    cmp dword[HiResDone],0
    je .okaytransp
    add esi,75036*4
    jmp .againtransp
.okaytransp
    sub esi,75036*4
.notransp
    inc ebx
    add esi,288*2
    cmp [resolutn],bx
    jne near .nextline
    ret

%macro TranspMMX 3
    mov esi,[vidbuffer]
    add esi,16*2+288*2
    mov ebx,1
.nextline
    test byte[FillSubScr+ebx],1
    jz near .notransp
    mov dword[HiResDone],0
.againtransp
    test byte[scadtng+ebx],40h
    jz near .fulltransp
    test byte[scadtng+ebx],80h
    jnz near .subtract

    ; Half Add
    push esi
    push ebx
    ; filter out all fixed color sub-screen
    mov ecx,64
    mov eax,[UnusedBit]
.faddl2h
    test dword[esi],eax
    jnz near .faddloopbh
    test dword[esi+4],eax
    jnz near .faddloopbh
    add esi,8
    dec ecx
    jnz .faddl2h
    jmp .faddloopdoneh
.prochalfadd
    test dword[esi+75036*2],eax
    jnz near .faddloopbh
    test dword[esi+75036*2+4],eax
    jnz near .faddloopbh
    mov ebx,[esi]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
    mov ebx,[esi+4]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
.prochalfadddo
    movq mm0,[esi]
    movq mm1,[esi+75036*2]
    pand mm0,[HalfTrans]
    pand mm1,[HalfTrans]
    psrlw mm0,1
    psrlw mm1,1
    paddw mm0,mm1
    movq [esi],mm0
    add esi,8
    dec ecx
    jnz .prochalfadd
    jmp .faddloopdoneh
.procfulladdnext:
    movq [esi-8],mm0
.procfulladd
    mov ebx,[esi]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
    mov ebx,[esi+4]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
    mov ebx,[esi+75036*2]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
    mov ebx,[esi+75036*2+4]
    and ebx,eax
    cmp ebx,eax
    jne near .faddlooph
.procfulladddo
    movq mm0,[esi]
    movq mm1,[esi+75036*2]
    pand mm0,[UnusedBitXor]
    movq mm4,mm1
    movq mm2,mm0
    %if %1>0
    psllw mm0,%1
    psllw mm1,%1
    movq mm3,mm2
    %else
    movq mm3,mm0
    %endif
    psllw mm2,%2
    pand mm1,[FullBitAnd]
    paddusw mm0,mm1
    pand mm0,[FullBitAnd]
    movq mm1,mm4
    psllw mm4,%2
    add esi,byte 8
    %if %1>0
    psrlw mm0,%1
    %endif
    paddusw mm2,mm4
    psllw mm3,%3
    pand mm2,[FullBitAnd]
    psllw mm1,%3
    psrlw mm2,%2
    pand mm1,[FullBitAnd]
    paddusw mm3,mm1
    por mm0,mm2
    pand mm3,[FullBitAnd]
    psrlw mm3,%3
    por mm0,mm3
    dec ecx
    jnz near .procfulladdnext
    movq [esi],mm0
    jmp .faddloopdoneh
.faddlooph
    mov ebx,[esi]
    test ebx,eax
    jz near .faddl2h
    and ebx,eax
    cmp ebx,eax
    jne .faddloopbh
    mov ebx,[esi+4]
    and ebx,eax
    cmp ebx,eax
    jne .faddloopbh
    mov ebx,[esi+75036*2]
    and ebx,eax
    or ebx,ebx
    jz .faddla
    cmp ebx,eax
    jne .faddloopbh
    mov ebx,[esi+75036*2+4]
    and ebx,eax
    cmp ebx,eax
    jne .faddloopbh
    jmp .procfulladddo
.faddla
    test dword[esi+75036*2+4],eax
    jz near .prochalfadddo
.faddloopbh
    movq mm0,[esi]
    movq mm5,mm0
    movq mm6,mm0
    pand mm5,[UnusedBitXor]
    movq mm1,[esi+75036*2]
    movq mm7,mm1
    movq mm2,mm0
    pand mm1,[UnusedBitXor]
    movq mm3,mm0
    movq mm4,mm1
    %if %1>0
    psllw mm0,%1
    psllw mm1,%1
    %endif
    pand mm1,[FullBitAnd]
    paddusw mm0,mm1
    pand mm0,[FullBitAnd]
    movq mm1,mm4
    %if %1>0
    psrlw mm0,%1
    %endif
    psllw mm2,%2
    psllw mm1,%2
    pand mm1,[FullBitAnd]
    paddusw mm2,mm1
    pand mm2,[FullBitAnd]
    movq mm1,mm4
    psrlw mm2,%2
    psllw mm3,%3
    psllw mm1,%3
    paddusw mm3,mm1
    pand mm3,[FullBitAnd]
    psrlw mm3,%3
    por mm0,mm3
    por mm0,mm2
    pand mm6,[UnusedBit]
    pcmpeqw mm6,[UnusedBit]
    pand mm0,mm6
    movq mm1,mm5
    pand mm1,mm6
    pand mm4,mm6
    pxor mm6,[UnusedBitXor]
    pand mm5,mm6
    pand mm7,[UnusedBit]
    pand mm4,[HalfTrans]
    pand mm1,[HalfTrans]
    psrlw mm1,1
    psrlw mm4,1
    paddw mm1,mm4
    pcmpeqw mm7,[UnusedBit]
    pand mm0,mm7
    pxor mm7,[UnusedBitXor]
    pand mm1,mm7
    por mm0,mm1
    por mm0,mm5
    movq [esi],mm0
    add esi,8
    dec ecx
    jnz near .faddlooph
.faddloopdoneh
    pop ebx
    pop esi
    jmp .donetransp

.subtract
    push ebx
    push esi
    mov ecx,64
    mov eax,[UnusedBit]
.fsubl2h
    test dword[esi],eax
    jnz .fsubloopbh
    test dword[esi+4],eax
    jnz .fsubloopbh
    add esi,8
    dec ecx
    jnz .fsubl2h
    jmp .fsubloopdoneh
.fsublooph
    test dword[esi],eax
    jz .fsubl2h
.fsubloopbh
    movq mm0,[esi]
    movq mm5,mm0
    movq mm6,mm0
    pxor mm0,[UnusedBitXor]
    pand mm5,[UnusedBitXor]
    movq mm1,[esi+75036*2]
    movq mm7,mm1
    movq mm2,mm0
    pand mm1,[UnusedBitXor]
    movq mm3,mm0
    movq mm4,mm1
    %if %1>0
    psllw mm0,%1
    psllw mm1,%1
    %endif
    pand mm1,[FullBitAnd]
    paddusw mm0,mm1
    pand mm0,[FullBitAnd]
    movq mm1,mm4
    %if %1>0
    psrlw mm0,%1
    %endif
    psllw mm2,%2
    psllw mm1,%2
    paddusw mm2,mm1
    pand mm2,[FullBitAnd]
    psrlw mm2,%2
    psllw mm3,%3
    psllw mm4,%3
    pand mm4,[FullBitAnd]
    paddusw mm3,mm4
    pand mm3,[FullBitAnd]
    psrlw mm3,%3
    por mm0,mm3
    por mm0,mm2
    pand mm6,[UnusedBit]
    pxor mm0,[UnusedBitXor]
    pcmpeqw mm6,[UnusedBit]
    pand mm0,mm6
    pxor mm6,[UnusedBitXor]
    pand mm5,mm6
    pand mm7,[UnusedBit]
    movq mm1,mm0
    pand mm1,[HalfTrans]
    psrlw mm1,1
    pcmpeqw mm7,[UnusedBit]
    pand mm0,mm7
    pxor mm7,[UnusedBitXor]
    pand mm1,mm7
    por mm0,mm1
    por mm0,mm5
    movq [esi],mm0
    add esi,8
    dec ecx
    jnz near .fsublooph
.fsubloopdoneh
    pop esi
    pop ebx
    jmp .donetransp

.fulltransp
    test byte[scadtng+ebx],80h
    jnz near .fullsubtract
    push ebx
    push esi
    mov ecx,64
    mov eax,[UnusedBit]
.faddl2
    test dword[esi],eax
    jnz .faddloopb
.faddl2_2
    test dword[esi+4],eax
    jnz .faddloopb
    add esi,8
    dec ecx
    jnz .faddl2
    jmp .faddloopdone
.faddloopnext
    movq [esi-8],mm0
.faddloop
    test dword[esi],eax
    jz .faddl2_2
.faddloopb
    movq mm0,[esi]
    movq mm1,[esi+75036*2]
    movq mm6,mm0
    pand mm0,[UnusedBitXor]
    movq mm4,mm1
    movq mm2,mm0
    %if %1>0
    psllw mm0,%1
    psllw mm1,%1
    movq mm3,mm2
    movq mm5,mm2
    %else
    movq mm3,mm0
    movq mm5,mm0
    %endif
    pand mm1,[FullBitAnd]
    paddusw mm0,mm1
    pand mm0,[FullBitAnd]
    psllw mm2,%2
    movq mm1,mm4
    psllw mm4,%2
    paddusw mm2,mm4
    psllw mm3,%3
    pand mm2,[FullBitAnd]
    psllw mm1,%3
    psrlw mm2,%2
    pand mm1,[FullBitAnd]
    paddusw mm3,mm1
    pand mm3,[FullBitAnd]
    %if %1>0
    psrlw mm0,%1
    %endif
    psrlw mm3,%3
    pand mm6,[UnusedBit]
    por mm0,mm2
    pcmpeqw mm6,[UnusedBit]
    por mm0,mm3
    pand mm0,mm6
    pxor mm6,[UnusedBitXor]
    pand mm5,mm6
    add esi,byte 8
    por mm0,mm5
    dec ecx
    jnz near .faddloopnext
    movq [esi-8],mm0
.faddloopdone
    pop esi
    pop ebx
    jmp .donetransp
.fullsubtract
    push ebx
    push esi
    mov ecx,64
    mov eax,[UnusedBit]
.fsubl2
    test dword[esi],eax
    jnz .fsubloopb
    test dword[esi+4],eax
    jnz .fsubloopb
    add esi,8
    dec ecx
    jnz .fsubl2
    jmp .fsubloopdone
.fsubloop
    test dword[esi],eax
    jz .fsubl2
.fsubloopb
    movq mm0,[esi]
    movq mm5,mm0
    movq mm6,mm0
    pxor mm0,[UnusedBitXor]
    pand mm5,[UnusedBitXor]
    movq mm1,[esi+75036*2]
    movq mm2,mm0
    pand mm1,[UnusedBitXor]
    movq mm3,mm0
    movq mm4,mm1
    %if %1>0
    psllw mm0,%1
    psllw mm1,%1
    %endif
    pand mm1,[FullBitAnd]
    paddusw mm0,mm1
    pand mm0,[FullBitAnd]
    movq mm1,mm4
    %if %1>0
    psrlw mm0,%1
    %endif
    psllw mm2,%2
    psllw mm1,%2
    paddusw mm2,mm1
    pand mm2,[FullBitAnd]
    psrlw mm2,%2
    psllw mm3,%3
    psllw mm4,%3
    pand mm4,[FullBitAnd]
    paddusw mm3,mm4
    pand mm3,[FullBitAnd]
    psrlw mm3,%3
    por mm0,mm3
    por mm0,mm2
    pand mm6,[UnusedBit]
    pxor mm0,[UnusedBitXor]
    pcmpeqw mm6,[UnusedBit]
    pand mm0,mm6
    pxor mm6,[UnusedBitXor]
    pand mm5,mm6
    por mm0,mm5
    movq [esi],mm0
    add esi,8
    dec ecx
    jnz near .fsubloop
.fsubloopdone
    pop esi
    pop ebx
.donetransp
    test byte[SpecialLine+ebx],3
    jz .notransp
    xor dword[HiResDone],1
    cmp dword[HiResDone],0
    je .okaytransp
    add esi,75036*4
    jmp .againtransp
.okaytransp
    sub esi,75036*4
.notransp
    inc ebx
    add esi,288*2
    cmp [resolutn],bx
    jne near .nextline
    emms
    ret
%endmacro

ProcessTransparenciesMMX:
    cmp byte[ngrposng],10
    je near ProcessTransparenciesMMXargb
    TranspMMX 0,5,11
ProcessTransparenciesMMXargb
    TranspMMX 1,6,11

; movq mm0,[esi]
; movq mm1,[esi+75036*2]
; movq mm2,mm0
; movq mm4,mm1
; movq mm3,mm0
; movq mm5,mm0
; movq mm6,mm0
; ;psllw mm0,0
; ;psllw mm1,0
; paddusw mm0,mm1
; pand mm6,[UnusedBitXor]
; pand mm0,[FullBitAnd]
; movq mm1,mm4
; ;psrlw mm0,0
; psllw mm2,5
; psllw mm1,5
; paddusw mm2,mm1
; pand mm2,[FullBitAnd]
; psrlw mm2,5
; psllw mm3,11
; psllw mm4,11
; paddusw mm3,mm4
; pand mm3,[FullBitAnd]
; psrlw mm3,11
; por mm0,mm3
; por mm0,mm2
; pand mm6,[UnusedBit]
; pcmpeqw mm6,[UnusedBit]
; pand mm0,mm6
; pxor mm6,[UnusedBitXor]
; pand mm5,mm6
; por mm0,mm5
; movq [esi],mm0

; PADDUSW - Add Unsigned with Saturation on Word
; PAND (source can be a memory location)
; PANDN - bitwise AND NOT
; PCMPEQW - packed compare for equal, word
; PSLLW - Shift Left, Logical, Word
; PSRLW - Shirt Right, Logical
; POR

section .data
ALIGN32
NEWSYM UnusedBit, dd 00000000001000000000000000100000b,00000000001000000000000000100000b
NEWSYM HalfTrans, dd 11110111110111101111011111011110b,11110111110111101111011111011110b,0,0
NEWSYM UnusedBitXor, dd 11111111110111111111111111011111b,11111111110111111111111111011111b
NEWSYM ngrposng, dd 11,0
NEWSYM nggposng, dd 6,0
NEWSYM ngbposng, dd 0,0
NEWSYM HiResDone, dd 0,0
NEWSYM FullBitAnd, dd 0F800F800h,0F800F800h
NEWSYM HalfTransB, dd 00001000010000010000100001000001b,00001000010000010000100001000001b
NEWSYM HalfTransC, dd 11110111100111101111011110011110b,11110111100111101111011110011110b
NEWSYM NGNoTransp, dd 0
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
%endmacro

%macro SCSubC 0
    xor ebx,0FFFFFFFFh
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
    mov ebx,[UnusedBit]
    %3
    mov ecx,256
    mov edx,256
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
    call BuildWindow
;ngwintable
    pop ebx
    pop eax
.nowindowing

    mov ebx,[UnusedBit]
    %3
    mov edx,256
    cmp dword[ngwinen],0
    jne .windowenabled
    cmp al,80h
    je near .finclipping
    mov ecx,256
    jmp .startclippingfull
.windowenabled
    cmp al,80h
    je near .outsideclipping
    mov edi,ngwintable
    mov ecx,[edi]
    cmp ecx,0
    je .nodec
    dec ecx
.nodec
    add edi,4
    or ecx,ecx
    jnz near .startclippingb
    mov ecx,[edi]
    add edi,4
    jmp .noclipping
.outsideclipping
    mov edi,ngwintable
    mov ecx,[edi]
    add edi,4
    or ecx,ecx
    jnz .noclipping
    mov ecx,[edi]
    cmp ecx,0
    je .nodec2
    dec ecx
.nodec2
    add edi,4
    jmp .startclippingb
.startclippingb
    cmp ecx,256
    jae near .startclippingfull
.startclipping
    %2
    add esi,2
    dec edx
    jz .finclipping
    dec ecx
    jnz .startclipping
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
    jmp .startclipping
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
    jne near .nextline
%endmacro

MainScreenClip:
    ScreenClip SCMainA,SCMainB,SCMainC,SCMainD
SubScreenClip:
    ScreenClip SCSubA,SCSubB,SCSubC,SCSubD
    ret
