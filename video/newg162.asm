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
EXTSYM c_drawtile2b_nt,c_drawtile2b_t,c_drawtile2b_mst,c_drawtile2b_msnt
EXTSYM c_drawtile4b_nt,c_drawtile4b_t,c_drawtile4b_mst,c_drawtile4b_msnt
EXTSYM c_drawtile8b_nt,c_drawtile8b_t,c_drawtile8b_mst,c_drawtile8b_msnt
EXTSYM c_drawtile2b_win,c_drawtile2b_wint,c_drawtile2b_mstmsw,c_drawtile2b_msntmsw
EXTSYM c_drawtile2b_mstmw,c_drawtile2b_mstsw,c_drawtile2b_msntmw,c_drawtile2b_msntsw
EXTSYM c_drawtile4b_win,c_drawtile4b_wint,c_drawtile4b_mstmsw,c_drawtile4b_msntmsw
EXTSYM c_drawtile4b_mstmw,c_drawtile4b_mstsw,c_drawtile4b_msntmw,c_drawtile4b_msntsw
EXTSYM c_drawtile8b_win,c_drawtile8b_wint,c_drawtile8b_mstmsw,c_drawtile8b_msntmsw
EXTSYM c_drawtile8b_mstmw,c_drawtile8b_mstsw,c_drawtile8b_msntmw,c_drawtile8b_msntsw
EXTSYM c_drawtile16x162b_nt,c_drawtile16x162b_t,c_drawtile16x162b_mst,c_drawtile16x162b_msnt
EXTSYM c_drawtile16x164b_nt,c_drawtile16x164b_t,c_drawtile16x164b_mst,c_drawtile16x164b_msnt
EXTSYM c_drawtile16x168b_nt,c_drawtile16x168b_t,c_drawtile16x168b_mst,c_drawtile16x168b_msnt
EXTSYM c_drawtile16x162b_win,c_drawtile16x162b_wint,c_drawtile16x162b_mstmsw,c_drawtile16x162b_msntmsw
EXTSYM c_drawtile16x162b_mstmw,c_drawtile16x162b_mstsw,c_drawtile16x162b_msntmw,c_drawtile16x162b_msntsw
EXTSYM c_drawtile16x164b_win,c_drawtile16x164b_wint,c_drawtile16x164b_mstmsw,c_drawtile16x164b_msntmsw
EXTSYM c_drawtile16x164b_mstmw,c_drawtile16x164b_mstsw,c_drawtile16x164b_msntmw,c_drawtile16x164b_msntsw
EXTSYM c_drawtile16x168b_win,c_drawtile16x168b_wint,c_drawtile16x168b_mstmsw,c_drawtile16x168b_msntmsw
EXTSYM c_drawtile16x168b_mstmw,c_drawtile16x168b_mstsw,c_drawtile16x168b_msntmw,c_drawtile16x168b_msntsw
EXTSYM c_drawline2b_nt,c_drawline2b_t,c_drawline2b_mst,c_drawline2b_msnt
EXTSYM c_drawline4b_nt,c_drawline4b_t,c_drawline4b_mst,c_drawline4b_msnt
EXTSYM c_drawline8b_nt,c_drawline8b_t,c_drawline8b_mst,c_drawline8b_msnt
EXTSYM c_drawline2b_win,c_drawline2b_wint,c_drawline2b_mstmsw,c_drawline2b_msntmsw
EXTSYM c_drawline2b_mstmw,c_drawline2b_mstsw,c_drawline2b_msntmw,c_drawline2b_msntsw
EXTSYM c_drawline4b_win,c_drawline4b_wint,c_drawline4b_mstmsw,c_drawline4b_msntmsw
EXTSYM c_drawline4b_mstmw,c_drawline4b_mstsw,c_drawline4b_msntmw,c_drawline4b_msntsw
EXTSYM c_drawline8b_win,c_drawline8b_wint,c_drawline8b_mstmsw,c_drawline8b_msntmsw
EXTSYM c_drawline8b_mstmw,c_drawline8b_mstsw,c_drawline8b_msntmw,c_drawline8b_msntsw
EXTSYM c_drawline16x162b_nt,c_drawline16x162b_t,c_drawline16x162b_mst,c_drawline16x162b_msnt
EXTSYM c_drawline16x164b_nt,c_drawline16x164b_t,c_drawline16x164b_mst,c_drawline16x164b_msnt
EXTSYM c_drawline16x168b_nt,c_drawline16x168b_t,c_drawline16x168b_mst,c_drawline16x168b_msnt
EXTSYM c_drawline16x162b_win,c_drawline16x162b_wint,c_drawline16x162b_mstmsw,c_drawline16x162b_msntmsw
EXTSYM c_drawline16x162b_mstmw,c_drawline16x162b_mstsw,c_drawline16x162b_msntmw,c_drawline16x162b_msntsw
EXTSYM c_drawline16x164b_win,c_drawline16x164b_wint,c_drawline16x164b_mstmsw,c_drawline16x164b_msntmsw
EXTSYM c_drawline16x164b_mstmw,c_drawline16x164b_mstsw,c_drawline16x164b_msntmw,c_drawline16x164b_msntsw
EXTSYM c_drawline16x168b_win,c_drawline16x168b_wint,c_drawline16x168b_mstmsw,c_drawline16x168b_msntmsw
EXTSYM c_drawline16x168b_mstmw,c_drawline16x168b_mstsw,c_drawline16x168b_msntmw,c_drawline16x168b_msntsw
EXTSYM ng2_mosaic
EXTSYM c_determinetransp,c_checkwindowing,c_determinewindow,ng_branch
EXTSYM ngwintable,ngwinen,ngcwinptr,ngcpixleft,ngcwinmode,tleftn,ng16bprval
EXTSYM vrama,bg1drwng,ng16bbgval,bg1totng,bgtxadd,taddnfy16x16,taddfy16x16
EXTSYM switch16x16,curmosaicsz,domosaicng16b,vidmemch2,vidmemch4,vidmemch8
EXTSYM mode0add,vcache4b,vcache2b,vcache8b,cachesingle2bng,cachesingle8bng
EXTSYM ngpalcon4b,ngpalcon8b,ngpalcon2b,tleftnb,tltype2b,tltype4b,tltype8b
EXTSYM yposng,flipyposng,ofsmcptr,ofsmtptr,ofsmmptr,ofsmcyps,ofsmady,ofsmadx
EXTSYM FillSubScr,UnusedBitXor,yposngom,flipyposngom,ofsmval,ofsmvalh
EXTSYM CPalPtrng,BGMS1,scadtng,CMainWinScr,CSubWinScr,UnusedBit,res640
EXTSYM mosclineval,mostranspval,vcache2bs,vcache4bs,vcache8bs,vidmemch2s
EXTSYM vidmemch4s,vidmemch8s,bgtxadd2,SpecialLine,cachesingle4bng
EXTSYM ofshvaladd,ofsmtptrs,ofsmcptr2,ngptrdat2

