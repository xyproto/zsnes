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

EXTSYM ngwintable,ngwinen,ngcwinptr,ngcpixleft,ngcwinmode,tleftn,ng16bprval
EXTSYM vrama,bg1drwng,ng16bbgval,bg1totng,bgtxadd,taddnfy16x16,taddfy16x16
EXTSYM switch16x16,curmosaicsz,domosaicng16b,vidmemch2,vidmemch4,vidmemch8
EXTSYM mode0add,vcache4b,vcache2b,vcache8b,cachesingle2bng,cachesingle8bng
EXTSYM ngpalcon4b,ngpalcon8b,ngpalcon2b,tleftnb,tltype2b,tltype4b,tltype8b
EXTSYM yposng,flipyposng,ofsmcptr,ofsmtptr,ofsmmptr,ofsmcyps,ofsmady,ofsmadx
EXTSYM FillSubScr,UnusedBitXor,yposngom,flipyposngom,cbgval,ofsmval,ofsmvalh
EXTSYM CPalPtrng,BGMS1,scadtng,CMainWinScr,CSubWinScr,UnusedBit,res640
EXTSYM mosclineval,mostranspval,vcache2bs,vcache4bs,vcache8bs,vidmemch2s
EXTSYM vidmemch4s,vidmemch8s,cpalval,bgtxadd2,SpecialLine,cachesingle4bng
EXTSYM ofshvaladd,ofsmtptrs,ofsmcptr2,ngptrdat2

%include "video/vidmacro.mac"
%include "video/newg162.mac"
%include "video/newgfx16.mac"
%include "video/newg16wn.mac"

;******************************************
; 16bitng caching functions
;******************************************

%macro cacheloopstuff 1
    mov bl,[esi+%1]
    or bl,bl
    jnz %%okay
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
    jnz .notileadd
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

%macro determinetransp 1
    mov [mostranspval],dl
    mov [mosclineval],ebx
    add ecx,[CMainWinScr]
    cmp byte[curmosaicsz],1
    jne .mosaic
    test byte[BGMS1+ebx*2],dl
    jz .nosubmain
    test byte[FillSubScr+ebx],1
    jnz near %1
    jmp .main
.nosubmain
    test byte[FillSubScr+ebx],1
    jz .main
    sub ecx,[CMainWinScr]
    add ecx,[CSubWinScr]
    add edi,75036*2
    jmp .main
.mosaic
    test byte[FillSubScr+ebx],1
    jz .main
    sub ecx,[CMainWinScr]
    add ecx,[CSubWinScr]
.main
%endmacro

%macro CheckWindowing 1
    cmp byte[ngwinen],0
    je %%nowindowing
    cmp byte[ecx],0
    jne near %1
%%nowindowing
%endmacro

%macro DetermineWindow 3        ; both,main,sub
    cmp byte[ngwinen],0
    je %%nowindow
    cmp byte[ecx],0
    jz near %3
    sub ecx,[CMainWinScr]
    add ecx,[CSubWinScr]
    cmp byte[ecx],0
    jnz near %1
    jmp %2
%%nowindow
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
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng2b16bt
drawtileng2b16bnt
    CheckWindowing drawtileng2bwin
    drawtile16b tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormal,tilenormalb
drawtileng2bwin:
    drawtile16bw tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng2b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng2b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bnt
    CheckWindowing drawtileng2bwint
    drawtile16b tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalt,tilenormalbt
drawtileng2bwint:
    drawtile16bw tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng2b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bmsnt
    DetermineWindow drawtileng2b16bmstmsw, drawtileng2b16bmstmw, drawtileng2b16bmstsw
    drawtile16b tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst
drawtileng2b16bmstmsw:
    drawtile16bw tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng2b16bmstmw:
    drawtile16bw2 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng2b16bmstsw:
    drawtile16bw2 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng2b16bmsnt
    DetermineWindow drawtileng2b16bmsntmsw, drawtileng2b16bmsntmw, drawtileng2b16bmsntsw
    drawtile16b tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng2b16bmsntmsw:
    drawtile16bw tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng2b16bmsntmw:
    drawtile16bw2 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng2b16bmsntsw:
    drawtile16bw2 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

