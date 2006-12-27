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

EXTSYM ngwintable,ngwinen,ngcwinptr,ngcpixleft,ngcwinmode,cachesingle4bng
EXTSYM tleftn,ng16bprval,vrama,bg1drwng,ng16bbgval,bg1totng,ngptrdat2
EXTSYM bgtxadd,taddnfy16x16,taddfy16x16,switch16x16,curmosaicsz,domosaicng
EXTSYM vidmemch4,vidmemch2,vidmemch8,mode0add,vcache4b,vcache2b,vcache8b
EXTSYM cachesingle2bng,cachesingle8bng,ngpalcon4b,ngpalcon8b,ofshvaladd
EXTSYM ngpalcon2b,tleftnb,tltype2b,tltype4b,tltype8b,yposng,flipyposng
EXTSYM ofsmcptr,ofsmtptr,ofsmmptr,ofsmcyps,ofsmady,ofsmadx,ofsmtptrs,ofsmcptr2
EXTSYM yposngom,flipyposngom,cbgval,ofsmval,ofsmvalh,bgtxadd2

%include "video/vidmacro.mac"
%include "video/newgfx2.mac"
%include "video/newgfx.mac"
%include "video/newgfxwn.mac"


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
    add edi,8
    inc dword[bg1totng+ebx*4]
    test eax,03Fh
    jnz .notileadd
    add ax,[bgtxadd]
.notileadd
    dec byte[tleftn]
    jnz .winclipped
    pop ebx
    cmp byte[curmosaicsz],1
    jne near domosaicng
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

SECTION .text

NEWSYM drawtileng2b
    cmp byte[ngwinen],1
    je near drawtileng2bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
    ret
drawtileng2bwin:
    WinClipMacro Processwinclip2bt
    drawtileng tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
Processwinclip2bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtilengwin tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng2bwin.winclipped
    jmp drawtileng2bwin.loop


NEWSYM drawtileng4b
    cmp byte[ngwinen],1
    je near drawtileng4bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
    ret
drawtileng4bwin:
    WinClipMacro Processwinclip4bt
    drawtileng tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
Processwinclip4bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtilengwin tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng4bwin.winclipped
    jmp drawtileng4bwin.loop

NEWSYM drawtileng8b
    cmp byte[ngwinen],1
    je near drawtileng8bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
    ret

drawtileng8bwin:
    WinClipMacro Processwinclip8bt
    drawtileng tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
Processwinclip8bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtilengwin tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng8bwin.winclipped
    jmp drawtileng8bwin.loop

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
    jne near domosaicng
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

NEWSYM drawtileng16x162b
    cmp byte[ngwinen],1
    je near drawtileng16x162bwin
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng16x16 tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
    ret
drawtileng16x162bwin:
    WinClipMacro16x16 Processwinclip16x162bt
    drawtileng16x16 tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
Processwinclip16x162bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtileng16x16win tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng16x162bwin.winclipped
    jmp drawtileng16x162bwin.loop

NEWSYM drawtileng16x164b
    cmp byte[ngwinen],1
    je near drawtileng16x164bwin
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng16x16 tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
    ret
drawtileng16x164bwin:
    WinClipMacro16x16 Processwinclip16x164bt
    drawtileng16x16 tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
Processwinclip16x164bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtileng16x16win tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng16x164bwin.winclipped
    jmp drawtileng16x164bwin.loop

NEWSYM drawtileng16x168b
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawtileng16x16 tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
    ret
drawtileng16x168bwin:
    WinClipMacro16x16 Processwinclip16x168bt
    drawtileng16x16 tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
Processwinclip16x168bt:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawtileng16x16win tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawtileng16x168bwin.winclipped
    jmp drawtileng16x168bwin.loop

;******************************************
; 8x8 tiles - line by line engine
;******************************************

NEWSYM drawlineng2b
    cmp byte[ngwinen],1
    je near drawlineng2bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
    ret
drawlineng2bwin:
    WinClipMacro Processwinclip2b
    drawlineng tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
Processwinclip2b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlinengwin tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng2bwin.winclipped
    jmp drawlineng2bwin.loop

NEWSYM drawlineng4b
    cmp byte[ngwinen],1
    je near drawlineng4bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
    ret