%include "video/newg162.mac"
%include "video/newg16wn.mac"

;******************************************
; 16bitng caching functions
;******************************************

%macro cacheloopstuff 1
    mov bl,[esi+%1]
    or bl,bl
    jnz short %%okay
    mov ax,0FFFFh
    jmp %%transp
%%okay
    or bl,dl
    mov ax,[ebp+ebx*2]
%%transp
    mov [edi+%1*2],ax
    mov [edi+14-%1*2+128],ax
%endmacro

%macro DoCache 2
    push ecx
    push eax
    push esi
    push edi
    mov esi,ecx
    xor ebx,ebx
    shl esi,6
    shl ecx,8
    add esi,[%1]
    add ecx,[%2]
    mov edi,ecx
    mov ecx,8

.loop
    cacheloopstuff 0
    cacheloopstuff 1
    cacheloopstuff 2
    cacheloopstuff 3
    cacheloopstuff 4
    cacheloopstuff 5
    cacheloopstuff 6
    cacheloopstuff 7
    add edi,16
    add esi,8
    dec ecx
    jnz near .loop
    pop edi
    pop esi
    pop eax
    pop ecx
    ret
%endmacro

SECTION .text

cache2b16b:
    DoCache vcache2b,vcache2bs
cache4b16b:
    DoCache vcache4b,vcache4bs
cache8b16b:
    DoCache vcache8b,vcache8bs

;******************************************
; 8x8 tiles - tile engine
;******************************************

%macro WinClipMacro 1
    mov byte[tleftn],33
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8
    add ax,2
    mov ebx,[ng16bbgval]
    add edi,16
    inc dword[bg1totng+ebx*4]
    test eax,03Fh
    jnz short .notileadd
    add ax,[bgtxadd]
.notileadd
    dec byte[tleftn]
    jnz .winclipped
    pop ebx
    cmp byte[curmosaicsz],1
    jne near domosaicng16b
    ret
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
%endmacro

; Ported to video/c_ng2gate.c. The C reports which branch to take in ng_branch;
; its writes to ecx and edi land in the pushad block, so popad keeps them.
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

%macro determinetransp 1
    pushad
    mov eax, esp
    ccall c_determinetransp, eax
    popad
    cmp dword[ng_branch],1
    je near %1
%endmacro

%macro CheckWindowing 1
    pushad
    mov eax, esp
    ccall c_checkwindowing, eax
    popad
    cmp dword[ng_branch],1
    je near %1
%endmacro

%macro DetermineWindow 3        ; both,main,sub
    pushad
    mov eax, esp
    ccall c_determinewindow, eax
    popad
    cmp dword[ng_branch],1
    je near %1
    cmp dword[ng_branch],2
    je near %2
    cmp dword[ng_branch],3
    je near %3
%endmacro

%macro drawtile16b 10
    mov byte[tleftn],33

%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawtileng16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%9,%10
    ret
%endmacro

%macro drawtile16bw 12
    WinClipMacro %%processwinclip2b
    drawtileng16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%9,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawtilengwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%11,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawtile16bw2 14
    mov byte[tleftn],33
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawtileng16b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%7,%8,%13,%14
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%9,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawtilengwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%11,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

NEWSYM drawtileng2b16b
    determinetransp drawtileng2b16bt
drawtileng2b16bnt
    CheckWindowing drawtileng2bwin
    ; plain leaf (no transparency, no window): video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_nt, eax
    popad
    pop ebx
    ret
drawtileng2bwin:
    ; windowed, plain: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng2b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bnt
    CheckWindowing drawtileng2bwint
    ; transparent leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_t, eax
    popad
    pop ebx
    ret