NEWSYM drawtileng4b16b
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng4b16bt
drawtileng4b16bnt
    CheckWindowing drawtileng4bwin
    drawtile16b tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormal,tilenormalb
drawtileng4bwin:
    drawtile16bw tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng4b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng4b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bnt
    CheckWindowing drawtileng4bwint
    drawtile16b tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalt,tilenormalbt
drawtileng4bwint:
    drawtile16bw tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng4b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bmsnt
    DetermineWindow drawtileng4b16bmstmsw, drawtileng4b16bmstmw, drawtileng4b16bmstsw
    drawtile16b tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst
drawtileng4b16bmstmsw:
    drawtile16bw tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng4b16bmstmw:
    drawtile16bw2 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng4b16bmstsw:
    drawtile16bw2 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng4b16bmsnt
    DetermineWindow drawtileng4b16bmsntmsw, drawtileng4b16bmsntmw, drawtileng4b16bmsntsw
    drawtile16b tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng4b16bmsntmsw:
    drawtile16bw tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng4b16bmsntmw:
    drawtile16bw2 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng4b16bmsntsw:
    drawtile16bw2 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

NEWSYM drawtileng8b16b
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng8b16bt
drawtileng8b16bnt
    CheckWindowing drawtileng8bwin
    drawtile16b tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0Fh,vidmemch8s,cache8b16b,tilenormal,tilenormalb
drawtileng8bwin:
    drawtile16bw tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0Fh,vidmemch8s,cache8b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng8b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng8b16bms
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bnt
    CheckWindowing drawtileng8bwint
    drawtile16b tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0Fh,vidmemch8s,cache8b16b,tilenormalt,tilenormalbt
drawtileng8bwint:
    drawtile16bw tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0Fh,vidmemch8s,cache8b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng8b16bms:
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bmsnt
    DetermineWindow drawtileng8b16bmstmsw, drawtileng8b16bmstmw, drawtileng8b16bmstsw
    drawtile16b tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst
drawtileng8b16bmstmsw:
    drawtile16bw tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng8b16bmstmw:
    drawtile16bw2 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng8b16bmstsw:
    drawtile16bw2 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng8b16bmsnt
    DetermineWindow drawtileng8b16bmsntmsw, drawtileng8b16bmsntmw, drawtileng8b16bmsntsw
    drawtile16b tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng8b16bmsntmsw:
    drawtile16bw tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng8b16bmsntmw:
    drawtile16bw2 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng8b16bmsntsw:
    drawtile16bw2 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

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
    jnz .notileadd
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
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng2b16bt16x16
drawtileng2b16bnt16x16
    CheckWindowing drawtileng2bwin16x16
    drawtile16b16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormal,tilenormalb
drawtileng2bwin16x16:
    drawtile16bw16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng2b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng2b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bnt16x16
    CheckWindowing drawtileng2bwint16x16
    drawtile16b16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalt,tilenormalbt
drawtileng2bwint16x16:
    drawtile16bw16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng2b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng2b16bmsnt16x16
    DetermineWindow drawtileng2b16bmstmsw16x16, drawtileng2b16bmstmw16x16, drawtileng2b16bmstsw16x16
    drawtile16b16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst
drawtileng2b16bmstmsw16x16:
    drawtile16bw16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng2b16bmstmw16x16:
    drawtile16bw216x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng2b16bmstsw16x16:
    drawtile16bw216x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng2b16bmsnt16x16
    DetermineWindow drawtileng2b16bmsntmsw16x16, drawtileng2b16bmsntmw16x16, drawtileng2b16bmsntsw16x16
    drawtile16b16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng2b16bmsntmsw16x16:
    drawtile16bw16x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng2b16bmsntmw16x16:
    drawtile16bw216x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng2b16bmsntsw16x16:
    drawtile16bw216x16 tltype2b, preparet2batile, cachesingle2bng,ngpalcon2b,test2ba,03h,vidmemch2s,cache2b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

