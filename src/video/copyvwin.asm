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

EXTSYM vidbuffer,GUIOn,MMXSupport,resolutn,En2xSaI,antienab,scanlines
EXTSYM hirestiledat,res512switch,curblank,spritetablea,lineleft,_2xSaILineW
EXTSYM _2xSaISuperEagleLineW,_2xSaISuper2xSaILineW,newengen,cfield,HalfTrans
EXTSYM GUIOn2,FilteredGUI,SpecialLine,vidbufferofsb,HalfTransB,HalfTransC

ALIGN32
SECTION .bss
NEWSYM AddEndBytes, resd 1         ; Number of bytes between each line
NEWSYM NumBytesPerLine, resd 1     ; Total number of bytes per line (1024+AddEndBytes)
NEWSYM WinVidMemStart, resd 1
SECTION .text

NEWSYM copy640x480x16bwin
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    mov ax,ds
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,[WinVidMemStart]
    add esi,16*2+256*2+32*2
%ifdef __UNIXSDL__
    cmp byte[GUIOn],1
    je .not239
    cmp byte[resolutn],239
    jne .not239
    add esi,8*288*2
.not239
%endif
    xor eax,eax
    ; Check if interpolation mode
    cmp byte[FilteredGUI],0
    jne .yi
    cmp byte[GUIOn2],1
    je .nointerp
.yi
    cmp byte[MMXSupport],1
    jne .nommx
    cmp byte[En2xSaI],0
    jne near Process2xSaIwin
.nommx
    cmp byte[antienab],1
    je near interpolate640x480x16bwin
.nointerp
%ifdef __UNIXSDL__
    mov dl,224
%else
    mov dl,[resolutn]
%endif
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlines
    cmp byte[scanlines],2
    je near .quartscanlines

    mov ebx,hirestiledat+1
    cmp byte[newengen],0
    je .loopa
    mov ebx,SpecialLine+1
.loopa
    mov ecx,256
    cmp byte[ebx],1
    je near .yeshires
    cmp byte[GUIOn],1
    je .ignorehr
    cmp byte[ebx],1
    ja near .yeshiresng
.ignorehr
    cmp byte[MMXSupport],1
    je near .mmx
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .a
    sub esi,256*2
    add edi,[AddEndBytes]
    mov ecx,256
.a2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .a2
.return
    add esi,64
    add edi,[AddEndBytes]
    inc ebx
    dec dl
    jnz near .loopa
    xor byte[res512switch],1
    cmp byte[MMXSupport],1
    je .mmx2
    ret
.mmx2
    emms
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
    push ebx
    mov ebx,[NumBytesPerLine]
.b
    mov ax,[esi]
    mov [edi],ax
    mov [edi+ebx],ax
    add esi,2
    add edi,4
    dec ecx
    jnz .b
    pop ebx
    add edi,[NumBytesPerLine]
    jmp .return
.rightside
    push ebx
    mov ebx,[NumBytesPerLine]
.c
    mov ax,[esi]
    mov [edi+2],ax
    mov [edi+2+ebx],ax
    add esi,2
    add edi,4
    dec ecx
    jnz .c
    pop ebx
    add edi,[NumBytesPerLine]
    jmp .return
.mmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxr
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [edi],mm0
    punpckhwd mm1,mm1
    movq [edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxr
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,[AddEndBytes]
.mmxr2
    movq mm0,[eax]
    movq [edi],mm0
    movq mm1,[eax+8]
    movq [edi+8],mm1
    movq mm2,[eax+16]
    movq [edi+16],mm2
    movq mm3,[eax+24]
    movq [edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2
    jmp .return
.yeshiresng
    call HighResProc
    jmp .return

.bng
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .bng
    add edi,[AddEndBytes]
    sub esi,256*2
    mov ecx,256
.bngb
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .bngb
    jmp .return

.scanlines
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopab
    cmp byte[newengen],0
    je .loopab
    mov ebx,SpecialLine+1
.loopab
    mov ecx,256
    cmp byte[ebx],1
    je .yeshiresb
    cmp byte[ebx],1
    jbe .ignorehrb
    call HighResProc
    jmp .returnb
.ignorehrb
    cmp byte[MMXSupport],1
    je near .mmxsl
.ab
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .ab
.returnb
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
    dec dl
    jnz .loopab
    xor byte[res512switch],1
    cmp byte[MMXSupport],1
    je near .mmx2
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
.mmxsl
    mov ecx,64
.mmxrsl
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [edi],mm0
    movq [edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrsl
    jmp .returnb

.halfscanlines
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabh
    cmp byte[newengen],0
    je .loopabh
    mov ebx,SpecialLine+1
.loopabh
    cmp byte[ebx],1
    jbe .ignorehrbh
    call HighResProc
    jmp .returnbh
.ignorehrbh
    cmp byte[MMXSupport],1
    je near .mmxslh
    mov ecx,256
.abh
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abh
    mov ecx,256
    sub esi,512
    add edi,[AddEndBytes]
.abhs
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    and eax,[HalfTrans]
    shr eax,1
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abhs
.returnbh
    add esi,64
    add edi,[AddEndBytes]
    inc ebx
    dec dl
    jnz near .loopabh
    cmp byte[MMXSupport],1
    je near .mmx2
    ret
.mmxslh
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxrslh
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [edi],mm0
    movq [edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrslh
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,[AddEndBytes]
    movq mm4,[HalfTrans]
.mmxr2h
    movq mm0,[eax]
    movq mm1,[eax+8]
    movq mm2,[eax+16]
    movq mm3,[eax+24]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    movq [edi],mm0
    movq [edi+8],mm1
    movq [edi+16],mm2
    movq [edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2h
    jmp .returnbh

.quartscanlines
    mov [lineleft],dl
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabh2
    cmp byte[newengen],0
    je .loopabh2
    mov ebx,SpecialLine+1
.loopabh2
    cmp byte[ebx],1
    jbe .ignorehrbh2
    call HighResProc
    jmp .returnbh2
.ignorehrbh2
    cmp byte[MMXSupport],1
    je near .mmxslh2
    mov ecx,256
.abh2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abh2
    mov ecx,256
    sub esi,512
    add edi,[AddEndBytes]
.abhs2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    and eax,[HalfTrans]
    shr eax,1
    mov edx,eax
    and edx,[HalfTrans]
    shr edx,1
    add eax,edx
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abhs2
.returnbh2
    add esi,64
    add edi,[AddEndBytes]
    inc ebx
    dec byte[lineleft]
    jnz near .loopabh2
    cmp byte[MMXSupport],1
    je near .mmx2
    ret
.mmxslh2
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxrslh2
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [edi],mm0
    movq [edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrslh2
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    add edi,[AddEndBytes]
    movq mm4,[HalfTrans]
.mmxr2h2
    movq mm0,[eax]
    movq mm1,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    psrlw mm0,1
    psrlw mm1,1
    movq mm2,mm0
    movq mm3,mm1
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm2
    paddd mm1,mm3
    movq [edi],mm0
    movq [edi+8],mm1
    add eax,16
    add edi,16
    dec ecx
    jnz .mmxr2h2
    jmp .returnbh2

HighResProc:
    mov ecx,256
    cmp byte[ebx],3
    je near .hiresmode7
    cmp byte[ebx],7
    je near .hiresmode7
    test byte[ebx],4
    jz .nofield
    cmp byte[scanlines],0
    jne .nofield
    test byte[cfield],1
    jz .nofield
    add edi,[NumBytesPerLine]
.nofield
    test byte[ebx],3
    jnz near .hires
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .a
    cmp byte[scanlines],0
    jne .nofield
    test byte[cfield],1
    jnz .nofielde
    add edi,[NumBytesPerLine]
.nofielde
    ret
.hiresmode7
    cmp byte[MMXSupport],1
    je .yeshiresngmmxmode7
.a2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .a2
    add edi,[AddEndBytes]
    sub esi,512
    mov ecx,256
    add esi,75036*4
.a2b
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .a2b
    sub esi,75036*4
    ret
.yeshiresngmmxmode7
    mov ecx,64
.mmxr
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [edi],mm0
    punpckhwd mm1,mm1
    movq [edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxr
    add edi,[AddEndBytes]
    sub esi,512
    add esi,75036*4
    mov ecx,64
.mmxrb
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [edi],mm0
    punpckhwd mm1,mm1
    movq [edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrb
    sub esi,75036*4
    ret
.hires
    cmp byte[MMXSupport],1
    je near .yeshiresngmmx
.bng
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .bng
    test byte[ebx],4
    jz .nofieldb
    cmp byte[scanlines],0
    jne .nofieldb
    test byte[cfield],1
    jnz .lowerfield
    add edi,[NumBytesPerLine]
.lowerfield
    ret
.nofieldb
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlines
    cmp byte[scanlines],2
    je near .quartscanlines
    add edi,[AddEndBytes]
    sub esi,256*2
    mov ecx,256
.bngb
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .bngb
    ret
.scanlines
    ret
.yeshiresngmmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.ngal
    movq mm0,[esi]
    movq mm1,[esi+75036*4]
    movq mm2,mm0
    punpcklwd mm0,mm1
    movq [edi],mm0
    punpckhwd mm2,mm1
    movq [edi+8],mm2
    movq [eax],mm0
    movq [eax+8],mm2
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .ngal
    test byte[ebx],4
    jz .nofieldc
    cmp byte[scanlines],0
    jne .nofieldc
    test byte[cfield],1
    jnz .lowerfieldb
    add edi,[NumBytesPerLine]
.lowerfieldb
    ret
.nofieldc
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlinesmmx
    cmp byte[scanlines],2
    je near .quartscanlinesmmx
    test byte[ebx+1],3
    jz .noaa
    cmp byte[En2xSaI],0
    jne near .antialias
    cmp byte[antienab],0
    jne near .antialias
.noaa
    add edi,[AddEndBytes]
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
.mmxr2
    movq mm0,[eax]
    movq [edi],mm0
    movq mm1,[eax+8]
    movq [edi+8],mm1
    movq mm2,[eax+16]
    movq [edi+16],mm2
    movq mm3,[eax+24]
    movq [edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2
    ret
.antialias
    add edi,[AddEndBytes]
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    movq mm4,[HalfTrans]
    sub esi,256*2
.mmxr2aa
    movq mm0,[esi+288*2]
    movq mm1,[esi+288*2+75036*4]
    movq mm2,mm0
    punpcklwd mm0,mm1
    punpckhwd mm2,mm1
    movq mm1,[eax]
    movq mm3,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm1
    paddd mm2,mm3
    movq [edi],mm0
    movq [edi+8],mm2
    add eax,16
    add edi,16
    add esi,8
    dec ecx
    jnz .mmxr2aa
    ret
.halfscanlines
    add edi,[AddEndBytes]
    sub esi,256*2
    mov ecx,256
.abhs
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    and eax,[HalfTrans]
    shr eax,1
    mov edx,eax
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abhs
    ret
.quartscanlines
    add edi,[AddEndBytes]
    sub esi,256*2
    mov ecx,256
.abhs2
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    and eax,[HalfTrans]
    shr eax,1
    mov edx,eax
    and edx,[HalfTrans]
    shr edx,1
    add eax,edx
    mov [edi],eax
    add esi,2
    add edi,4
    dec ecx
    jnz .abhs2
    ret
.halfscanlinesmmx
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,[AddEndBytes]
    movq mm4,[HalfTrans]
.mmxr2h
    movq mm0,[eax]
    movq mm1,[eax+8]
    movq mm2,[eax+16]
    movq mm3,[eax+24]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    movq [edi],mm0
    movq [edi+8],mm1
    movq [edi+16],mm2
    movq [edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2h
    ret
.quartscanlinesmmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    add edi,[AddEndBytes]
    movq mm4,[HalfTransC]
.mmxr2h2
    movq mm0,[eax]
    movq mm1,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    psrlw mm0,1
    psrlw mm1,1
    movq mm2,mm0
    movq mm3,mm1
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm2
    paddd mm1,mm3
    movq [edi],mm0
    movq [edi+8],mm1
    add eax,16
    add edi,16
    dec ecx
    jnz .mmxr2h2
    ret

Process2xSaIwin:
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabi
    cmp byte[newengen],0
    je .loopabi
    mov ebx,SpecialLine+1
.loopabi
    mov [InterPtr],ebx

;    add edi,[VESAAddr]
%ifdef __UNIXSDL__
    mov dl,223
%else
    mov dl,[resolutn]
    sub dl,1    ; Compensate for top/bottom line + 2 lines in 2xSaI
%endif
    mov [lineleft],dl
    mov dword[esi+512],0
    mov dword[esi+512+576*2],0
    mov ebx,[vidbufferofsb]
    add ebx,288*2

.next
    mov dword[esi+512+576*3],0

    mov eax,[InterPtr]
    cmp byte[eax],1
    jbe .ignorehr
    push ebx
    mov ebx,[InterPtr]
    call HighResProc
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
    call _2xSaILineW
    jmp .normal
.supereagle
    call _2xSaISuperEagleLineW
    jmp .normal
.super2xSaI
    call _2xSaISuper2xSaILineW
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

MMXInterpolwin:
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopab
    cmp byte[newengen],0
    je .loopab
    mov ebx,SpecialLine+1
.loopab

%ifdef __UNIXSDL__
    mov dl,224
%else
    mov dl,[resolutn]
%endif
    movq mm2,[HalfTransC]
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    inc ebx
    mov [lineleft],dl
    ; do scanlines
    mov edx,[spritetablea]
    mov ecx,64
    mov eax,[esi+510]
    add edx,512
    mov [esi+512],eax
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
    call HighResProc
    movq mm2,[HalfTransC]
    jmp .returninterp
.ignorehr
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
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
    add edx,512
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
    inc dl
    mov [lineleft],dl
    ; do scanlines
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
.asl
    cmp byte[ebx],1
    jbe .ignorehrs
    call HighResProc
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
    inc dl
    mov [lineleft],dl
    ; do scanlines
.ahb
    cmp byte[ebx],1
    jbe .ignorehrhs
    call HighResProc
    movq mm2,[HalfTrans]
    jmp .returninterphs
.ignorehrhs
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
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
    inc dl
    mov [lineleft],dl
    ; do scanlines
.ahb2
    cmp byte[ebx],1
    jbe .ignorehrqs
    call HighResProc
    movq mm2,[HalfTransC]
    jmp .returninterpqs
.ignorehrqs
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
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

NEWSYM interpolate640x480x16bwin
    cmp byte[MMXSupport],1
    je near MMXInterpolwin

    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabi
    cmp byte[newengen],0
    je .loopabi
    mov ebx,SpecialLine+1
.loopabi
    mov [InterPtr],ebx

%ifdef __UNIXSDL__
    mov dl,224
%else
    mov dl,[resolutn]
%endif
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    inc dword[InterPtr]
    mov [lineleft],dl
    ; do first line
    mov ecx,255
    mov edx,[spritetablea]
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
    call HighResProc
    jmp .returninterp
.ignorehr
    mov ecx,255
    mov edx,[spritetablea]
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
    xor eax,eax
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabis
    cmp byte[newengen],0
    je .loopabis
    mov ebx,SpecialLine+1
.loopabis
.loopab
    mov ecx,255
    cmp byte[ebx],1
    jbe .ignorehrs
    call HighResProc
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
    xor eax,eax
    mov [lineleft],dl
.loopab2
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrhs
    call HighResProc
    jmp .returninterphs
.ignorehrhs
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
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
    add edx,512
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
    xor eax,eax
    mov [lineleft],dl
.loopab3
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrqs
    call HighResProc
    jmp .returninterpqs
.ignorehrqs
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
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
    add edx,512
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
InterPtr dd 0
SECTION .text