drawtileng2bwint:
    ; windowed, transparent: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bmsnt
    DetermineWindow drawtileng2b16bmstmsw, drawtileng2b16bmstmw, drawtileng2b16bmstsw
    ; transparent + sub screen leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_mst, eax
    popad
    pop ebx
    ret
drawtileng2b16bmstmsw:
    ; windowed on both screens, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmstmw:
    ; main-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmstsw:
    ; sub-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsnt
    DetermineWindow drawtileng2b16bmsntmsw, drawtileng2b16bmsntmw, drawtileng2b16bmsntsw
    ; sub screen leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_msnt, eax
    popad
    pop ebx
    ret
drawtileng2b16bmsntmsw:
    ; windowed on both screens, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsntmw:
    ; main-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsntsw:
    ; sub-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile2b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng4b16b
    determinetransp drawtileng4b16bt
drawtileng4b16bnt
    CheckWindowing drawtileng4bwin
    ; plain leaf (no transparency, no window) leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_nt, eax
    popad
    pop ebx
    ret
drawtileng4bwin:
    ; windowed, plain: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng4b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bnt
    CheckWindowing drawtileng4bwint
    ; transparent leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_t, eax
    popad
    pop ebx
    ret
drawtileng4bwint:
    ; windowed, transparent: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bmsnt
    DetermineWindow drawtileng4b16bmstmsw, drawtileng4b16bmstmw, drawtileng4b16bmstsw
    ; transparent + sub screen leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_mst, eax
    popad
    pop ebx
    ret
drawtileng4b16bmstmsw:
    ; windowed on both screens, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmstmw:
    ; main-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmstsw:
    ; sub-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsnt
    DetermineWindow drawtileng4b16bmsntmsw, drawtileng4b16bmsntmw, drawtileng4b16bmsntsw
    ; sub screen leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_msnt, eax
    popad
    pop ebx
    ret
drawtileng4b16bmsntmsw:
    ; windowed on both screens, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsntmw:
    ; main-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsntsw:
    ; sub-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile4b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng8b16b
    determinetransp drawtileng8b16bt
drawtileng8b16bnt
    CheckWindowing drawtileng8bwin
    ; plain leaf (no transparency, no window) leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_nt, eax
    popad
    pop ebx
    ret
drawtileng8bwin:
    ; windowed, plain: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng8b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bnt
    CheckWindowing drawtileng8bwint
    ; transparent leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_t, eax
    popad
    pop ebx
    ret
drawtileng8bwint:
    ; windowed, transparent: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bmsnt
    DetermineWindow drawtileng8b16bmstmsw, drawtileng8b16bmstmw, drawtileng8b16bmstsw
    ; transparent + sub screen leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_mst, eax
    popad
    pop ebx
    ret
drawtileng8b16bmstmsw:
    ; windowed on both screens, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmstmw:
    ; main-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmstsw:
    ; sub-screen window only, transparent + sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsnt
    DetermineWindow drawtileng8b16bmsntmsw, drawtileng8b16bmsntmw, drawtileng8b16bmsntsw
    ; sub screen leaf leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_msnt, eax
    popad
    pop ebx
    ret
drawtileng8b16bmsntmsw:
    ; windowed on both screens, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsntmw:
    ; main-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsntsw:
    ; sub-screen window only, sub: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile8b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

;******************************************
; 16x16 tiles - tile engine
;******************************************

%macro WinClipMacro16x16 1
    mov byte[tleftn],17
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %1
    sub dword[ebx],16
    add ax,2
    mov ebx,[ng16bbgval]
    add edi,32
    inc dword[bg1totng+ebx*4]
    test eax,03Fh
    jnz short .notileadd
    add ax,[bgtxadd]
.notileadd
    dec byte[tleftn]
    jnz .winclipped
    pop ebx
    cmp byte[curmosaicsz],1
    jne near domosaicng16b
    ret
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %1
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
%endmacro

%macro drawtile16b16x16 10
    mov byte[tleftn],17
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawtileng16x1616b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%9,%10
    ret
%endmacro

%macro drawtile16bw16x16 12
    WinClipMacro16x16 %%processwinclip2b
    drawtileng16x1616b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%9,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawtileng16x16win16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%11,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawtile16bw216x16 14
    mov byte[tleftn],17
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %%processwinclip2b
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawtileng16x1616b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%7,%8,%13,%14
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %%processwinclip2b
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng16x1616b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%9,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawtileng16x16win16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%11,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

NEWSYM drawtileng16x162b16b
    determinetransp drawtileng2b16bt16x16
drawtileng2b16bnt16x16
    CheckWindowing drawtileng2bwin16x16
    ; plain (no transparency, no window) leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_nt, eax
    popad
    pop ebx
    ret
drawtileng2bwin16x16:
    ; windowed, plain, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng2b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bnt16x16
    CheckWindowing drawtileng2bwint16x16
    ; transparent leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_t, eax
    popad
    pop ebx
    ret
drawtileng2bwint16x16:
    ; windowed, transparent, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bmsnt16x16
    DetermineWindow drawtileng2b16bmstmsw16x16, drawtileng2b16bmstmw16x16, drawtileng2b16bmstsw16x16
    ; transparent + sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_mst, eax
    popad
    pop ebx
    ret
drawtileng2b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmstmw16x16:
    ; main-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmstsw16x16:
    ; sub-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsnt16x16
    DetermineWindow drawtileng2b16bmsntmsw16x16, drawtileng2b16bmsntmw16x16, drawtileng2b16bmsntsw16x16
    ; sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_msnt, eax
    popad
    pop ebx
    ret