NEWSYM drawtileng16x164b16b
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng4b16bt16x16
drawtileng4b16bnt16x16
    CheckWindowing drawtileng4bwin16x16
    drawtile16b16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormal,tilenormalb
drawtileng4bwin16x16:
    drawtile16bw16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng4b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng4b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bnt16x16
    CheckWindowing drawtileng4bwint16x16
    drawtile16b16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalt,tilenormalbt
drawtileng4bwint16x16:
    drawtile16bw16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng4b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng4b16bmsnt16x16
    DetermineWindow drawtileng4b16bmstmsw16x16, drawtileng4b16bmstmw16x16, drawtileng4b16bmstsw16x16
    drawtile16b16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst
drawtileng4b16bmstmsw16x16:
    drawtile16bw16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng4b16bmstmw16x16:
    drawtile16bw216x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng4b16bmstsw16x16:
    drawtile16bw216x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng4b16bmsnt16x16
    DetermineWindow drawtileng4b16bmsntmsw16x16, drawtileng4b16bmsntmw16x16, drawtileng4b16bmsntsw16x16
    drawtile16b16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng4b16bmsntmsw16x16:
    drawtile16bw16x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng4b16bmsntmw16x16:
    drawtile16bw216x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng4b16bmsntsw16x16:
    drawtile16bw216x16 tltype4b, preparet4batile, cachesingle4bng,ngpalcon4b,test4ba,0Fh,vidmemch4s,cache4b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

NEWSYM drawtileng16x168b16b
    mov ebp,[cpalval+ebx*4]
    determinetransp drawtileng8b16bt16x16
drawtileng8b16bnt16x16
    CheckWindowing drawtileng8bwin16x16
    drawtile16b16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormal,tilenormalb
drawtileng8bwin16x16:
    drawtile16bw16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormal,tilenormalb,tilenormalw,tilenormalwb
drawtileng8b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawtileng8b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bnt16x16
    CheckWindowing drawtileng8bwint16x16
    drawtile16b16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalt,tilenormalbt
drawtileng8bwint16x16:
    drawtile16bw16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalt,tilenormalbt,tilenormalwt,tilenormalwbt
drawtileng8b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawtileng8b16bmsnt16x16
    DetermineWindow drawtileng8b16bmstmsw16x16, drawtileng8b16bmstmw16x16, drawtileng8b16bmstsw16x16
    drawtile16b16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst
drawtileng8b16bmstmsw16x16:
    drawtile16bw16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwmst,tilenormalwbmst
drawtileng8b16bmstmw16x16:
    drawtile16bw216x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwsmt,tilenormalwbsmt,tilenormals,tilenormalbs
drawtileng8b16bmstsw16x16:
    drawtile16bw216x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmst,tilenormalbmst,tilenormalwmsbt,tilenormalwbmsbt,tilenormalt,tilenormalbt
drawtileng8b16bmsnt16x16
    DetermineWindow drawtileng8b16bmsntmsw16x16, drawtileng8b16bmsntmw16x16, drawtileng8b16bmsntsw16x16
    drawtile16b16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt
drawtileng8b16bmsntmsw16x16:
    drawtile16bw16x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsnt,tilenormalwbmsnt
drawtileng8b16bmsntmw16x16:
    drawtile16bw216x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwsmnt,tilenormalwbsmnt,tilenormals,tilenormalbs
drawtileng8b16bmsntsw16x16:
    drawtile16bw216x16 tltype8b, preparet8batile, cachesingle8bng,ngpalcon8b,test8ba,0FFh,vidmemch8s,cache8b16b,tilenormalmsnt,tilenormalbmsnt,tilenormalwmsbnt,tilenormalwbmsbnt,tilenormal,tilenormalb

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
    drawline16bmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst
drawlineng2bwin:
    drawline16bwmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,procpixelstw
drawlineng2b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bnt
    CheckWindowing drawlineng2bwint
    drawline16bmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt
drawlineng2bwint:
    drawline16bwmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,procpixelstwt
drawlineng2b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsnt
    DetermineWindow drawlineng2b16bmstmsw, drawlineng2b16bmstmw, drawlineng2b16bmstsw
    drawline16bmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst
drawlineng2b16bmstmsw:
    drawline16bwmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng2b16bmstmw:
    drawline16bw2macro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng2b16bmstsw:
    drawline16bw2macro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng2b16bmsnt
    DetermineWindow drawlineng2b16bmsntmsw, drawlineng2b16bmsntmw, drawlineng2b16bmsntsw
    drawline16bmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt
drawlineng2b16bmsntmsw:
    drawline16bwmacro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng2b16bmsntmw:
    drawline16bw2macro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng2b16bmsntsw:
    drawline16bw2macro tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

NEWSYM drawlineng4b16b
    determinetransp drawlineng4b16bt
drawlineng4b16bnt
    CheckWindowing drawlineng4bwin
    drawline16bmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst
drawlineng4bwin:
    drawline16bwmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,procpixelstw
drawlineng4b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bnt
    CheckWindowing drawlineng4bwint
    drawline16bmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt
drawlineng4bwint:
    drawline16bwmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,procpixelstwt
drawlineng4b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsnt
    DetermineWindow drawlineng4b16bmstmsw, drawlineng4b16bmstmw, drawlineng4b16bmstsw
    drawline16bmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst
drawlineng4b16bmstmsw:
    drawline16bwmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng4b16bmstmw:
    drawline16bw2macro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng4b16bmstsw:
    drawline16bw2macro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng4b16bmsnt
    DetermineWindow drawlineng4b16bmsntmsw, drawlineng4b16bmsntmw, drawlineng4b16bmsntsw
    drawline16bmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt
drawlineng4b16bmsntmsw:
    drawline16bwmacro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng4b16bmsntmw:
    drawline16bw2macro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng4b16bmsntsw:
    drawline16bw2macro tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

NEWSYM drawlineng8b16b
    determinetransp drawlineng8b16bt
drawlineng8b16bnt
    CheckWindowing drawlineng8bwin
    drawline16bmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst
drawlineng8bwin:
    drawline16bwmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,procpixelstw
drawlineng8b16bt
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bms
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bnt
    CheckWindowing drawlineng8bwint
    drawline16bmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt
drawlineng8bwint:
    drawline16bwmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,procpixelstwt
drawlineng8b16bms:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsnt
    DetermineWindow drawlineng8b16bmstmsw, drawlineng8b16bmstmw, drawlineng8b16bmstsw
    drawline16bmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst
drawlineng8b16bmstmsw:
    drawline16bwmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng8b16bmstmw:
    drawline16bw2macro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng8b16bmstsw:
    drawline16bw2macro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng8b16bmsnt
    DetermineWindow drawlineng8b16bmsntmsw, drawlineng8b16bmsntmw, drawlineng8b16bmsntsw
    drawline16bmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt
drawlineng8b16bmsntmsw:
    drawline16bwmacro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng8b16bmsntmw:
    drawline16bw2macro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng8b16bmsntsw:
    drawline16bw2macro tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

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
    drawline16bmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst
drawlineng2bwin16x16:
    drawline16bwmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixels,procpixelst,procpixelstw
drawlineng2b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng2b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bnt16x16
    CheckWindowing drawlineng2bwint16x16
    drawline16bmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt
drawlineng2bwint16x16:
    drawline16bwmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelstr,procpixelstt,procpixelstwt
drawlineng2b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng2b16bmsnt16x16
    DetermineWindow drawlineng2b16bmstmsw16x16, drawlineng2b16bmstmw16x16, drawlineng2b16bmstsw16x16
    drawline16bmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst
drawlineng2b16bmstmsw16x16:
    drawline16bwmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng2b16bmstmw16x16:
    drawline16bw2macro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng2b16bmstsw16x16:
    drawline16bw2macro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng2b16bmsnt16x16
    DetermineWindow drawlineng2b16bmsntmsw16x16, drawlineng2b16bmsntmw16x16, drawlineng2b16bmsntsw16x16
    drawline16bmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt
drawlineng2b16bmsntmsw16x16:
    drawline16bwmacro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng2b16bmsntmw16x16:
    drawline16bw2macro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng2b16bmsntsw16x16:
    drawline16bw2macro16x16 tltype2b,preparet2ba,cachesingle2bng,ngpalcon2b,test2ba,03h,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

NEWSYM drawlineng16x164b16b
    determinetransp drawlineng4b16bt16x16
drawlineng4b16bnt16x16
    CheckWindowing drawlineng4bwin16x16
    drawline16bmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst
drawlineng4bwin16x16:
    drawline16bwmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,procpixelstw
drawlineng4b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bnt16x16
    CheckWindowing drawlineng4bwint16x16
    drawline16bmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt
drawlineng4bwint16x16:
    drawline16bwmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,procpixelstwt
drawlineng4b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsnt16x16
    DetermineWindow drawlineng4b16bmstmsw16x16, drawlineng4b16bmstmw16x16, drawlineng4b16bmstsw16x16
    drawline16bmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst
drawlineng4b16bmstmsw16x16:
    drawline16bwmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng4b16bmstmw16x16:
    drawline16bw2macro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng4b16bmstsw16x16:
    drawline16bw2macro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng4b16bmsnt16x16
    DetermineWindow drawlineng4b16bmsntmsw16x16, drawlineng4b16bmsntmw16x16, drawlineng4b16bmsntsw16x16
    drawline16bmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt
drawlineng4b16bmsntmsw16x16:
    drawline16bwmacro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng4b16bmsntmw16x16:
    drawline16bw2macro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng4b16bmsntsw16x16:
    drawline16bw2macro16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

NEWSYM drawlineng16x168b16b
    determinetransp drawlineng8b16bt16x16
drawlineng8b16bnt16x16
    CheckWindowing drawlineng8bwin16x16
    drawline16bmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst
drawlineng8bwin16x16:
    drawline16bwmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixels,procpixelst,procpixelstw
drawlineng8b16bt16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng8b16bms16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bnt16x16
    CheckWindowing drawlineng8bwint16x16
    drawline16bmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt
drawlineng8bwint16x16:
    drawline16bwmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelstr,procpixelstt,procpixelstwt
drawlineng8b16bms16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng8b16bmsnt16x16
    DetermineWindow drawlineng8b16bmstmsw16x16, drawlineng8b16bmstmw16x16, drawlineng8b16bmstsw16x16
    drawline16bmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst
drawlineng8b16bmstmsw16x16:
    drawline16bwmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng8b16bmstmw16x16:
    drawline16bw2macro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng8b16bmstsw16x16:
    drawline16bw2macro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng8b16bmsnt16x16
    DetermineWindow drawlineng8b16bmsntmsw16x16, drawlineng8b16bmsntmw16x16, drawlineng8b16bmsntsw16x16
    drawline16bmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt
drawlineng8b16bmsntmsw16x16:
    drawline16bwmacro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng8b16bmsntmw16x16:
    drawline16bw2macro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng8b16bmsntsw16x16:
    drawline16bw2macro16x16 tltype8b,preparet8ba,cachesingle8bng,ngpalcon8b,test8ba,0FFh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

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
    jnz %%mosloop
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

%macro WinClipMacroom 1
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

    mov ebx,[cbgval]
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
    jnz .next
    mov bx,[bgtxadd]
    add ax,bx
    add [ofsmmptr],bx
    add word[ofsmtptr],bx
