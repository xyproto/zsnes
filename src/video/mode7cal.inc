;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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








%macro Mode7Calculate 0
    ; mode 7, ax = curyposition, dx = curxposition (left side)
    ; draw center map coordinates at (X0-bg1scrolx,Y0-bg1scroly) on screen
    ; center map coordinates = (X0,Y0)
    ; 1.) cx=X0-bg1scrolx, cy =Y0-ax

    mov bx,[mode7X0]
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonega
    or bx,1110000000000000b
.nonega
    mov [.cxloc],bx
    mov bx,dx
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegb
    or bx,1110000000000000b
.nonegb
    sub [.cxloc],bx
    mov bx,ax
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegc
    or bx,1110000000000000b
.nonegc
    mov [.cyloc],bx
    mov bx,[mode7Y0]
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegd
    or bx,1110000000000000b
.nonegd
    sub word[.cyloc],bx

    ; 2.) Find position at scaled y, centered x at SCX=X0-(cy*C),SCY=Y0-(cy*D)

    movsx eax,word[mode7B]
    movsx ebx,word[.cyloc]
    imul eax,ebx
    mov [.mode7xpos],eax
    mov bx,[mode7X0]
    add [.mode7xpos+1],bx

    movsx ebx,word[.cyloc]
    movsx eax,word[mode7D]
    imul eax,ebx
    mov [.mode7ypos],eax
    mov bx,[mode7Y0]
    add [.mode7ypos+1],bx

    ; 3.) Find left scaled location : SCX=SCX-(cx*A),SCY=SCY-(cx*B)

    movsx ebx,word[.cxloc]
    movsx eax,word[mode7A]
    mov [.mode7xadder],eax
    imul eax,ebx
    neg eax
    add [.mode7xpos],eax

    movsx eax,word[mode7C]
    movsx ebx,word[.cxloc]
    neg eax
    mov [.mode7yadder],eax
    imul eax,ebx
    add [.mode7ypos],eax

    test byte[mode7set],1
    jz .nohflip
    mov eax,[.mode7xadder]
    shl eax,8
    add [.mode7xpos],eax
    neg dword[.mode7xadder]
    mov eax,[.mode7yadder]
    shl eax,8
    sub [.mode7ypos],eax
    neg dword[.mode7yadder]
.nohflip
%endmacro

%macro Mode7Process 3
    mov dword[.temp],256
    test byte[mode7set],80h
    jnz near %%norep2

    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ecx,ecx
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx

    mov eax,[.mode7xrpos]
    mov ebx,[.mode7ptr]
    jmp %%nextval
    ALIGN16
%%nextval
    test ah,08h
    jnz near %%rposoffx
%%nextposx
    test byte[.mode7yrpos+1],08h
    jnz near %%rposoffy
%%nextposy
    mov ch,ah
    mov edx,[.mode7yadder]
    mov cl,[.mode7yrpos+1]
    sub dword[.mode7yrpos],edx
    xor edx,edx
    add eax,[.mode7xadder]
    mov dl,[mode7tab+ecx]
    mov dl,[edi+edx]
    %1
    dec dword[.temp]
    jnz near %%nextval
    jmp %%finishmode7

%%rposoffx
    add bl,[.mode7xinc]
    xor ecx,ecx
    mov cl,[vrama+ebx]
    shl ecx,7
    sub eax,[.mode7xadd2]
    lea edi,[ecx+vrama]
    jmp %%nextposx

%%rposoffy
    sub bh,[.mode7yinc]
    and ebx,07FFFh
    xor ecx,ecx
    mov cl,[vrama+ebx]
    mov edx,[.mode7yadd2]
    shl ecx,7
    add dword[.mode7yrpos],edx
    lea edi,[ecx+vrama]
    jmp %%nextposy

%%finishmode7
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near %2
    ret

;**********************************************************
; Mode 7, no repetition mode
;**********************************************************

%%norep2
    test byte[mode7set],40h
    jnz %%tilerep2
    jmp %%nextvalb2
    ALIGN16
%%nextvalb2
    cmp byte[.mode7ypos+2],3
    ja %%offscr2
    cmp byte[.mode7xpos+2],3
    jbe near %%offscr3
%%offscr2
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    add esi,%3
    dec dword[.temp]
    jnz near %%nextvalb2
    jmp %%finishmode7
%%tilerep2
%%nextvalb3
    cmp byte[.mode7ypos+2],3
    ja %%offscr2b
    cmp byte[.mode7xpos+2],3
    jbe near %%offscr3
%%offscr2b
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov dl,[vrama+edx]
    sub [.mode7ypos],ebx
    %1
    dec dword[.temp]
    jnz near %%nextvalb3
    jmp %%finishmode7
%%offscr3
    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ch,ch
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx

    jmp %%nextvalr
    ALIGN16
%%nodr2
    add esi,%3
    dec dword[.temp]
    jz near %%fin2
%%nextvalr
    test byte[.mode7xrpos+1],08h
    jnz near %%rposoffxr
%%nextposxr
    test byte[.mode7yrpos+1],08h
    jnz near %%rposoffyr
%%nextposyr
    mov cl,[.mode7yrpos+1]
    mov ch,[.mode7xrpos+1]
    mov edx,[.mode7xadder]
    add dword[.mode7xrpos],edx
    mov edx,[.mode7xadder]
    sub dword[.mode7yrpos],edx
    xor edx,edx
    mov dl,[mode7tab+ecx]
    mov dl,[edi+edx]
    %1
    dec dword[.temp]
    jnz near %%nextvalr
%%fin2
    jmp %%finishmode7
%%rposoffxr
    mov al,[.mode7xinc]
    mov edi,[vram]
    add [.mode7ptr],al
    mov cl,[.mode7xincc]
    cmp byte[.mode7ptr],cl
    je %%roff
%%roffxretb
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7xadd2]
    shl ecx,7
    sub [.mode7xrpos],eax
    add edi,ecx
    jmp %%nextposxr
%%rposoffyr
    mov al,[.mode7yinc]
    mov edi,[vram]
    sub [.mode7ptr+1],al
    js %%roff
%%roffyretb
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7yadd2]
    shl ecx,7
    add [.mode7yrpos],eax
    add edi,ecx
    jmp %%nextposyr
%%roff
    test byte[mode7set],40h
    jnz %%tilerep3
    jmp %%finishmode7
%%tilerep3
    and byte[.mode7yrpos+1],07h
    and byte[.mode7xrpos+1],07h
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov dl,[vrama+edx]
    sub [.mode7yrpos],eax
    %1
    dec dword[.temp]
    jnz near %%tilerep3
    jmp %%finishmode7
%endmacro