drawtileng2b16bmsntmsw16x16:
    ; windowed on both screens, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsntmw16x16:
    ; main-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng2b16bmsntsw16x16:
    ; sub-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x162b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng16x164b16b
    determinetransp drawtileng4b16bt16x16
drawtileng4b16bnt16x16
    CheckWindowing drawtileng4bwin16x16
    ; plain (no transparency, no window) leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_nt, eax
    popad
    pop ebx
    ret
drawtileng4bwin16x16:
    ; windowed, plain, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng4b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bnt16x16
    CheckWindowing drawtileng4bwint16x16
    ; transparent leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_t, eax
    popad
    pop ebx
    ret
drawtileng4bwint16x16:
    ; windowed, transparent, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bmsnt16x16
    DetermineWindow drawtileng4b16bmstmsw16x16, drawtileng4b16bmstmw16x16, drawtileng4b16bmstsw16x16
    ; transparent + sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_mst, eax
    popad
    pop ebx
    ret
drawtileng4b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmstmw16x16:
    ; main-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmstsw16x16:
    ; sub-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsnt16x16
    DetermineWindow drawtileng4b16bmsntmsw16x16, drawtileng4b16bmsntmw16x16, drawtileng4b16bmsntsw16x16
    ; sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_msnt, eax
    popad
    pop ebx
    ret
drawtileng4b16bmsntmsw16x16:
    ; windowed on both screens, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsntmw16x16:
    ; main-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng4b16bmsntsw16x16:
    ; sub-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x164b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawtileng16x168b16b
    determinetransp drawtileng8b16bt16x16
drawtileng8b16bnt16x16
    CheckWindowing drawtileng8bwin16x16
    ; plain (no transparency, no window) leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_nt, eax
    popad
    pop ebx
    ret
drawtileng8bwin16x16:
    ; windowed, plain, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng8b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bnt16x16
    CheckWindowing drawtileng8bwint16x16
    ; transparent leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_t, eax
    popad
    pop ebx
    ret
drawtileng8bwint16x16:
    ; windowed, transparent, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bmsnt16x16
    DetermineWindow drawtileng8b16bmstmsw16x16, drawtileng8b16bmstmw16x16, drawtileng8b16bmstsw16x16
    ; transparent + sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_mst, eax
    popad
    pop ebx
    ret
drawtileng8b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmstmw16x16:
    ; main-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmstsw16x16:
    ; sub-screen window only, transparent + sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsnt16x16
    DetermineWindow drawtileng8b16bmsntmsw16x16, drawtileng8b16bmsntmw16x16, drawtileng8b16bmsntsw16x16
    ; sub screen leaf, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_msnt, eax
    popad
    pop ebx
    ret
drawtileng8b16bmsntmsw16x16:
    ; windowed on both screens, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsntmw16x16:
    ; main-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawtileng8b16bmsntsw16x16:
    ; sub-screen window only, sub, 16x16: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawtile16x168b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

;******************************************
; 8x8 tiles - line by line engine
;******************************************

%macro drawline16bmacro 8
    mov byte[tleftn],33
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlineng16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8
    ret
%endmacro

%macro drawline16bwmacro 9
    WinClipMacro %%processwinclip2b
    drawlineng16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawline16bw2macro 11
    mov byte[tleftn],33
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawlineng16b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

NEWSYM drawlineng2b16b
    determinetransp drawlineng2b16bt
drawlineng2b16bnt
    CheckWindowing drawlineng2bwin
    ; plain line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2bwin:
    ; windowed, plain line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bnt
    CheckWindowing drawlineng2bwint
    ; transparent line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2bwint:
    ; windowed, transparent line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsnt
    DetermineWindow drawlineng2b16bmstmsw, drawlineng2b16bmstmw, drawlineng2b16bmstsw
    ; transparent + sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstmsw:
    ; windowed on both screens, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstmw:
    ; main-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstsw:
    ; sub-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsnt
    DetermineWindow drawlineng2b16bmsntmsw, drawlineng2b16bmsntmw, drawlineng2b16bmsntsw
    ; sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntmsw:
    ; windowed on both screens, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntmw:
    ; main-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntsw:
    ; sub-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline2b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng4b16b
    determinetransp drawlineng4b16bt
drawlineng4b16bnt
    CheckWindowing drawlineng4bwin
    ; plain line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4bwin:
    ; windowed, plain line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bnt
    CheckWindowing drawlineng4bwint
    ; transparent line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4bwint:
    ; windowed, transparent line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsnt
    DetermineWindow drawlineng4b16bmstmsw, drawlineng4b16bmstmw, drawlineng4b16bmstsw
    ; transparent + sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstmsw:
    ; windowed on both screens, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstmw:
    ; main-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstsw:
    ; sub-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsnt
    DetermineWindow drawlineng4b16bmsntmsw, drawlineng4b16bmsntmw, drawlineng4b16bmsntsw
    ; sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntmsw:
    ; windowed on both screens, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntmw:
    ; main-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntsw:
    ; sub-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline4b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng8b16b
    determinetransp drawlineng8b16bt
