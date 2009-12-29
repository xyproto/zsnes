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
EXTSYM HighResProc
%ifdef __WIN32__
EXTSYM cvidmode,GUIDSMODE,GUIWFVID
%endif

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

NEWSYM Process2xSaIwin
    SelectTile
    mov [InterPtr],ebx
%ifdef __UNIXSDL__
    mov dl,224
%else
    mov dl,[resolutn]
%endif
    mov [lineleft],dl
    mov word[esi+512],0

    mov ebx,[vidbufferofsb]
    add ebx,288*2

.next
    mov word[esi+512+576],0
    mov dword[edi+512*2-6],0
    mov word[edi+512*2-2],0
%ifdef __WIN32__
    xor eax,eax
    mov al,[cvidmode]
    cmp byte[GUIDSMODE+eax],0
    jne near .isdsmode
    cmp byte[GUIWFVID+eax],0
    je .isdsmode
    mov dword[edi+576*4-6],0
    mov word[edi+576*4-2],0
    jmp near .notdsmode
%endif
.isdsmode
    mov dword[edi+512*4-6],0
    mov word[edi+512*4-2],0
.notdsmode
    mov eax,[InterPtr]
    cmp byte[eax],1
    jbe .ignorehr
    push ebx
    mov ebx,[InterPtr]

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    pop ebx
    push ebx
    mov ecx,144
.nextb
    mov dword[ebx],0FFFFFFFFh
    add ebx,4
    dec ecx
    jnz .nextb
    pop ebx
    jmp .returninterp
.ignorehr

;srcPtr        equ 8
;deltaPtr      equ 12
;srcPitch      equ 16
;width         equ 20
;dstOffset     equ 24
;dstPitch      equ 28
;dstSegment    equ 32


    push ebx
    mov eax,[NumBytesPerLine]
    push eax
    mov eax,edi         ; destination offset
    push eax
    mov eax,256         ; width
    push eax
    mov eax,576         ; source pitch
    push eax
    push ebx
    mov eax,esi         ; source pointer
    push eax
    cmp byte[En2xSaI],2
    je .supereagle
    cmp byte[En2xSaI],3
    je .super2xSaI
    call _2xSaILine
    jmp .normal
.supereagle
    call _2xSaISuperEagleLine
    jmp .normal
.super2xSaI
    call _2xSaISuper2xSaILine
.normal
    add esp,24
    pop ebx
    add esi,576
    add edi,[NumBytesPerLine]
    add edi,[NumBytesPerLine]
    add ebx,576
    inc dword[InterPtr]
    dec dword[lineleft]
    jnz near .next
    mov ecx,256
    sub edi,[NumBytesPerLine]
.loop
    mov dword[es:edi],0
    add edi,4
    dec ecx
    jnz .loop
    emms
    ret
.returninterp
    add esi,64
    inc dword[InterPtr]
    add edi,[AddEndBytes]
    add ebx,576
    dec byte[lineleft]
    jnz near .next
    emms
    ret

NEWSYM interpolate640x480x16bwin
    SelectTile
    cmp byte[MMXSupport],1
    je near MMXInterpolwin
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

MMXInterpolwin:
    SelectTile
    movq mm2,[HalfTransC]
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    mov [lineleft],dl
    ; do scanlines
    mov eax,[esi+510]
    mov [esi+512],eax
    mov edx,[spritetablea]
    mov ecx,64
    add edx,512*256
.a2
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edi],mm4
    movq [edi+8],mm5
    movq [edx],mm4
    movq [edx+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .a2
    add esi,64
    add edi,[AddEndBytes]
.a5
    cmp byte[ebx],1
    jbe .ignorehr

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    movq mm2,[HalfTransC]
    jmp .returninterp
.ignorehr
    mov eax,[esi+510]
    mov [esi+512],eax
    mov edx,[spritetablea]
    mov ecx,64
    add edx,512*256
    ; Process next line
.a3
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    movq mm6,[edx]
    movq mm7,[edx+8]
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    movq mm0,mm6
    por mm0,mm4
    pand mm4,mm2
    pand mm6,mm2
    psrlw mm4,1
    psrlw mm6,1
    pand mm0,[HalfTransB]
    paddd mm4,mm6
    paddw mm4,mm0
    movq mm0,mm5
    por mm0,mm7
    pand mm5,mm2
    pand mm7,mm2
    psrlw mm5,1
    pand mm0,[HalfTransB]
    psrlw mm7,1
    paddd mm5,mm7
    paddw mm5,mm0
    movq [edi],mm4
    movq [edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz near .a3
    add edi,[AddEndBytes]
    mov edx,[spritetablea]
    add edx,512*256
    mov ecx,64
.a4
    movq mm0,[edx]
    movq mm1,[edx+8]
    movq [edi],mm0
    movq [edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .a4
.returninterp
    add esi,64
    add edi,[AddEndBytes]
    inc ebx
    dec byte[lineleft]
    jnz near .a5
    emms
    ret

.scanlines
    mov [lineleft],dl
    ; do scanlines
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
.asl
    cmp byte[ebx],1
    jbe .ignorehrs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    movq mm2,[HalfTrans]
    jmp .returninterps
.ignorehrs
.a
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edi],mm4
    movq [edi+8],mm5
    add esi,8
    add edi,16
    dec ecx
    jnz .a
.returninterps
    add esi,64
    add edi,[AddEndBytes]
    mov ecx,256
.fslloop
    mov dword[edi],0
    add edi,4
    dec ecx
    jnz .fslloop
    add edi,[AddEndBytes]
    inc ebx
    mov ecx,64
    dec byte[lineleft]
    jnz near .asl
    emms
    ret

.scanlineshalf
    mov [lineleft],dl
    ; do scanlines
.ahb
    cmp byte[ebx],1
    jbe .ignorehrhs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    movq mm2,[HalfTrans]
    jmp .returninterphs
.ignorehrhs
    mov eax,[esi+510]
    mov [esi+512],eax
    mov edx,[spritetablea]
    mov ecx,64
    add edx,512*256
.ah
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    movq [edi],mm4
    movq [edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .ah
    add edi,[AddEndBytes]
    sub edx,16*64
    mov ecx,64
.ahc
    movq mm0,[edx]
    movq mm1,[edx+8]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    movq [edi],mm0
    movq [edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .ahc
.returninterphs
    add edi,[AddEndBytes]
    add esi,64
    inc ebx
    dec byte[lineleft]
    jnz near .ahb
    emms
    ret

.scanlinesquart
    mov [lineleft],dl
    ; do scanlines
.ahb2
    cmp byte[ebx],1
    jbe .ignorehrqs

    push esi
    mov esi, esp
    push edi
    mov edi, esp
    ccallv HighResProc, esi, edi, ebx
    pop edi
    pop esi

    movq mm2,[HalfTransC]
    jmp .returninterpqs
.ignorehrqs
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512*256
.ah2
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    movq [edi],mm4
    movq [edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .ah2
    add edi,[AddEndBytes]
    sub edx,16*64
    mov ecx,64
    movq mm3,mm2
.ahc2
    movq mm0,[edx]
    movq mm1,[edx+8]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    movq mm4,mm0
    movq mm5,mm1
    pand mm4,mm2
    pand mm5,mm2
    psrlw mm4,1
    psrlw mm5,1
    paddd mm0,mm4
    paddd mm1,mm5
    movq [edi],mm0
    movq [edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .ahc2
.returninterpqs
    add esi,64
    add edi,[AddEndBytes]
    inc ebx
    dec byte[lineleft]
    jnz near .ahb2
    emms
    ret

SECTION .data
InterPtr dd 0
SECTION .text