drawlineng4bwin:
    WinClipMacro Processwinclip4b
    drawlineng tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
Processwinclip4b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlinengwin tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng4bwin.winclipped
    jmp drawlineng4bwin.loop

NEWSYM drawlineng8b
    cmp byte[ngwinen],1
    je near drawlineng8bwin
    mov byte[tleftn],33
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
    ret
drawlineng8bwin:
    WinClipMacro Processwinclip8b
    drawlineng tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
Processwinclip8b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlinengwin tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng8bwin.winclipped
    jmp drawlineng8bwin.loop

;******************************************
; 16x16 tiles - line by line engine
;******************************************

NEWSYM drawlineng16x162b
    cmp byte[ngwinen],1
    je near drawlineng16x162bwin
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng16x16 tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
    ret
drawlineng16x162bwin:
    WinClipMacro16x16 Processwinclip16x162b
    drawlineng16x16 tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
Processwinclip16x162b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlineng16x16win tltype2b, preparet2ba, cachesingle2bng,.loop,.finline,ngpalcon2b,test2ba,03h
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng16x162bwin.winclipped
    jmp drawlineng16x162bwin.loop

NEWSYM drawlineng16x164b
    cmp byte[ngwinen],1
    je near drawlineng16x164bwin
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng16x16 tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
    ret
drawlineng16x164bwin:
    WinClipMacro16x16 Processwinclip16x164b
    drawlineng16x16 tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
Processwinclip16x164b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlineng16x16win tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng16x164bwin.winclipped
    jmp drawlineng16x164bwin.loop

NEWSYM drawlineng16x168b
    cmp byte[ngwinen],1
    je near drawlineng16x168bwin
    mov byte[tleftn],17
.loop
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .finline
    drawlineng16x16 tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
    ret
drawlineng16x168bwin:
    WinClipMacro16x16 Processwinclip16x168b
    drawlineng16x16 tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
Processwinclip16x168b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .finline
    drawlineng16x16win tltype8b, preparet8ba, cachesingle8bng,.loop,.finline,ngpalcon8b,test8ba,0FFh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlineng16x168bwin.winclipped
    jmp drawlineng16x168bwin.loop

NEWSYM drawlineng16x84b
    mov byte[tleftn],33
.loop2b
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .fintile2b
    drawlineng16x8 tltype4b, preparet4ba, cachesingle4bng,.loop2b,.fintile2b,ngpalcon4b,test4ba,0Fh

NEWSYM drawlineng16x82b
    mov byte[tleftn],33
.loopb2b
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .fintileb2b
    drawlineng16x8 tltype2b, preparet2ba, cachesingle2bng,.loopb2b,.fintileb2b,ngpalcon2b,test2ba,03h

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
    add edi,8
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
    jne near domosaicng
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

NEWSYM drawlinengom4b
    cmp byte[ngwinen],1
    je near drawlinengom4bwin
    mov byte[tleftn],33
.loopd
    mov cx,[vrama+eax]
    xor ecx,[ng16bprval]
    test ecx,2000h
    jnz near .fintiled
    drawlinengom  tltype4b, preparet4ba, cachesingle4bng,.loopd,.fintiled,ngpalcon4b,test4ba,0Fh
drawlinengom4bwin:
    WinClipMacroom Processwinclipom4b
    drawlinengom  tltype4b, preparet4ba, cachesingle4bng,.loop,.finline,ngpalcon4b,test4ba,0Fh
Processwinclipom4b:
    mov ebx,[ngcwinptr]
    mov cx,[vrama+eax]
    mov ebx,[ebx]
    xor ecx,[ng16bprval]
    mov [ngcpixleft],ebx
    test ecx,2000h
    jnz near .fintiled
    drawlinengomwin  tltype4b, preparet4ba, cachesingle4bng,.loop,.fintiled,ngpalcon4b,test4ba,0Fh
.loop
    push eax
    mov ebx,[ngcwinptr]
    mov eax,[ngcpixleft]
    mov [ebx],eax
    pop eax
    cmp dword[ngcwinmode],1
    je near drawlinengom4bwin.winclipped
    jmp drawlinengom4bwin.loop