drawlineng8b16bnt
    CheckWindowing drawlineng8bwin
    ; plain line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8bwin:
    ; windowed, plain line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bnt
    CheckWindowing drawlineng8bwint
    ; transparent line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8bwint:
    ; windowed, transparent line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsnt
    DetermineWindow drawlineng8b16bmstmsw, drawlineng8b16bmstmw, drawlineng8b16bmstsw
    ; transparent + sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstmsw:
    ; windowed on both screens, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstmw:
    ; main-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstsw:
    ; sub-screen window only, transparent + sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsnt
    DetermineWindow drawlineng8b16bmsntmsw, drawlineng8b16bmsntmw, drawlineng8b16bmsntsw
    ; sub screen line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntmsw:
    ; windowed on both screens, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntmw:
    ; main-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntsw:
    ; sub-screen window only, sub line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline8b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

;******************************************
; 16x16 tiles - line by line engine
;******************************************

%macro drawline16bmacro16x16 8
    mov byte[tleftn],17
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlineng16x1616b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8
    ret
%endmacro

%macro drawline16bwmacro16x16 9
    WinClipMacro16x16 %%processwinclip2b
    drawlineng16x1616b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlineng16x16win16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawline16bw2macro16x16 11
    mov byte[tleftn],17
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %%processwinclip2b
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawlineng16x1616b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %%processwinclip2b
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng16x1616b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlineng16x16win16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro


NEWSYM drawlineng16x162b16b
    determinetransp drawlineng2b16bt16x16
drawlineng2b16bnt16x16
    CheckWindowing drawlineng2bwin16x16
    ; plain 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2bwin16x16:
    ; windowed, plain 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bnt16x16
    CheckWindowing drawlineng2bwint16x16
    ; transparent 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2bwint16x16:
    ; windowed, transparent 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsnt16x16
    DetermineWindow drawlineng2b16bmstmsw16x16, drawlineng2b16bmstmw16x16, drawlineng2b16bmstsw16x16
    ; transparent + sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstmw16x16:
    ; main-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmstsw16x16:
    ; sub-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsnt16x16
    DetermineWindow drawlineng2b16bmsntmsw16x16, drawlineng2b16bmsntmw16x16, drawlineng2b16bmsntsw16x16
    ; sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntmsw16x16:
    ; windowed on both screens, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntmw16x16:
    ; main-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng2b16bmsntsw16x16:
    ; sub-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x162b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x164b16b
    determinetransp drawlineng4b16bt16x16
drawlineng4b16bnt16x16
    CheckWindowing drawlineng4bwin16x16
    ; plain 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4bwin16x16:
    ; windowed, plain 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bnt16x16
    CheckWindowing drawlineng4bwint16x16
    ; transparent 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4bwint16x16:
    ; windowed, transparent 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsnt16x16
    DetermineWindow drawlineng4b16bmstmsw16x16, drawlineng4b16bmstmw16x16, drawlineng4b16bmstsw16x16
    ; transparent + sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstmw16x16:
    ; main-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmstsw16x16:
    ; sub-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsnt16x16
    DetermineWindow drawlineng4b16bmsntmsw16x16, drawlineng4b16bmsntmw16x16, drawlineng4b16bmsntsw16x16
    ; sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntmsw16x16:
    ; windowed on both screens, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntmw16x16:
    ; main-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng4b16bmsntsw16x16:
    ; sub-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x164b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

NEWSYM drawlineng16x168b16b
    determinetransp drawlineng8b16bt16x16
drawlineng8b16bnt16x16
    CheckWindowing drawlineng8bwin16x16
    ; plain 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_nt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8bwin16x16:
    ; windowed, plain 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_win, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bnt16x16
    CheckWindowing drawlineng8bwint16x16
    ; transparent 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_t, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8bwint16x16:
    ; windowed, transparent 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_wint, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsnt16x16
    DetermineWindow drawlineng8b16bmstmsw16x16, drawlineng8b16bmstmw16x16, drawlineng8b16bmstsw16x16
    ; transparent + sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_mst, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstmsw16x16:
    ; windowed on both screens, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_mstmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstmw16x16:
    ; main-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_mstmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmstsw16x16:
    ; sub-screen window only, transparent + sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_mstsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsnt16x16
    DetermineWindow drawlineng8b16bmsntmsw16x16, drawlineng8b16bmsntmw16x16, drawlineng8b16bmsntsw16x16
    ; sub screen 16x16 line leaf: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_msnt, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntmsw16x16:
    ; windowed on both screens, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_msntmsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntmw16x16:
    ; main-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_msntmw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret
drawlineng8b16bmsntsw16x16:
    ; sub-screen window only, sub 16x16 line: video/c_ng2tile.c
    pushad
    mov eax, esp
    ccall c_drawline16x168b_msntsw, eax
    popad
    pop ebx
    cmp dword[ng2_mosaic],0
    jne near domosaicng16b
    ret

%macro drawline16bmacro16x8 10
    cmp byte[curmosaicsz],1
    ja near %%res640
    cmp byte[res640],0
    je near %%res640
    mov byte[tleftn],33
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlineng16x816b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8
    ret
%%res640
    push ebx
    mov cl,[curmosaicsz]
%%mosloop
    mov byte[SpecialLine+ebx],0
    inc ebx
    dec cl
    jnz short %%mosloop
    pop ebx
    mov byte[tleftn],33
%%loopb
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finlineb
    drawlineng16x816b %1,%2,%3,%%loopb,%%finlineb,%4,%5,%6,%9,%10
    ret
