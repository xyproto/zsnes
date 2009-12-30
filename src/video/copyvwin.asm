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



%include "macros.mac"

EXTSYM vidbuffer,GUIOn,MMXSupport,resolutn,En2xSaI,antienab,scanlines
EXTSYM hirestiledat,res512switch,curblank,spritetablea,lineleft,_2xSaILine
EXTSYM _2xSaISuperEagleLine,_2xSaISuper2xSaILine,newengen,cfield,HalfTrans
EXTSYM GUIOn2,FilteredGUI,SpecialLine,vidbufferofsb,HalfTransB,HalfTransC
EXTSYM HighResProc,MMXInterpolwin

%macro SelectTile 0
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je %%loopab
    cmp byte[newengen],0
    je %%loopab
    mov ebx,SpecialLine+1
%%loopab
%endmacro

ALIGN32
SECTION .bss
NEWSYM AddEndBytes, resd 1
NEWSYM NumBytesPerLine, resd 1
NEWSYM WinVidMemStart, resd 1
SECTION .text

NEWSYM interpolate640x480x16bwin
    SelectTile
    cmp byte[MMXSupport],1
    jne .noMMXSupport
    ccallv MMXInterpolwin, esi, edi, edx
    ret
.noMMXSupport
    mov [InterPtr],ebx
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    mov [lineleft],dl
    ; do first line
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512*256
.a
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edi],ebx
    mov [edx],ebx
    add esi,2
    add edi,4
    add edx,4
    dec ecx
    jnz .a
    add esi,66
    add edi,[AddEndBytes]
    add edi,4
.loopb
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehr

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    jmp .returninterp
.ignorehr
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512*256
.c
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov eax,[edx]
    mov bx,[esi]
    and eax,[HalfTrans]
    mov [edx],ebx
    and ebx,[HalfTrans]
    shr eax,1
    shr ebx,1
    add eax,ebx
    mov [edi],eax
    add esi,2
    add edi,4
    add edx,4
    dec ecx
    jnz .c
    add edi,4
    add edi,[AddEndBytes]
    mov edx,[spritetablea]
    add edx,512*256
    mov ecx,255
.d
    mov eax,[edx]
    mov [edi],eax
    add edx,4
    add edi,4
    dec ecx
    jnz .d
    add esi,66
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    add edi,4
    dec byte[lineleft]
    jnz near .loopb
    ret
.returninterp
    add esi,64
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopb
    ret

.scanlines
.loopab
    mov ecx,255
    cmp byte[ebx],1
    jbe .ignorehrs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    jmp .returninterps
.ignorehrs
    cmp byte[ebx],1
    je .yeshiresb
.ignorehrb
    push ebx
.ab
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edi],ebx
    add esi,2
    add edi,4
    dec ecx
    jnz .ab
    pop ebx
.returnb
    add esi,66
    add edi,4
    add edi,[AddEndBytes]
    mov ecx,256
.fslloop
    mov dword[edi],0
    add edi,4
    dec ecx
    jnz .fslloop
    add edi,[AddEndBytes]
    inc ebx
    dec dl
    jnz .loopab
    xor byte[res512switch],1
    ret
.yeshiresb
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightsideb
.bb
    mov ax,[esi]
    mov [edi],ax
    add esi,2
    add edi,4
    dec ecx
    jnz .bb
    jmp .returnb
.rightsideb
.cb
    mov ax,[esi]
    mov [edi+2],ax
    add esi,2
    add edi,4
    dec ecx
    jnz .cb
    jmp .returnb
.returninterps
    add esi,64
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    mov ecx,256
.fslloop2
    mov dword[edi],0
    add edi,4
    dec ecx
    jnz .fslloop2
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopab
    ret

.scanlineshalf
    mov [lineleft],dl
.loopab2
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrhs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    jmp .returninterphs
.ignorehrhs
    mov edx,[spritetablea]
    mov ecx,255
    add edx,512*256
.ab2
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edx],ebx
    mov [edi],ebx
    add esi,2
    add edi,4
    add edx,4
    dec ecx
    jnz .ab2
    add edi,4
    add edi,[AddEndBytes]
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512*256
.ab2b
    mov eax,[edx]
    and eax,[HalfTrans]
    shr eax,1
    mov [edi],eax
    add edi,4
    add edx,4
    dec ecx
    jnz .ab2b
    inc dword[InterPtr]
    add esi,66
    add edi,4
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopab2
    ret
.returninterphs
    add esi,64
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopab2
    ret

.scanlinesquart
    mov [lineleft],dl
.loopab3
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrqs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    jmp .returninterpqs
.ignorehrqs
    mov edx,[spritetablea]
    mov ecx,255
    add edx,512*256
.ab3
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edx],ebx
    mov [edi],ebx
    add esi,2
    add edi,4
    add edx,4
    dec ecx
    jnz .ab3
    add edi,[AddEndBytes]
    add edi,4
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512*256
.ab3b
    mov eax,[edx]
    and eax,[HalfTrans]
    shr eax,1
    mov ebx,eax
    and ebx,[HalfTrans]
    shr ebx,1
    add eax,ebx
    mov [edi],eax
    add edi,4
    add edx,4
    dec ecx
    jnz .ab3b
    inc dword[InterPtr]
    add esi,66
    add edi,4
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopab3
    ret
.returninterpqs
    add esi,64
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    dec byte[lineleft]
    jnz near .loopab3
    ret

SECTION .data
NEWSYM InterPtr, dd 0
SECTION .text