.next
    mov ebx,[ofsmcptr]
    add ebx,[ofsmcptr2]
    mov ecx,[ofsmval]
    add dword[ofshvaladd],8
    test dword[ebx],ecx
    jz .noofsm2
    mov ebx,[ebx]
    mov ax,[ofsmtptr]
    and ebx,3FFh
    add ebx,[ofsmcyps]
    test ebx,100h
    jz .noupper2
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
    test dword[ebx-40h],ecx
    jz .noofsmh
    mov ebx,[ebx-40h]
    sub ax,[ofsmtptr]
    add ax,[ofsmtptrs]
    add ebx,[ofshvaladd]
    test ebx,100h
    jz .noleft
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

%macro drawline16bmacroom 8
    mov byte[tleftn],33
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlinengom16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8
    ret
%endmacro

%macro drawline16bwmacroom 9
    WinClipMacroom %%processwinclip2b
    drawlinengom16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
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

%macro drawline16bw2macroom 11
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
    drawlinengom16b %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],8
    jbe near %%processwinclip2b
    sub dword[ebx],8
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlinengom16b %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
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

NEWSYM drawlinengom4b16b
    determinetransp drawlineng4b16btom
drawlineng4b16bntom
    CheckWindowing drawlineng4bwinom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst
drawlineng4bwinom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst,procpixelstw
drawlineng4b16btom
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bmsom
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bntom
    CheckWindowing drawlineng4bwintom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt
drawlineng4bwintom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt,procpixelstwt
drawlineng4b16bmsom:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsntom
    DetermineWindow drawlineng4b16bmstmswom, drawlineng4b16bmstmwom, drawlineng4b16bmstswom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst
drawlineng4b16bmstmswom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmst
drawlineng4b16bmstmwom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwsmt,procpixelss,procpixelsts
drawlineng4b16bmstswom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst,procpixelstwmsbt,procpixelstr,procpixelstt
drawlineng4b16bmsntom
    DetermineWindow drawlineng4b16bmsntmswom, drawlineng4b16bmsntmwom, drawlineng4b16bmsntswom
    drawline16bmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt
drawlineng4b16bmsntmswom:
    drawline16bwmacroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsnt
drawlineng4b16bmsntmwom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwsmnt,procpixelss,procpixelsts
drawlineng4b16bmsntswom:
    drawline16bw2macroom tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt,procpixelstwmsbnt,procpixelsnt,procpixelstnt

%macro drawline16bmacroom16x16 8
    mov byte[tleftn],17
%%loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near %%finline
    drawlinengom16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%7,%8
    ret
%endmacro

%macro drawline16bwmacroom16x16 9
    WinClipMacro16x16 %%processwinclip2b
    drawlinengom16b16x16 %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
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

%macro drawline16bw2macroom16x16 11
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
    drawlinengom16b16x16 %1,%2,%3,.winclipped,.finlineb,%4,%5,%6,%10,%11
.loop
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    cmp dword[ebx],16
    jbe near %%processwinclip2b
    sub dword[ebx],16
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlinengom16b16x16 %1,%2,%3,.loop,.finline,%4,%5,%6,%7,%8
%%processwinclip2b
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near %%finline
    drawlinengomwin16b16x16 %1,%2,%3,%%loop,%%finline,%4,%5,%6,%9
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

NEWSYM drawlinengom16x164b16b
    determinetransp drawlineng4b16btom16x16
drawlineng4b16bntom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixels,procpixelst
drawlineng4b16btom16x16
    test byte[BGMS1+ebx*2+1],dl
    jnz near drawlineng4b16bmsom16x16
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bntom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelstr,procpixelstt
drawlineng4b16bmsom16x16:
    test byte[scadtng+ebx],dl
    jz near drawlineng4b16bmsntom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmst,procpixelstmst
drawlineng4b16bmsntom16x16
    drawline16bmacroom16x16 tltype4b,preparet4ba,cachesingle4bng,ngpalcon4b,test4ba,0Fh,procpixelsmsnt,procpixelstmsnt