%endmacro

NEWSYM drawlineng16x84b16b
    determinetransp drawlineng4b16bt16x8
drawlineng4b16bnt16x8
    drawline16bmacro16x8 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels16x8,procpixelst16x8,procpixels16x8b,procpixelst16x8b
drawlineng4b16bt16x8
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bms16x8
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bnt16x8
    drawline16bmacro16x8 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr16x8,procpixelstt16x8,procpixelstr16x8b,procpixelstt16x8b
drawlineng4b16bms16x8:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsnt16x8
    drawline16bmacro16x8 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst16x8,procpixelstmst16x8,procpixelsmst16x8b,procpixelstmst16x8b
drawlineng4b16bmsnt16x8
    drawline16bmacro16x8 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt16x8,procpixelstmsnt16x8,procpixelsmsnt16x8b,procpixelstmsnt16x8b

NEWSYM drawlineng16x82b16b
    determinetransp drawlineng2b16bt16x8
drawlineng2b16bnt16x8
    drawline16bmacro16x8 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels16x8,procpixelst16x8,procpixels16x8b,procpixelst16x8b
drawlineng2b16bt16x8
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bms16x8
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bnt16x8
    drawline16bmacro16x8 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr16x8,procpixelstt16x8,procpixelstr16x8b,procpixelstt16x8b
drawlineng2b16bms16x8:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsnt16x8
    drawline16bmacro16x8 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst16x8,procpixelstmst16x8,procpixelsmst16x8b,procpixelstmst16x8b
drawlineng2b16bmsnt16x8
    drawline16bmacro16x8 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt16x8,procpixelstmsnt16x8,procpixelsmsnt16x8b,procpixelstmsnt16x8b

%macro WinClipMacroom 2
    mov byte[tleftn],33
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8

    mov ebx,[ng16bbgval]
    add word[ofsmmptr],2
    inc dword[bg1totng+ebx*4]
    add word[ofsmtptr],2
    mov ax,[ofsmmptr]
    mov ebx,[yposngom]
    mov edx,[flipyposngom]
    mov [yposng],ebx
    mov [flipyposng],edx
    add edi,16
    test eax,03Fh
    jnz short .next
    mov bx,[bgtxadd]
    add ax,bx
    add [ofsmmptr],bx
    add word[ofsmtptr],bx
.next
    mov ebx,[ofsmcptr]
    add ebx,[ofsmcptr2]
    mov ecx,[ofsmval]
    add dword[ofshvaladd],8
%if %2=4
    test dword[ebx-40h],8000h
    jz .noofsm2
    test dword[ebx-40h],ecx
    jz .noofsm2
    mov ebx,[ebx-40h]
%else
    test dword[ebx],ecx
    jz .noofsm2
    mov ebx,[ebx]
%endif
    mov ax,[ofsmtptr]
    and ebx,3FFh
    add ebx,[ofsmcyps]
    test ebx,100h
    jz short .noupper2
    add ax,[ofsmady]
.noupper2
    and ebx,0FFh
    mov edx,ebx
    shr ebx,3
    and edx,07h
    shl ebx,6
    shl edx,3
    add ax,bx
    mov [yposng],edx
    xor edx,38h
    mov [flipyposng],edx
.noofsm2
    mov ebx,[ofsmcptr]
    add ebx,[ofsmcptr2]
    add dword[ofsmcptr2],2
    mov ecx,[ofsmvalh]
    and dword[ofsmcptr2],3Fh
%if %2=4
    test dword[ebx-40h],8000h
    jnz .noofsmh
%endif
    test dword[ebx-40h],ecx
    jz .noofsmh
    mov ebx,[ebx-40h]
    sub ax,[ofsmtptr]
    add ax,[ofsmtptrs]
    add ebx,[ofshvaladd]
    test ebx,100h
    jz short .noleft
    add ax,[ofsmadx]
.noleft
    and ebx,0F8h
    shr ebx,2
    add ax,bx
.noofsmh

    dec byte[tleftn]
    jnz near .winclipped
    pop ebx
    cmp byte[curmosaicsz],1
    jne near domosaicng16b
    ret
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
%endmacro

%macro drawline16bmacroom 9
    mov byte[tleftn],33
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlinengom16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%9
    ret
%endmacro

%macro drawline16bwmacroom 10
    WinClipMacroom %%processwinclip2b,%10
    drawlinengom16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9,%10
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawline16bw2macroom 12
    mov byte[tleftn],33
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawlinengom16b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11,%12
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlinengom16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%12
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

NEWSYM drawlinengom2b16b
    determinetransp drawlineng2b16btom
drawlineng2b16bntom
    CheckWindowing drawlineng2bwinom
    drawline16bmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,4
drawlineng2bwinom:
    drawline16bwmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,procpixelstw,4
drawlineng2b16btom
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bmsom
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bntom
    CheckWindowing drawlineng2bwintom
    drawline16bmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,4
drawlineng2bwintom:
    drawline16bwmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,procpixelstwt,4
drawlineng2b16bmsom:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsntom
    DetermineWindow drawlineng2b16bmstmswom, drawlineng2b16bmstmwom, drawlineng2b16bmstswom
    drawline16bmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,4
drawlineng2b16bmstmswom:
    drawline16bwmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmst,4
drawlineng2b16bmstmwom:
    drawline16bw2macroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,4
drawlineng2b16bmstswom:
    drawline16bw2macroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,4
drawlineng2b16bmsntom
    DetermineWindow drawlineng2b16bmsntmswom, drawlineng2b16bmsntmwom, drawlineng2b16bmsntswom
    drawline16bmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,4
drawlineng2b16bmsntmswom:
    drawline16bwmacroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,4
drawlineng2b16bmsntmwom:
    drawline16bw2macroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,4
drawlineng2b16bmsntswom:
    drawline16bw2macroom tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,4

NEWSYM drawlinengom4b16b
    determinetransp drawlineng4b16btom
drawlineng4b16bntom
    CheckWindowing drawlineng4bwinom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,2
drawlineng4bwinom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,procpixelstw,2
drawlineng4b16btom
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bmsom
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bntom
    CheckWindowing drawlineng4bwintom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,2
drawlineng4bwintom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,procpixelstwt,2
drawlineng4b16bmsom:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsntom
    DetermineWindow drawlineng4b16bmstmswom, drawlineng4b16bmstmwom, drawlineng4b16bmstswom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,2
drawlineng4b16bmstmswom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmst,2
drawlineng4b16bmstmwom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,2
drawlineng4b16bmstswom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,2
drawlineng4b16bmsntom
    DetermineWindow drawlineng4b16bmsntmswom, drawlineng4b16bmsntmwom, drawlineng4b16bmsntswom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,2
drawlineng4b16bmsntmswom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,2
drawlineng4b16bmsntmwom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,2
drawlineng4b16bmsntswom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,2

NEWSYM drawlinengom8b16b
    determinetransp drawlineng8b16btom
drawlineng8b16bntom
    CheckWindowing drawlineng8bwinom
    drawline16bmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,4
drawlineng8bwinom:
    drawline16bwmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,procpixelstw,4
drawlineng8b16btom
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bmsom
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bntom
    CheckWindowing drawlineng8bwintom
    drawline16bmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,4
drawlineng8bwintom:
    drawline16bwmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,procpixelstwt,4
drawlineng8b16bmsom:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsntom
    DetermineWindow drawlineng8b16bmstmswom, drawlineng8b16bmstmwom, drawlineng8b16bmstswom
    drawline16bmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,4
drawlineng8b16bmstmswom:
    drawline16bwmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmst,4
drawlineng8b16bmstmwom:
    drawline16bw2macroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,4
drawlineng8b16bmstswom:
    drawline16bw2macroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,4
drawlineng8b16bmsntom
    DetermineWindow drawlineng8b16bmsntmswom, drawlineng8b16bmsntmwom, drawlineng8b16bmsntswom
    drawline16bmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,4
drawlineng8b16bmsntmswom:
    drawline16bwmacroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,4
drawlineng8b16bmsntmwom:
    drawline16bw2macroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,4
drawlineng8b16bmsntswom:
    drawline16bw2macroom tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,4

%macro WinClipMacroom16x16 2
    mov byte[tleftn],17
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8

    test dword[switch16x16],1
    jz short .skip
    add word[ofsmmptr],2
    add word[ofsmtptr],2
.skip
    mov ebx,[ng16bbgval]
    inc dword[bg1totng+ebx*4]
    mov ax,[ofsmmptr]
    mov ebx,[yposngom]
    mov edx,[flipyposngom]
    mov [yposng],ebx
    mov [flipyposng],edx
    add edi,16
    test eax,03Fh
    jnz short .next
    test dword[switch16x16],1
    jz short .next
    mov bx,[bgtxadd]
    add ax,bx
    add [ofsmmptr],bx
    add word[ofsmtptr],bx
.next
    mov ebx,[ofsmcptr]
    add ebx,[ofsmcptr2]
    mov ecx,[ofsmval]
    add dword[ofshvaladd],8
%if %2=4
    test dword[ebx-40h],8000h
    jz .noofsm2
    test dword[ebx-40h],ecx
    jz .noofsm2
    mov ebx,[ebx-40h]
%else
    test dword[ebx],ecx
    jz .noofsm2
    mov ebx,[ebx]
%endif
    mov ax,[ofsmtptr]
    and ebx,3FFh
    add ebx,[ofsmcyps]
    test ebx,200h
    jz short .noupper2
    add ax,[ofsmady]
.noupper2
    and ebx,1FFh
    mov edx,ebx
    shr ebx,3
    and edx,07h
    shl ebx,6
    shl edx,3
    add ax,bx
    mov [yposng],edx
    xor edx,38h
    mov [flipyposng],edx
.noofsm2
    mov ebx,[ofsmcptr]
    add ebx,[ofsmcptr2]
    add dword[ofsmcptr2],2
    mov ecx,[ofsmvalh]
    and dword[ofsmcptr2],3Fh
%if %2=4
    test dword[ebx-40h],8000h
    jnz .noofsmh
%endif
    test dword[ebx-40h],ecx
    jz .noofsmh
    mov ebx,[ebx-40h]
    sub ax,[ofsmtptr]
    add ax,[ofsmtptrs]
    add ebx,[ofshvaladd]
    test ebx,200h
    jz short .noleft
    add ax,[ofsmadx]
.noleft
    and ebx,0F8h
    shr ebx,2
    add ax,bx
.noofsmh

    xor dword[switch16x16],1
    jnz .winclipped
    dec byte[tleftn]
    jnz near .winclipped
    pop ebx
    cmp byte[curmosaicsz],1
    jne near domosaicng16b
    ret
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %1
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
%endmacro

%macro drawline16bmacroom16x16 9
    mov byte[tleftn],17
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlinengom16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8,%9
    ret
%endmacro

%macro drawline16bwmacroom16x16 10
    WinClipMacroom16x16 %%processwinclip2b,%10
    drawlinengom16b16x16 %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%10
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9,%10
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

%macro drawline16bw2macroom16x16 12
    mov byte[tleftn],17
    mov dword[ngcwinptr],ngwintable
    mov dword[ngcwinmode],0
    cmp dword[ngwintable],0
    jne near .loop
    add dword[ngcwinptr],4
    mov dword[ngcwinmode],1
.winclipped
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finlineb
    drawlinengom16b16x16 %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11,%12
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlinengom16b16x16 %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8,%12
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9,%12
%%loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near .winclipped
    jmp .loop
%endmacro

NEWSYM drawlinengom16x162b16b
    determinetransp drawlineng2b16btom16x16
drawlineng2b16bntom16x16
    CheckWindowing drawlineng2bwinom16x16
    drawline16bmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,4
drawlineng2bwinom16x16:
    drawline16bwmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,procpixelstw,4
drawlineng2b16btom16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bmsom16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bntom16x16
    CheckWindowing drawlineng2bwintom16x16
    drawline16bmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,4
drawlineng2bwintom16x16:
    drawline16bwmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,procpixelstwt,4
drawlineng2b16bmsom16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsntom16x16
    DetermineWindow drawlineng2b16bmstmswom16x16, drawlineng2b16bmstmwom16x16, drawlineng2b16bmstswom16x16
    drawline16bmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,4
drawlineng2b16bmstmswom16x16:
    drawline16bwmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmst,4
drawlineng2b16bmstmwom16x16:
    drawline16bw2macroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,4
drawlineng2b16bmstswom16x16:
    drawline16bw2macroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,4
drawlineng2b16bmsntom16x16
    DetermineWindow drawlineng2b16bmsntmswom16x16, drawlineng2b16bmsntmwom16x16, drawlineng2b16bmsntswom16x16
    drawline16bmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,4
drawlineng2b16bmsntmswom16x16:
    drawline16bwmacroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,4
drawlineng2b16bmsntmwom16x16:
    drawline16bw2macroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,4
drawlineng2b16bmsntswom16x16:
    drawline16bw2macroom16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,4

NEWSYM drawlinengom16x164b16b
    determinetransp drawlineng4b16btom16x16
drawlineng4b16bntom16x16
    CheckWindowing drawlineng4bwinom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,2
drawlineng4bwinom16x16:
    drawline16bwmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,procpixelstw,2
drawlineng4b16btom16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bmsom16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bntom16x16
    CheckWindowing drawlineng4bwintom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,2
drawlineng4bwintom16x16:
    drawline16bwmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,procpixelstwt,2
drawlineng4b16bmsom16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsntom16x16
    DetermineWindow drawlineng4b16bmstmswom16x16, drawlineng4b16bmstmwom16x16, drawlineng4b16bmstswom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,2
drawlineng4b16bmstmswom16x16:
    drawline16bwmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmst,2
drawlineng4b16bmstmwom16x16:
    drawline16bw2macroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,2
drawlineng4b16bmstswom16x16:
    drawline16bw2macroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,2
drawlineng4b16bmsntom16x16
    DetermineWindow drawlineng4b16bmsntmswom16x16, drawlineng4b16bmsntmwom16x16, drawlineng4b16bmsntswom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,2
drawlineng4b16bmsntmswom16x16:
    drawline16bwmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,2
drawlineng4b16bmsntmwom16x16:
    drawline16bw2macroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,2
drawlineng4b16bmsntswom16x16:
    drawline16bw2macroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,2

NEWSYM drawlinengom16x168b16b
    determinetransp drawlineng8b16btom16x16
drawlineng8b16bntom16x16
    CheckWindowing drawlineng8bwinom16x16
    drawline16bmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,4
drawlineng8bwinom16x16:
    drawline16bwmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,procpixelstw,4
drawlineng8b16btom16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bmsom16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bntom16x16
    CheckWindowing drawlineng8bwintom16x16
    drawline16bmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,4
drawlineng8bwintom16x16:
    drawline16bwmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,procpixelstwt,4
drawlineng8b16bmsom16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsntom16x16
    DetermineWindow drawlineng8b16bmstmswom16x16, drawlineng8b16bmstmwom16x16, drawlineng8b16bmstswom16x16
    drawline16bmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,4
drawlineng8b16bmstmswom16x16:
    drawline16bwmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmst,4
drawlineng8b16bmstmwom16x16:
    drawline16bw2macroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts,4
drawlineng8b16bmstswom16x16:
    drawline16bw2macroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt,4
drawlineng8b16bmsntom16x16
    DetermineWindow drawlineng8b16bmsntmswom16x16, drawlineng8b16bmsntmwom16x16, drawlineng8b16bmsntswom16x16
    drawline16bmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,4
drawlineng8b16bmsntmswom16x16:
    drawline16bwmacroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt,4
drawlineng8b16bmsntmwom16x16:
    drawline16bw2macroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts,4
drawlineng8b16bmsntswom16x16:
    drawline16bw2macroom16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixels,procpixelst,4
